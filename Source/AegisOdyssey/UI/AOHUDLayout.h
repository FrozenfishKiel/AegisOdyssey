// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOActivatableWidget.h"
#include "ViewModel/MVVM_HUD.h"
#include "AOHUDLayout.generated.h"

class UAOInteractionSessionComponent;
class UAOInteractionSessionModel;
class UAOInteractionSessionWidget;
class UAOItemHoverTooltipWidget;
class UMVVM_ItemHoverTooltip;

// HUD 根布局。
// 它负责通用菜单输入，也负责把对象侧交互会话转成玩家客户端上的实际界面显示。
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisplayName = "AO HUD Layout", Category = "AO|HUD"))
class AEGISODYSSEY_API UAOHUDLayout : public UAOActivatableWidget
{
	GENERATED_BODY()

public:
	UAOHUDLayout(const FObjectInitializer& InObjectInitializer);

	virtual void NativeOnInitialized() override;

public:
	UFUNCTION(BlueprintPure)
	UMVVM_HUD* GetHUDViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_ItemHoverTooltip* GetItemHoverTooltipViewModel() const;

protected:
	// 绑定玩家身上的交互会话组件，统一监听对象交互界面的打开 / 关闭。
	void BindInteractionSessionComponent();
	void EnsureItemHoverTooltipWidget();

	// 当玩家当前交互会话变化时，决定是否在本地客户端 push 对象提供的界面。
	void HandleCurrentInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel);

	// 关闭当前由交互会话打开的界面。
	void CloseCurrentInteractionWidget();

	void HandleEscapeAction();
	void HandleInventoryMenuAction();

protected:
	// HUD 上用于弹出逃生菜单的布局类。
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;

	// HUD 上用于弹出通用背包菜单的布局类。
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> InventoryMenuClass;

	// 逃生菜单动作数据。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|HUD")
	FDataTableRowHandle EscapeMenuRowActionData;

	// 当前 HUD 正在监听的交互会话组件。
	UPROPERTY(Transient)
	TObjectPtr<UAOInteractionSessionComponent> BoundInteractionSessionComponent = nullptr;

	// 通过 Tab 打开的库存界面实例。
	// 如果对象交互也想复用同一个布局类，可以直接复用这个实例，避免重复 push。
	UPROPERTY(Transient)
	TObjectPtr<UAOInteractionSessionWidget> ActiveInventoryMenuWidget = nullptr;

	// 当前由交互会话驱动的界面实例。
	UPROPERTY(Transient)
	TObjectPtr<UAOInteractionSessionWidget> ActiveInteractionWidget = nullptr;

	// HUD 根布局下全局唯一的物品 Tooltip Widget。
	// 所有库存格子和制造条目都只复用这一份实例，不在各自界面里重复持有。
	UPROPERTY(Transient)
	TObjectPtr<UAOItemHoverTooltipWidget> ActiveItemHoverTooltipWidget = nullptr;

	// 当前交互界面是否由交互会话主动 push 出来。
	// 如果只是复用了 Tab 已经打开的库存界面，会话关闭时不应把整个界面一起关掉。
	bool bIsActiveInteractionWidgetOwnedBySession = false;

	// 当前交互会话切换的委托句柄。
	FDelegateHandle InteractionSessionChangedHandle;

	UPROPERTY(EditDefaultsOnly, Category = "AO|HUD")
	TSubclassOf<UAOItemHoverTooltipWidget> ItemHoverTooltipWidgetClass;
};
