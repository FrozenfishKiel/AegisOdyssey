#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOInventoryAcquisitionMessage.generated.h"

// 一条“物品已经正式进入库存”的统一消息。
// 它描述的是库存真相，不包含任何 UI 表现决策。
USTRUCT(BlueprintType)
struct FAOInventoryAcquisitionMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<AActor> Receiver = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;

	bool IsValid() const
	{
		return Receiver != nullptr && ItemDefinitionClass != nullptr && Count > 0;
	}
};
