#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentInstance.h"

#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentInstance)

UAOFormalEquipmentInstance::UAOFormalEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAOInventoryManagerComponent* UAOFormalEquipmentInstance::FindTargetInventoryManager() const
{
	if (APawn* OwnerPawn = GetPawn())
	{
		return OwnerPawn->FindComponentByClass<UAOFormalEquipmentManagerComponent>();
	}

	return nullptr;
}

bool UAOFormalEquipmentInstance::CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const
{
	return InventoryEntry.Instance == this && UserPawn != nullptr && GetFormalEquipmentDefinition() != nullptr && FindTargetInventoryManager() != nullptr;
}

const UAOFormalEquipmentDefinition* UAOFormalEquipmentInstance::GetFormalEquipmentDefinition() const
{
	return Cast<UAOFormalEquipmentDefinition>(GetItemCDO());
}
