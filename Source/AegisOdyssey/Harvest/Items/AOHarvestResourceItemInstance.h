// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AOHarvestResourceItemInstance.generated.h"

class UAOHarvestResourceDefinition;

// 采集产物的正式物品实例类型。
// 它给背包里的木材、矿石、草药等采集资源保留独立实例层入口。
UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOHarvestResourceItemInstance : public UAOInventoryItemInstance
{
	GENERATED_BODY()

public:
	UAOHarvestResourceItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	const UAOHarvestResourceDefinition* GetHarvestResourceDefinition() const;
};
