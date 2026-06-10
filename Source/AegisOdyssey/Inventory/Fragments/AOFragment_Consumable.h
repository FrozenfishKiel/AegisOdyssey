// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_Consumable.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOFragment_Consumable : public UAOInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	bool bAllowUseFromInventory = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float EffectLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;

	bool HasUsableEffect() const;
};
