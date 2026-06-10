// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Common/Inventory/AOInventorySlotBase.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AOQuickBarSlot.generated.h"

UCLASS()
class AEGISODYSSEY_API UAOQuickBarSlot : public UAOInventorySlotBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct();

	void InitializeSlot();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemCount = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FAOInventorySlot InQuickBarSlot;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* Durability = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* InputIndex = nullptr;
};
