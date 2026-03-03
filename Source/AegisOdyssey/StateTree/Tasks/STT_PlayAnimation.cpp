#include "STT_PlayAnimation.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_LightAttack.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayAnimation)

EStateTreeRunStatus FSTT_PlayAnimation::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner());
	if (!Character) 
	{
		UE_LOG(LogStateTree, Error, TEXT("FSTT_PlayAnimation::EnterState: Character is null"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!InstanceData.AbilitySystemComponent) 
	{
		UE_LOG(LogStateTree, Error, TEXT("FSTT_PlayAnimation::EnterState: AbilitySystemComponent is null"));
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.InputTag.IsValid()) 
	{
		UE_LOG(LogStateTree, Error, TEXT("FSTT_PlayAnimation::EnterState: InputTag is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Creating ULightAttackParams..."));

	ULightAttackParams* LightAttackParams = NewObject<ULightAttackParams>(Character);
	if (!LightAttackParams) 
	{
		UE_LOG(LogStateTree, Error, TEXT("FSTT_PlayAnimation::EnterState: Failed to create ULightAttackParams"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: ULightAttackParams created successfully"));

	LightAttackParams->InputTag = InstanceData.InputTag;
	LightAttackParams->Montage = InstanceData.Montage;
	LightAttackParams->PlayRate = InstanceData.PlayRate;
	LightAttackParams->StartSection = InstanceData.StartSection;
	LightAttackParams->StartTime = InstanceData.StartTime;

	FGameplayEventData EventData;
	EventData.OptionalObject = LightAttackParams;
	EventData.EventTag = InstanceData.InputTag;

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Sending event with tag: %s"), *InstanceData.InputTag.ToString());

	int32 ActivatedCount = InstanceData.AbilitySystemComponent->HandleGameplayEvent(InstanceData.InputTag, &EventData);

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: ActivatedCount: %d"), ActivatedCount);

	if (ActivatedCount > 0)
	{
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

void FSTT_PlayAnimation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	
}

EStateTreeRunStatus FSTT_PlayAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.AbilitySystemComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.IsActive())
			{
				return EStateTreeRunStatus::Running;
			}
			else
			{
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayAnimation::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}
