// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryUseTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "StateTreeTaskBase.h"
#include "STT_UseResolvedInventoryItem.generated.h"

USTRUCT()
struct FSTT_UseResolvedInventoryItemInstanceData
{
	GENERATED_BODY()

	// 为 true 时优先消费正式提交下来的库存决策，而不是固定 UseCommand。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bPreferDecisionPendingCommand = true;

	// 统一提交主链已经正式提交的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Input")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;

	// 当不走决策结果时，直接执行的固定库存使用命令。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIInventoryUseCommand UseCommand;

	// 本次任务是否成功触发了库存使用。
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bUseSucceeded = false;

	// 本次库存使用的执行结果。
	UPROPERTY(EditAnywhere, Category = "Output")
	FAOAIInventoryUseExecutionResult ExecutionResult;
};

USTRUCT(DisplayName = "Use Resolved Inventory Item", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_UseResolvedInventoryItem : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_UseResolvedInventoryItemInstanceData;

	FSTT_UseResolvedInventoryItem() = default;

	// 返回任务实例数据结构。
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 进入状态时解析库存动作来源，并通过正式库存执行入口触发使用。
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// 从 StateTree Owner 上解析真正执行库存动作的 Pawn。
	APawn* ResolveUserPawn(const FStateTreeExecutionContext& Context) const;

	// 按优先级解析要执行的库存决策结果和实际 UseCommand。
	bool ResolveUseCommand(
		const FStateTreeExecutionContext& Context,
		APawn* UserPawn,
		const FInstanceDataType& InstanceData,
		FAOAIInventoryDecisionResult& OutResolvedDecisionResult,
		FAOAIInventoryUseCommand& OutUseCommand) const;
};
