// Fill out your copyright notice in the Description page of Project Settings.


#include "AOQuickBarUI.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_PickUpIcon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarUI)

void UAOQuickBarUI::NativeConstruct()
{
	Super::NativeConstruct();
}
void UAOQuickBarUI::NativeDestruct()
{
	Super::NativeDestruct();
}

void UAOQuickBarUI::RefreshInventoryBox()
{
	check(QuickBarSlotClass);
	QuickBarBox->ClearChildren();
	
	const UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel();  //从角色VM组件中获取VM
	check(ViewModel);

	int32 TempIndex = 1;
	for (int32 i = 0 ; i < ViewModel->GetQuickBarList().Num(); i++)
	{
		const int32 InIndex = i;
		const FAOInventoryEntry Entry = ViewModel->GetQuickBarList()[i];
		check(Entry.SlotOwnerComponent);
		
		UAOQuickBarSlot* QuickBarSlot = CreateWidget<UAOQuickBarSlot>(GetOwningPlayer() , QuickBarSlotClass);
		
		if (TempIndex >= ViewModel->GetQuickBarList().Num()){QuickBarSlot->InputIndex->SetText(FText::AsNumber(0));}
		else {QuickBarSlot->InputIndex->SetText(FText::AsNumber(TempIndex));}

		TempIndex++;
		QuickBarSlot->Index = InIndex;
		QuickBarSlot->InQuickBarSlot = Entry;
		QuickBarSlot->InitializeSlot();
		QuickBarBox->AddChild(QuickBarSlot);  //添加到WrapBox中
	}
}
