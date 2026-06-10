#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"

#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_InventoryItemContextMenu)

UMVVM_InventoryItemContextMenu::UMVVM_InventoryItemContextMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_InventoryItemContextMenu::SetRequestingInventoryUI(UAOInventoryUI* InRequestingInventoryUI)
{
	RequestingInventoryUI = InRequestingInventoryUI;
}

void UMVVM_InventoryItemContextMenu::SetRequestingCraftingViewModel(UMVVM_Crafting* InCraftingViewModel)
{
	RequestingCraftingViewModel = InCraftingViewModel;
}

void UMVVM_InventoryItemContextMenu::OpenForInventorySlot(
	UAOInventoryComponent* InSourceInventory,
	int32 InSourceSlotIndex,
	UAOInventoryItemInstance* InItemInstance,
	const FVector2D& InScreenSpacePosition,
	const TArray<FAOInventoryItemContextAction>& InResolvedActions)
{
	SourceInventory = InSourceInventory;
	SourceSlotIndex = InSourceSlotIndex;
	ItemInstance = InItemInstance;
	CraftingRecipeRowName = NAME_None;

	SetScreenSpacePosition(InScreenSpacePosition);
	RefreshHeaderFromCurrentContext();
	RebuildActionViewModels(InResolvedActions);
	SetMenuVisible(true);
}

void UMVVM_InventoryItemContextMenu::OpenForCraftingRecipe(
	FName InRecipeRowName,
	const FText& InDisplayName,
	const FSlateBrush& InIconBrush,
	bool bInHasValidIcon,
	const FText& InInfoText,
	const FVector2D& InScreenSpacePosition,
	const TArray<FAOInventoryItemContextAction>& InResolvedActions)
{
	SourceInventory = nullptr;
	SourceSlotIndex = INDEX_NONE;
	ItemInstance = nullptr;
	CraftingRecipeRowName = InRecipeRowName;

	SetScreenSpacePosition(InScreenSpacePosition);
	SetItemDisplayName(InDisplayName);
	SetItemInfoText(InInfoText);
	SetItemIconBrush(InIconBrush);
	SetHasValidItemIcon(bInHasValidIcon);
	RebuildActionViewModels(InResolvedActions);
	SetMenuVisible(true);
}

void UMVVM_InventoryItemContextMenu::SetItemDisplayName(const FText& InItemDisplayName)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ItemDisplayName, InItemDisplayName))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	}
}

void UMVVM_InventoryItemContextMenu::SetItemInfoText(const FText& InItemInfoText)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ItemInfoText, InItemInfoText))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemInfoText);
	}
}

void UMVVM_InventoryItemContextMenu::SetItemIconBrush(const FSlateBrush& InItemIconBrush)
{
	ItemIconBrush = InItemIconBrush;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIconBrush);
}

void UMVVM_InventoryItemContextMenu::SetHasValidItemIcon(bool bInHasValidItemIcon)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bHasValidItemIcon, bInHasValidItemIcon))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasValidItemIcon);
	}
}

void UMVVM_InventoryItemContextMenu::CloseMenu()
{
	SetMenuVisible(false);
}

TArray<UMVVM_InventoryItemContextAction*> UMVVM_InventoryItemContextMenu::GetActionViewModels() const
{
	TArray<UMVVM_InventoryItemContextAction*> Result;
	Result.Reserve(ActionViewModels.Num());

	for (UMVVM_InventoryItemContextAction* ActionViewModel : ActionViewModels)
	{
		Result.Add(ActionViewModel);
	}

	return Result;
}

bool UMVVM_InventoryItemContextMenu::ExecuteResolvedAction(const FAOInventoryItemContextAction& InResolvedAction)
{
	bool bExecuted = false;

	if (UAOInventoryUI* InventoryUI = RequestingInventoryUI.Get())
	{
		bExecuted = InventoryUI->ExecuteInventoryItemContextAction(
			InResolvedAction,
			SourceInventory.Get(),
			SourceSlotIndex,
			ItemInstance.Get(),
			CraftingRecipeRowName);
	}
	else if (CraftingRecipeRowName.IsValid())
	{
		if (UMVVM_Crafting* CraftingViewModel = RequestingCraftingViewModel.Get())
		{
			switch (InResolvedAction.ActionType)
			{
			case EAOInventoryItemActionType::CraftOne:
				bExecuted = CraftingViewModel->RequestCraftRecipe(CraftingRecipeRowName, EAOCraftingRequestType::Single);
				break;
			case EAOInventoryItemActionType::CraftTen:
				bExecuted = CraftingViewModel->RequestCraftRecipe(CraftingRecipeRowName, EAOCraftingRequestType::Ten);
				break;
			case EAOInventoryItemActionType::CraftAll:
				bExecuted = CraftingViewModel->RequestCraftRecipe(CraftingRecipeRowName, EAOCraftingRequestType::All);
				break;
			case EAOInventoryItemActionType::Close:
				bExecuted = true;
				break;
			default:
				break;
			}
		}
	}

	if (bExecuted && InResolvedAction.bCloseMenuAfterExecute)
	{
		CloseMenu();
	}

	return bExecuted;
}

void UMVVM_InventoryItemContextMenu::BroadcastCurrentSnapshot()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDisplayName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemInfoText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemIconBrush);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasValidItemIcon);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsMenuVisible);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetScreenSpacePosition);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetActionViewModels);

	for (UMVVM_InventoryItemContextAction* ActionViewModel : ActionViewModels)
	{
		if (ActionViewModel != nullptr)
		{
			ActionViewModel->BroadcastCurrentSnapshot();
		}
	}
}

