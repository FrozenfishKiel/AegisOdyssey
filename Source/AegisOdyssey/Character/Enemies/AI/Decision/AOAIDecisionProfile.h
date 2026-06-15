// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "AOAIDecisionProfile.generated.h"

UCLASS(BlueprintType, Const, Meta = (DisplayName = "AO AI Decision Profile"))
class AEGISODYSSEY_API UAOAIDecisionProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 主战斗意图配置表。
	// 这里定义 AI 会参与评估的主意图，以及每个意图的基础欲望、距离因子、冷却和切换参数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Decision")
	TArray<FAOAIDecisionIntentDefinition> IntentDefinitions;

	// 库存动作配置表。
	// 这里定义 AI 在什么战术上下文下考虑“使用库存”，以及每种库存动作下面有哪些具体候选物品策略。
	// 如果这里为空，库存 Evaluator 将不会产出任何有效库存决策结果。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|InventoryDecision")
	TArray<FAOAIInventoryActionDefinition> InventoryActionDefinitions;
};
