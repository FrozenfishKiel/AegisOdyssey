#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "Styling/SlateBrush.h"
#include "MVVM_ItemHoverTooltip.generated.h"

class UAOInventoryItemDefinition;

// HUD 全局唯一的物品悬浮信息框 ViewModel。
// 它只保存当前正在显示的 Tooltip 快照，不持有库存或制造系统里的长期业务真相。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_ItemHoverTooltip : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnTooltipSnapshotChanged);

	UMVVM_ItemHoverTooltip(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 用当前悬浮条目的 Definition 与屏幕位置刷新 Tooltip。
	// SourceToken 只用于本地生命周期管理，避免旧条目的 Hide 请求把新 Tooltip 误关掉。
	void ShowTooltip(const UAOInventoryItemDefinition* ItemDefinition, const FVector2D& InScreenSpacePosition, const UObject* SourceToken);

	// 只有当前活动来源自己离开时，才允许关闭 Tooltip。
	void HideTooltip(const UObject* SourceToken);

	// 强制关闭当前 Tooltip，不校验来源。
	void ForceHideTooltip();

	// 菜单 / Tooltip Widget 复用时，把当前整份快照重播给现有绑定。
	void BroadcastCurrentSnapshot();

	// 纯 C++ Widget 也需要知道“当前整份快照已经换了一次”，
	// 这样在没有额外蓝图绑定的情况下，仍然能跟着 Show / Hide 正常刷新显示。
	FOnTooltipSnapshotChanged& OnTooltipSnapshotChanged() { return TooltipSnapshotChangedEvent; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	FText GetItemDisplayName() const { return ItemDisplayName; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	FText GetItemDescription() const { return ItemDescription; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	FSlateBrush GetItemIconBrush() const { return ItemIconBrush; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	bool HasValidItemIcon() const { return bHasValidItemIcon; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	bool IsTooltipVisible() const { return bTooltipVisible; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Item Tooltip")
	FVector2D GetScreenSpacePosition() const { return ScreenSpacePosition; }

private:
	// 以下 Setter 只维护“当前 Tooltip 显示快照”，
	// 不回写库存或制造系统里的底层数据。
	void SetItemDisplayName(const FText& InItemDisplayName);
	void SetItemDescription(const FText& InItemDescription);
	void SetItemIconBrush(const FSlateBrush& InItemIconBrush);
	void SetHasValidItemIcon(bool bInHasValidItemIcon);
	void SetTooltipVisible(bool bInTooltipVisible);
	void SetScreenSpacePosition(const FVector2D& InScreenSpacePosition);
	void ResetDisplay();

private:
	// 当前活动 Tooltip 的本地来源标识。
	// 只用来判断 Hide 请求是不是来自当前真正悬浮着的那个条目。
	TWeakObjectPtr<const UObject> ActiveSourceToken;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemDisplayName, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	FText ItemDisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemDescription, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	FText ItemDescription;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetItemIconBrush, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	FSlateBrush ItemIconBrush;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = HasValidItemIcon, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	bool bHasValidItemIcon = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsTooltipVisible, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	bool bTooltipVisible = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetScreenSpacePosition, Category = "AO|Item Tooltip",
		meta = (AllowPrivateAccess = "true"))
	FVector2D ScreenSpacePosition = FVector2D::ZeroVector;

	FOnTooltipSnapshotChanged TooltipSnapshotChangedEvent;
};
