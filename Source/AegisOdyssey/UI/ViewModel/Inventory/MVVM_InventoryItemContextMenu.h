#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/AOInventoryItemContextMenuTypes.h"
#include "Styling/SlateBrush.h"
#include "MVVM_InventoryItemContextMenu.generated.h"

class UAOInventoryComponent;
class UAOInventoryItemInstance;
class UAOInventoryUI;
class UAOInventoryItemDefinition;
class UMVVM_Crafting;
class UMVVM_InventoryItemContextAction;

// 右键菜单主 ViewModel。
// 它统一持有本次右键菜单的显示快照和执行上下文。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_InventoryItemContextMenu : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_InventoryItemContextMenu(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 记录库存右键菜单的回调入口。
	// 库存物品菜单最终仍通过 InventoryUI 统一转发行为。
	void SetRequestingInventoryUI(UAOInventoryUI* InRequestingInventoryUI);

	// 记录制造配方右键菜单的回调入口。
	// 当菜单不是从 InventoryUI 打开时，会直接回落到 Crafting ViewModel 发起制作请求。
	void SetRequestingCraftingViewModel(UMVVM_Crafting* InCraftingViewModel);

	// 用库存槽位上下文打开菜单。
	void OpenForInventorySlot(
		UAOInventoryComponent* InSourceInventory,
		int32 InSourceSlotIndex,
		UAOInventoryItemInstance* InItemInstance,
		const FVector2D& InScreenSpacePosition,
		const TArray<FAOInventoryItemContextAction>& InResolvedActions);

	// 用制造配方上下文打开菜单。
	void OpenForCraftingRecipe(
		FName InRecipeRowName,
		const FText& InDisplayName,
		const FSlateBrush& InIconBrush,
		bool bInHasValidIcon,
		const FText& InInfoText,
		const FVector2D& InScreenSpacePosition,
		const TArray<FAOInventoryItemContextAction>& InResolvedActions);

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory Context Menu")
	void CloseMenu();

	// 执行已经决议好的菜单动作。
	// 这里不再做“菜单该显示什么”的推导，只负责把当前上下文回传给正确入口。
	bool ExecuteResolvedAction(const FAOInventoryItemContextAction& InResolvedAction);

	// 菜单复用时把当前快照重新广播给已绑定 Widget。
	void BroadcastCurrentSnapshot();

	void SetItemDisplayName(const FText& InItemDisplayName);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	FText GetItemDisplayName() const { return ItemDisplayName; }

	void SetItemInfoText(const FText& InItemInfoText);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	FText GetItemInfoText() const { return ItemInfoText; }

	void SetItemIconBrush(const FSlateBrush& InItemIconBrush);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	FSlateBrush GetItemIconBrush() const { return ItemIconBrush; }

	void SetHasValidItemIcon(bool bInHasValidItemIcon);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	bool HasValidItemIcon() const { return bHasValidItemIcon; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	bool IsMenuVisible() const { return bMenuVisible; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	FVector2D GetScreenSpacePosition() const { return ScreenSpacePosition; }

	// 当前菜单动作列表的唯一消费入口。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	TArray<UMVVM_InventoryItemContextAction*> GetActionViewModels() const;

	// 菜单复用但本次上下文已失效时，统一清空显示内容。
	void ResetDisplay();

private:
	void SetMenuVisible(bool bInMenuVisible);
	void SetScreenSpacePosition(const FVector2D& InScreenSpacePosition);

	// 按本次动作决议重建动作项 ViewModel 列表。
	void RebuildActionViewModels(const TArray<FAOInventoryItemContextAction>& InResolvedActions);

	// 从库存上下文构造头部辅助信息。
	FText BuildItemInfoText() const;

	// 从当前缓存上下文刷新头部名称、图标和补充文案。
	void RefreshHeaderFromCurrentContext();

	// 从库存物品右键菜单回到统一 UI 执行链的通道。
	TWeakObjectPtr<UAOInventoryUI> RequestingInventoryUI;

	// 从制造配方右键菜单直接回到 Crafting ViewModel 的通道。
	TWeakObjectPtr<UMVVM_Crafting> RequestingCraftingViewModel;

	// 当前被右键命中的库存来源。
	TWeakObjectPtr<UAOInventoryComponent> SourceInventory;

	// 当前被右键命中的物品实例。
	TWeakObjectPtr<UAOInventoryItemInstance> ItemInstance;

	// 当前被右键命中的制造配方行名。
	FName CraftingRecipeRowName = NAME_None;

	// 当前被右键命中的来源槽位。
	int32 SourceSlotIndex = INDEX_NONE;

	// 以下字段都是“当前菜单显示快照”，不是库存系统里的长期真相。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemDisplayName, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	FText ItemDisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemInfoText, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	FText ItemInfoText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemIconBrush, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	FSlateBrush ItemIconBrush;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = HasValidItemIcon, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	bool bHasValidItemIcon = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsMenuVisible, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	bool bMenuVisible = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetScreenSpacePosition, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	FVector2D ScreenSpacePosition = FVector2D::ZeroVector;

	// 当前菜单持有的动作项 ViewModel 列表。
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVVM_InventoryItemContextAction>> ActionViewModels;
};
