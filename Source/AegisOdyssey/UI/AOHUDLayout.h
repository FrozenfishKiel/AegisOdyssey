// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOActivatableWidget.h"
#include "ViewModel/MVVM_HUD.h"
#include "AOHUDLayout.generated.h"

/**
 * 
 */
UCLASS(Abstract,BlueprintType, Blueprintable,meta = (DisplayName = "AO HUD Layout" , Category = "AO|HUD"))
class AEGISODYSSEY_API UAOHUDLayout : public UAOActivatableWidget
{
	GENERATED_BODY()
public:
	UAOHUDLayout(const FObjectInitializer& InObjectInitializer);

	virtual void NativeOnInitialized() override;
public:
	UFUNCTION(BlueprintPure)
	UMVVM_HUD* GetHUDViewModel() const;
protected:
	void HandleEscapeAction();
	void HandleInventoryMenuAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;  //允许从HUD界面Push的Layout
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> InventoryMenuClass;
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "AO|HUD")
	FDataTableRowHandle EscapeMenuRowActionData;  //唤出退出菜单
	FUIActionBindingHandle EscapeMenuHandle;
};
