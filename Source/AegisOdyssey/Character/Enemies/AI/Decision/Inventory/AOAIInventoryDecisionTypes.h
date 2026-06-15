// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryUseTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AOAIInventoryDecisionTypes.generated.h"

class UAOInventoryComponent;

UENUM(BlueprintType)
enum class EAOAIInventoryActionCoordinationMode : uint8
{
	// 独占型库存动作。
	// 这类动作会被当成一个正式主链决策提交，通常需要单独进入对应的执行状态。
	Exclusive UMETA(DisplayName = "Exclusive"),

	// 并行型库存动作。
	// 这类动作只会在 TacticalState 打开 AdditiveInventoryWindow 时参与评估，用于在主战斗意图旁边穿插执行。
	Additive UMETA(DisplayName = "Additive")
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionCandidateFacts
{
	GENERATED_BODY()

	// 当前动作下命中的候选数量，不区分是否真的可执行。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 CandidateCount = 0;

	// 当前动作下是否至少命中过一个库存候选。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasAnyCandidate = false;

	// 当前动作下是否至少存在一个“现在就能执行”的候选。
	// 这是库存动作能否进入正式评估的硬前置条件之一。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasUsableCandidate = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryConcreteCandidateFacts
{
	GENERATED_BODY()

	// 运行时最终解析出的具体库存目标。
	// 它会指向实际的库存组件、槽位或快捷栏位，供执行层直接消费。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIResolvedInventoryUseTarget ResolvedTarget;

	// 该候选当前栈数量。
	// 用于让“数量越多越值得优先使用”之类的候选打分生效。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 StackCount = 0;

	// 如果候选物品是武器，这里记录该武器的 AI 攻击距离。
	// 非武器候选通常保持为 0。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float WeaponAIAttackRange = 0.0f;

	// 该具体候选是否适配当前战斗距离。
	// 例如当前离目标很远时，长枪或远程武器更可能命中这个条件。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bMatchesCurrentCombatDistance = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionFacts
{
	GENERATED_BODY()

	// 自身当前生命值比例，范围通常是 0~1。
	// 用于驱动治疗、保命换装等库存动作的欲望计算。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float HealthRatio = 1.0f;

	// 自身当前耐力值比例，范围通常是 0~1。
	// 用于驱动恢复类物品或切换低耗武器等库存动作。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StaminaRatio = 1.0f;

	// 最近受击强度比例。
	// 这是库存评估层观察到的“最近压力”，用于抬高防御或恢复类动作的倾向。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float RecentDamageRatio = 0.0f;

	// 当前是否存在有效目标。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasTarget = false;

	// 当前目标距离。
	// 只有在存在目标时这个值才有真实意义。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float TargetDistance = 0.0f;

	// 当前自己手上武器的 AI 攻击距离。
	// 常用于判断“现有武器是否适合当前战斗距离”。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float SelfAIAttackRange = 0.0f;

	// 当前武器是否已经适配当前战斗距离。
	// 这是武器切换类库存决策的重要输入。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bCurrentWeaponFitsCombatDistance = false;

	// 目标当前是否处于可被压制/可抓窗口期。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInCombatWindow = false;

	// 目标当前是否正在稳定战斗中。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetCombating = false;

	// 目标当前是否处于恢复期。
	// 一些高收益库存动作会偏好在这个时机插入。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInRecovery = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionCandidateDefinition
{
	GENERATED_BODY()

	// 候选标签。
	// 用于区分同一库存动作下的不同物品策略，例如“切剑”“切枪”“喝大药”。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag CandidateTag;

	// 具体使用指令。
	// 这是从“候选定义”落到“实际操作库存”的桥梁，运行时会靠它去匹配物品并执行。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIInventoryUseCommand UseCommand;

	// 候选基础分。
	// 在所有环境修正生效前，这个候选默认拥有的起始优先级。
	UPROPERTY(EditAnywhere, Category = "Config")
	float BaseScore = 0.0f;

	// 生命值越低，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredHealthDeficitWeight = 0.0f;

	// 耐力值越低，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredStaminaDeficitWeight = 0.0f;

	// 最近受击越重，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredRecentDamageWeight = 0.0f;

	// 当前越打不到目标，该候选额外增加多少分。
	// 常用于鼓励切换更合适攻击距离的武器。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredOutOfRangeWeight = 0.0f;

	// 目标处于 CombatWindow 时，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireCombatWindowWeight = 0.0f;

	// 目标处于 Recovery 时，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireTargetRecoveryWeight = 0.0f;

	// 当前武器不适合战斗距离时，该候选额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireNeedWeaponSwapWeight = 0.0f;

	// 该候选堆叠数量越多，额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredHighStackCountWeight = 0.0f;

	// 该候选如果天然适配当前战斗距离，额外增加多少分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredDistanceFitWeight = 0.0f;

	// 是否偏好更靠前的槽位。
	// 数值越大，后面的槽位会被扣分越明显。
	UPROPERTY(EditAnywhere, Category = "Config")
	float PreferLowerSlotIndexWeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryActionDefinition
{
	GENERATED_BODY()

	// 库存动作标签。
	// 这是动作级别的身份，例如“武器切换”“消耗品使用”，不是具体某个物品。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ActionTag;

	// 该库存动作是独占执行还是允许在主战斗意图旁并行穿插。
	UPROPERTY(EditAnywhere, Category = "Config")
	EAOAIInventoryActionCoordinationMode CoordinationMode = EAOAIInventoryActionCoordinationMode::Exclusive;

	// 是否要求当前必须有有效目标才能评估这个动作。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireTarget = false;

	// 只有当前主意图命中这些标签之一时，这个库存动作才允许参与评估。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer RequiredMainIntentTags;

	// 当前主意图命中这些标签之一时，这个库存动作会被直接屏蔽。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer BlockedMainIntentTags;

	// 当前战术标签必须全部满足，库存动作才允许参与评估。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer RequiredTacticalTags;

	// 当前战术标签只要命中任意一个，库存动作就会被直接屏蔽。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer BlockedTacticalTags;

	// 动作基础欲望。
	// 它是动作层打分的起点，不是最终候选分。
	UPROPERTY(EditAnywhere, Category = "Config")
	float BaseDesire = 0.0f;

	// 随着“距离上次执行已经过去多久”而逐渐恢复的额外欲望权重。
	UPROPERTY(EditAnywhere, Category = "Config")
	float CadenceWeight = 0.0f;

	// 节奏恢复周期。
	// 时间越接近或超过这个值，CadenceWeight 的贡献越完整。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CadenceSeconds = 1.0f;

	// 基于生命值比例的欲望修正曲线。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor HealthRatioFactor;

	// 基于耐力值比例的欲望修正曲线。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor StaminaRatioFactor;

	// 基于当前目标距离比值的欲望修正曲线。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor DistanceFactor;

	// 基于最近受击强度的欲望修正曲线。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor RecentDamageFactor;

	// 统计 RecentDamageFactor 时使用的回看窗口秒数。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RecentDamageWindowSeconds = 2.0f;

	// 基于属性区间的额外欲望修正。
	// 适合表达“血量低于 30% 时更想喝药”这一类规则。
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionAttributeIntervalFactor> AttributeIntervalFactors;

	// 基于目标状态标签的额外欲望修正。
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionTagScoreFactor> TargetStateTagFactors;

	// 执行后冷却秒数。
	// 仍处于冷却内时，最终动作分会乘以 CooldownPenaltyMultiplier。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownSeconds = 0.0f;

	// 处于冷却期间时，对动作分施加的乘法惩罚。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownPenaltyMultiplier = 1.0f;

	// 重复执行同一库存动作时，每次额外扣掉的分数。
	UPROPERTY(EditAnywhere, Category = "Config")
	float RepeatPenalty = 0.0f;

	// 与上一轮执行动作不同的时候，给予的切换奖励。
	UPROPERTY(EditAnywhere, Category = "Config")
	float SwitchBonus = 0.0f;

	// 当多个动作都没有正分时，用于选择 fallback 动作的权重。
	UPROPERTY(EditAnywhere, Category = "Config")
	float FallbackSelectionWeight = 0.0f;

	// 这个动作下可供运行时挑选的具体候选物品策略列表。
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIInventoryDecisionCandidateDefinition> CandidateDefinitions;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionRuntimeState
{
	GENERATED_BODY()

	// 这个库存动作在当前帧算出的动作欲望。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	// 这个库存动作在当前帧算出的最终动作分。
	// 这是动作层的分，不是具体某个候选物品的分。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	// 这个库存动作上一次被正式执行的时间。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastExecutedTime = -1.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionResult
{
	GENERATED_BODY()

	// 当前是否已经得到一个有效的库存动作结果。
	// 只有它为 true，这份结果才允许被提交到决策组件和执行层。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasAction = false;

	// 最终选中的库存动作标签。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag ActionTag;

	// 在这个动作下，最终胜出的具体候选策略标签。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag CandidateTag;

	// 该结果在执行时的协同模式。
	// 执行层可以据此判断它是否应该独占状态，还是允许并行插入。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	EAOAIInventoryActionCoordinationMode CoordinationMode = EAOAIInventoryActionCoordinationMode::Exclusive;

	// 动作层本帧欲望快照。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	// 动作层本帧最终分数快照。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	// 执行层真正要使用的库存操作指令。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIInventoryUseCommand UseCommand;

	// 当前是否已经在评估阶段解析出具体库存目标。
	// 为 true 时，执行层可以直接使用 ResolvedTarget，不必再次模糊检索。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasResolvedTarget = false;

	// 评估阶段已经锁定的具体库存目标。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIResolvedInventoryUseTarget ResolvedTarget;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionExecutionRecord
{
	GENERATED_BODY()

	// 最近一次正式执行的库存动作标签。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag LastExecutedActionTag;

	// 连续重复执行同一库存动作的次数。
	// 用于给动作层施加重复惩罚。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 RepeatedActionCount = 0;
};
