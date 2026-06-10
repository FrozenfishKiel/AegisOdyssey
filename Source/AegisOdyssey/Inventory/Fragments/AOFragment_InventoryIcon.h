// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "Styling/SlateBrush.h"
#include "AOFragment_InventoryIcon.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOFragment_InventoryIcon : public UAOInventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere , BlueprintReadOnly  , Category = Inventory)
	FSlateBrush Brush;  //物品在背包内的图标显示
};
