#include "STT_PlayAnimationMontage.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/AbilitySystem/Abilities/GA_PlayAnimationMontage.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayAnimationMontage)

EStateTreeRunStatus FSTT_PlayAnimationMontage::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Montage.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimationMontage::EnterState: Montage is not set!"));
		return EStateTreeRunStatus::Failed;
	}

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimationMontage::EnterState: Owner is not an actor!"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!InstanceData.AbilitySystemComponent)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimationMontage::EnterState: Owner has no AbilitySystemComponent!"));
		return EStateTreeRunStatus::Failed;
	}

	TSubclassOf<UGameplayAbility> AbilityClass = InstanceData.AnimationAbilityClass;
	if (!AbilityClass)
	{
		AbilityClass = UGA_PlayAnimationMontage::StaticClass();
	}

	FGameplayAbilitySpec* FoundSpec = nullptr;
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->IsA(AbilityClass))
		{
			if (!AbilitySpec.IsActive())
			{
				FoundSpec = const_cast<FGameplayAbilitySpec*>(&AbilitySpec);
				break;
			}
		}
	}

	if (!FoundSpec)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimationMontage::EnterState: No valid ability found for %s!"), *AbilityClass->GetName());
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySpecHandle = FoundSpec->Handle;

	FPlayAnimationMontageTargetData* TargetData = new FPlayAnimationMontageTargetData();
	TargetData->DataMontage = InstanceData.Montage.Get();

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	FGameplayEventData EventData;
	EventData.TargetData = TargetDataHandle;

	InstanceData.bAnimationStarted = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(
		InstanceData.AbilitySpecHandle,
		FPredictionKey(),
		nullptr,
		nullptr,
		&EventData);

	if (!InstanceData.bAnimationStarted)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimationMontage::EnterState: Failed to activate ability!"));
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bWaitForAnimation)
	{
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Succeeded;
}

void FSTT_PlayAnimationMontage::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AbilitySpecHandle.IsValid() || !InstanceData.AbilitySystemComponent)
	{
		return;
	}

	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (AbilitySpec && AbilitySpec->IsActive())
	{
		UE_LOG(LogStateTree, Log, TEXT("FSTT_PlayAnimationMontage::ExitState: Canceling active animation ability!"));
		InstanceData.AbilitySystemComponent->CancelAbilityHandle(InstanceData.AbilitySpecHandle);
	}
}

EStateTreeRunStatus FSTT_PlayAnimationMontage::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AbilitySpecHandle.IsValid() || !InstanceData.AbilitySystemComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (!AbilitySpec)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!AbilitySpec->IsActive())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
