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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|Decision")
	TArray<FAOAIDecisionIntentDefinition> IntentDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AI|InventoryDecision")
	TArray<FAOAIInventoryActionDefinition> InventoryActionDefinitions;
};
