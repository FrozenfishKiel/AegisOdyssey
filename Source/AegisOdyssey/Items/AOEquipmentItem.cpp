// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEquipmentItem.h"

#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentItem)

AAOEquipmentItem::AAOEquipmentItem()
{
}

void AAOEquipmentItem::SetEquipmentDefinition(TSubclassOf<UAOEquipmentDefinition> InEquipmentDefinition)
{
	EquipmentDefinition = InEquipmentDefinition;
}

void AAOEquipmentItem::InitializeActorSpawnConfig()
{
	// 当世界装备物被挂到角色身上作为表现体时，后续不再保留可拾取碰撞。
	DisableEquippedPresentationCollision();
}

FInventoryPickUp AAOEquipmentItem::GetPickUpInventory() const
{
	// 只要配置了装备定义，就优先动态生成拾取模板，
	// 避免世界物和库存定义各写一份造成后续脱节。
	if (EquipmentDefinition != nullptr)
	{
		FInventoryPickUp InventoryPickUp;
		FPickUpTemplate& PickUpTemplate = InventoryPickUp.Templates.AddDefaulted_GetRef();
		PickUpTemplate.StackCount = 1;
		PickUpTemplate.ItemDef = EquipmentDefinition;
		PickUpTemplate.ItemInstanceDef = UAOInventoryItemDefinition::ResolveItemInstanceClass(EquipmentDefinition);
		return InventoryPickUp;
	}

	return Super::GetPickUpInventory();
}
