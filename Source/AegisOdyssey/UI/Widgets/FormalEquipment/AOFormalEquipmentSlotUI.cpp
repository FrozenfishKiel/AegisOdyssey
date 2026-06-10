#include "AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.h"

#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentSlotUI)

void UAOFormalEquipmentSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshFormalEquipmentSlotDisplay();
}

bool UAOFormalEquipmentSlotUI::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	(void)InGeometry;
	(void)InDragDropEvent;

	UAOInventoryComponent* DraggedSourceInventory = nullptr;
	UAOInventoryItemInstance* DraggedItemInstance = nullptr;
	int32 DraggedSourceSlotIndex = INDEX_NONE;
	return ResolveDraggedSourceSlotFromOperation(
		InOperation,
		DraggedSourceInventory,
		DraggedSourceSlotIndex,
		DraggedItemInstance)
		&& CanAcceptDraggedSourceSlotForThisFormalSlot(DraggedSourceInventory, DraggedSourceSlotIndex);
}

bool UAOFormalEquipmentSlotUI::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	(void)InGeometry;
	(void)InDragDropEvent;

	UAOInventoryComponent* DraggedSourceInventory = nullptr;
	UAOInventoryItemInstance* DraggedItemInstance = nullptr;
	int32 DraggedSourceSlotIndex = INDEX_NONE;
	if (!ResolveDraggedSourceSlotFromOperation(
		InOperation,
		DraggedSourceInventory,
		DraggedSourceSlotIndex,
		DraggedItemInstance))
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	return RequestEquipDraggedSourceSlotToThisFormalSlot(DraggedSourceInventory, DraggedSourceSlotIndex);
}

void UAOFormalEquipmentSlotUI::SetObservedSlotData(
	int32 InObservedSlotIndex,
	EAOFormalEquipmentSlotType InFormalSlotType,
	UAOInventoryComponent* InSourceContainer)
{
	Index = InObservedSlotIndex;
	ObservedSlotIndex = InObservedSlotIndex;
	FormalSlotType = InFormalSlotType;
	FormalSlotDisplayName = BuildFormalSlotLabel();
	SourceContainer = InSourceContainer;
	RefreshFormalEquipmentSlotDisplay();
}

void UAOFormalEquipmentSlotUI::SetObservedSlotEntry(const FAOInventoryEntry& InObservedSlotEntry)
{
	ObservedSlotEntry = InObservedSlotEntry;
	RefreshFormalEquipmentSlotDisplay();
}

void UAOFormalEquipmentSlotUI::SetOwningFormalEquipmentManager(UAOFormalEquipmentManagerComponent* InFormalEquipmentManager)
{
	FormalEquipmentManager = InFormalEquipmentManager;
}

void UAOFormalEquipmentSlotUI::SetOwningFormalEquipmentSlotInventory(
	UAOFormalEquipmentSlotInventoryComponent* InFormalEquipmentSlotInventory)
{
	FormalEquipmentSlotInventory = InFormalEquipmentSlotInventory;
}

