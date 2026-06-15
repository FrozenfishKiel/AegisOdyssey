#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "StateTreeEvaluatorBase.h"
#include "STE_UpdateInventoryDecision.generated.h"

class AActor;

USTRUCT()
struct FUpdateInventoryDecisionInstanceData
{
	GENERATED_BODY()

	// 当前库存评估观察到的目标。
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// 当前目标与自身的距离。
	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget = 0.0f;

	// 当前是否存在有效目标。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasTarget = false;

	// 由战斗 Evaluator 产出的当前战术态。
	UPROPERTY(EditAnywhere, Category = "Input")
	FAOAIDecisionTacticalState TacticalState;

	// 当前是否存在已经正式提交到执行侧的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasCurrentSubmittedInventoryDecision = false;

	// 当前已经正式提交到执行侧的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Output")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;

	// 当前主战术态是否给 Additive 库存动作留出了并行窗口。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasAdditiveInventoryWindow = false;
};

USTRUCT(DisplayName = "Update Inventory Decision", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTE_UpdateInventoryDecision : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUpdateInventoryDecisionInstanceData;

	FSTE_UpdateInventoryDecision() = default;

	// 返回库存决策 Evaluator 使用的实例数据结构。
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 每帧执行库存决策评估。
	// 它负责把当前战斗事实和 TacticalState 转换成库存评估结果，再同步回 UAOAIDecisionComponent。
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