void UMVVM_InventoryItemContextMenu::ResetDisplay()
{
	SetItemDisplayName(FText::GetEmpty());
	SetItemInfoText(FText::GetEmpty());
	SetItemIconBrush(FSlateBrush());
	SetHasValidItemIcon(false);
	RebuildActionViewModels(TArray<FAOInventoryItemContextAction>());
	SetMenuVisible(false);
	SetScreenSpacePosition(FVector2D::ZeroVector);
	SourceInventory = nullptr;
	ItemInstance = nullptr;
	SourceSlotIndex = INDEX_NONE;
	CraftingRecipeRowName = NAME_None;
	RequestingCraftingViewModel = nullptr;
}

void UMVVM_InventoryItemContextMenu::SetMenuVisible(bool bInMenuVisible)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bMenuVisible, bInMenuVisible))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsMenuVisible);
	}
}

void UMVVM_InventoryItemContextMenu::SetScreenSpacePosition(const FVector2D& InScreenSpacePosition)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ScreenSpacePosition, InScreenSpacePosition))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetScreenSpacePosition);
	}
}

void UMVVM_InventoryItemContextMenu::RebuildActionViewModels(const TArray<FAOInventoryItemContextAction>& InResolvedActions)
{
	while (ActionViewModels.Num() < InResolvedActions.Num())
	{
		UMVVM_InventoryItemContextAction* NewActionViewModel = NewObject<UMVVM_InventoryItemContextAction>(this);
		if (NewActionViewModel == nullptr)
		{
			break;
		}

		NewActionViewModel->SetOwningContextMenuViewModel(this);
		ActionViewModels.Add(NewActionViewModel);
	}

	for (int32 ActionIndex = 0; ActionIndex < ActionViewModels.Num(); ++ActionIndex)
	{
		UMVVM_InventoryItemContextAction* ActionViewModel = ActionViewModels[ActionIndex];
		if (ActionViewModel == nullptr)
		{
			continue;
		}

		if (InResolvedActions.IsValidIndex(ActionIndex))
		{
			ActionViewModel->SetResolvedAction(InResolvedActions[ActionIndex]);
		}
		else
		{
			ActionViewModel->SetResolvedAction(FAOInventoryItemContextAction());
		}
	}

	ActionViewModels.SetNum(InResolvedActions.Num(), EAllowShrinking::No);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetActionViewModels);
}

FText UMVVM_InventoryItemContextMenu::BuildItemInfoText() const
{
	const UAOInventoryComponent* ResolvedInventory = SourceInventory.Get();
	const UAOInventoryItemInstance* ResolvedItemInstance = ItemInstance.Get();
	if (ResolvedInventory == nullptr || ResolvedItemInstance == nullptr || !ResolvedInventory->IsValidInventorySlotIndex(SourceSlotIndex))
	{
		return FText::GetEmpty();
	}

	const TArray<FAOInventoryEntry> SourceEntries = ResolvedInventory->GetInventoryContainer();
	if (!SourceEntries.IsValidIndex(SourceSlotIndex))
	{
		return FText::GetEmpty();
	}

	const FAOInventoryEntry& Entry = SourceEntries[SourceSlotIndex];
	const bool bCanUseItem = Entry.Instance == ResolvedItemInstance
		&& Entry.StackCount > 0
		&& Entry.Instance->GetItemCDO() != nullptr
		&& RequestingInventoryUI.IsValid()
		&& Entry.Instance->CanUseFromInventory(Entry, RequestingInventoryUI->GetOwningPlayerPawn());

	return FText::Format(
		FText::FromString(TEXT("堆叠 {0}  |  {1}")),
		FText::AsNumber(Entry.StackCount),
		bCanUseItem ? FText::FromString(TEXT("可使用")) : FText::FromString(TEXT("不可使用")));
}

void UMVVM_InventoryItemContextMenu::RefreshHeaderFromCurrentContext()
{
	const UAOInventoryItemInstance* ResolvedItemInstance = ItemInstance.Get();
	if (ResolvedItemInstance == nullptr)
	{
		SetItemDisplayName(FText::GetEmpty());
		SetItemInfoText(FText::GetEmpty());
		SetItemIconBrush(FSlateBrush());
		SetHasValidItemIcon(false);
		return;
	}

	FText ResolvedName = FText::FromString(TEXT("Empty Slot"));
	if (const UAOInventoryItemDefinition* ItemDefinition = ResolvedItemInstance->GetItemCDO())
	{
		ResolvedName = FText::FromName(ItemDefinition->DisplayName);
	}

	FSlateBrush ResolvedIconBrush;
	bool bHasValidIcon = false;
	if (const UAOInventoryItemDefinition* ItemDefinition = ResolvedItemInstance->GetItemCDO())
	{
		if (const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
		{
			ResolvedIconBrush = IconFragment->Brush;
			bHasValidIcon = IconFragment->Brush.GetResourceObject() != nullptr;
		}
	}

	SetItemDisplayName(ResolvedName);
	SetItemInfoText(BuildItemInfoText());
	SetItemIconBrush(ResolvedIconBrush);
	SetHasValidItemIcon(bHasValidIcon);
}
