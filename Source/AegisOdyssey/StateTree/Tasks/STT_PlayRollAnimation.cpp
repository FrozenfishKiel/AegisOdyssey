#include "STT_PlayRollAnimation.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Roll.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayRollAnimation)

EStateTreeRunStatus FSTT_PlayRollAnimation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAOCharacter* Character = ResolveCharacter(Context);
	if (Character == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (InstanceData.AbilitySystemComponent == nullptr || !InstanceData.InputTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability != nullptr && AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag) && AbilitySpec.IsActive())
		{
			InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
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

	FVector PendingActionDirection = FVector::ZeroVector;
	if (UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(Character))
	{
		if (DecisionComponent->GetPendingActionDirection(PendingActionDirection))
		{
			TargetData->ExplicitDirection = PendingActionDirection;
			DecisionComponent->ClearPendingActionDirection();
		}
	}

	TargetData->MoveInputDirection = Character->GetLastMovementInputVector();

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability != nullptr && AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag) && !AbilitySpec.IsActive())
		{
			InstanceData.AbilitySpecHandle = AbilitySpec.Handle;
		}
	}

	if (!InstanceData.AbilitySpecHandle.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bActivated = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(
		InstanceData.AbilitySpecHandle,
		FPredictionKey(),
		nullptr,
		nullptr,
		&EventData);

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayRollAnimation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
}

void FSTT_PlayRollAnimation::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_PlayRollAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.AbilitySystemComponent == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (AbilitySpec == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bActivated = AbilitySpec->IsActive();
	return InstanceData.bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

AAOCharacter* FSTT_PlayRollAnimation::ResolveCharacter(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAOCharacter* Character = Cast<AAOCharacter>(OwnerActor))
	{
		return Character;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerActor))
	{
		return Cast<AAOCharacter>(AIController->GetPawn());
	}

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		return Cast<AAOCharacter>(Pawn);
	}

	return nullptr;
}
