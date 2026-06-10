#include "STT_SendCombatCommand.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AIController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayAbilitySpec.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_SendCombatCommand)

EStateTreeRunStatus FSTT_SendCombatCommand::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCommandSent = false;
	InstanceData.bTimedOut = false;
	InstanceData.bObservedAbilityActivation = false;
	InstanceData.bFinishedWithoutActivation = false;
	InstanceData.ElapsedWaitTime = 0.0f;
	InstanceData.AbilitySystemComponent = nullptr;
	InstanceData.MatchingAbilitySpecHandles.Reset();
	InstanceData.ActiveAbilitySpecHandlesBeforeCommand.Reset();
	InstanceData.ObservedAbilitySpecHandle = FGameplayAbilitySpecHandle();
	InstanceData.EnterWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);

	if (!InstanceData.InputTag.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_SendCombatCommand::EnterState: InputTag is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* CommandTarget = ResolveCommandTarget(Context);
	if (CommandTarget == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_SendCombatCommand::EnterState: Failed to resolve command target."));
		return EStateTreeRunStatus::Failed;
	}

	// 只有真的要观察“能力激活/结束状态”的等待模式，才需要依赖 ASC。
	// 新增的 WaitFixedDuration 是给技能输入窗口用的，
	// 它只关心命令是否发出，不应该因为观察不到能力生命周期就失败。
	const bool bRequiresAbilityObservation =
		InstanceData.WaitMode == ESTTCommandWaitMode::WaitForActivation ||
		InstanceData.WaitMode == ESTTCommandWaitMode::WaitForCompletion ||
		InstanceData.WaitMode == ESTTCommandWaitMode::WaitForCompletionIfActivated;

	if (bRequiresAbilityObservation)
	{
		InstanceData.AbilitySystemComponent = ResolveAbilitySystemComponent(CommandTarget);
		if (InstanceData.AbilitySystemComponent == nullptr)
		{
			UE_LOG(LogStateTree, Warning, TEXT("FSTT_SendCombatCommand::EnterState: No AbilitySystemComponent found on %s while waiting for command %s."),
				*GetNameSafe(CommandTarget),
				*InstanceData.InputTag.ToString());
			return EStateTreeRunStatus::Failed;
		}

		GatherMatchingAbilitySpecHandles(*InstanceData.AbilitySystemComponent, InstanceData.InputTag, InstanceData.MatchingAbilitySpecHandles, InstanceData.ActiveAbilitySpecHandlesBeforeCommand);
		if (InstanceData.MatchingAbilitySpecHandles.IsEmpty())
		{
			if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitForCompletionIfActivated)
			{
				InstanceData.bFinishedWithoutActivation = true;
				InstanceData.bCommandSent = SendCombatCommand(CommandTarget, InstanceData.InputTag, InstanceData.InputType);
				return InstanceData.bCommandSent ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
			}

			UE_LOG(LogStateTree, Warning, TEXT("FSTT_SendCombatCommand::EnterState: No ability specs matched input %s on %s."),
				*InstanceData.InputTag.ToString(),
				*GetNameSafe(CommandTarget));
			return EStateTreeRunStatus::Failed;
		}
	}

	InstanceData.bCommandSent = SendCombatCommand(CommandTarget, InstanceData.InputTag, InstanceData.InputType);
	if (!InstanceData.bCommandSent)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_SendCombatCommand::EnterState: Failed to route combat command %s (%d) on %s."),
			*InstanceData.InputTag.ToString(),
			static_cast<int32>(InstanceData.InputType.GetValue()),
			*GetNameSafe(CommandTarget));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogStateTree, Log, TEXT("FSTT_SendCombatCommand::EnterState: Injected combat command %s (%d) on %s."),
		*InstanceData.InputTag.ToString(),
		static_cast<int32>(InstanceData.InputType.GetValue()),
		*GetNameSafe(CommandTarget));

	if (InstanceData.WaitMode == ESTTCommandWaitMode::None)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 这是这次专门补给技能输入链的正式行为：
	// 命令发出后，只固定维持一个短输入窗口，
	// 然后继续往下走，不再关心技能是否仍在执行。
	if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitFixedDuration)
	{
		if (InstanceData.FixedWaitSeconds <= 0.0f)
		{
			return EStateTreeRunStatus::Succeeded;
		}

		return EStateTreeRunStatus::Running;
	}

	if (FGameplayAbilitySpec* ActiveAbilitySpec = FindNewlyActivatedAbility(*InstanceData.AbilitySystemComponent, InstanceData.MatchingAbilitySpecHandles, InstanceData.ActiveAbilitySpecHandlesBeforeCommand))
	{
		InstanceData.ObservedAbilitySpecHandle = ActiveAbilitySpec->Handle;
		InstanceData.bObservedAbilityActivation = true;
		if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitForActivation)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_SendCombatCommand::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.WaitMode == ESTTCommandWaitMode::None)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitFixedDuration)
	{
		const float CurrentWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);
		if (CurrentWorldTimeSeconds >= 0.0f && InstanceData.EnterWorldTimeSeconds >= 0.0f)
		{
			InstanceData.ElapsedWaitTime = CurrentWorldTimeSeconds - InstanceData.EnterWorldTimeSeconds;
		}
		else
		{
			InstanceData.ElapsedWaitTime += DeltaTime;
		}

		return InstanceData.ElapsedWaitTime >= InstanceData.FixedWaitSeconds
			? EStateTreeRunStatus::Succeeded
			: EStateTreeRunStatus::Running;
	}

	if (InstanceData.AbilitySystemComponent == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float CurrentWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);
	if (CurrentWorldTimeSeconds >= 0.0f && InstanceData.EnterWorldTimeSeconds >= 0.0f)
	{
		InstanceData.ElapsedWaitTime = CurrentWorldTimeSeconds - InstanceData.EnterWorldTimeSeconds;
	}
	else
	{
		InstanceData.ElapsedWaitTime += DeltaTime;
	}

	// 把那些“发送命令前就已经激活，但现在已经结束”的 Spec 从旧快照里剔掉。
	// 这样后面继续找“新激活能力”时，旧快照才能保持准确。
	for (int32 HandleIndex = InstanceData.ActiveAbilitySpecHandlesBeforeCommand.Num() - 1; HandleIndex >= 0; --HandleIndex)
	{
		const FGameplayAbilitySpecHandle Handle = InstanceData.ActiveAbilitySpecHandlesBeforeCommand[HandleIndex];
		const FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec == nullptr || !AbilitySpec->IsActive())
		{
			InstanceData.ActiveAbilitySpecHandlesBeforeCommand.RemoveAtSwap(HandleIndex);
		}
	}

	if (!InstanceData.bObservedAbilityActivation)
	{
		if (FGameplayAbilitySpec* ActiveAbilitySpec = FindNewlyActivatedAbility(*InstanceData.AbilitySystemComponent, InstanceData.MatchingAbilitySpecHandles, InstanceData.ActiveAbilitySpecHandlesBeforeCommand))
		{
			InstanceData.ObservedAbilitySpecHandle = ActiveAbilitySpec->Handle;
			InstanceData.bObservedAbilityActivation = true;
			if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitForActivation)
			{
				return EStateTreeRunStatus::Succeeded;
			}
		}
		else
		{
			if (InstanceData.ActivationTimeoutSeconds > 0.0f && InstanceData.ElapsedWaitTime >= InstanceData.ActivationTimeoutSeconds)
			{
				if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitForCompletionIfActivated)
				{
					InstanceData.bFinishedWithoutActivation = true;
					return EStateTreeRunStatus::Succeeded;
				}

				InstanceData.bTimedOut = true;
				return EStateTreeRunStatus::Failed;
			}

			return EStateTreeRunStatus::Running;
		}
	}

	if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitForActivation)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	FGameplayAbilitySpec* ObservedAbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.ObservedAbilitySpecHandle);
	if (ObservedAbilitySpec == nullptr || !ObservedAbilitySpec->IsActive())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.CompletionTimeoutSeconds > 0.0f && InstanceData.ElapsedWaitTime >= InstanceData.CompletionTimeoutSeconds)
	{
		InstanceData.bTimedOut = true;
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

AActor* FSTT_SendCombatCommand::ResolveCommandTarget(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerActor))
	{
		return AIController->GetPawn();
	}

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return OwnerPawn;
	}

	return OwnerActor;
}

