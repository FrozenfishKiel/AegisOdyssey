// Fill out your copyright notice in the Description page of Project Settings.


#include "AOBackPackSlot.h"

#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackSlot)

void UAOBackPackSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAOBackPackSlot::InitializeSlot()
{
	SourceContainer = InInventorySlot.SlotOwnerComponent;
	if (InInventorySlot.Instance != nullptr)
	{
		UAOInventoryItemInstance* Instance = InInventorySlot.Instance;
		ItemInstance = Instance;
		UAOInventoryItemDefinition* ItemCDO = Instance->GetItemCDO();
		if (ItemCDO)
		{
			if (const UAOFragment_InventoryIcon* InventoryIcon = ItemCDO->FindFragmentByClass<UAOFragment_InventoryIcon>())
			{
				Icon->SetVisibility(ESlateVisibility::Visible);
				Icon->SetBrush(InventoryIcon->Brush);
			}
			int32 StackCount = InInventorySlot.StackCount;
			if (StackCount > 0)
			{
				ItemCount->SetVisibility(ESlateVisibility::Visible);
				ItemCount->SetText(FText::AsNumber(StackCount));
			}
		}
	}
}
