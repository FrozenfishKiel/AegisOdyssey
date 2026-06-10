// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWeaponManagerComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AOEquipmentInstance.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
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
	if (TargetItem.Instance && !bIsUse)
	{
		EquipItem(TargetItem);
	}
	else if (bIsUse)
	{
		UnequipItem(TargetItem);
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
	if (!CurrentWeaponInstance) return;


	APawn* CurrentWeaponOwnerPawn = Cast<APawn>(CurrentWeaponInstance->GetOuter());
	UAOEquipmentDefinition* EquipmentCDO = Cast<UAOEquipmentDefinition>(CurrentWeaponInstance->ItemCDO);
	if (!EquipmentCDO) return;

	if (UAOAbilitySystem* ASC = Cast<UAOAbilitySystem>( UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentWeaponOwnerPawn)))
	{
		if (GetOwner()->HasAuthority())
		{
			//赋予能力和效果
			for (const TObjectPtr<UAOAbilitySet>& AbilitySet : EquipmentCDO->GetAbilitySetsToGrant())
			{
				AbilitySet->GiveToAbilitySystem(ASC,&InEquipment.GrantedHandles,CurrentWeaponInstance);
			}
		}
	}
	//先赋予能力，再装备物品
	
	CurrentWeaponInstance->OnEquiped();

	if (GetOwner()->HasAuthority())
	{
		CurrentWeaponInstance->SpawnEquipmentActors(EquipmentCDO->ActorToSpawn);  //生成武器
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(InEquipment.Instance);
		}
	}
	
	Weapon = InEquipment;  //保存当前信息
	
	bIsUse = true;
}

void UAOWeaponManagerComponent::UnequipItem(FAOInventoryEntry& InItem)
{
	if (!CurrentWeaponInstance) return;
	//if (CurrentWeaponInstance == ItemInstance) return;

	CurrentWeaponInstance->OnUnEquiped(); //先移除效果和技能再取消装备

	if (APawn* WeaponOwnerPawn = Cast<APawn>(GetOuter()))
	{
		if (GetOwner()->HasAuthority())
		{
			if (UAOAbilitySystem* SourceASC = Cast<UAOAbilitySystem>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(WeaponOwnerPawn)))
			{
				// 武器虽然不纳入正式装备槽，但它授予出去的 GE 也可能带堆叠语义。
				// 因此这里和正式装备保持一致，只要走“卸下这件来源物”的回收路径，
				// 就统一按 stack-aware 语义移除，避免把同类可堆叠效果整条误删。
				// 武器来源属性集现在改回角色常驻的 UAOWeaponAttributeSet，
				// 卸下时只回收这件武器授予出去的能力和效果，不再处理 AttributeSet 的存在性。
				InItem.GrantedHandles.TakeFromAbilitySystemStackAware(SourceASC);
			}
		}
	}

	if (GetOwner()->HasAuthority())
	{
		CurrentWeaponInstance->DestoryEquipmentActors();  //移除生成的Actor
	
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			RemoveReplicatedSubObject(InItem.Instance);
		}
		CurrentWeaponInstance = nullptr;//GC回收
	}
	Weapon = nullptr;
	bIsUse = false;
}

void UAOWeaponManagerComponent::OnRep_CurrentWeaponInstance(UAOEquipmentInstance* LastWeaponInstance)
{
	if (LastWeaponInstance && LastWeaponInstance != CurrentWeaponInstance)
	{
		LastWeaponInstance->OnUnEquiped();
	}

	if (CurrentWeaponInstance)
	{
		CurrentWeaponInstance->OnEquiped();
	}
}

void UAOWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,CurrentWeaponInstance);
	DOREPLIFETIME(ThisClass , bIsUse);
	DOREPLIFETIME(ThisClass , Weapon);
}

void UAOWeaponManagerComponent::ChangedItemOnSlot(const int32 ChangedIndex, const int32 CurrentIndex, TArray<FAOInventoryEntry>* Slots)
{
	if (Slots == nullptr || Slots->IsEmpty() || !Slots->IsValidIndex(CurrentIndex))
	{
		if (Weapon.Instance)
		{
			UnequipItem(Weapon);
		}

		return;
	}

	const UAOInventoryItemInstance* EquippedInstance = Weapon.Instance;
	FAOInventoryEntry& CurrentSlotEntry = (*Slots)[CurrentIndex];
	const UAOInventoryItemInstance* CurrentSlotInstance = CurrentSlotEntry.Instance;
	const bool bCurrentSlotStillHoldsEquippedInstance =
		EquippedInstance != nullptr && CurrentSlotInstance == EquippedInstance;

	if (ChangedIndex == CurrentIndex)
	{
		if (EquippedInstance == nullptr)
		{
			EquipItem(CurrentSlotEntry);
			return;
		}
	}

	if (EquippedInstance != nullptr && !bCurrentSlotStillHoldsEquippedInstance)
	{
		UnequipItem(Weapon);

		if (CurrentSlotEntry.Instance != nullptr)
		{
			EquipItem(CurrentSlotEntry);
		}
	}
}

void UAOWeaponManagerComponent::OnRep_Weapon()
{
}
