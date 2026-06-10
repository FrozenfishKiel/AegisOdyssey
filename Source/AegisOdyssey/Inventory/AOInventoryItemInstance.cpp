// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryItemInstance.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS
#include "AOInventoryItemDefinition.h"
#include "AOInventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_Consumable.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryItemInstance)



UAOInventoryItemInstance::UAOInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
{
	
}


void UAOInventoryItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass , ItemDef);
	DOREPLIFETIME(ThisClass , ItemCDO);
	DOREPLIFETIME(ThisClass , RuntimeOwnerActor);
}

UAOInventoryManagerComponent* UAOInventoryItemInstance::FindTargetInventoryManager() const
{
	if (AActor* OwnerActor = GetRuntimeOwnerActor())
	{
		return OwnerActor->FindComponentByClass<UAOInventoryManagerComponent>();
	}
	return nullptr;
}

bool UAOInventoryItemInstance::CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const
{
	return CanUseConsumableFromInventory();
}

bool UAOInventoryItemInstance::TryUseFromInventory(FAOInventoryEntry& InventoryEntry, APawn* UserPawn, int32& OutConsumeCount)
{
	OutConsumeCount = 0;
	if (!CanUseFromInventory(InventoryEntry, UserPawn) || !TryApplyConsumableUseEffects(UserPawn))
	{
		return false;
	}

	OutConsumeCount = 1;
	return true;
}

bool UAOInventoryItemInstance::CanUseConsumableFromInventory() const
{
	const UAOInventoryItemDefinition* ItemDefinition = GetItemCDO();
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const UAOFragment_Consumable* ConsumableFragment = ItemDefinition->FindFragmentByClass<UAOFragment_Consumable>();
	return ConsumableFragment != nullptr && ConsumableFragment->HasUsableEffect();
}

bool UAOInventoryItemInstance::TryApplyConsumableUseEffects(APawn* UserPawn) const
{
	if (UserPawn == nullptr)
	{
		return false;
	}

	const UAOInventoryItemDefinition* ItemDefinition = GetItemCDO();
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const UAOFragment_Consumable* ConsumableFragment = ItemDefinition->FindFragmentByClass<UAOFragment_Consumable>();
	if (ConsumableFragment == nullptr || !ConsumableFragment->HasUsableEffect())
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(UserPawn);
	if (TargetASC == nullptr)
	{
		return false;
	}

	bool bAppliedAnyEffect = false;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : ConsumableFragment->EffectsToApply)
	{
		if (EffectClass == nullptr)
		{
			continue;
		}

		const FActiveGameplayEffectHandle AppliedHandle =
			TargetASC->ApplyGameplayEffectToSelf(
				EffectClass.GetDefaultObject(),
				ConsumableFragment->EffectLevel,
				TargetASC->MakeEffectContext());
		bAppliedAnyEffect |= AppliedHandle.IsValid();
	}

	return bAppliedAnyEffect;
}

//在创建Instance的时候要设定ItemDef
void UAOInventoryItemInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
	if (AActor* Outer = Cast<AActor>(GetOuter()))
	{
		RuntimeOwnerActor = Outer;
		if (!ItemCDO)
		{
			ItemCDO = NewObject<UAOInventoryItemDefinition>(Outer , InDef);
		}
		Outer->AddReplicatedSubObject(ItemCDO);
	}
}

void UAOInventoryItemInstance::SetRuntimeOwnerActor(AActor* InRuntimeOwnerActor)
{
	RuntimeOwnerActor = InRuntimeOwnerActor;
}

AActor* UAOInventoryItemInstance::GetRuntimeOwnerActor() const
{
	if (RuntimeOwnerActor != nullptr)
	{
		return RuntimeOwnerActor;
	}

	return Cast<AActor>(GetOuter());
}

UAOInventoryItemDefinition* UAOInventoryItemInstance::GetItemCDO() const
{
	if (ItemCDO)
	{
		return ItemCDO;
	}
	return nullptr;
}
