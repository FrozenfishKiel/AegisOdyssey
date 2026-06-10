#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "StateTreeTaskBase.h"
#include "STT_MoveToLocation.generated.h"

class UAITask_MoveTo;
class IGameplayTaskOwnerInterface;

USTRUCT()
struct FSTT_MoveToLocationInstanceData
{
	GENERATED_BODY()

	// 要前往的目标点。建议绑定到父状态共享参数里的 PatrolLocation 之类的变量。
	UPROPERTY(EditAnywhere, Category = "Input")
	FVector GoalLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float AcceptableRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAllowStrafe = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAllowPartialPath = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireNavigableEndLocation = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bProjectGoalLocation = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bReachTestIncludesAgentRadius = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bReachTestIncludesGoalRadius = true;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(Transient)
	TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAITask_MoveTo> MoveToTask = nullptr;
};

USTRUCT(DisplayName = "Move To Location", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_MoveToLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_MoveToLocationInstanceData;

	FSTT_MoveToLocation();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AAIController* ResolveAIController(const FStateTreeExecutionContext& Context) const;
	EStateTreeRunStatus PerformMoveTask(FStateTreeExecutionContext& Context, AAIController& AIController) const;
};
