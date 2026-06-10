#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Cue/AOHarvestGameplayCueNotify_Burst.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "Engine/DataAsset.h"
#include "AOHarvestableDefinition.generated.h"

// 采集节点定义只描述“这个采集节点本身是什么”。
// 当前阶段这里只放静态定义，不放任何运行时占用者、当前进度或联机状态。
UCLASS(BlueprintType, Const)
class AEGISODYSSEY_API UAOHarvestableDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAOHarvestableDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HarvestableDefinition"), GetFName());
	}

	// 节点对外展示用的静态名字。
	// 这是“这个采集对象在设计与界面上叫什么”，不是运行时实例名，也不是掉落物名。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText HarvestableName;

	// 节点完整采空前拥有的总采集进度。
	// RuntimeState.CurrentProgress 会以它为初始值，并在每次合法命中后递减；它不是“单次命中伤害”。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TotalHarvestProgress = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestRespawnConfig RespawnConfig;

	// 这是对象在没有命中特定 ToolProfile 规则时的默认响应。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestToolTuning DefaultToolResponse;

	// 这是对象侧对不同工具机械语义的差异化响应入口。
	// 例如允许徒手、斧头、镐头都来采，但倍率和额外修正不同。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TArray<FAOHarvestToolProfileResponse> ToolProfileResponses;

	// 掉落基础数据归节点定义所有。
	// 后续真正的数量、概率修正和最终结算，会统一进入 HarvestResolver。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TArray<FAOHarvestDropEntry> DropEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest|Cue")
	FAOHarvestCueVisualSet HarvestHitCueVisuals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest|Cue")
	FAOHarvestCueVisualSet HarvestDepletedCueVisuals;
};
