// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "AOBackPackSlot.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOBackPackSlot : public UAOInventoryUI
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void InitializeSlot();
	
	UPROPERTY(BlueprintReadWrite , meta = (BindWidget))
	UImage* Icon;

	UPROPERTY(BlueprintReadWrite , meta = (BindWidget))
	UTextBlock* ItemCount;

	UPROPERTY(BlueprintReadWrite)
	UAOInventoryItemInstance* ItemInstance = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 Index = 0;

	UPROPERTY(BlueprintReadWrite)
	UAOInventoryComponent* SourceContainer = nullptr;  //背包格子隶属于哪个背包

	UPROPERTY(BlueprintReadWrite)
	FAOInventorySlot InInventorySlot;
};
