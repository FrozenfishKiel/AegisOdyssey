#include "AegisOdyssey/Harvest/System/AOHarvestResolver.h"

#include "AegisOdyssey/Harvest/Core/AOHarvestableComponent.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestToolProfile.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestableDefinition.h"
#include "AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.h"
#include "AegisOdyssey/Harvest/Items/AOHarvestToolInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestResolver)

namespace AOHarvestResolver_Private
{
	static TAutoConsoleVariable<int32> CVarHarvestDebugDraw(
		TEXT("ao.Harvest.DebugDraw"),
		0,
		TEXT("Enable debug draw for AO harvest hit validation.")
	);

	static const FName Reject_InvalidContext(TEXT("InvalidContext"));
	static const FName Reject_InvalidTarget(TEXT("InvalidTarget"));
	static const FName Reject_InvalidTool(TEXT("InvalidTool"));
	static const FName Reject_TargetRejected(TEXT("TargetRejected"));

	static FColor GetColorForRejectReason(FName RejectReason)
	{
		if (RejectReason == NAME_None)
		{
			return FColor::Green;
		}

		if (RejectReason == Reject_TargetRejected)
		{
			return FColor::Cyan;
		}

		return FColor::Silver;
	}

	static void DrawFacingEnvelope(UWorld* World, const FAOHarvestHitContext& HitContext, const FAOHarvestHitCheckConfig& HitCheckConfig, const FColor& Color, float Duration)
	{
		if (World == nullptr)
		{
			return;
		}

		const FVector Forward = HitContext.FacingDirection.GetSafeNormal();
		if (Forward.IsNearlyZero())
		{
			return;
		}

		const FVector UpVector = FVector::UpVector;
		const FVector LeftDirection = Forward.RotateAngleAxis(-HitCheckConfig.MaxFacingAngleDegrees, UpVector);
		const FVector RightDirection = Forward.RotateAngleAxis(HitCheckConfig.MaxFacingAngleDegrees, UpVector);
		const FVector ArcCenter = HitContext.TraceStart;

		DrawDebugLine(World, ArcCenter, ArcCenter + LeftDirection * HitCheckConfig.MaxDistance, Color, false, Duration, 0, 1.0f);
		DrawDebugLine(World, ArcCenter, ArcCenter + RightDirection * HitCheckConfig.MaxDistance, Color, false, Duration, 0, 1.0f);

		constexpr int32 ArcSegments = 12;
		FVector PreviousPoint = ArcCenter + LeftDirection * HitCheckConfig.MaxDistance;
		for (int32 SegmentIndex = 1; SegmentIndex <= ArcSegments; ++SegmentIndex)
		{
			const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(ArcSegments);
			const float Angle = FMath::Lerp(-HitCheckConfig.MaxFacingAngleDegrees, HitCheckConfig.MaxFacingAngleDegrees, Alpha);
			const FVector CurrentDirection = Forward.RotateAngleAxis(Angle, UpVector);
			const FVector CurrentPoint = ArcCenter + CurrentDirection * HitCheckConfig.MaxDistance;
			DrawDebugLine(World, PreviousPoint, CurrentPoint, Color, false, Duration, 0, 0.75f);
			PreviousPoint = CurrentPoint;
		}
	}
}

