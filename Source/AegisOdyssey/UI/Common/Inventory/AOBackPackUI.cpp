// Fill out your copyright notice in the Description page of Project Settings.


#include "AOBackPackUI.h"
#include "AOBackPackSlot.h"
#include "ToolMenusEditor.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "Tests/ToolMenusTestUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackUI)

void UAOBackPackUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBoxDelegateHandle = ViewModel->OnQuickBarListChangedDynamic.AddUObject(this,&ThisClass::RefreshInventoryBox);
	}
}

void UAOBackPackUI::NativeDestruct()
{
	Super::NativeDestruct();
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshInventoryBoxDelegateHandle.IsValid())
		{
			ViewModel->OnQuickBarListChangedDynamic.Remove(RefreshInventoryBoxDelegateHandle);
		}
	}
}

void UAOBackPackUI::RefreshInventoryBox()
{
	
	check(BackPackSlotClass);
	DefaultInventoryBox->ClearChildren();
	

	if (UAOBackPackComponent* QuickBarComponent = FindTargetComponent<UAOBackPackComponent>())
	{
		TArray<FAOInventoryEntry> Entries = QuickBarComponent->GetInventoryContainer();
		for (int32 i = 0 ; i < Entries.Num(); i++)
		{
			const int32 InIndex = i;
			const FAOInventoryEntry Entry = Entries[i];
			check(Entry.SlotOwnerComponent);
		
			UAOBackPackSlot* BackPackSlot = CreateWidget<UAOBackPackSlot>(GetOwningPlayer() , BackPackSlotClass);
		
			BackPackSlot->Index = InIndex;
			BackPackSlot->InInventorySlot = Entry;
			BackPackSlot->InitializeSlot();
			DefaultInventoryBox->AddChild(BackPackSlot);  //添加到WrapBox中
		}
	}

}
