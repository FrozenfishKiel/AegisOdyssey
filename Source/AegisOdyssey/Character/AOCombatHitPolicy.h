// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "AOCombatHitPolicy.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EAOCombatHitPolicyType : uint8
{
	// 整次攻击生命周期里，同一目标只允许结算一次。
	SingleHit UMETA(DisplayName = "Single Hit"),

	// 攻击被拆成多个明确段位，每一段各自允许命中一次。
	SegmentedHit UMETA(DisplayName = "Segmented Hit"),

	// 允许重复命中，但相邻两次命中必须间隔一段时间。
	IntervalRepeat UMETA(DisplayName = "Interval Repeat"),

	// 允许持续周期命中，常见于持续区域伤害或长驻打击体。
	PeriodicRepeat UMETA(DisplayName = "Periodic Repeat")
};

// 命中策略配置。
// 这一层描述的是“同一攻击对同一目标，多久 / 以什么规则允许再次成立命中”。
USTRUCT(BlueprintType)
struct FAOCombatHitPolicy
{
	GENERATED_BODY()

	// 单次命中：整次攻击生命周期里，同一目标只结算一次。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Policy")
	EAOCombatHitPolicyType PolicyType = EAOCombatHitPolicyType::SingleHit;

	// 间隔重复/周期持续命中时，同一目标再次结算所需的最小时间间隔。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Policy", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "PolicyType == EAOCombatHitPolicyType::IntervalRepeat || PolicyType == EAOCombatHitPolicyType::PeriodicRepeat", EditConditionHides))
	float RepeatIntervalSeconds = 0.2f;
};

// 运行时命中跟踪器。
// 每个攻击实例维护自己的跟踪状态，用来判断当前这次接触是否允许真正进入战斗结算入口。
struct FAOCombatHitTracker
{
	using FTargetKey = TObjectKey<AActor>;

	// 尝试记录一次命中。
	// 返回 true 表示本次命中允许继续进入战斗系统；返回 false 表示被命中策略拦截掉。
	bool TryMarkHit(AActor* TargetActor, const FAOCombatHitPolicy& HitPolicy, FName SegmentKey, double CurrentTimeSeconds)
	{
		if (TargetActor == nullptr)
		{
			return false;
		}

		const FTargetKey TargetKey(TargetActor);
		switch (HitPolicy.PolicyType)
		{
		case EAOCombatHitPolicyType::SingleHit:
			// SingleHit 只记“这个目标是否已经被当前攻击打中过”。
			if (SingleHitTargets.Contains(TargetKey))
			{
				return false;
			}

			SingleHitTargets.Add(TargetKey);
			return true;

		case EAOCombatHitPolicyType::SegmentedHit:
			{
				// SegmentedHit 的关键是“每个段位自己维护一份命中记录”，
				// 这样同一目标可以在第 1 段、第 2 段分别各中一次。
				TSet<FTargetKey>& SegmentTargets = SegmentedHitTargets.FindOrAdd(SegmentKey);
				if (SegmentTargets.Contains(TargetKey))
				{
					return false;
				}

				SegmentTargets.Add(TargetKey);
				return true;
			}

		case EAOCombatHitPolicyType::IntervalRepeat:
			return TryMarkTimedHit(IntervalRepeatHitTimes, TargetKey, HitPolicy.RepeatIntervalSeconds, CurrentTimeSeconds);

		case EAOCombatHitPolicyType::PeriodicRepeat:
			return TryMarkTimedHit(PeriodicRepeatHitTimes, TargetKey, HitPolicy.RepeatIntervalSeconds, CurrentTimeSeconds);

		default:
			return true;
		}
	}

	void Reset()
	{
		// 攻击实例结束时应整体清空全部命中记录。
		SingleHitTargets.Reset();
		SegmentedHitTargets.Reset();
		IntervalRepeatHitTimes.Reset();
		PeriodicRepeatHitTimes.Reset();
	}

	void ResetSegment(FName SegmentKey)
	{
		// 某个分段攻击结束后，只清它自己的命中记录。
		SegmentedHitTargets.Remove(SegmentKey);
	}

private:
	static bool TryMarkTimedHit(TMap<FTargetKey, double>& TargetHitTimes, const FTargetKey& TargetKey, float IntervalSeconds, double CurrentTimeSeconds)
	{
		// 时间型重复命中的统一判定 helper。
		// 无论是 IntervalRepeat 还是 PeriodicRepeat，当前都先复用这一套最小时间间隔判断。
		const double RequiredInterval = FMath::Max(0.0, static_cast<double>(IntervalSeconds));
		if (const double* LastHitTime = TargetHitTimes.Find(TargetKey))
		{
			if ((CurrentTimeSeconds - *LastHitTime) < RequiredInterval)
			{
				return false;
			}
		}

		TargetHitTimes.Add(TargetKey, CurrentTimeSeconds);
		return true;
	}

private:
	TSet<FTargetKey> SingleHitTargets;
	TMap<FName, TSet<FTargetKey>> SegmentedHitTargets;
	TMap<FTargetKey, double> IntervalRepeatHitTimes;
	TMap<FTargetKey, double> PeriodicRepeatHitTimes;
};

// 统一生成分段命中键的工具。
// 这样不同攻击实现不需要各自拼字符串，统一复用同一套 SegmentKey 生成规则。
struct FAOCombatHitPolicyKeyBuilder
{
	static FName BuildSegmentKey(FName ExplicitSegmentKey, const FGameplayTag& AttackTag, int32 SegmentIndex = INDEX_NONE)
	{
		FName BaseSegmentKey = ExplicitSegmentKey;
		if (BaseSegmentKey == NAME_None && AttackTag.IsValid())
		{
			BaseSegmentKey = AttackTag.GetTagName();
		}

		if (SegmentIndex == INDEX_NONE)
		{
			return BaseSegmentKey;
		}

		const FString BaseSegmentString = BaseSegmentKey != NAME_None
			? BaseSegmentKey.ToString()
			: FString(TEXT("Segment"));
		return FName(*FString::Printf(TEXT("%s_%d"), *BaseSegmentString, SegmentIndex));
	}

	static FName BuildAttackWindowKey(const FGameplayTag& AttackTag, uint32 AttackActivationId, int32 WindowIndex)
	{
		const FString AttackKey = AttackTag.IsValid()
			? AttackTag.ToString()
			: FString(TEXT("Attack"));
		return FName(*FString::Printf(TEXT("%s_%u_%d"), *AttackKey, AttackActivationId, WindowIndex));
	}
};
