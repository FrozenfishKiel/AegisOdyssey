// Fill out your copyright notice in the Description page of Project Settings.

#include "AOQuickBarUI.h"

#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarUI)

void UAOQuickBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBoxDelegateHandle =
			ViewModel->OnQuickBarListChangedDynamic.AddUObject(this, &ThisClass::RefreshInventoryBox);
	}

	RefreshInventoryBox();
}

void UAOQuickBarUI::NativeDestruct()
{
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshInventoryBoxDelegateHandle.IsValid())
		{
			ViewModel->OnQuickBarListChangedDynamic.Remove(RefreshInventoryBoxDelegateHandle);
		}
	}

	RefreshInventoryBoxDelegateHandle.Reset();
	Super::NativeDestruct();
}

void UAOQuickBarUI::SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext)
{
	// QuickBar follows the same rule as backpack:
	// the outer inventory page decides the display target, this panel only rebinds and redraws.
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshInventoryBoxDelegateHandle.IsValid())
		{
			ViewModel->OnQuickBarListChangedDynamic.Remove(RefreshInventoryBoxDelegateHandle);
		}
	}

	RefreshInventoryBoxDelegateHandle.Reset();
	DisplayContext = InDisplayContext;

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshInventoryBoxDelegateHandle =
			ViewModel->OnQuickBarListChangedDynamic.AddUObject(this, &ThisClass::RefreshInventoryBox);
	}

	RefreshInventoryBox();
}

void UAOQuickBarUI::RefreshInventoryBox()
{
	check(QuickBarSlotClass);

	if (QuickBarBox == nullptr)
	{
		return;
	}

	QuickBarBox->ClearChildren();

	if (UAOQuickBarComponent* QuickBarComponent = GetObservedQuickBarComponent())
	{
		int32 TempIndex = 1;
		const TArray<FAOInventoryEntry> Entries = QuickBarComponent->GetInventoryContainer();

		for (int32 i = 0; i < Entries.Num(); i++)
		{
			const FAOInventoryEntry& Entry = Entries[i];
			check(Entry.SlotOwnerComponent);

			UAOQuickBarSlot* QuickBarSlot = CreateWidget<UAOQuickBarSlot>(GetOwningPlayer(), QuickBarSlotClass);
			if (QuickBarSlot == nullptr)
			{
				continue;
			}

			if (TempIndex >= Entries.Num())
			{
				QuickBarSlot->InputIndex->SetText(FText::AsNumber(0));
			}
			else
			{
				QuickBarSlot->InputIndex->SetText(FText::AsNumber(TempIndex));
			}

			TempIndex++;
			QuickBarSlot->SetSlotContext(i, Entry.SlotOwnerComponent, Entry.Instance);
			QuickBarSlot->InQuickBarSlot = Entry;
			QuickBarSlot->InitializeSlot();
			QuickBarBox->AddChild(QuickBarSlot);
		}
	}
}

UMVVM_InventoryMenu* UAOQuickBarUI::GetInventoryViewModel() const
{
	if (const UAOQuickBarComponent* QuickBarComponent = GetObservedQuickBarComponent())
	{
		return QuickBarComponent->GetQuickBarViewModel();
	}

	return nullptr;
}

UAOQuickBarComponent* UAOQuickBarUI::GetObservedQuickBarComponent() const
{
	return ResolveQuickBarComponentFromDisplayContext(DisplayContext);
}
