// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUpable.h"

#include "AegisOdyssey/Inventory/AOInventoryStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PickUpable)

TScriptInterface<IPickUpable> UPickUpableStatics::GetFirstPickUpableFromActor(AActor* TargetActor)
{
	TScriptInterface<IPickUpable> PickupableActor(TargetActor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// 如果 Actor 本身没有实现拾取接口，则继续尝试它身上的组件。
	TArray<UActorComponent*> PickUpableComponents = TargetActor ? TargetActor->GetComponentsByInterface(UPickUpable::StaticClass()) : TArray<UActorComponent*>();
	if (PickUpableComponents.Num() > 0)
	{
		return TScriptInterface<IPickUpable>(PickUpableComponents[0]);
	}

	return TScriptInterface<IPickUpable>();
}

bool UPickUpableStatics::BuildInventoryReceiveBatchFromPickup(TScriptInterface<IPickUpable>& PickUp, FAOInventoryReceiveBatch& OutReceiveBatch)
{
	OutReceiveBatch = FAOInventoryReceiveBatch();

	if (!PickUp)
	{
		return false;
	}

	const FInventoryPickUp PickupInventory = PickUp->GetPickUpInventory();

	for (const FPickUpTemplate& Template : PickupInventory.Templates)
	{
		if (Template.StackCount <= 0 || Template.ItemDef == nullptr)
		{
			return false;
		}

		FAOInventoryDefinitionEntry& DefinitionEntry = OutReceiveBatch.DefinitionEntries.AddDefaulted_GetRef();
		DefinitionEntry.Count = Template.StackCount;
		DefinitionEntry.ItemDefinitionClass = Template.ItemDef;
		// 拾取模板可以显式指定 override。
		// 留空时，库存链路会回到 Definition 自己去解析实例类。
		DefinitionEntry.ItemInstanceClass = Template.ItemInstanceDef;
	}

	for (const FPickUpInstance& Instance : PickupInventory.Instances)
	{
		if (Instance.StackCount <= 0 || Instance.Item == nullptr)
		{
			return false;
		}

		FAOInventoryInstanceEntry& InstanceEntry = OutReceiveBatch.InstanceEntries.AddDefaulted_GetRef();
		InstanceEntry.Count = Instance.StackCount;
		InstanceEntry.ItemInstance = Instance.Item;
	}

	return true;
}

void UPickUpableStatics::TryAddPickupToActorInventories(AActor* Instigator, TScriptInterface<IPickUpable>& PickUp, bool& bCheck)
{
	bCheck = false;

	if (!Instigator || !PickUp)
	{
		return;
	}

	FAOInventoryReceiveBatch ReceiveBatch;
	if (!BuildInventoryReceiveBatchFromPickup(PickUp, ReceiveBatch))
	{
		return;
	}

	bCheck = UAOInventoryStatics::TryAddInventoryBatchToActor(Instigator, ReceiveBatch);
}

void UPickUpableStatics::FindCanAddPickUpToInventoryComponent(AActor* Instigator, TScriptInterface<IPickUpable>& PickUp, bool& bCheck)
{
	// 兼容旧调用名，内部转发到更明确的统一入口。
	TryAddPickupToActorInventories(Instigator, PickUp, bCheck);
}
