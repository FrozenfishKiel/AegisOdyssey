// Fill out your copyright notice in the Description page of Project Settings.

#include "AOFormalEquipmentItem.h"

#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentItem)

void AAOFormalEquipmentItem::SetFormalEquipmentDefinition(TSubclassOf<UAOFormalEquipmentDefinition> InFormalEquipmentDefinition)
{
	// 正式装备仍然复用通用世界装备物的底层拾取翻译逻辑。
	SetEquipmentDefinition(InFormalEquipmentDefinition);
}

TSubclassOf<UAOFormalEquipmentDefinition> AAOFormalEquipmentItem::GetFormalEquipmentDefinition() const
{
	return TSubclassOf<UAOFormalEquipmentDefinition>(GetEquipmentDefinition());
}
