// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/AOInteractionSessionWidget.h"
#include "AOLayout_Inventory.generated.h"

class UAOCraftingPanelWidget;
class UAOInventoryPageUI;
class UAOInteractionSessionModel;

// 库存主布局。
// 当前正式制造 UI 就挂在这个布局里，作为库存界面的一部分被承载和刷新。
UCLASS()
class AEGISODYSSEY_API UAOLayout_Inventory : public UAOInteractionSessionWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;

	// 返回当前库存布局里承载的制造面板。
	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI")
	UAOCraftingPanelWidget* GetCraftingPanelWidget() const { return CraftingPanelWidget; }

protected:
	virtual void HandleInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel) override;

private:
	void HandleInventoryMenuAction();
	void RefreshInventoryPageContext();

protected:
	// 交互会话感知停在库存布局层。
	// 具体展示谁的库存，仍然交给库存页自己根据当前会话去取数。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UAOInventoryPageUI> InventoryPageWidget = nullptr;

	// 库存布局层只负责承载角色制造面板。
	// 不在这里重新管理制造状态真相。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UAOCraftingPanelWidget> CraftingPanelWidget = nullptr;
};
