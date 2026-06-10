// Fill out your copyright notice in the Description page of Project Settings.


#include "AOEquipmentDefinition.h"

#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentDefinition)

UAOEquipmentDefinition::UAOEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
{
}

TSubclassOf<UAOInventoryItemInstance> UAOEquipmentDefinition::GetPreferredInstanceType() const
{
	if (PreferredInstanceType != nullptr)
	{
		return PreferredInstanceType;
	}

	return UAOEquipmentInstance::StaticClass();
}
