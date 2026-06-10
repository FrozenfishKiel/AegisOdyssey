#include "STT_MoveToTarget.h"

#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "GameplayTaskOwnerInterface.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "Tasks/AITask.h"
#include "Tasks/AITask_MoveTo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_MoveToTarget)

FSTT_MoveToTarget::FSTT_MoveToTarget()
{
	bShouldCallTick = true;
	bShouldCopyBoundPropertiesOnTick = true;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_MoveToTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.CachedTargetActor = nullptr;
	InstanceData.MoveToTask = nullptr;
	InstanceData.TaskOwner = nullptr;
	// StateTree 的 Owner 可能是 Controller，也可能是 Pawn，
	// 这里统一解析成真正负责移动的 AIController。
	InstanceData.AIController = ResolveAIController(Context);

	if (InstanceData.AIController == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToTarget::EnterState: AIController is missing."));
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

EStateTreeRunStatus FSTT_MoveToTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.AIController == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* ResolvedTargetActor = ResolveTargetActor(Context);
	if (ResolvedTargetActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToTarget::Tick: TargetActor is missing."));
		return EStateTreeRunStatus::Failed;
	}

	// bTrackMovingGoal 只负责“同一个目标在移动时持续更新位置”。
	// 如果这里连目标 Actor 引用都变了，说明已经从旧目标切到新目标，
	// 那就要整条 Move 请求重建一次，而不是只沿用旧请求继续追。
	if (InstanceData.bTrackTargetActorChanges && ResolvedTargetActor != InstanceData.CachedTargetActor)
	{
		UE_LOG(LogStateTree, Log, TEXT("FSTT_MoveToTarget::Tick: CurrentTarget changed from %s to %s, restarting move."),
			*GetNameSafe(InstanceData.CachedTargetActor),
			*GetNameSafe(ResolvedTargetActor));
		return PerformMoveTask(Context, *InstanceData.AIController);
	}

	if (InstanceData.MoveToTask == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_MoveToTarget::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.MoveToTask && InstanceData.MoveToTask->GetState() != EGameplayTaskState::Finished)
	{
		UE_LOG(LogStateTree, Log, TEXT("FSTT_MoveToTarget::ExitState: Canceling active move task."));
		InstanceData.MoveToTask->ExternalCancel();
	}

	InstanceData.MoveToTask = nullptr;
	InstanceData.CachedTargetActor = nullptr;
	InstanceData.TaskOwner = nullptr;
}

AActor* FSTT_MoveToTarget::ResolveTargetActor(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.TargetActor != nullptr)
	{
		return InstanceData.TargetActor;
	}

	// 如果 StateTree 没有外部绑定目标，就直接读取控制器当前维护的 CurrentTarget。
	if (AAOAIPlayerBotController* AOAIController = Cast<AAOAIPlayerBotController>(InstanceData.AIController))
	{
		return AOAIController->GetCurrentTarget();
	}

	return nullptr;
}

AAIController* FSTT_MoveToTarget::ResolveAIController(const FStateTreeExecutionContext& Context) const
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

EStateTreeRunStatus FSTT_MoveToTarget::PerformMoveTask(FStateTreeExecutionContext& Context, AAIController& AIController) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AActor* TargetActor = ResolveTargetActor(Context);
	if (TargetActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToTarget::PerformMoveTask: No target actor available."));
		return EStateTreeRunStatus::Failed;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(TargetActor);
	MoveRequest.SetNavigationFilter(InstanceData.FilterClass ? InstanceData.FilterClass : AIController.GetDefaultNavigationFilterClass());
	MoveRequest.SetAllowPartialPath(InstanceData.bAllowPartialPath);
	// 这是最核心的调参项之一：
	// 规定“离目标多近就算到达”，从而决定 MoveTo 何时成功结束。
	MoveRequest.SetAcceptanceRadius(InstanceData.AcceptableRadius);
	MoveRequest.SetCanStrafe(InstanceData.bAllowStrafe);
	MoveRequest.SetReachTestIncludesAgentRadius(InstanceData.bReachTestIncludesAgentRadius);
	MoveRequest.SetReachTestIncludesGoalRadius(InstanceData.bReachTestIncludesGoalRadius);
	MoveRequest.SetRequireNavigableEndLocation(InstanceData.bRequireNavigableEndLocation);
	MoveRequest.SetProjectGoalLocation(InstanceData.bProjectGoalLocation);
	MoveRequest.SetUsePathfinding(true);

	if (!MoveRequest.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToTarget::PerformMoveTask: Move request is invalid for target %s."), *GetNameSafe(TargetActor));
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.MoveToTask == nullptr)
	{
		InstanceData.MoveToTask = UAITask::NewAITask<UAITask_MoveTo>(AIController, *InstanceData.TaskOwner);
	}

	if (InstanceData.MoveToTask == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_MoveToTarget::PerformMoveTask: Failed to create UAITask_MoveTo."));
		return EStateTreeRunStatus::Failed;
	}

	// 这个 State 存活期间尽量复用同一个 UAITask_MoveTo，
	// 只刷新它内部的 MoveRequest，避免反复新建底层 Task 对象。
	InstanceData.MoveToTask->SetUp(&AIController, MoveRequest);
	InstanceData.MoveToTask->SetContinuousGoalTracking(InstanceData.bTrackMovingGoal);
	InstanceData.CachedTargetActor = TargetActor;

	const bool bIsGameplayTaskAlreadyActive = InstanceData.MoveToTask->IsActive();
	if (bIsGameplayTaskAlreadyActive)
	{
		// 如果底层 Task 已经在跑，就直接让它立刻执行这次更新后的 Move 请求，
		// 而不是把 Task 整个重新激活一遍。
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

		// 只在第一次激活这个 Task 对象时绑定完成回调。
		// 后续移动成功 / 失败时，由底层路径跟随结果异步通知 StateTree 收尾。
		InstanceData.MoveToTask->OnMoveTaskFinished.AddLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](TEnumAsByte<EPathFollowingResult::Type> Result, AAIController* Controller)
			{
				WeakContext.FinishTask(Result == EPathFollowingResult::Success ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed);
			});
	}

	UE_LOG(LogStateTree, Log, TEXT("FSTT_MoveToTarget::PerformMoveTask: Moving toward %s with acceptance radius %.2f."),
		*GetNameSafe(TargetActor),
		InstanceData.AcceptableRadius);

	return EStateTreeRunStatus::Running;
}
