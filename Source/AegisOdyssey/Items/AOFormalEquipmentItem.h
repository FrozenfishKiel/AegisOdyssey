// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Items/AOEquipmentItem.h"
#include "AOFormalEquipmentItem.generated.h"

class UAOFormalEquipmentDefinition;

// 世界中的正式装备物 Actor。
// 它只是把通用装备世界物的定义入口收窄成正式装备定义，方便蓝图和摆放时少出错。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOFormalEquipmentItem : public AAOEquipmentItem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment")
	void SetFormalEquipmentDefinition(TSubclassOf<UAOFormalEquipmentDefinition> InFormalEquipmentDefinition);

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment")
	TSubclassOf<UAOFormalEquipmentDefinition> GetFormalEquipmentDefinition() const;
};
