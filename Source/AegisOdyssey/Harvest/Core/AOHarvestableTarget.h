#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "UObject/Interface.h"
#include "AOHarvestableTarget.generated.h"

class UAOHarvestableComponent;

// 可采对象接口只暴露两件事：
// 1. 这个对象的公共采集运行时组件在哪里
// 2. 这个对象在耗尽/重生后的专属生命周期反应怎么处理
UINTERFACE(MinimalAPI, Blueprintable)
class UAOHarvestableTarget : public UInterface
{
	GENERATED_BODY()
};

class AEGISODYSSEY_API IAOHarvestableTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AO|Harvest")
	UAOHarvestableComponent* GetHarvestableComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AO|Harvest")
	void HandleHarvestNodeDepleted(const FAOHarvestLifecycleContext& LifecycleContext);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AO|Harvest")
	void HandleHarvestNodeRespawned();
};