bool UAOHarvestResolver::ShouldDrawHarvestDebug(bool bForceEnable)
{
#if ENABLE_DRAW_DEBUG
	return bForceEnable || AOHarvestResolver_Private::CVarHarvestDebugDraw.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

void UAOHarvestResolver::DrawHarvestDebugPreview(const FAOHarvestHitContext& HitContext, const FAOHarvestHitCheckConfig& HitCheckConfig,
	bool bForceDebugDraw, float Duration)
{
#if ENABLE_DRAW_DEBUG
	if (!ShouldDrawHarvestDebug(bForceDebugDraw))
	{
		return;
	}

	UWorld* World = HitContext.HarvesterActor ? HitContext.HarvesterActor->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return;
	}

	const FVector SweepMidPoint = (HitContext.TraceStart + HitContext.TraceEnd) * 0.5f;
	DrawDebugSphere(World, HitContext.TraceStart, 6.0f, 12, FColor::White, false, Duration);
	DrawDebugLine(World, HitContext.TraceStart, HitContext.TraceEnd, FColor::White, false, Duration, 0, 1.0f);
	DrawDebugSphere(World, SweepMidPoint, HitCheckConfig.SweepRadius, 16, FColor::White, false, Duration);
	DrawDebugSphere(World, HitContext.TraceEnd, HitCheckConfig.SweepRadius, 16, FColor::White, false, Duration);
	AOHarvestResolver_Private::DrawFacingEnvelope(World, HitContext, HitCheckConfig, FColor::White, Duration);

	if (HitContext.RuntimeContext.TargetActor != nullptr)
	{
		DrawDebugSphere(World, HitContext.RuntimeContext.TargetActor->GetActorLocation(), 18.0f, 12, FColor::Blue, false, Duration);
	}
#endif
}

void UAOHarvestResolver::DrawHarvestDebugResult(const FAOHarvestHitContext& HitContext, const FAOHarvestHitCheckConfig* HitCheckConfig, const FAOHarvestResult& HarvestResult,
	const FHitResult* SweepHitResult, const FHitResult* OcclusionHitResult, bool bForceDebugDraw, float Duration)
{
#if ENABLE_DRAW_DEBUG
	if (!ShouldDrawHarvestDebug(bForceDebugDraw))
	{
		return;
	}

	UWorld* World = HitContext.HarvesterActor ? HitContext.HarvesterActor->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return;
	}

	const FColor ResultColor = AOHarvestResolver_Private::GetColorForRejectReason(HarvestResult.bSuccess ? NAME_None : HarvestResult.RejectReason);
	DrawDebugSphere(World, HitContext.TraceStart, 8.0f, 12, ResultColor, false, Duration);
	DrawDebugLine(World, HitContext.TraceStart, HitContext.TraceEnd, ResultColor, false, Duration, 0, 2.0f);

	if (HitCheckConfig != nullptr)
	{
		const FVector SweepMidPoint = (HitContext.TraceStart + HitContext.TraceEnd) * 0.5f;
		DrawDebugSphere(World, SweepMidPoint, HitCheckConfig->SweepRadius, 20, ResultColor, false, Duration);
		DrawDebugSphere(World, HitContext.TraceEnd, HitCheckConfig->SweepRadius, 20, ResultColor, false, Duration);
		AOHarvestResolver_Private::DrawFacingEnvelope(World, HitContext, *HitCheckConfig, ResultColor, Duration);
	}

	if (HitContext.RuntimeContext.TargetActor != nullptr)
	{
		DrawDebugSphere(World, HitContext.RuntimeContext.TargetActor->GetActorLocation(), 20.0f, 12, HarvestResult.bSuccess ? FColor::Green : FColor::Blue, false, Duration);
		DrawDebugLine(World, HitContext.TraceStart, HitContext.RuntimeContext.TargetActor->GetActorLocation(), FColor::Blue, false, Duration, 0, 1.0f);
	}

	if (SweepHitResult != nullptr && SweepHitResult->bBlockingHit)
	{
		DrawDebugPoint(World, SweepHitResult->ImpactPoint, 16.0f, HarvestResult.bSuccess ? FColor::Green : FColor::Red, false, Duration);
	}

	if (OcclusionHitResult != nullptr && OcclusionHitResult->bBlockingHit)
	{
		DrawDebugPoint(World, OcclusionHitResult->ImpactPoint, 14.0f, FColor::Purple, false, Duration);
		DrawDebugLine(World, HitContext.TraceStart, OcclusionHitResult->ImpactPoint, FColor::Purple, false, Duration, 0, 2.5f);
	}
