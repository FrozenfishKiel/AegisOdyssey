#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.h"

#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_FormalEquipment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentDefinition)

UAOFormalEquipmentDefinition::UAOFormalEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSubclassOf<UAOInventoryItemInstance> UAOFormalEquipmentDefinition::GetPreferredInstanceType() const
{
	if (PreferredInstanceType != nullptr)
	{
		return PreferredInstanceType;
	}

	return UAOFormalEquipmentInstance::StaticClass();
}

EAOFormalEquipmentSlotType UAOFormalEquipmentDefinition::GetFormalSlotType() const
{
	if (const UAOFragment_FormalEquipment* FormalEquipmentFragment = GetFormalEquipmentFragment())
	{
		return FormalEquipmentFragment->FormalSlotType;
	}

	return EAOFormalEquipmentSlotType::None;
}

const UAOFragment_FormalEquipment* UAOFormalEquipmentDefinition::GetFormalEquipmentFragment() const
{
	return FindFragmentByClass<UAOFragment_FormalEquipment>();
}