bool FSTT_SendCombatCommand::SendCombatCommand(AActor* CommandTarget, const FGameplayTag& InputTag, TEnumAsByte<EInputType> InputType) const
{
	if (CommandTarget == nullptr || !InputTag.IsValid())
	{
		return false;
	}

	// StateTree 这里也坚持只走 HeroComponent 这一条统一分发口。
	// 后面的技能系统或其他监听者自己决定要不要响应这次输入，
	// 不在这里做额外分流。
	if (UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(CommandTarget))
	{
		return HeroComponent->InjectAbilityInputCommand(InputTag, InputType);
	}

	return false;
}

UAbilitySystemComponent* FSTT_SendCombatCommand::ResolveAbilitySystemComponent(AActor* CommandTarget) const
{
	// 这个项目里更稳定、更长期的 ASC 归属通常在 PlayerState 上。
	// Pawn 更多只是 Avatar，真正更持久、更利于复制的拥有者往往是 PlayerState。
	if (const APlayerState* PlayerState = Cast<APlayerState>(CommandTarget))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<APlayerState*>(PlayerState)))
		{
			return AbilitySystemComponent;
		}
	}

	if (const APawn* Pawn = Cast<APawn>(CommandTarget))
	{
		if (APlayerState* PlayerState = Pawn->GetPlayerState())
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}
	}

	if (const AController* Controller = Cast<AController>(CommandTarget))
	{
		if (APlayerState* PlayerState = Controller->PlayerState)
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}
	}

	if (const UAOExtPawnComponent* PawnExtComponent = UAOExtPawnComponent::FindAOExtPawnComponent(CommandTarget))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = PawnExtComponent->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent;
		}
	}

	return CommandTarget ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CommandTarget) : nullptr;
}

