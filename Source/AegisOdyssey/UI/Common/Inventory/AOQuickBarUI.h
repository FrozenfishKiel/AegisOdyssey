// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOQuickBarSlot.h"
#include "Components/WrapBox.h"
#include "AOQuickBarUI.generated.h"

class UAOQuickBarComponent;

UCLASS()
class AEGISODYSSEY_API UAOQuickBarUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const override;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	void SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWrapBox> QuickBarBox = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAOQuickBarSlot> QuickBarSlotClass;

private:
	UAOQuickBarComponent* GetObservedQuickBarComponent() const;

	UFUNCTION(BlueprintCallable)
	void RefreshInventoryBox();

private:
	UPROPERTY(Transient)
	FAOInventoryDisplayContext DisplayContext;

	FDelegateHandle RefreshInventoryBoxDelegateHandle;
};
