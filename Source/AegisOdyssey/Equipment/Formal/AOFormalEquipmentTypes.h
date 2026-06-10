#pragma once

#include "CoreMinimal.h"
#include "AOFormalEquipmentTypes.generated.h"

UENUM(BlueprintType)
enum class EAOFormalEquipmentSlotType : uint8
{
	None UMETA(DisplayName = "None"),
	Helmet UMETA(DisplayName = "Helmet"),
	Armor UMETA(DisplayName = "Armor"),
	Gloves UMETA(DisplayName = "Gloves"),
	Necklace UMETA(DisplayName = "Necklace"),
	Boots UMETA(DisplayName = "Boots")
};

