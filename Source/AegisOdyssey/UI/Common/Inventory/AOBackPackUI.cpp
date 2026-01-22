// Fill out your copyright notice in the Description page of Project Settings.


#include "AOBackPackUI.h"
#include "AOBackPackSlot.h"
#include "ToolMenusEditor.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "Tests/ToolMenusTestUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackUI)

void UAOBackPackUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAOBackPackUI::RefreshInventoryBox()
{
	
	check(BackPackSlotClass);
	DefaultInventoryBox->ClearChildren();
	
	const UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel();
	check(ViewModel);

	for (int32 i = 0 ; i < ViewModel->GetInventoryList().Num(); i++)
	{
		const int32 InIndex = i;
		const FAOInventoryEntry Entry = ViewModel->GetInventoryList()[i];
		check(Entry.SlotOwnerComponent);
		
		UAOBackPackSlot* BackPackSlot = CreateWidget<UAOBackPackSlot>(GetOwningPlayer() , BackPackSlotClass);
		
		BackPackSlot->Index = InIndex;
		BackPackSlot->InInventorySlot = Entry;
		BackPackSlot->InitializeSlot();
		DefaultInventoryBox->AddChild(BackPackSlot);  //添加到WrapBox中
	}
}
