// Fill out your copyright notice in the Description page of Project Settings.

#include "AOContainerSlot.h"

#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOContainerSlot)

void UAOContainerSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAOContainerSlot::InitializeSlot()
{
	ItemInstance = nullptr;
	InInventorySlot = FAOInventorySlot();

	if (Icon)
	{
		Icon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ItemCount)
	{
		ItemCount->SetVisibility(ESlateVisibility::Hidden);
		ItemCount->SetText(FText::GetEmpty());
	}

	Index = ObservedSlot.SlotIndex;
	ItemInstance = ObservedSlot.Instance;
	InInventorySlot.Instance = ObservedSlot.Instance;
	InInventorySlot.StackCount = ObservedSlot.StackCount;
	InInventorySlot.SlotOwnerComponent = SourceContainer;

	if (SourceContainer)
	{
		const TArray<FAOInventoryEntry> SourceEntries = SourceContainer->GetInventoryContainer();
		if (SourceEntries.IsValidIndex(Index))
		{
			const FAOInventoryEntry& SourceEntry = SourceEntries[Index];
			InInventorySlot = SourceEntry;
			if (SourceEntry.Instance)
			{
				ItemInstance = SourceEntry.Instance;
			}
			else
			{
				InInventorySlot.Instance = ItemInstance;
				InInventorySlot.StackCount = ObservedSlot.StackCount;
				InInventorySlot.SlotOwnerComponent = SourceContainer;
			}
		}
	}

	if (!ObservedSlot.ItemDefClass)
	{
		return;
	}

	const UAOInventoryItemDefinition* ItemDefCDO = GetDefault<UAOInventoryItemDefinition>(ObservedSlot.ItemDefClass);
	if (!ItemDefCDO)
	{
		return;
	}

	if (Icon)
	{
		if (const UAOFragment_InventoryIcon* InventoryIcon = ItemDefCDO->FindFragmentByClass<UAOFragment_InventoryIcon>())
		{
			Icon->SetVisibility(ESlateVisibility::Visible);
			Icon->SetBrush(InventoryIcon->Brush);
		}
	}

	if (ItemCount && ObservedSlot.StackCount > 0)
	{
		ItemCount->SetVisibility(ESlateVisibility::Visible);
		ItemCount->SetText(FText::AsNumber(ObservedSlot.StackCount));
	}
}