void UAOFormalEquipmentSlotUI::RefreshFormalEquipmentSlotDisplay()
{
	FAOInventoryEntry CurrentEntry;
	const bool bHasValidItem = ResolveCurrentSlotEntry(CurrentEntry) && CurrentEntry.Instance != nullptr;

	if (SlotNameText != nullptr)
	{
		SlotNameText->SetText(FormalSlotDisplayName.IsEmpty() ? BuildFormalSlotLabel() : FormalSlotDisplayName);
	}

	if (ItemNameText != nullptr)
	{
		if (bHasValidItem && CurrentEntry.Instance->GetItemCDO() != nullptr)
		{
			ItemNameText->SetText(FText::FromName(CurrentEntry.Instance->GetItemCDO()->DisplayName));
		}
		else
		{
			ItemNameText->SetText(FText::GetEmpty());
		}
	}

	if (ItemIcon != nullptr)
	{
		if (bHasValidItem)
		{
			if (const UAOInventoryItemDefinition* ItemDefinition = CurrentEntry.Instance->GetItemCDO())
			{
				if (const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
				{
					ItemIcon->SetBrush(IconFragment->Brush);
				}
				else
				{
					ItemIcon->SetBrushFromTexture(nullptr, false);
				}
			}
		}
		else
		{
			ItemIcon->SetBrushFromTexture(nullptr, false);
		}

		ItemIcon->SetVisibility(bHasValidItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	HandleFormalEquipmentSlotUpdated(bHasValidItem);
}

UAOFormalEquipmentManagerComponent* UAOFormalEquipmentSlotUI::GetOwningFormalEquipmentManager() const
{
	return FormalEquipmentManager.Get();
}

UAOFormalEquipmentSlotInventoryComponent* UAOFormalEquipmentSlotUI::GetOwningFormalEquipmentSlotInventory() const
{
	return FormalEquipmentSlotInventory.Get();
}

bool UAOFormalEquipmentSlotUI::CanAcceptDraggedSourceSlotForThisFormalSlot(
	UAOInventoryComponent* InSourceContainer,
	int32 InSourceSlotIndex) const
{
	if (InSourceContainer == nullptr)
	{
		return false;
	}

	const TArray<FAOInventoryEntry> SourceEntries = InSourceContainer->GetInventoryContainer();
	if (!SourceEntries.IsValidIndex(InSourceSlotIndex))
	{
		return false;
	}

	return CanAcceptDraggedItemForThisFormalSlot(SourceEntries[InSourceSlotIndex].Instance);
}

bool UAOFormalEquipmentSlotUI::CanAcceptDraggedItemForThisFormalSlot(const UAOInventoryItemInstance* SourceItemInstance) const
{
	if (SourceItemInstance == nullptr)
	{
		return false;
	}

	if (const UAOFormalEquipmentManagerComponent* ResolvedFormalEquipmentManager = GetOwningFormalEquipmentManager())
	{
		return ResolvedFormalEquipmentManager->CanAcceptInventoryItemForFormalSlot(SourceItemInstance, ObservedSlotIndex);
	}

	return false;
}

bool UAOFormalEquipmentSlotUI::RequestEquipDraggedSourceSlotToThisFormalSlot(
	UAOInventoryComponent* InSourceContainer,
	int32 InSourceSlotIndex)
{
	UAOInventoryComponent* DraggedInventory = InSourceContainer;
	UAOInventoryComponent* DropInventory = SourceContainer;
	if (DraggedInventory == nullptr || DropInventory == nullptr)
	{
		return false;
	}

	if (!DraggedInventory->IsValidInventorySlotIndex(InSourceSlotIndex))
	{
		return false;
	}

	const int32 DropSlotIndex = ObservedSlotIndex;
	if (!DropInventory->IsValidInventorySlotIndex(DropSlotIndex))
	{
		return false;
	}

	if (!CanAcceptDraggedSourceSlotForThisFormalSlot(DraggedInventory, InSourceSlotIndex))
	{
		return false;
	}

	RequestExchangeBetweenInventories(DraggedInventory, InSourceSlotIndex, DropInventory, DropSlotIndex);
	return true;
}

TArray<FAOInventoryItemContextAction> UAOFormalEquipmentSlotUI::BuildInventoryItemContextActions(
	UAOInventoryComponent* SourceInventory,
	int32 SourceSlotIndex,
	UAOInventoryItemInstance* ItemInstance) const
{
	TArray<FAOInventoryItemContextAction> Actions;

	FAOInventoryEntry CurrentEntry;
	if (!ResolveCurrentSlotEntry(CurrentEntry) || CurrentEntry.Instance == nullptr || ItemInstance == nullptr)
	{
		return Actions;
	}

	if (CurrentEntry.Instance == ItemInstance)
	{
		FAOInventoryItemContextAction& UnequipAction = Actions.AddDefaulted_GetRef();
		UnequipAction.ActionType = EAOInventoryItemActionType::Unequip;
		UnequipAction.Label = FText::FromString(TEXT("卸下"));
		UnequipAction.bEnabled = true;
		UnequipAction.bCloseMenuAfterExecute = true;
		UnequipAction.SortOrder = 0;

		FAOInventoryItemContextAction& CloseAction = Actions.AddDefaulted_GetRef();
		CloseAction.ActionType = EAOInventoryItemActionType::Close;
		CloseAction.Label = FText::FromString(TEXT("关闭"));
		CloseAction.bEnabled = true;
		CloseAction.bCloseMenuAfterExecute = true;
		CloseAction.SortOrder = 1000;
	}

	Actions.Sort([](const FAOInventoryItemContextAction& Left, const FAOInventoryItemContextAction& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});

	return Actions;
}

bool UAOFormalEquipmentSlotUI::ExecuteInventoryItemContextAction(
	const FAOInventoryItemContextAction& Action,
	UAOInventoryComponent* SourceInventory,
	int32 SourceSlotIndex,
	UAOInventoryItemInstance* ItemInstance,
	FName CraftingRecipeRowName)
{
	(void)SourceInventory;
	(void)SourceSlotIndex;
	(void)ItemInstance;
	(void)CraftingRecipeRowName;

	switch (Action.ActionType)
	{
	case EAOInventoryItemActionType::Unequip:
		if (UAOFormalEquipmentManagerComponent* ResolvedFormalEquipmentManager = GetOwningFormalEquipmentManager())
		{
			return ResolvedFormalEquipmentManager->RequestUnequipFormalSlot(ObservedSlotIndex);
		}
		return false;
	case EAOInventoryItemActionType::Close:
		return true;
	default:
		return UAOInventoryUI::ExecuteInventoryItemContextAction(
			Action,
			SourceInventory,
			SourceSlotIndex,
			ItemInstance,
			CraftingRecipeRowName);
	}
}

bool UAOFormalEquipmentSlotUI::ResolveInventoryItemContextMenuRequest(
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	OutSourceInventory = nullptr;
	OutSourceSlotIndex = INDEX_NONE;
	OutItemInstance = nullptr;

	FAOInventoryEntry CurrentEntry;
	if (!ResolveCurrentSlotEntry(CurrentEntry) || CurrentEntry.Instance == nullptr || SourceContainer == nullptr)
	{
		return false;
	}

	OutSourceInventory = SourceContainer;
	OutSourceSlotIndex = ObservedSlotIndex;
	OutItemInstance = CurrentEntry.Instance;
	return true;
}

const UAOInventoryItemDefinition* UAOFormalEquipmentSlotUI::ResolveHoverTooltipItemDefinition() const
{
	FAOInventoryEntry CurrentEntry;
	if (!ResolveCurrentSlotEntry(CurrentEntry) || CurrentEntry.Instance == nullptr)
	{
		return nullptr;
	}

	return CurrentEntry.Instance->GetItemCDO();
}

void UAOFormalEquipmentSlotUI::HandleFormalEquipmentSlotUpdated(bool bHasValidItem)
{
	(void)bHasValidItem;
}

bool UAOFormalEquipmentSlotUI::ResolveCurrentSlotEntry(FAOInventoryEntry& OutEntry) const
{
	OutEntry = ObservedSlotEntry;
	return ObservedSlotEntry.SlotOwnerComponent != nullptr || ObservedSlotEntry.Instance != nullptr || SourceContainer != nullptr;
}

bool UAOFormalEquipmentSlotUI::ResolveDraggedSourceSlotFromOperation(
	const UDragDropOperation* InOperation,
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	OutSourceInventory = nullptr;
	OutSourceSlotIndex = INDEX_NONE;
	OutItemInstance = nullptr;

	if (InOperation == nullptr || InOperation->Payload == nullptr)
	{
		return false;
	}

	const UAOInventoryUI* DraggedInventoryUI = Cast<UAOInventoryUI>(InOperation->Payload);
	if (DraggedInventoryUI == nullptr)
	{
		return false;
	}

	UAOInventoryComponent* ResolvedSourceInventory = nullptr;
	UAOInventoryItemInstance* ResolvedItemInstance = nullptr;
	int32 ResolvedSourceSlotIndex = INDEX_NONE;
	if (!DraggedInventoryUI->TryResolveInventorySlotContext(
		ResolvedSourceInventory,
		ResolvedSourceSlotIndex,
		ResolvedItemInstance))
	{
		return false;
	}

	OutSourceInventory = ResolvedSourceInventory;
	OutSourceSlotIndex = ResolvedSourceSlotIndex;
	OutItemInstance = ResolvedItemInstance;
	return ResolvedSourceInventory != nullptr && ResolvedSourceSlotIndex != INDEX_NONE;
}

FText UAOFormalEquipmentSlotUI::BuildFormalSlotLabel() const
{
	switch (FormalSlotType)
	{
	case EAOFormalEquipmentSlotType::Helmet:
		return FText::FromString(TEXT("头盔"));
	case EAOFormalEquipmentSlotType::Armor:
		return FText::FromString(TEXT("护甲"));
	case EAOFormalEquipmentSlotType::Gloves:
		return FText::FromString(TEXT("手套"));
	case EAOFormalEquipmentSlotType::Necklace:
		return FText::FromString(TEXT("项链"));
	case EAOFormalEquipmentSlotType::Boots:
		return FText::FromString(TEXT("靴子"));
	default:
		return FText::FromString(TEXT("正式装备"));
	}
}
