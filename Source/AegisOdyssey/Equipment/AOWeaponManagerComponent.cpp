// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWeaponManagerComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AOEquipmentInstance.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Inventory/Weapons/AOWeaponInstance.h"
#include "Net/UnrealNetwork.h"
#include "Tests/ToolMenusTestUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOWeaponManagerComponent)
UAOWeaponManagerComponent::UAOWeaponManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

//May Run From QuickBarComponent.
void UAOWeaponManagerComponent::OnItemUse(FAOInventoryEntry& TargetItem)
{
	if (TargetItem.Instance)
	{
		EquipItem(TargetItem);
	}
}
//May Run From QuickBarComponent.
void UAOWeaponManagerComponent::OnItemUnUse(FAOInventoryEntry& TargetItem)
{
	if (TargetItem.Instance)
	{
		UnequipItem(TargetItem);
	}
}


void UAOWeaponManagerComponent::EquipItem(
	FAOInventoryEntry& InEquipment)
{
	if (CurrentWeaponInstance) return;  //CurrentWeaponInstance为空时再来

	UAOInventoryItemInstance* EquipmentInstance = InEquipment.Instance;
	CurrentWeaponInstance = Cast<UAOEquipmentInstance>(EquipmentInstance);
	
	CurrentWeaponInstance->OnEquiped();

	APawn* CurrentWeaponOwnerPawn = Cast<APawn>(CurrentWeaponInstance->GetOuter());
	UAOEquipmentDefinition* EquipmentCDO = Cast<UAOEquipmentDefinition>(CurrentWeaponInstance->ItemCDO);
	if (!EquipmentCDO) return;

	if (UAOAbilitySystem* ASC = Cast<UAOAbilitySystem>( UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentWeaponOwnerPawn)))
	{
		
		
		for (const TObjectPtr<const UAOAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC,&InEquipment.GrantedHandles,CurrentWeaponInstance);
		}
	}
	

	CurrentWeaponInstance->SpawnEquipmentActors(EquipmentCDO->ActorToSpawn);  //生成武器
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InEquipment.Instance);
	}
}

void UAOWeaponManagerComponent::UnequipItem(FAOInventoryEntry& InItem)
{
	if (!CurrentWeaponInstance) return;
	//if (CurrentWeaponInstance == ItemInstance) return;
	
	CurrentWeaponInstance->OnUnEquiped();

	if (APawn* WeaponOwnerPawn = Cast<APawn>(GetOuter()))
	{
		if (UAOAbilitySystem* SourceASC = Cast<UAOAbilitySystem>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(WeaponOwnerPawn)))
		{
			InItem.GrantedHandles.TakeFromAbilitySystem(SourceASC);  //移除ASC
		}
	}
	
	CurrentWeaponInstance->DestoryEquipmentActors();  //移除生成的Actor
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(InItem.Instance);
	}
	CurrentWeaponInstance = nullptr;//GC回收
}

void UAOWeaponManagerComponent::OnRep_CurrentWeaponInstance()
{
	
}

void UAOWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,CurrentWeaponInstance);
}
