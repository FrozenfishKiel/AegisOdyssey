#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOHarvestToolFragment.generated.h"

// HarvestToolFragment 是工具 Definition 里的采集配置块。
// 它描述的是“这把工具如何进行采集”，不是节点配置，也不是运行时实例状态。
UCLASS(DefaultToInstanced, EditInlineNew, BlueprintType)
class AEGISODYSSEY_API UAOHarvestToolFragment : public UAOInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 这是工具每次有效命中时提供的基础采集强度。
	// 节点侧还会再叠加 ProgressMultiplier，最终决定这一下能推进多少采集进度。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BaseHarvestPower = 1.0f;

	// 这是“这把工具如何判定自己打到了采集目标”的检测模板。
	// 真正服务端结算时会基于它做距离、朝向、Sweep 和遮挡复核。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestHitCheckConfig HitCheckConfig;
};
