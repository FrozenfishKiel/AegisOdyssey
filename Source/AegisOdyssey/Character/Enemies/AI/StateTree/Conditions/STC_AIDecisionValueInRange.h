// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeConditionBase.h"
#include "STC_AIDecisionValueInRange.generated.h"

UENUM(BlueprintType)
enum class EAOAIDecisionValueType : uint8
{
	IntentDesire,
	IntentScore,
	RepeatedIntentCount,
};

USTRUCT()
struct FAIDecisionValueInRangeInstanceData
{
	GENERATED_BODY()

	// 选择要检查的是意图 Desire、意图 Score，还是连续执行次数。
	UPROPERTY(EditAnywhere, Category = "Config")
	EAOAIDecisionValueType ValueType = EAOAIDecisionValueType::IntentScore;

	// 要查询的意图标签。
	// 不填时默认查询当前 SelectedIntentTag 对应的指标。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag IntentTag;

	// 允许区间下限，内部会自动与 MaxValue 做一次有序化。
	UPROPERTY(EditAnywhere, Category = "Config")
	float MinValue = 0.0f;

	// 允许区间上限。
	UPROPERTY(EditAnywhere, Category = "Config")
	float MaxValue = 1.0f;
};

USTRUCT(DisplayName = "AI Decision Value In Range", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTC_AIDecisionValueInRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAIDecisionValueInRangeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 检查指定决策数值是否落在给定区间内。
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	// 为 true 时对条件结果取反。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
