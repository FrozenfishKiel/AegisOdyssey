// Fill out your copyright notice in the Description page of Project Settings.

#include "AOInventorySlotBase.h"

#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventorySlotBase)

void UAOInventorySlotBase::SetSlotIndex(int32 InIndex)
{
	Index = InIndex;
}

void UAOInventorySlotBase::SetSourceContainer(UAOInventoryComponent* InSourceContainer)
{
	SourceContainer = InSourceContainer;
}

void UAOInventorySlotBase::SetItemInstance(UAOInventoryItemInstance* InItemInstance)
{
	ItemInstance = InItemInstance;
}

void UAOInventorySlotBase::SetSlotContext(
	int32 InIndex,
	UAOInventoryComponent* InSourceContainer,
	UAOInventoryItemInstance* InItemInstance)
{
	Index = InIndex;
	SourceContainer = InSourceContainer;
	ItemInstance = InItemInstance;
}

bool UAOInventorySlotBase::ResolveInventoryItemContextMenuRequest(
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	if (ItemInstance == nullptr || SourceContainer == nullptr || Index == INDEX_NONE)
	{
		return false;
	}

	OutSourceInventory = SourceContainer;
	OutSourceSlotIndex = Index;
	OutItemInstance = ItemInstance;
	return true;
}

bool UAOInventorySlotBase::ResolveDropTargetInventorySlot(
	UAOInventoryComponent*& OutTargetInventory,
	int32& OutTargetSlotIndex) const
{
	if (SourceContainer == nullptr || Index == INDEX_NONE)
	{
		return false;
	}

	OutTargetInventory = SourceContainer;
	OutTargetSlotIndex = Index;
	return true;
}