#endif
}

bool UAOHarvestResolver::ResolveHarvestRequest(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult, bool bForceDebugDraw, float DebugDrawDuration)
{
	OutResult = FAOHarvestResult();

	// Resolver 的职责是把一次命中请求收口成“是否合法、能扣多少、会掉什么”。
	// 它只做统一重判定和统一结算，不直接改节点状态，也不直接把奖励塞进库存。
	if (!ValidateHarvestContext(HitContext, OutResult))
	{
		return false;
	}

	const UAOHarvestToolFragment* ToolFragment = HitContext.RuntimeContext.ToolFragment.Get();
	const UAOHarvestableDefinition* HarvestableDefinition = HitContext.RuntimeContext.TargetComponent->GetHarvestableDefinition();
	if (ToolFragment == nullptr || HarvestableDefinition == nullptr)
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_InvalidContext;
		return false;
	}

	if (!ValidateTargetStillAcceptsHarvest(HitContext, OutResult))
	{
		return false;
	}

	const FAOHarvestHitCheckConfig& HitCheckConfig = ToolFragment->HitCheckConfig;
	const FAOHarvestToolTuning ToolTuning = ResolveToolTuning(*HarvestableDefinition, HitContext);
	if (!ToolTuning.bCanHarvest)
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_TargetRejected;
		DrawHarvestDebugResult(HitContext, &HitCheckConfig, OutResult, nullptr, nullptr, bForceDebugDraw, DebugDrawDuration);
		return false;
	}

	// 先算“理论请求值”，再结合节点当前剩余进度裁成真正可应用值。
	// 这里只负责给出这次挥击理论上想推进多少进度。
	// 真正的当前进度读取、扣减和 depleted 判定统一收口到 HarvestableComponent 自身。
	const float RequestedProgress = ToolFragment->BaseHarvestPower * FMath::Max(0.0f, ToolTuning.ProgressMultiplier);
	OutResult.RequestedProgress = RequestedProgress;
	OutResult.bSuccess = RequestedProgress > 0.0f;

	if (!OutResult.bSuccess)
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_TargetRejected;
		DrawHarvestDebugResult(HitContext, &HitCheckConfig, OutResult, nullptr, nullptr, bForceDebugDraw, DebugDrawDuration);
		return false;
	}

	DrawHarvestDebugResult(HitContext, &HitCheckConfig, OutResult, nullptr, nullptr, bForceDebugDraw, DebugDrawDuration);
	return true;
}

bool UAOHarvestResolver::FinalizeHarvestRewards(const FAOHarvestHitContext& HitContext, FAOHarvestResult& InOutResult)
{
	const UAOHarvestableDefinition* HarvestableDefinition = HitContext.RuntimeContext.TargetComponent != nullptr
		? HitContext.RuntimeContext.TargetComponent->GetHarvestableDefinition()
		: nullptr;
	if (HarvestableDefinition == nullptr || !InOutResult.bSuccess)
	{
		return false;
	}

	const FAOHarvestToolTuning ToolTuning = ResolveToolTuning(*HarvestableDefinition, HitContext);
	if (!ToolTuning.bCanHarvest)
	{
		return false;
	}

	InOutResult.RewardEntries.Reset();
	BuildRewardEntries(*HarvestableDefinition, ToolTuning, InOutResult.bDepletedAfterHit, InOutResult);
	return true;
}

bool UAOHarvestResolver::ValidateHarvestContext(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult)
{
	if (HitContext.HarvesterActor == nullptr ||
		HitContext.RuntimeContext.TargetActor == nullptr ||
		HitContext.RuntimeContext.TargetComponent == nullptr)
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_InvalidContext;
		return false;
	}

	if (HitContext.RuntimeContext.ToolDefinition == nullptr || HitContext.RuntimeContext.ToolFragment == nullptr)
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_InvalidTool;
		return false;
	}

	return true;
}

