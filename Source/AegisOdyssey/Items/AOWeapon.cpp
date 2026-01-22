// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWeapon.h"

#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/Weapons/AOWeaponInstance.h"

void AAOWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void AAOWeapon::InitializeActorSpawnConfig()
{
	PickUpBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

