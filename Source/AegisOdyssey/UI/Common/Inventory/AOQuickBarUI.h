// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOQuickBarSlot.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "AOQuickBarUI.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOQuickBarUI : public UAOInventoryUI
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(BlueprintReadWrite,meta = (BindWidget))
	UWrapBox* QuickBarBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAOQuickBarSlot> QuickBarSlotClass;
private:
	UFUNCTION(BlueprintCallable)
	void RefreshInventoryBox(); //刷新WrapBox机制
	FDelegateHandle RefreshInventoryBoxDelegateHandle;
};
