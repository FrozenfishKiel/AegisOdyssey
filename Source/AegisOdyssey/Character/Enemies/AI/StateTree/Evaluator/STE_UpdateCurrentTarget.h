#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "STE_UpdateCurrentTarget.generated.h"

class AActor;

USTRUCT()
struct FUpdateCurrentTargetInstanceData
{
	GENERATED_BODY()

	// [输出] 当前目标
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// [输出] 到目标的距离
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.0f;

	// [输出] 是否在攻击范围内
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsInAttackRange = false;

	// [输出] 是否有有效目标
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasTarget = false;
};

USTRUCT(DisplayName="Update Current Target", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTE_UpdateCurrentTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUpdateCurrentTargetInstanceData;

	FSTE_UpdateCurrentTarget() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 只负责更新“当前目标相关事实”，不掺杂战斗决策分数和意图。
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
