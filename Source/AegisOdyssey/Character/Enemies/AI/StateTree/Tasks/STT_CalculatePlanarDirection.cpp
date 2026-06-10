#include "STT_CalculatePlanarDirection.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_CalculatePlanarDirection)

FSTT_CalculatePlanarDirection::FSTT_CalculatePlanarDirection()
{
	bShouldCallTick = false;
	bShouldCopyBoundPropertiesOnTick = false;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_CalculatePlanarDirection::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.CalculatedDirection = FVector::ZeroVector;
	InstanceData.CalculatedDistance = 0.0f;
	InstanceData.bHasValidDirection = false;

	FVector StartLocation = InstanceData.SourceLocation;
	if (InstanceData.bUseSourceActorLocation)
	{
		AActor* SourceActor = ResolveSourceActor(Context, InstanceData);
		if (SourceActor == nullptr)
		{
			return EStateTreeRunStatus::Failed;
		}

		StartLocation = SourceActor->GetActorLocation();
	}

	FVector Direction = InstanceData.TargetLocation - StartLocation;
	if (InstanceData.bProjectToPlane)
	{
		Direction.Z = 0.0f;
	}

	const float Distance = Direction.Length();
	InstanceData.CalculatedDistance = Distance;

	if (Distance <= InstanceData.NearlyZeroTolerance)
	{
		return InstanceData.bFailIfDirectionIsNearlyZero ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.bNormalizeDirection)
	{
		Direction /= Distance;
	}

	InstanceData.CalculatedDirection = Direction;
	InstanceData.bHasValidDirection = true;

	if (UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(Cast<AActor>(Context.GetOwner())))
	{
		DecisionComponent->SetPendingActionDirection(Direction);
	}

	return EStateTreeRunStatus::Succeeded;
}

AActor* FSTT_CalculatePlanarDirection::ResolveSourceActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const
{
	if (InstanceData.SourceActor != nullptr)
	{
		return InstanceData.SourceActor;
	}

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerActor))
	{
		if (APawn* ControlledPawn = AIController->GetPawn())
		{
			return ControlledPawn;
		}
	}

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return OwnerPawn;
	}

	return OwnerActor;
}
