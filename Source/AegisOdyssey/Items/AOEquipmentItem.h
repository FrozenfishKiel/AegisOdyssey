// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "AOEquipmentItem.generated.h"

class UAOEquipmentDefinition;

// 世界中的装备物 Actor 基类。
// 它负责把场景中的装备物翻译成库存系统可接收的装备定义，而不是直接充当库存实例。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOEquipmentItem : public AAOItem
{
	GENERATED_BODY()

public:
	AAOEquipmentItem();

	UFUNCTION(BlueprintCallable, Category = "AO|Equipment")
	void SetEquipmentDefinition(TSubclassOf<UAOEquipmentDefinition> InEquipmentDefinition);

	UFUNCTION(BlueprintPure, Category = "AO|Equipment")
	TSubclassOf<UAOEquipmentDefinition> GetEquipmentDefinition() const { return EquipmentDefinition; }

	virtual void InitializeActorSpawnConfig() override;
	virtual FInventoryPickUp GetPickUpInventory() const override;

protected:
	// 这份定义决定了世界装备被拾取后会转成哪种库存装备实例。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Equipment")
	TSubclassOf<UAOEquipmentDefinition> EquipmentDefinition = nullptr;
};
