#include "STT_StartToJump.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_LightAttack.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayPrediction.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Jump.h"

EStateTreeRunStatus FSTT_StartToJump::EnterState(FStateTreeExecutionContext& Context,
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

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Canceling current abilities with tag: %s"), *InstanceData.InputTag.ToString());
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && AbilitySpec.IsActive())
			{
				UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Canceling ability: %s with AbilityTag: %s"), 
					*AbilitySpec.Ability->GetName(), *InstanceData.InputTag.ToString());
				InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
			}
		}
	}

	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;

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

	InstanceData.bActivated = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(InstanceData.AbilitySpecHandle,FPredictionKey(),nullptr,
		nullptr, &EventData);

	return EStateTreeRunStatus::Running;
}

void FSTT_StartToJump::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

void FSTT_StartToJump::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_StartToJump::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (!AbilitySpec) return EStateTreeRunStatus::Failed;
	InstanceData.bActivated = AbilitySpec->IsActive();
	if (!InstanceData.bActivated)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_StartToJump::Tick: Ability activation failed"));
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}
