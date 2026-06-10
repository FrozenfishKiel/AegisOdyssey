// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeConditionBase.h"
#include "STC_AIDecisionIntentMatches.generated.h"

USTRUCT()
struct FAIDecisionIntentMatchesInstanceData
{
	GENERATED_BODY()

	// 期望匹配到的决策标签。
	// 例如填 AI.Intent.Attack，表示只有当前主链决策匹配攻击时条件才成立。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ExpectedIntentTag;
};

USTRUCT(DisplayName = "AI Decision Intent Matches", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTC_AIDecisionIntentMatches : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAIDecisionIntentMatchesInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 检查当前正式决策主链是否匹配 ExpectedIntentTag。
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	// 为 true 时对条件结果取反，便于复用同一 Condition。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
