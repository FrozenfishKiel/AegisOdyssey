#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h"

#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_InventoryItemContextAction)

UMVVM_InventoryItemContextAction::UMVVM_InventoryItemContextAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_InventoryItemContextAction::SetOwningContextMenuViewModel(UMVVM_InventoryItemContextMenu* InOwningContextMenuViewModel)
{
	OwningContextMenuViewModel = InOwningContextMenuViewModel;
}

void UMVVM_InventoryItemContextAction::SetResolvedAction(const FAOInventoryItemContextAction& InResolvedAction)
{
	ResolvedAction = InResolvedAction;

	if (UE_MVVM_SET_PROPERTY_VALUE(Label, ResolvedAction.Label))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLabel);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(bEnabled, ResolvedAction.bEnabled))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsEnabled);
	}
}

bool UMVVM_InventoryItemContextAction::ExecuteAction()
{
	if (!ResolvedAction.bEnabled)
	{
		return false;
	}

	if (UMVVM_InventoryItemContextMenu* ContextMenuViewModel = OwningContextMenuViewModel.Get())
	{
		return ContextMenuViewModel->ExecuteResolvedAction(ResolvedAction);
	}

	return false;
}

void UMVVM_InventoryItemContextAction::BroadcastCurrentSnapshot()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLabel);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsEnabled);
}
