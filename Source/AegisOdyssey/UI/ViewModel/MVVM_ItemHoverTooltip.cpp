#include "MVVM_ItemHoverTooltip.h"

#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_ItemHoverTooltip)

UMVVM_ItemHoverTooltip::UMVVM_ItemHoverTooltip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_ItemHoverTooltip::ShowTooltip(
	const UAOInventoryItemDefinition* ItemDefinition,
	const FVector2D& InScreenSpacePosition,
	const UObject* SourceToken)
{
	// Tooltip 的正式输入只认 Definition。
	// 这里不关心它来自背包、快捷栏、正式装备还是制造材料条目。
	if (ItemDefinition == nullptr || SourceToken == nullptr)
	{
		ForceHideTooltip();
		return;
	}

	ActiveSourceToken = SourceToken;
	SetScreenSpacePosition(InScreenSpacePosition);
	SetItemDisplayName(!ItemDefinition->DisplayName.IsNone() ? FText::FromName(ItemDefinition->DisplayName) : FText::GetEmpty());
	SetItemDescription(ItemDefinition->Description);

	FSlateBrush ResolvedIconBrush;
	bool bHasValidIcon = false;
	if (const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
	{
		ResolvedIconBrush = IconFragment->Brush;
		bHasValidIcon = ResolvedIconBrush.GetResourceObject() != nullptr;
	}

	SetItemIconBrush(ResolvedIconBrush);
	SetHasValidItemIcon(bHasValidIcon);
	SetTooltipVisible(true);
	TooltipSnapshotChangedEvent.Broadcast();
}

void UMVVM_ItemHoverTooltip::HideTooltip(const UObject* SourceToken)
{
	// 旧来源离开时不能误关掉新来源已经顶替显示的 Tooltip，
	// 所以这里只接受“当前活动来源自己发起的关闭请求”。
	if (SourceToken == nullptr || ActiveSourceToken.Get() != SourceToken)
	{
		return;
	}

	ForceHideTooltip();
}

void UMVVM_ItemHoverTooltip::ForceHideTooltip()
{
	ActiveSourceToken = nullptr;
	ResetDisplay();
	TooltipSnapshotChangedEvent.Broadcast();
}

void UMVVM_ItemHoverTooltip::BroadcastCurrentSnapshot()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDescription);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIconBrush);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasValidItemIcon);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsTooltipVisible);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetScreenSpacePosition);
	TooltipSnapshotChangedEvent.Broadcast();
}

void UMVVM_ItemHoverTooltip::SetItemDisplayName(const FText& InItemDisplayName)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ItemDisplayName, InItemDisplayName))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	}
}

void UMVVM_ItemHoverTooltip::SetItemDescription(const FText& InItemDescription)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ItemDescription, InItemDescription))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDescription);
	}
}

void UMVVM_ItemHoverTooltip::SetItemIconBrush(const FSlateBrush& InItemIconBrush)
{
	ItemIconBrush = InItemIconBrush;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIconBrush);
}

void UMVVM_ItemHoverTooltip::SetHasValidItemIcon(bool bInHasValidItemIcon)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bHasValidItemIcon, bInHasValidItemIcon))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasValidItemIcon);
	}
}

void UMVVM_ItemHoverTooltip::SetTooltipVisible(bool bInTooltipVisible)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bTooltipVisible, bInTooltipVisible))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsTooltipVisible);
	}
}

void UMVVM_ItemHoverTooltip::SetScreenSpacePosition(const FVector2D& InScreenSpacePosition)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ScreenSpacePosition, InScreenSpacePosition))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetScreenSpacePosition);
	}
}

void UMVVM_ItemHoverTooltip::ResetDisplay()
{
	// 关闭 Tooltip 时统一清空整份显示快照，
	// 避免 Widget 复用时残留上一条物品的数据。
	SetItemDisplayName(FText::GetEmpty());
	SetItemDescription(FText::GetEmpty());
	SetItemIconBrush(FSlateBrush());
	SetHasValidItemIcon(false);
	SetTooltipVisible(false);
	SetScreenSpacePosition(FVector2D::ZeroVector);
}
