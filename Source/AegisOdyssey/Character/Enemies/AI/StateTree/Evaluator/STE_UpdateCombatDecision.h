#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "GameplayTagContainer.h"
#include "StateTreeEvaluatorBase.h"
#include "STE_UpdateCombatDecision.generated.h"

class AActor;

USTRUCT()
struct FUpdateCombatDecisionInstanceData
{
	GENERATED_BODY()

	// 当前决策评估所观察到的目标。
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// 当前目标与自身的距离，供决策组件计算距离相关因子。
	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget = 0.0f;

	// 当前目标是否已经进入自身攻击距离。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bIsInAttackRange = false;

	// 当前是否存在有效目标。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasTarget = false;

	// 可选的观测意图标签；未配置时默认回落到本帧选中的主意图。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ObservedIntentTag;

	// 本帧评估出的主战斗意图，属于旧接口兼容输出，同时仍可用于调试观察。
	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag SelectedIntentTag;

	// 选中主意图的 Desire 值。
	UPROPERTY(EditAnywhere, Category = "Output")
	float SelectedIntentDesire = 0.0f;

	// 选中主意图的最终 Score 值。
	UPROPERTY(EditAnywhere, Category = "Output")
	float SelectedIntentScore = 0.0f;

	// 实际被拿来查询指标的意图标签。
	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag ResolvedObservedIntentTag;

	// 观测意图的 Desire 值。
	UPROPERTY(EditAnywhere, Category = "Output")
	float ObservedIntentDesire = 0.0f;

	// 观测意图的 Score 值。
	UPROPERTY(EditAnywhere, Category = "Output")
	float ObservedIntentScore = 0.0f;

	// 当前是否成功拿到了观测意图的运行时指标。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasObservedIntentMetrics = false;

	// 上一次正式进入执行链的主意图标签。
	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag LastExecutedIntentTag;

	// 当前连续重复执行同一主意图的次数。
	UPROPERTY(EditAnywhere, Category = "Output")
	int32 RepeatedIntentCount = 0;

	UPROPERTY(EditAnywhere, Category = "Output")
	FAOAIDecisionTacticalState TacticalState;

};

USTRUCT(DisplayName = "Update Combat Decision", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTE_UpdateCombatDecision : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUpdateCombatDecisionInstanceData;

	FSTE_UpdateCombatDecision() = default;

	// 返回本 Evaluator 使用的实例数据结构。
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 每帧把当前战斗事实送入决策组件，并同步战斗侧评估结果。
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
