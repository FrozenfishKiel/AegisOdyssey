// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_ResetAIDecisionState.generated.h"

USTRUCT()
struct FSTT_ResetAIDecisionStateInstanceData
{
	GENERATED_BODY()

	// 调试输出：这次是否真的找到了组件并完成重置。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bResetSucceeded = false;
};

USTRUCT(DisplayName = "Reset AI Decision State", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_ResetAIDecisionState : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ResetAIDecisionStateInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	// 有些退出状态可能只是“顺手清理一下”，不希望因为组件缺失把整条状态链判失败。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bSucceedIfComponentMissing = true;
};
