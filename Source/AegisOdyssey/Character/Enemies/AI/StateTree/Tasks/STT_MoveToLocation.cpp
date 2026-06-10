#include "STT_MoveToLocation.h"

#include "AITypes.h"
#include "GameplayTaskOwnerInterface.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "Tasks/AITask.h"
#include "Tasks/AITask_MoveTo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_MoveToLocation)

FSTT_MoveToLocation::FSTT_MoveToLocation()
{
	bShouldCallTick = false;
	bShouldCopyBoundPropertiesOnTick = false;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_MoveToLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.MoveToTask = nullptr;
	InstanceData.AIController = ResolveAIController(Context);
	InstanceData.TaskOwner = nullptr;

	if (InstanceData.AIController == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToLocation::EnterState: AIController is missing."));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.TaskOwner = TScriptInterface<IGameplayTaskOwnerInterface>(
		InstanceData.AIController->FindComponentByInterface(UGameplayTaskOwnerInterface::StaticClass()));
	if (!InstanceData.TaskOwner)
	{
		InstanceData.TaskOwner = InstanceData.AIController;
	}

	return PerformMoveTask(Context, *InstanceData.AIController);
}

EStateTreeRunStatus FSTT_MoveToLocation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Running;
}

void FSTT_MoveToLocation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.MoveToTask && InstanceData.MoveToTask->GetState() != EGameplayTaskState::Finished)
	{
		UE_LOG(LogStateTree, Log, TEXT("FSTT_MoveToLocation::ExitState: Canceling active move task."));
		InstanceData.MoveToTask->ExternalCancel();
	}

	InstanceData.MoveToTask = nullptr;
	InstanceData.TaskOwner = nullptr;
}

AAIController* FSTT_MoveToLocation::ResolveAIController(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* DirectController = Cast<AAIController>(OwnerActor))
	{
		return DirectController;
	}

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return Cast<AAIController>(OwnerPawn->GetController());
	}

	return nullptr;
}

EStateTreeRunStatus FSTT_MoveToLocation::PerformMoveTask(FStateTreeExecutionContext& Context, AAIController& AIController) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FVector ResolvedGoalLocation = InstanceData.GoalLocation;
	if (!FAISystem::IsValidLocation(ResolvedGoalLocation))
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToLocation::PerformMoveTask: GoalLocation is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(ResolvedGoalLocation);
	MoveRequest.SetNavigationFilter(InstanceData.FilterClass ? InstanceData.FilterClass : AIController.GetDefaultNavigationFilterClass());
	MoveRequest.SetAllowPartialPath(InstanceData.bAllowPartialPath);
	MoveRequest.SetAcceptanceRadius(InstanceData.AcceptableRadius);
	MoveRequest.SetCanStrafe(InstanceData.bAllowStrafe);
	MoveRequest.SetReachTestIncludesAgentRadius(InstanceData.bReachTestIncludesAgentRadius);
	MoveRequest.SetReachTestIncludesGoalRadius(InstanceData.bReachTestIncludesGoalRadius);
	MoveRequest.SetRequireNavigableEndLocation(InstanceData.bRequireNavigableEndLocation);
	MoveRequest.SetProjectGoalLocation(InstanceData.bProjectGoalLocation);
	MoveRequest.SetUsePathfinding(true);

	if (!MoveRequest.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToLocation::PerformMoveTask: Move request is invalid for location %s."),
			*ResolvedGoalLocation.ToString());
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.MoveToTask == nullptr)
	{
		InstanceData.MoveToTask = UAITask::NewAITask<UAITask_MoveTo>(AIController, *InstanceData.TaskOwner);
	}

	if (InstanceData.MoveToTask == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToLocation::PerformMoveTask: Failed to create UAITask_MoveTo."));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.MoveToTask->SetUp(&AIController, MoveRequest);

	const bool bIsGameplayTaskAlreadyActive = InstanceData.MoveToTask->IsActive();
	if (bIsGameplayTaskAlreadyActive)
	{
		InstanceData.MoveToTask->ConditionalPerformMove();
	}
	else
	{
		InstanceData.MoveToTask->ReadyForActivation();
	}

	if (!bIsGameplayTaskAlreadyActive)
	{
		if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
		{
			return InstanceData.MoveToTask->WasMoveSuccessful() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
		}

		InstanceData.MoveToTask->OnMoveTaskFinished.AddLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](TEnumAsByte<EPathFollowingResult::Type> Result, AAIController* Controller)
			{
				WeakContext.FinishTask(Result == EPathFollowingResult::Success ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed);
			});
	}

	UE_LOG(LogStateTree, Log, TEXT("FSTT_MoveToLocation::PerformMoveTask: Moving toward %s with acceptance radius %.2f."),
		*ResolvedGoalLocation.ToString(),
		InstanceData.AcceptableRadius);

	return EStateTreeRunStatus::Running;
}
