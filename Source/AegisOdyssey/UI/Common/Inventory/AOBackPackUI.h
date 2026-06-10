// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "Components/WrapBox.h"
#include "AOBackPackUI.generated.h"

class UAOBackPackComponent;
class UAOBackPackSlot;

UCLASS()
class AEGISODYSSEY_API UAOBackPackUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const override;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	void SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWrapBox> DefaultInventoryBox = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAOBackPackSlot> BackPackSlotClass;

private:
	UAOBackPackComponent* GetObservedBackPackComponent() const;

	UFUNCTION(BlueprintCallable)
	void RefreshInventoryBox();

private:
	UPROPERTY(Transient)
	FAOInventoryDisplayContext DisplayContext;

	FDelegateHandle RefreshInventoryBoxDelegateHandle;
};
