#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AOCombatResultMessage.h"
#include "AOCombatFeedbackViewData.generated.h"

// HUD / 飘字 / 提示层消费的轻量战斗反馈观察数据。
// 这里只消费统一结果，不重新解释结算。
USTRUCT(BlueprintType)
struct FAOCombatFeedbackViewData
{
	GENERATED_BODY()

	// 本地反馈序号。
	// 主要用于保证瞬时表现消费时有稳定顺序，不依赖外部自己再拼时间戳。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 SequenceId = 0;

	// 这条反馈进入 HUD 数据层时的世界时间。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float EventWorldTimeSeconds = 0.0f;

	// 以下字段基本都是统一战斗结果的平铺镜像，
	// 这样蓝图和 UMG 可以直接消费，不用再拿着 Message 结构层层拆。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAOCombatResultType ResultType = EAOCombatResultType::None;

	// 结算层推荐的提示字类别。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAOCombatFloatingTextType FloatingTextType = EAOCombatFloatingTextType::None;

	// 这条反馈默认是否建议表现层显示提示字。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bShouldDisplayFloatingText = false;

	// 这组字段是“本地玩家视角下的路由语义”。
	// 战斗系统先给出统一真相，再由 HUD 桥接层把它解释成“和本地玩家有没有关系、建议进哪个表现通道”。
	// UI 直接读这些结果，不要自己再从 Instigator / Target 反推一遍。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsLocalRelevant = false;

	// 本地玩家是否是这次结果的发起方。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsLocalInstigator = false;

	// 本地玩家是否是这次结果的承受方。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsLocalTarget = false;

	// 这条反馈是否应该进入 HUD 常规反馈通道。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bShouldEnqueueForHUD = false;

	// 这条反馈是否应该进入世界飘字通道。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bShouldEnqueueForWorldFloatingText = false;

	// 是否属于需要重点保留的强交互反馈。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsImportantCombatFeedback = false;

	// 以下字段是统一结果中的关键战斗真相镜像。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWasBlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWasParried = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitInvulnerability = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bTargetBroken = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float HealthDamage = 0.0f;

	// 本次反馈造成或代表的韧性改变量。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float StaminaDamage = 0.0f;

	// 本次反馈造成或代表的体力消耗。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float VigorCost = 0.0f;

	// 攻击来源语义。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag AttackTag;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag SkillTag;

	// 武器来源标签。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag WeaponTag;

	// 伤害类型标签集合。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer DamageTypeTags;

	// 推荐 GameplayCue 标签。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag CueTag;

	// 空间信息。
	// 世界飘字、受击特效、提示锚点这类表现层可以直接用。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FHitResult HitResult;

	// 归属对象信息。
	// HUD / 表现层如果确实需要按对象做进一步过滤，也应该优先读取这里，而不是自己回头找旧上下文。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> Target = nullptr;

	// 真正造成这次命中的对象，例如武器、投射体、技能生成物。
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> EffectCauser = nullptr;

	void ApplyCombatResult(
		const FAOCombatResultMessage& Message,
		int32 InSequenceId,
		float InEventWorldTimeSeconds,
		const AActor* LocalPlayerActor = nullptr)
	{
		// 先平铺统一结果消息本身。
		SequenceId = InSequenceId;
		EventWorldTimeSeconds = InEventWorldTimeSeconds;
		ResultType = Message.ResultType;
		FloatingTextType = Message.FloatingTextType;
		bShouldDisplayFloatingText = Message.bShouldDisplayFloatingText;
		bIsCritical = Message.bIsCritical;
		bWasBlocked = Message.bWasBlocked;
		bWasParried = Message.bWasParried;
		bHitInvulnerability = Message.bHitInvulnerability;
		bTargetBroken = Message.bTargetBroken;
		HealthDamage = Message.HealthDamage;
		StaminaDamage = Message.StaminaDamage;
		VigorCost = Message.VigorCost;
		AttackTag = Message.AttackTag;
		SkillTag = Message.SkillTag;
		WeaponTag = Message.WeaponTag;
		DamageTypeTags = Message.DamageTypeTags;
		CueTag = Message.CueTag;
		HitResult = Message.HitResult;
		Instigator = Message.Instigator;
		Target = Message.Target;
		EffectCauser = Message.EffectCauser;

		// 再在 HUD 桥接层视角下补齐“本地路由语义”。
		ApplyLocalRouting(LocalPlayerActor, Message.EffectCauser);
	}

private:
	// 判断 CandidateActor 是否就是 ExpectedActor，或者是否属于它的 Owner 链。
	// 这样做是为了兼容武器、投射体、技能生成物这类“命中载体和本体不是同一个 Actor”的情况。
	static bool IsSameActorOrOwnedByActor(const AActor* CandidateActor, const AActor* ExpectedActor)
	{
		if (CandidateActor == nullptr || ExpectedActor == nullptr)
		{
			return false;
		}

		for (const AActor* CurrentActor = CandidateActor; CurrentActor != nullptr; CurrentActor = CurrentActor->GetOwner())
		{
			if (CurrentActor == ExpectedActor)
			{
				return true;
			}
		}

		return false;
	}

	void ApplyLocalRouting(const AActor* LocalPlayerActor, const AActor* EffectCauserActor)
	{
		// 这里统一把一条“全局战斗结果”翻译成“本地玩家视角下怎么路由”。
		// 后续无论 HUD、飘字还是别的局部表现层，都不要再各自复制一套相同规则。
		bIsLocalInstigator =
			IsSameActorOrOwnedByActor(Instigator.Get(), LocalPlayerActor)
			|| IsSameActorOrOwnedByActor(EffectCauserActor, LocalPlayerActor);
		bIsLocalTarget = IsSameActorOrOwnedByActor(Target.Get(), LocalPlayerActor);
		bIsLocalRelevant = bIsLocalInstigator || bIsLocalTarget;

		// 弹反和破韧属于用户明确要求保留的“强交互结果”。
		// 只要和本地玩家相关，就应该稳定保留下来给 HUD / 提示层消费。
		bIsImportantCombatFeedback =
			ResultType == EAOCombatResultType::Parry
			|| ResultType == EAOCombatResultType::Broken;
		bShouldEnqueueForHUD = bIsLocalRelevant;

		// 世界飘字当前只给“本地玩家打出去的结果”。
		// 完全格挡和无敌命中虽然也会进入 HUD 数据，但不会默认进入世界飘字通道。
		bShouldEnqueueForWorldFloatingText = bIsLocalInstigator && bShouldDisplayFloatingText;
	}
};