bool UAOHarvestResolver::ValidateTargetStillAcceptsHarvest(const FAOHarvestHitContext& HitContext, FAOHarvestResult& OutResult)
{
	if (!IsValid(HitContext.RuntimeContext.TargetComponent) || !HitContext.RuntimeContext.TargetComponent->CanAcceptHarvestRequest())
	{
		OutResult.RejectReason = AOHarvestResolver_Private::Reject_InvalidTarget;
		return false;
	}

	return true;
}

FAOHarvestToolTuning UAOHarvestResolver::ResolveToolTuning(const UAOHarvestableDefinition& HarvestableDefinition, const FAOHarvestHitContext& HitContext)
{
	const UAOHarvestToolDefinition* ToolDefinition = HitContext.RuntimeContext.ToolDefinition.Get();
	const UAOHarvestToolInstance* ToolInstance = HitContext.RuntimeContext.ToolInstance.Get();
	const UAOHarvestToolProfile* ToolProfile =
		ToolInstance
			? ToolInstance->GetHarvestToolProfile()
			: (ToolDefinition ? ToolDefinition->GetHarvestToolProfile() : nullptr);
	if (ToolProfile == nullptr)
	{
		// 没有 ToolProfile 时直接退回节点默认响应，避免把“没配 Profile”误解成“禁止采集”。
		return HarvestableDefinition.DefaultToolResponse;
	}

	for (const FAOHarvestToolProfileResponse& ResponseEntry : HarvestableDefinition.ToolProfileResponses)
	{
		if (ResponseEntry.ToolProfile == ToolProfile)
		{
			return ResponseEntry.Response;
		}
	}

	return HarvestableDefinition.DefaultToolResponse;
}

void UAOHarvestResolver::BuildRewardEntries(const UAOHarvestableDefinition& HarvestableDefinition, const FAOHarvestToolTuning& ToolTuning,
	bool bWillDeplete, FAOHarvestResult& OutResult)
{
	for (const FAOHarvestDropEntry& DropEntry : HarvestableDefinition.DropEntries)
	{
		// 奖励时机按条目自己决定。
		// 这样不同掉落可以分别配置“每击掉”“采空掉”或“两边都掉”，而不是系统全局写死一套。
		const bool bTimingMatched =
			DropEntry.RewardTiming == EAOHarvestRewardTiming::Both ||
			(DropEntry.RewardTiming == EAOHarvestRewardTiming::PerHit && !bWillDeplete) ||
			(DropEntry.RewardTiming == EAOHarvestRewardTiming::OnDepleted && bWillDeplete);

		if (!bTimingMatched)
		{
			continue;
		}

		const float FinalChance = FMath::Clamp(DropEntry.BaseDropChance + ToolTuning.RareDropChanceBonus, 0.0f, 1.0f);
		if (!FMath::IsNearlyEqual(FinalChance, 1.0f) && FMath::FRand() > FinalChance)
		{
			continue;
		}

		// YieldMultiplier 只放大最终数量，不回头改命中进度。
		// 这样“砍得快”和“产量高”可以被设计成两条独立调参轴。
		const int32 BaseCount = FMath::RandRange(DropEntry.MinCount, FMath::Max(DropEntry.MinCount, DropEntry.MaxCount));
		const int32 FinalCount = FMath::Max(0, FMath::FloorToInt(static_cast<float>(BaseCount) * FMath::Max(0.0f, ToolTuning.YieldMultiplier)));
		if (DropEntry.ItemId == INDEX_NONE || FinalCount <= 0)
		{
			continue;
		}

		FAOHarvestRewardEntry& RewardEntry = OutResult.RewardEntries.AddDefaulted_GetRef();
		RewardEntry.ItemId = DropEntry.ItemId;
		RewardEntry.Count = FinalCount;
		RewardEntry.RewardTiming = DropEntry.RewardTiming;
	}
}

