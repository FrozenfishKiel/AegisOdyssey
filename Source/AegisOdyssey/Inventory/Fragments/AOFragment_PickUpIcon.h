// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_PickUpIcon.generated.h"

/**
 * 
 */
//拾取物品的信息类
UCLASS()
class AEGISODYSSEY_API UAOFragment_PickUpIcon : public UAOInventoryItemFragment
{
	GENERATED_BODY()
public:

	UAOFragment_PickUpIcon();
	
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = InventoryIcon)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = InventoryIcon)
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = InventoryIcon)
	FLinearColor PadColor;
};
