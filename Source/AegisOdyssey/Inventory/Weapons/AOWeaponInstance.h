// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment//AOEquipmentInstance.h"
#include "AOWeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOWeaponInstance : public UAOEquipmentInstance
{
	GENERATED_BODY()
public:
	UAOWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef) override;
};
