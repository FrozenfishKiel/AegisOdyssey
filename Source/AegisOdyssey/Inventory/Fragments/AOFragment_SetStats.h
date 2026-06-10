// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_SetStats.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOFragment_SetStats : public UAOInventoryItemFragment
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool CanStack = false;  //是否允许堆叠?

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 MaxStack = 0;  //单格最大允许储存
};
