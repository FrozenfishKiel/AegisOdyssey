#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AegisOdyssey/Inventory/AOInventoryAcquisitionMessage.h"
#include "AOInventoryMessageSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOInventoryAcquisitionMessageDynamicDelegate, FAOInventoryAcquisitionMessage, Message);

// 世界级库存获得消息子系统。
// 它只负责广播“已经成功入库”的结果，不参与来源系统逻辑和 UI 表现。
UCLASS()
class AEGISODYSSEY_API UAOInventoryMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AO|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UAOInventoryMessageSubsystem* Get(const UObject* WorldContextObject);

	void BroadcastInventoryAcquisition(const FAOInventoryAcquisitionMessage& Message);

	UPROPERTY(BlueprintAssignable, Category = "AO|Inventory")
	FAOInventoryAcquisitionMessageDynamicDelegate OnInventoryAcquisitionMessage;
};
