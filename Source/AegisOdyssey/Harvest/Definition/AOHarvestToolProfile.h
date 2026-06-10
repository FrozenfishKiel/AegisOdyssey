#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AOHarvestToolProfile.generated.h"

// ToolProfile 不代表某一把具体工具。
// 它表达的是“这类工具在采集规则里的机械身份”，用于让节点按对象规则响应不同工具语义。
UCLASS(BlueprintType, Const)
class AEGISODYSSEY_API UAOHarvestToolProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAOHarvestToolProfile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HarvestToolProfile"), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText ProfileName;

	// 这里先保留轻量标签入口，方便后续做编辑器检索、规则分组或额外扩展。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FGameplayTagContainer ProfileTags;
};
