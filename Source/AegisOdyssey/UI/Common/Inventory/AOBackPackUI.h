// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "Components/WrapBox.h"
#include "AOBackPackUI.generated.h"

class UAOBackPackSlot;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOBackPackUI : public UAOInventoryUI
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(BlueprintReadWrite,meta = (BindWidget))
	UWrapBox* DefaultInventoryBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAOBackPackSlot> BackPackSlotClass;

	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const override;
private:
	UFUNCTION(BlueprintCallable)
	void RefreshInventoryBox(); //刷新WrapBox机制
	FDelegateHandle RefreshInventoryBoxDelegateHandle;
};