void FSTT_SendCombatCommand::GatherMatchingAbilitySpecHandles(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTag& InputTag, TArray<FGameplayAbilitySpecHandle>& OutMatchingHandles, TArray<FGameplayAbilitySpecHandle>& OutActiveHandles) const
{
	OutMatchingHandles.Reset();
	OutActiveHandles.Reset();

	// 这里同时记录两类信息：
	// 1. 哪些 Spec 命中了这个 InputTag；
	// 2. 在发送命令之前，这些命中的 Spec 里有哪些本来就已经在激活。
	// 后面正是靠这两份信息来判断“这次是否真的新激活出一个能力”。
	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent.GetActivatableAbilities())
	{
		if (AbilitySpec.Ability == nullptr)
		{
			continue;
		}

		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		OutMatchingHandles.Add(AbilitySpec.Handle);
		if (AbilitySpec.IsActive())
		{
			OutActiveHandles.Add(AbilitySpec.Handle);
		}
	}
}

FGameplayAbilitySpec* FSTT_SendCombatCommand::FindNewlyActivatedAbility(const UAbilitySystemComponent& AbilitySystemComponent, const TArray<FGameplayAbilitySpecHandle>& MatchingHandles, const TArray<FGameplayAbilitySpecHandle>& PreviouslyActiveHandles) const
{
	// 这里说的“新激活”并不是指新建了一个 Handle，
	// 而是指它现在处于 Active，但发送命令之前并不在旧激活快照里。
	for (const FGameplayAbilitySpecHandle& Handle : MatchingHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent.FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec == nullptr || !AbilitySpec->IsActive())
		{
			continue;
		}

		if (PreviouslyActiveHandles.Contains(Handle))
		{
			continue;
		}

		return AbilitySpec;
	}

	return nullptr;
}

float FSTT_SendCombatCommand::GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const
{
	if (AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
	{
		if (const UWorld* World = OwnerActor->GetWorld())
		{
			return World->GetTimeSeconds();
		}
	}

	return -1.0f;
}
