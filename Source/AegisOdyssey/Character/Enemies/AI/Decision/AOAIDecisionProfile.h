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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Decision")
	TArray<FAOAIDecisionIntentDefinition> IntentDefinitions;

	// STE 只从这里读取“想要什么”的语义标签配置，不读取库存位置或候选物。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|InventoryDecision")
	TArray<FAOAIInventoryDesireDefinition> InventoryDesireDefinitions;
};
