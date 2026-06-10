// Fill out your copyright notice in the Description page of Project Settings.

#include "AOBackPackUI.h"

#include "AOBackPackSlot.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackUI)

void UAOBackPackUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBoxDelegateHandle =
			ViewModel->OnInventoryListChangedDynamic.AddUObject(this, &ThisClass::RefreshInventoryBox);
	}

	RefreshInventoryBox();
}

void UAOBackPackUI::NativeDestruct()
{
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshInventoryBoxDelegateHandle.IsValid())
		{
			ViewModel->OnInventoryListChangedDynamic.Remove(RefreshInventoryBoxDelegateHandle);
		}
	}

	RefreshInventoryBoxDelegateHandle.Reset();
	Super::NativeDestruct();
}

void UAOBackPackUI::SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext)
{
	// This widget never decides "self or target" by itself.
	// The page injects the current display context, then this panel rebinds to that inventory view model.
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshInventoryBoxDelegateHandle.IsValid())
		{
			ViewModel->OnInventoryListChangedDynamic.Remove(RefreshInventoryBoxDelegateHandle);
		}
	}

	RefreshInventoryBoxDelegateHandle.Reset();
	DisplayContext = InDisplayContext;

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBoxDelegateHandle =
			ViewModel->OnInventoryListChangedDynamic.AddUObject(this, &ThisClass::RefreshInventoryBox);
	}

	RefreshInventoryBox();
}

void UAOBackPackUI::RefreshInventoryBox()
{
	check(BackPackSlotClass);

	if (DefaultInventoryBox == nullptr)
	{
		return;
	}

	DefaultInventoryBox->ClearChildren();

	if (UAOBackPackComponent* BackPackComponent = GetObservedBackPackComponent())
	{
		const TArray<FAOInventoryEntry> Entries = BackPackComponent->GetInventoryContainer();
		for (int32 i = 0; i < Entries.Num(); i++)
		{
			const FAOInventoryEntry& Entry = Entries[i];
			check(Entry.SlotOwnerComponent);

			UAOBackPackSlot* BackPackSlot = CreateWidget<UAOBackPackSlot>(GetOwningPlayer(), BackPackSlotClass);
			if (BackPackSlot == nullptr)
			{
				continue;
			}

			BackPackSlot->SetSlotContext(i, Entry.SlotOwnerComponent, Entry.Instance);
			BackPackSlot->InInventorySlot = Entry;
			BackPackSlot->InitializeSlot();
			DefaultInventoryBox->AddChild(BackPackSlot);
		}
	}
}

UMVVM_InventoryMenu* UAOBackPackUI::GetInventoryViewModel() const
{
	if (const UAOBackPackComponent* BackPackComponent = GetObservedBackPackComponent())
	{
		return BackPackComponent->GetInventoryViewModel();
	}

	return nullptr;
}

UAOBackPackComponent* UAOBackPackUI::GetObservedBackPackComponent() const
{
	return ResolveBackPackComponentFromDisplayContext(DisplayContext);
}
