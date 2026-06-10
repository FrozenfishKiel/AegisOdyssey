// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOInventoryIteminstance.h"
#include "AOInventoryComponent.h"
#include "Components/PawnComponent.h"
#include "AOInventoryManagerComponent.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class AEGISODYSSEY_API UAOInventoryManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
public:
	UAOInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void OnItemUse(FAOInventoryEntry& TargetItem){}
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void OnItemUnUse(FAOInventoryEntry& TargetItem){}
	virtual void ChangedItemOnSlot(const int32 ChangedIndex, const int32 CurrentIndex, TArray<FAOInventoryEntry>* Slots){};
};
