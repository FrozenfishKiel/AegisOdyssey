// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "AOMainUI.generated.h"

class UMVVM_CombatFeedbackFeed;
class UMVVM_CombatResources;
class UMVVM_Crafting;
class UMVVM_LocalCombatState;
class UMVVM_TargetHealthBarCollection;
struct FAOInventoryAcquisitionNotification;

UCLASS()
class AEGISODYSSEY_API UAOMainUI : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_HUD* GetMainHUDViewModel() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_CombatResources* GetCombatResourcesViewModel() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_LocalCombatState* GetLocalCombatStateViewModel() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_CombatFeedbackFeed* GetCombatFeedbackFeedViewModel() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_TargetHealthBarCollection* GetTargetHealthBarCollectionViewModel() const;

	// 正式制造 UI 获取 Crafting ViewModel 的 HUD 级入口。
	// 如果不是继承 UAOCraftingWidgetBase 的普通蓝图 Widget，而是需要从 HUD 主面板拿制造 ViewModel，优先走这里。
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AO|HUD")
	UMVVM_Crafting* GetCraftingViewModel() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Combat UI")
	TArray<FAOCombatFeedbackViewData> ConsumePendingCombatFeedback() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	TArray<FAOInventoryAcquisitionNotification> ConsumePendingInventoryAcquisition() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Inventory UI")
	void BP_HandleInventoryAcquisitionReceived(const FAOInventoryAcquisitionNotification& Notification);

protected:
	UFUNCTION()
	void HandleInventoryAcquisitionReceived(const FAOInventoryAcquisitionNotification& Notification);

	void BindInventoryAcquisitionNotifications();
	void UnbindInventoryAcquisitionNotifications();

private:
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_HUD> BoundHUDViewModel = nullptr;
};
