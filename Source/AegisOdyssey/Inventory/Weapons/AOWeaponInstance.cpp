// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWeaponInstance.h"

UAOWeaponInstance::UAOWeaponInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAOWeaponInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	Super::SetItemDef(InDef);
}
