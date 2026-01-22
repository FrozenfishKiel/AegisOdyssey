// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpable.h"

#include "AegisOdyssey/Inventory/AOInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PickUpable)
// Add default functionality here for any IPickUpable functions that are not pure virtual.
TScriptInterface<IPickUpable> UPickUpableStatics::GetFirstPickUpableFromActor(AActor* TargetActor)
{
	TScriptInterface<IPickUpable> PickupableActor(TargetActor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	//如果Actor没有pickupInterface 那么可能他的组件有
	TArray<UActorComponent*> PickUpableComponents = TargetActor ? TargetActor->GetComponentsByInterface(UPickUpable::StaticClass()) : TArray<UActorComponent* >();
	if (PickUpableComponents.Num() > 0)
	{
		return TScriptInterface<IPickUpable>(PickUpableComponents[0]);
	}
	return TScriptInterface<IPickUpable>();
}

void UPickUpableStatics::AddPickupToInventory(UAOInventoryComponent* TargetInventoryManagerComp,
	TScriptInterface<IPickUpable>& PickUp , bool& bCheck)
{
	if (TargetInventoryManagerComp)
	{
		FInventoryPickUp PickupInventory = PickUp->GetPickUpInventory();

		for (FPickUpTemplate& Template : PickupInventory.Templates)
		{
			TargetInventoryManagerComp->AddItemDefinition(Template.ItemInstanceDef,Template.ItemDef , Template.StackCount , bCheck);
		}
		for (FPickUpInstance& Instance : PickupInventory.Instances)
		{
			TargetInventoryManagerComp->AddItemInstance(Instance.Item, Instance.StackCount, bCheck);
		}
	}
}

//检查角色那些库存是可以装的
void UPickUpableStatics::FindCanAddPickUpToInventoryComponent(AActor* Instigator, TScriptInterface<IPickUpable>& PickUp,
	bool& bCheck)
{
	if (!Instigator || !PickUp) return;

	for (const auto& ActorComps : Instigator->GetComponents())
	{
		TScriptInterface<IInventoryInterface> InventoryInterface(ActorComps);
		const FInventoryPickUp& PickupInventory = PickUp->GetPickUpInventory();

		if (InventoryInterface)
		{
			UAOInventoryComponent* InventoryComponent = InventoryInterface->GetInventoryComponent();
			AddPickupToInventory(InventoryComponent,PickUp,bCheck);
			if (bCheck || PickupInventory.Templates[0].StackCount <= 0)
			{
				break;
			}
		}
	}
}
