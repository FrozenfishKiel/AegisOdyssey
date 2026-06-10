#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "AOHarvestResolver.generated.h"

class UAOHarvestableDefinition;
struct FHitResult;

// HarvestResolver 只负责第四阶段的“服务端最终重判定 + 统一结算”。
// 它不直接改背包，也不承担玩家输入、动画生命周期或节点持有者管理职责。
UCLASS()
class AEGISODYSSEY_API UAOHarvestResolver : public UObject
{
	GENERATED_BODY()

public:
	static bool ResolveHarvestRequest(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult, bool bForceDebugDraw = false, float DebugDrawDuration = 2.0f);
	static bool FinalizeHarvestRewards(const FAOHarvestHitContext& HitContext, FAOHarvestResult& InOutResult);
	static bool ShouldDrawHarvestDebug(bool bForceEnable = false);
	static void DrawHarvestDebugPreview(const FAOHarvestHitContext& HitContext, const FAOHarvestHitCheckConfig& HitCheckConfig,
		bool bForceDebugDraw = false, float Duration = 0.15f);
	static void DrawHarvestDebugResult(const FAOHarvestHitContext& HitContext, const FAOHarvestHitCheckConfig* HitCheckConfig, const FAOHarvestResult& HarvestResult,
		const FHitResult* SweepHitResult = nullptr, const FHitResult* OcclusionHitResult = nullptr, bool bForceDebugDraw = false, float Duration = 2.0f);

private:
	static bool ValidateHarvestContext(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult);
	static bool ValidateTargetStillAcceptsHarvest(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult);

	static FAOHarvestToolTuning ResolveToolTuning(const UAOHarvestableDefinition& HarvestableDefinition, const FAOHarvestHitContext& HitContext);
	static void BuildRewardEntries(const UAOHarvestableDefinition& HarvestableDefinition, const FAOHarvestToolTuning& ToolTuning,
		bool bWillDeplete, FAOHarvestResult& OutResult);
};
