// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/AOActivatableWidget.h"
#include "AOLayout_Inventory.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOLayout_Inventory : public UAOActivatableWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
private:
	void HandleInventoryMenuAction();
};
