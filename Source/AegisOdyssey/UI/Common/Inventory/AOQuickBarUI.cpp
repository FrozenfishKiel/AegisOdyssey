// Fill out your copyright notice in the Description page of Project Settings.


#include "AOQuickBarUI.h"

#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_PickUpIcon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarUI)

void UAOQuickBarUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBox();
		RefreshInventoryBoxDelegateHandle = ViewModel->OnQuickBarListChangedDynamic.AddUObject(this,&ThisClass::RefreshInventoryBox);
	}
}
void UAOQuickBarUI::NativeDestruct()
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

void UAOQuickBarUI::RefreshInventoryBox()
{
	check(QuickBarSlotClass);
	QuickBarBox->ClearChildren();
	
	if (UAOQuickBarComponent* QuickBarComponent = FindTargetComponent<UAOQuickBarComponent>())
	{
		int32 TempIndex = 1;
		TArray<FAOInventoryEntry> Entries = QuickBarComponent->GetInventoryContainer();
		for (int32 i = 0 ; i < Entries.Num(); i++)
		{
			const int32 InIndex = i;
			const FAOInventoryEntry Entry = Entries[i];
			check(Entry.SlotOwnerComponent);
		
			UAOQuickBarSlot* QuickBarSlot = CreateWidget<UAOQuickBarSlot>(GetOwningPlayer() , QuickBarSlotClass);
		
			if (TempIndex >= Entries.Num()){QuickBarSlot->InputIndex->SetText(FText::AsNumber(0));}
			else {QuickBarSlot->InputIndex->SetText(FText::AsNumber(TempIndex));}

			TempIndex++;
			QuickBarSlot->Index = InIndex;
			QuickBarSlot->InQuickBarSlot = Entry;
			QuickBarSlot->InitializeSlot();
			QuickBarBox->AddChild(QuickBarSlot);  //添加到WrapBox中
		}
	}
}

UMVVM_InventoryMenu* UAOQuickBarUI::GetInventoryViewModel() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			if (APawn* ControlledPawn = SourcePC->GetPawn())
			{
				if (UAOQuickBarComponent* ViewModelPawn = ControlledPawn->FindComponentByClass<UAOQuickBarComponent>())
				{
					return ViewModelPawn->GetQuickBarViewModel();
				}
			}
		}
	}
	return nullptr;
}
