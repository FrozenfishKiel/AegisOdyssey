// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_CommitAIDecisionIntent.generated.h"

USTRUCT()
struct FSTT_CommitAIDecisionIntentInstanceData
{
	GENERATED_BODY()

	// 当前状态真正开始执行的主意图标签。
	// 例如攻击状态填 AI.Intent.Attack，走位状态填 AI.Intent.Strafe。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ExecutedIntentTag;

	// 调试输出：这次是否成功把执行记录回写给决策组件。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bCommitSucceeded = false;
};

USTRUCT(DisplayName = "Commit AI Decision Intent", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_CommitAIDecisionIntent : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_CommitAIDecisionIntentInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 一进入状态就回写“已经开始执行了哪个意图”的记录。
	// 这里承担的是执行记忆回写职责，不是新的正式决策入口。
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
