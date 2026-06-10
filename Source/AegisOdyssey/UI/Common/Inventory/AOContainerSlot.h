// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/UI/Common/Inventory/AOInventorySlotBase.h"
#include "AOContainerSlot.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class AEGISODYSSEY_API UAOContainerSlot : public UAOInventorySlotBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void InitializeSlot();

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemCount = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Interaction")
	FAOObservedInventorySlot ObservedSlot;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Interaction")
	FAOInventorySlot InInventorySlot;
};
