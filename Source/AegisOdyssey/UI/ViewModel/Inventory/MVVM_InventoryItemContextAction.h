#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/AOInventoryItemContextMenuTypes.h"
#include "MVVM_InventoryItemContextAction.generated.h"

class UMVVM_InventoryItemContextMenu;

// 单个右键菜单动作项的 ViewModel。
// 它只承载一行菜单项的显示状态与点击转发，不直接拥有业务执行权限。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_InventoryItemContextAction : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_InventoryItemContextAction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 设置所属的父菜单 ViewModel。
	// 动作项点击后会通过它回传执行请求。
	void SetOwningContextMenuViewModel(UMVVM_InventoryItemContextMenu* InOwningContextMenuViewModel);

	// 写入这一行动作的运行时决议结果。
	// 这里不重复做业务判断，只把决议结果拆成 UI 可直接绑定的字段。
	void SetResolvedAction(const FAOInventoryItemContextAction& InResolvedAction);

	// 尝试执行当前动作。
	// 如果动作被禁用，或者父菜单已经失效，则直接返回 false。
	UFUNCTION(BlueprintCallable, Category = "AO|Inventory Context Menu")
	bool ExecuteAction();

	// 当前动作项在 UI 上显示的标题文本。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	FText GetLabel() const { return Label; }

	// 当前动作项是否允许点击。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory Context Menu")
	bool IsEnabled() const { return bEnabled; }

	// 当前动作项对应的完整动作决议。
	UFUNCTION(BlueprintPure, Category = "AO|Inventory Context Menu")
	const FAOInventoryItemContextAction& GetResolvedAction() const { return ResolvedAction; }

	// 菜单复用时重新广播这条动作项当前快照。
	void BroadcastCurrentSnapshot();

private:
	// 所属父菜单的弱引用。
	TWeakObjectPtr<UMVVM_InventoryItemContextMenu> OwningContextMenuViewModel;

	// 当前显示给玩家看的动作标题。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetLabel, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	FText Label;

	// 当前动作是否允许点击。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsEnabled, Category = "AO|Inventory Context Menu",
		meta = (AllowPrivateAccess = "true"))
	bool bEnabled = true;

	// 当前动作项底层保存的完整决议数据。
	FAOInventoryItemContextAction ResolvedAction;
};
