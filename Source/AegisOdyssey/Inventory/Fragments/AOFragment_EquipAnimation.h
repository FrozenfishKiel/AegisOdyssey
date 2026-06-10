// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_EquipAnimation.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOFragment_EquipAnimation : public UAOInventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere , BlueprintReadOnly)
	UAnimMontage* EquipMontage;  //装备动画

	UPROPERTY(EditAnywhere , BlueprintReadOnly)
	UAnimMontage* UnEquipMontage;  //不装备动画
};
