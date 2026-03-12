#include "STT_PlayBlockAnimation.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayPrediction.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_Block.h"

EStateTreeRunStatus FSTT_PlayBlockAnimation::EnterState(FStateTreeExecutionContext& Context,
                                                        const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner());
	if (!Character) 
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!InstanceData.AbilitySystemComponent) 
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.InputTag.IsValid()) 
	{
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayBlockAnimation::EnterState: Canceling current abilities with tag: %s"), *InstanceData.InputTag.ToString());
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && AbilitySpec.IsActive())
			{
				UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayBlockAnimation::EnterState: Canceling ability: %s with AbilityTag: %s"), 
					*AbilitySpec.Ability->GetName(), *InstanceData.InputTag.ToString());
				InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
			}
		}
	}

	FBlockTargetData* TargetData = new FBlockTargetData();
	TargetData->InputTag = InstanceData.InputTag;
	TargetData->InputType = InstanceData.InputType;
	TargetData->StartBlockMontage = InstanceData.StartBlockMontage;
	TargetData->LoopBlockMontage = InstanceData.LoopBlockMontage;
	TargetData->EndBlockMontage = InstanceData.EndBlockMontage;
	TargetData->PlayRate = InstanceData.PlayRate;
	TargetData->StartTime = InstanceData.StartTime;
	TargetData->StartSection = InstanceData.StartSection;
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));
	
	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayBlockAnimation::EnterState: Activating GA_Block with TargetData - InputTag: %s, InputType: %d, PlayRate: %.2f"), 
		*InstanceData.InputTag.ToString(), 
		(int32)InstanceData.InputType,
		InstanceData.PlayRate);

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && !AbilitySpec.IsActive())
			{
				InstanceData.AbilitySpecHandle = AbilitySpec.Handle;
			}
		}
	}
	if (!InstanceData.AbilitySpecHandle.IsValid()) return EStateTreeRunStatus::Failed;

	InstanceData.AbilitySystemComponent->InternalTryActivateAbility(InstanceData.AbilitySpecHandle, FPredictionKey(), nullptr, nullptr, &EventData);

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayBlockAnimation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

void FSTT_PlayBlockAnimation::StateCompleted(FStateTreeExecutionContext& Context,
	const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_PlayBlockAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (!AbilitySpec) return EStateTreeRunStatus::Failed;
	InstanceData.bActivated = AbilitySpec->IsActive();
	if (!InstanceData.bActivated)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayBlockAnimation::Tick: Ability activation failed"));
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}
