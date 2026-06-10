// Fill out your copyright notice in the Description page of Project Settings.

#include "AOQuickBarSlot.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_PickUpIcon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarSlot)

void UAOQuickBarSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAOQuickBarSlot::InitializeSlot()
{
	SourceContainer = InQuickBarSlot.SlotOwnerComponent;
	if (InQuickBarSlot.Instance != nullptr)
	{
		UAOInventoryItemInstance* Instance = InQuickBarSlot.Instance;
		ItemInstance = Instance;
		UAOInventoryItemDefinition* ItemCDO = Instance->GetItemCDO();
		if (ItemCDO)
		{
			if (const UAOFragment_InventoryIcon* InventoryIcon = ItemCDO->FindFragmentByClass<UAOFragment_InventoryIcon>())
			{
				Icon->SetVisibility(ESlateVisibility::Visible);
				Icon->SetBrush(InventoryIcon->Brush);
			}
			int32 StackCount = InQuickBarSlot.StackCount;
			if (StackCount > 0)
			{
				ItemCount->SetVisibility(ESlateVisibility::Visible);
				ItemCount->SetText(FText::AsNumber(StackCount));
			}
		}
	}
}
