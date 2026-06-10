#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "StateTreeTaskBase.h"
#include "STT_MoveToTarget.generated.h"

class AActor;
class UAITask_MoveTo;
class IGameplayTaskOwnerInterface;

USTRUCT()
struct FSTT_MoveToTargetInstanceData
{
	GENERATED_BODY()

	// 可选的外部目标绑定。
	// 如果 StateTree 没有显式绑定这个值，就回退到 AAOAIPlayerBotController::CurrentTarget。
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 到达判定半径。
	// AI 只要进入这个半径范围内，就算这次移动成功，
	// 并不是必须精确站到目标原点上。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float AcceptableRadius = 100.0f;

	// 可选的导航过滤器。
	// 不填时使用 AIController 默认的导航过滤器。
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	// 是否允许横移。
	// 开启后，移动朝向和面朝方向可以不完全一致。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAllowStrafe = false;

	// 是否接受部分路径。
	// 开启后，即使当前只能寻到一条“不完整但可走”的路径，也会先执行移动。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAllowPartialPath = true;

	// 是否持续追踪移动中的目标。
	// 适用于追逐正在移动的 Actor。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bTrackMovingGoal = true;

	// 是否在“目标 Actor 本身发生切换”时，重建一次 Move 请求。
	// 这和持续追踪目标位置不是一回事：前者处理“换目标”，后者处理“同一目标在移动”。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bTrackTargetActorChanges = true;

	// 终点如果不能落在可导航区域上，是否直接判定为无效请求。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireNavigableEndLocation = true;

	// 是否先把目标位置投影到 NavMesh 上再寻路。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bProjectGoalLocation = true;

	// 到达判定时，是否把 AI 自己的胶囊体半径也算进去。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bReachTestIncludesAgentRadius = true;

	// 到达判定时，是否把目标自身的包围体半径也算进去。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bReachTestIncludesGoalRadius = true;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(Transient)
	TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner = nullptr;

	// 记录这次 Move 请求实际追的是谁。
	// Tick 时靠它判断“目标是不是已经从 A 换成了 B”。
	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedTargetActor = nullptr;

	// 真正和寻路 / 路径跟随系统打交道的底层 AI Task。
	UPROPERTY(Transient)
	TObjectPtr<UAITask_MoveTo> MoveToTask = nullptr;
};

USTRUCT(DisplayName="Move To Target", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_MoveToTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_MoveToTargetInstanceData;

	FSTT_MoveToTarget();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AActor* ResolveTargetActor(FStateTreeExecutionContext& Context) const;
	AAIController* ResolveAIController(const FStateTreeExecutionContext& Context) const;
	EStateTreeRunStatus PerformMoveTask(FStateTreeExecutionContext& Context, AAIController& AIController) const;
};
