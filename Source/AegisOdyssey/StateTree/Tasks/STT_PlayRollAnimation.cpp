#include "STT_PlayRollAnimation.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayRollAnimation)
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Roll.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayPrediction.h"

EStateTreeRunStatus FSTT_PlayRollAnimation::EnterState(FStateTreeExecutionContext& Context,
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

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayRollAnimation::EnterState: Canceling current abilities with tag: %s"), *InstanceData.InputTag.ToString());
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && AbilitySpec.IsActive())
			{
				UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayRollAnimation::EnterState: Canceling ability: %s with AbilityTag: %s"), 
					*AbilitySpec.Ability->GetName(), *InstanceData.InputTag.ToString());
				InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
			}
		}
	}

	FRollTargetData* TargetData = new FRollTargetData();
	TargetData->InputTag = InstanceData.InputTag;
	TargetData->PlayRate = InstanceData.PlayRate;
	TargetData->StartTime = InstanceData.StartTime;
	TargetData->StartSection = InstanceData.StartSection;

	TargetData->ForwardMontage = InstanceData.ForwardMontage;
	TargetData->RightMontage = InstanceData.RightMontage;
	TargetData->LeftMontage = InstanceData.LeftMontage;
	TargetData->BackwardMontage = InstanceData.BackwardMontage;
	TargetData->ForwardLeftMontage = InstanceData.ForwardLeftMontage;
	TargetData->ForwardRightMontage = InstanceData.ForwardRightMontage;
	TargetData->BackwardLeftMontage = InstanceData.BackwardLeftMontage;
	TargetData->BackwardRightMontage = InstanceData.BackwardRightMontage;
	TargetData->MoveInputDirection = Character->GetLastMovementInputVector();
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));
	
	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayRollAnimation::EnterState: Activating GA_Roll with TargetData - InputTag: %s, PlayRate: %.2f"), 
		*InstanceData.InputTag.ToString(), 
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

	InstanceData.bActivated = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(InstanceData.AbilitySpecHandle,FPredictionKey(),nullptr,
		nullptr, &EventData);
	

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayRollAnimation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
}

void FSTT_PlayRollAnimation::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_PlayRollAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (!AbilitySpec) return EStateTreeRunStatus::Failed;
	InstanceData.bActivated = AbilitySpec->IsActive();
	if (!InstanceData.bActivated)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::Tick: Ability activation failed"));
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}
