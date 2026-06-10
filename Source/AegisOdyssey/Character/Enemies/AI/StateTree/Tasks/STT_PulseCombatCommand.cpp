#include "STT_PulseCombatCommand.h"

#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PulseCombatCommand)

FSTT_PulseCombatCommand::FSTT_PulseCombatCommand()
{
	bShouldCallTick = true;
	bShouldCopyBoundPropertiesOnTick = true;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_PulseCombatCommand::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bLastPulseSucceeded = false;
	InstanceData.PulseCount = 0;
	InstanceData.DurationLimit = 0.0f;
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.TimeUntilNextPulse = 0.0f;
	InstanceData.StartTimeSeconds = GetCurrentWorldTimeSeconds(Context);

	if (!InstanceData.InputTag.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PulseCombatCommand::EnterState: InputTag is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* CommandTarget = ResolveCommandTarget(Context);
	if (CommandTarget == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PulseCombatCommand::EnterState: Failed to resolve command target."));
		return EStateTreeRunStatus::Failed;
	}

	if (UAOHeroComponent::FindHeroComponent(CommandTarget) == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PulseCombatCommand::EnterState: %s has no valid combat command receiver."),
			*GetNameSafe(CommandTarget));
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.DurationMin > 0.0f || InstanceData.DurationMax > 0.0f)
	{
		InstanceData.DurationLimit = GetRandomizedDelay(InstanceData.DurationMin, InstanceData.DurationMax);
	}

	// 这个 Task 故意不等待技能结束。
	// 它只负责在当前状态仍然激活时，持续模拟“玩家又按了一次键”，
	// 至于预输入、连招窗口和 ASC 如何消费这次输入，全部交给既有输入链处理。
	if (InstanceData.bSendImmediatelyOnEnter)
	{
		InstanceData.bLastPulseSucceeded = SendPulse(Context);
		if (!InstanceData.bLastPulseSucceeded)
		{
			return EStateTreeRunStatus::Failed;
		}

		if (ShouldFinishPulsing(InstanceData))
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}
	else
	{
		InstanceData.TimeUntilNextPulse = GetRandomizedDelay(InstanceData.InitialDelayMin, InstanceData.InitialDelayMax);
	}

	if (InstanceData.bSendImmediatelyOnEnter || InstanceData.TimeUntilNextPulse <= 0.0f)
	{
		InstanceData.TimeUntilNextPulse = GetRandomizedDelay(InstanceData.PulseIntervalMin, InstanceData.PulseIntervalMax);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_PulseCombatCommand::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const float CurrentWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);
	if (CurrentWorldTimeSeconds >= 0.0f && InstanceData.StartTimeSeconds >= 0.0f)
	{
		InstanceData.ElapsedTime = CurrentWorldTimeSeconds - InstanceData.StartTimeSeconds;
	}
	else
	{
		// 如果拿不到世界时间，就退回到 DeltaTime 累加，至少保证逻辑仍然可用。
		InstanceData.ElapsedTime += DeltaTime;
	}

	// 内部结束条件只负责“可以自然收手了”，不负责战术决策。
	// 是否需要更早切走，仍然交给外部 Condition 或 Transition。
	if (ShouldFinishPulsing(InstanceData))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// Attack 状态保持 Running，本 Task 只在内部按节奏重复发输入，
	// 而不是每次都靠重进状态来模拟连点。
	InstanceData.TimeUntilNextPulse -= DeltaTime;
	if (InstanceData.TimeUntilNextPulse > 0.0f)
	{
		return EStateTreeRunStatus::Running;
	}

	InstanceData.bLastPulseSucceeded = SendPulse(Context);
	if (!InstanceData.bLastPulseSucceeded)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (ShouldFinishPulsing(InstanceData))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.TimeUntilNextPulse = GetRandomizedDelay(InstanceData.PulseIntervalMin, InstanceData.PulseIntervalMax);
	return EStateTreeRunStatus::Running;
}

AActor* FSTT_PulseCombatCommand::ResolveCommandTarget(const FStateTreeExecutionContext& Context) const
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

bool FSTT_PulseCombatCommand::SendPulse(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AActor* CommandTarget = ResolveCommandTarget(Context);
	if (CommandTarget == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PulseCombatCommand::SendPulse: Failed to resolve command target."));
		return false;
	}

	const bool bSent = SendCombatCommand(CommandTarget, InstanceData.InputTag, InstanceData.InputType);
	if (!bSent)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PulseCombatCommand::SendPulse: Failed to route input type %d for tag %s on %s."),
			static_cast<int32>(InstanceData.InputType.GetValue()),
			*InstanceData.InputTag.ToString(),
			*GetNameSafe(CommandTarget));
		return false;
	}

	++InstanceData.PulseCount;

	UE_LOG(LogStateTree, Verbose, TEXT("FSTT_PulseCombatCommand::SendPulse: Sent pulse %d for %s on %s."),
		InstanceData.PulseCount,
		*InstanceData.InputTag.ToString(),
		*GetNameSafe(CommandTarget));

	return true;
}

bool FSTT_PulseCombatCommand::SendCombatCommand(AActor* CommandTarget, const FGameplayTag& InputTag, TEnumAsByte<EInputType> InputType) const
{
	if (CommandTarget == nullptr || !InputTag.IsValid())
	{
		return false;
	}

	// 脉冲输入同样只走 Hero 的统一发送口。
	// 技能组件如果关心这类输入，会通过订阅 Hero 委托在自己内部识别。
	if (UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(CommandTarget))
	{
		return HeroComponent->InjectAbilityInputCommand(InputTag, InputType);
	}

	return false;
}

float FSTT_PulseCombatCommand::GetRandomizedDelay(float MinDelay, float MaxDelay) const
{
	// 统一在这里做一次归一化，避免策划只填了一个值，
	// 或者把最小最大填反以后直接把运行时节奏弄坏。
	const float ClampedMin = FMath::Max(0.0f, MinDelay);
	const float ClampedMax = FMath::Max(ClampedMin, MaxDelay);
	return FMath::FRandRange(ClampedMin, ClampedMax);
}

bool FSTT_PulseCombatCommand::ShouldFinishPulsing(const FInstanceDataType& InstanceData) const
{
	if (InstanceData.MaxPulseCount > 0 && InstanceData.PulseCount >= InstanceData.MaxPulseCount)
	{
		return true;
	}

	if (InstanceData.DurationLimit > 0.0f && InstanceData.ElapsedTime >= InstanceData.DurationLimit)
	{
		return true;
	}

	return false;
}

float FSTT_PulseCombatCommand::GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const
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
