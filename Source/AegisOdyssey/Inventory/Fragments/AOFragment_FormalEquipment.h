#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentTypes.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_FormalEquipment.generated.h"

UCLASS()
class AEGISODYSSEY_API UAOFragment_FormalEquipment : public UAOInventoryItemFragment
{
	GENERATED_BODY()

public:
	// FormalEquipment Fragment 现在只负责声明“这件正式装备属于哪个唯一槽位”。
	// 属性授予统一收敛到 Definition.AbilitySetsToGrant，和武器走同一条授权链。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FormalEquipment")
	EAOFormalEquipmentSlotType FormalSlotType = EAOFormalEquipmentSlotType::None;
};
