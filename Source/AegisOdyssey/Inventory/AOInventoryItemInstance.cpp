// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryItemInstance.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS
#include "AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
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
}

UAOInventoryManagerComponent* UAOInventoryItemInstance::FindTargetInventoryManager() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOuter()))
	{
		return OwnerPawn->FindComponentByClass<UAOInventoryManagerComponent>();
	}
	return nullptr;
}

//在创建Instance的时候要设定ItemDef
void UAOInventoryItemInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
	if (APawn* Outer = Cast<APawn>(GetOuter()))
	{
		if (!ItemCDO)
		{
			ItemCDO = NewObject<UAOInventoryItemDefinition>(Outer , InDef);
		}
		Outer->AddReplicatedSubObject(ItemCDO);
	}
}

UAOInventoryItemDefinition* UAOInventoryItemInstance::GetItemCDO() const
{
	if (ItemCDO)
	{
		return ItemCDO;
	}
	return nullptr;
}

