// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "StateTreeConditionBase.h"
#include "STC_AIPendingInventoryDecisionMatches.generated.h"

UENUM(BlueprintType)
enum class EAOAIInventoryDecisionCoordinationFilter : uint8
{
	Any UMETA(DisplayName = "Any"),
	Exclusive UMETA(DisplayName = "Exclusive"),
	Additive UMETA(DisplayName = "Additive")
};

USTRUCT()
struct FAIPendingInventoryDecisionMatchesInstanceData
{
	GENERATED_BODY()

	// 当前是否存在已经正式提交的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasCurrentSubmittedInventoryDecision = false;

	// 当前正式提交中的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Input")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;

	// 当前主战术态是否存在 Additive 库存窗口。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasAdditiveInventoryWindow = false;

	// 为 true 时要求当前必须存在一个可匹配的库存决策结果。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequirePendingInventoryDecision = true;

	// 为 true 时要求当前主链已经给 Additive 库存动作留出了执行窗口。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireAdditiveInventoryWindow = false;

	// 可选的库存协同模式过滤条件。
	UPROPERTY(EditAnywhere, Category = "Config")
	EAOAIInventoryDecisionCoordinationFilter CoordinationFilter = EAOAIInventoryDecisionCoordinationFilter::Any;

	// 仅当配置有效 Tag 时，要求结果的 ActionTag 精确匹配。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ExpectedActionTag;

	// 仅当配置有效 Tag 时，要求结果的 CandidateTag 精确匹配。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ExpectedCandidateTag;

	// 为 true 时要求该库存结果已经裁定出 resolved target。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireResolvedTarget = false;
};

USTRUCT(DisplayName = "AI Pending Inventory Decision Matches", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTC_AIPendingInventoryDecisionMatches : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAIPendingInventoryDecisionMatchesInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 检查当前正式提交中的库存决策结果是否满足给定约束。
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	// 为 true 时对条件结果取反。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
