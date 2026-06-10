#include "AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.h"

#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.h"
#include "Components/PanelWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentBarUI)

void UAOFormalEquipmentBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshFormalEquipmentBarDelegateHandle =
			ViewModel->OnFormalEquipmentListChangedDynamic.AddUObject(this, &ThisClass::HandleFormalEquipmentListChanged);
	}

	RefreshFormalEquipmentBar();
}

void UAOFormalEquipmentBarUI::NativeDestruct()
{
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshFormalEquipmentBarDelegateHandle.IsValid())
		{
			ViewModel->OnFormalEquipmentListChangedDynamic.Remove(RefreshFormalEquipmentBarDelegateHandle);
		}
	}

	RefreshFormalEquipmentBarDelegateHandle.Reset();
	Super::NativeDestruct();
}

void UAOFormalEquipmentBarUI::SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext)
{
	// Formal equipment is also page-injected.
	// The widget does not inspect session target actors on its own.
	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		if (RefreshFormalEquipmentBarDelegateHandle.IsValid())
		{
			ViewModel->OnFormalEquipmentListChangedDynamic.Remove(RefreshFormalEquipmentBarDelegateHandle);
		}
	}

	RefreshFormalEquipmentBarDelegateHandle.Reset();
	DisplayContext = InDisplayContext;

	if (UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel())
	{
		RefreshFormalEquipmentBarDelegateHandle =
			ViewModel->OnFormalEquipmentListChangedDynamic.AddUObject(this, &ThisClass::HandleFormalEquipmentListChanged);
	}

	RefreshFormalEquipmentBar();
}

UMVVM_InventoryMenu* UAOFormalEquipmentBarUI::GetInventoryViewModel() const
{
	if (const UAOFormalEquipmentSlotInventoryComponent* FormalEquipmentInventory = GetFormalEquipmentSlotInventory())
	{
		return FormalEquipmentInventory->GetFormalEquipmentViewModel();
	}

	return nullptr;
}

void UAOFormalEquipmentBarUI::RefreshFormalEquipmentBar()
{
	if (FormalEquipmentSlotContainer == nullptr)
	{
		return;
	}

	check(FormalEquipmentSlotClass);

	FormalEquipmentSlotContainer->ClearChildren();

	const UAOFormalEquipmentSlotInventoryComponent* FormalEquipmentInventory = GetFormalEquipmentSlotInventory();
	const UAOFormalEquipmentManagerComponent* FormalEquipmentManager = GetFormalEquipmentManager();
	const UMVVM_InventoryMenu* ViewModel = GetInventoryViewModel();
	if (FormalEquipmentInventory == nullptr || FormalEquipmentManager == nullptr || ViewModel == nullptr)
	{
		return;
	}

	const TArray<FAOInventoryEntry> FormalEquipmentEntries = ViewModel->GetFormalEquipmentList();
	const int32 FormalSlotCount = FormalEquipmentManager->GetFormalEquipmentSlots().Num();
	for (int32 SlotIndex = 0; SlotIndex < FormalSlotCount; ++SlotIndex)
	{
		UAOFormalEquipmentSlotUI* FormalEquipmentSlotWidget =
			CreateWidget<UAOFormalEquipmentSlotUI>(GetOwningPlayer(), FormalEquipmentSlotClass);
		if (FormalEquipmentSlotWidget == nullptr)
		{
			continue;
		}

		FormalEquipmentSlotWidget->SetObservedSlotData(
			SlotIndex,
			FormalEquipmentManager->GetFormalSlotTypeByIndex(SlotIndex),
			const_cast<UAOFormalEquipmentSlotInventoryComponent*>(FormalEquipmentInventory));
		FormalEquipmentSlotWidget->SetOwningFormalEquipmentManager(
			const_cast<UAOFormalEquipmentManagerComponent*>(FormalEquipmentManager));
		FormalEquipmentSlotWidget->SetOwningFormalEquipmentSlotInventory(
			const_cast<UAOFormalEquipmentSlotInventoryComponent*>(FormalEquipmentInventory));

		if (FormalEquipmentEntries.IsValidIndex(SlotIndex))
		{
			FormalEquipmentSlotWidget->SetObservedSlotEntry(FormalEquipmentEntries[SlotIndex]);
		}
		else
		{
			FormalEquipmentSlotWidget->SetObservedSlotEntry(
				FAOInventoryEntry(const_cast<UAOFormalEquipmentSlotInventoryComponent*>(FormalEquipmentInventory)));
		}

		FormalEquipmentSlotContainer->AddChild(FormalEquipmentSlotWidget);
	}

	HandleFormalEquipmentBarRebuilt();
}

void UAOFormalEquipmentBarUI::HandleFormalEquipmentBarRebuilt()
{
}

UAOFormalEquipmentSlotInventoryComponent* UAOFormalEquipmentBarUI::GetFormalEquipmentSlotInventory() const
{
	return ResolveFormalEquipmentInventoryFromDisplayContext(DisplayContext);
}

UAOFormalEquipmentManagerComponent* UAOFormalEquipmentBarUI::GetFormalEquipmentManager() const
{
	return ResolveFormalEquipmentManagerFromDisplayContext(DisplayContext);
}

void UAOFormalEquipmentBarUI::HandleFormalEquipmentListChanged()
{
	RefreshFormalEquipmentBar();
}
