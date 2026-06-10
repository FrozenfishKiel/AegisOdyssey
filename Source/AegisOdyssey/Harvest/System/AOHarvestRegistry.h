#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPtr.h"
#include "AOHarvestRegistry.generated.h"

class UAOHarvestToolDefinition;
class UAOHarvestToolProfile;
class UAOHarvestableDefinition;

// HarvestRegistry 只做“集中收集定义”的入口。
// 它不是系统逻辑类，不在 C++ 里写死现有资源，后续由策划 / 内容侧手动维护。
UCLASS(BlueprintType, Const)
class AEGISODYSSEY_API UAOHarvestRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAOHarvestRegistry(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HarvestRegistry"), GetFName());
	}

	const TArray<TSoftObjectPtr<UAOHarvestableDefinition>>& GetHarvestableDefinitions() const { return HarvestableDefinitions; }
	const TArray<TSoftClassPtr<UAOHarvestToolDefinition>>& GetHarvestToolDefinitions() const { return HarvestToolDefinitions; }
	const TArray<TSoftObjectPtr<UAOHarvestToolProfile>>& GetHarvestToolProfiles() const { return HarvestToolProfiles; }

protected:
	// 节点定义统一收口在这里，方便后续遍历、校验和编辑器总览。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TArray<TSoftObjectPtr<UAOHarvestableDefinition>> HarvestableDefinitions;

	// 工具定义仍然沿用项目里“物品定义以类为主”的现有组织方式。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TArray<TSoftClassPtr<UAOHarvestToolDefinition>> HarvestToolDefinitions;

	// ToolProfile 单独收集，是为了后续对象规则、工具规则和编辑器配置能共享同一套语义资产。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TArray<TSoftObjectPtr<UAOHarvestToolProfile>> HarvestToolProfiles;
};
