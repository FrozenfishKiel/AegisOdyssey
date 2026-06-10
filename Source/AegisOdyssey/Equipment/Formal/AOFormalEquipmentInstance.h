#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AOFormalEquipmentInstance.generated.h"

class UAOFormalEquipmentDefinition;

UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOFormalEquipmentInstance : public UAOEquipmentInstance
{
	GENERATED_BODY()

public:
	UAOFormalEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAOInventoryManagerComponent* FindTargetInventoryManager() const override;
	virtual bool CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const override;

	const UAOFormalEquipmentDefinition* GetFormalEquipmentDefinition() const;
};

