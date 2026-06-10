#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentTypes.h"
#include "AOFormalEquipmentDefinition.generated.h"

class UAOFragment_FormalEquipment;

UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class AEGISODYSSEY_API UAOFormalEquipmentDefinition : public UAOEquipmentDefinition
{
	GENERATED_BODY()

public:
	UAOFormalEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TSubclassOf<UAOInventoryItemInstance> GetPreferredInstanceType() const override;

	EAOFormalEquipmentSlotType GetFormalSlotType() const;

private:
	const UAOFragment_FormalEquipment* GetFormalEquipmentFragment() const;
};
