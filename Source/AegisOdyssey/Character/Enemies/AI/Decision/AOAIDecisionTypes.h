#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GameplayTagContainer.h"
#include "AOAIDecisionTypes.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FAOAIDecisionResponseCurveFactor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否启用这个响应因子。关闭后，这一项完全不参与计算。"))
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "该因子最终加到意图分数里的权重。最终贡献 = 曲线输出或直接输入结果 × Weight。\nWeight > 0：顺着当前结果方向加分。\nWeight < 0：把当前结果方向反过来作为减分。\n|Weight| 越大，影响越强。\nWeight = 0：这一项没有实际作用。"))
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "输入缩放。在送进曲线前，会先按“原始输入 × InputScale + InputBias”处理。\nInputScale > 1：放大输入变化。\n0 到 1：压缩输入变化。\nInputScale < 0：把输入方向翻转。"))
	float InputScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "输入偏移。它会在缩放之后，把整体输入向左或向右平移。\nInputBias > 0：整体抬高输入。\nInputBias < 0：整体压低输入。"))
	float InputBias = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否在曲线采样前限制输入范围。开启后，输入会被限制在 InputClampRange 内。"))
	bool bClampInput = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "输入裁剪区间。只有在 bClampInput 为 True 时生效。\nX 越小，下限越低。\nY 越大，上限越高。", EditCondition = "bClampInput", ClampMin = "0.0", UIMin = "0.0"))
	FVector2D InputClampRange = FVector2D(0.0f, 2.0f);

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "输入到输出的响应曲线。横轴是处理后的输入，纵轴是曲线输出。\n纵轴 > 0：会推高意图。\n纵轴 < 0：会压低意图。\n若这里没有配置有效曲线，系统会直接使用处理后的输入值继续计算。"))
	FRuntimeFloatCurve ResponseCurve;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionTagScoreFactor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "需要检查的目标状态标签。命中后会把 ScoreDelta 加到该意图的 Desire 中。"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否要求精确匹配。\nTrue：只接受完全相同的标签。\nFalse：允许父子标签匹配。"))
	bool bExactMatch = false;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "命中这个标签后的分数变化。\nScoreDelta > 0：提高该意图的 Desire。\nScoreDelta < 0：压低该意图的 Desire。\n|ScoreDelta| 越大，影响越强。\nScoreDelta = 0：命中后也不会产生实际作用。"))
	float ScoreDelta = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionAttributeIntervalFactor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否启用这个属性区间因子。关闭后，这一项完全不参与计算。"))
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "属性分子，也就是公式里的当前值。最终会先读取这个属性的运行时数值。\n例如：当前生命值、当前耐力、当前攻击力。\n如果这里没有配置有效属性，这一项直接记 0 分。"))
	FGameplayAttribute NumeratorAttribute;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "属性分母，也就是公式里的归一化基准值。\n例如：最大生命值、最大耐力。\n如果这里没有配置，或者运行时读出来小于等于 0，就会自动改用 ManualDenominator。"))
	FGameplayAttribute DenominatorAttribute;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "手动分母。只有在 DenominatorAttribute 没有配置、无效，或者运行时值小于等于 0 时才会使用它。\n如果这里填了小于等于 0 的值，系统会回退到默认分母 1。", ClampMin = "0.0001", UIMin = "0.0001"))
	float ManualDenominator = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "区间左边界，也就是较小的一端。\n只有当“分子 / 分母”的归一化结果大于等于这个值时，这一项才可能开始得分。"))
	float RangeMin = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "区间右边界，也就是较大的一端。\n只有当“分子 / 分母”的归一化结果小于等于这个值时，这一项才可能开始得分。"))
	float RangeMax = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "区间内的得分方向。\nTrue：越靠近 RangeMin，得分越高；越靠近 RangeMax，得分越低。\nFalse：反过来，越靠近 RangeMax，得分越高；越靠近 RangeMin，得分越低。"))
	bool bHigherScoreNearMin = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "区间内的响应曲线，可选。\n它的横轴不是原始属性值，而是“已经按方向换算好的接近度”。\n输入 1：表示最接近你想偏爱的那一侧。\n输入 0：表示最远离你想偏爱的那一侧。\n如果这里没配有效曲线，系统会直接使用线性的接近度继续计算。"))
	FRuntimeFloatCurve ResponseCurve;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "这一项最终加到意图 Desire 里的权重。最终贡献 = 区间内接近度（或曲线输出）× Weight。\nWeight > 0：顺着当前方向加分。\nWeight < 0：把当前方向反过来作为减分。\n|Weight| 越大，影响越强。\nWeight = 0：这一项即使启用也不会产生实际影响。"))
	float Weight = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionCombatFacts
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前用于决策的目标。为空表示当前没有有效目标。"))
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前目标与自身的距离。数值越大，表示离目标越远；数值越小，表示离目标越近。"))
	float TargetDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "自身当前武器或战斗方式对应的 AI 攻击距离。数值越大，表示这个角色理论上愿意在更远距离下把攻击视为有效。"))
	float SelfAIAttackRange = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "目标当前武器或战斗方式对应的 AI 攻击距离。数值越大，表示目标理论上可以在更远距离上威胁自身。"))
	float TargetAIAttackRange = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "调试显示用的最近受伤比例。数值越大，表示在最近统计窗口内掉血越多；数值越小，表示最近越安全。\n如果任一意图把 RecentDamageWindowSeconds 设为 0，这里会显示累计历史受伤比例。真正计算时，仍会按每个意图自己的时间窗单独计算。"))
	float RecentDamageRatio = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前是否存在有效目标。True 表示有目标，False 表示无目标。"))
	bool bHasTarget = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前是否已经进入自身的 AI 攻击距离。这个值现在只用于事实展示，不再直接作为一票否决条件。"))
	bool bIsInAttackRange = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "目标是否处于前摇或准备阶段。True 表示目标正在准备动作。"))
	bool bTargetInPreparation = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "目标是否处于可交互的战斗窗口阶段。True 表示当前是一个可利用的窗口。"))
	bool bTargetInCombatWindow = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "目标是否处于正式攻击过程中。True 表示目标正在执行主要攻击动作。"))
	bool bTargetCombating = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "目标是否处于后摇或恢复阶段。True 表示目标当前更容易被反制或压制。"))
	bool bTargetInRecovery = false;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionTacticalState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前这一帧决策最终选中的主战斗意图。"))
	FGameplayTag CurrentMainIntentTag;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "由战斗事实和当前主意图汇总出来的决策侧战术标签。库存动作的协同行为规则会匹配这些标签。"))
	FGameplayTagContainer TacticalTags;

	UPROPERTY(VisibleAnywhere, Category = "Runtime", meta = (ToolTip = "当前这一帧主行为是否给并行附着型库存动作留出了执行窗口。"))
	bool bHasAdditiveInventoryWindow = false;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionQueueItem
{
	GENERATED_BODY()

	// 正式排入统一提交队列的决策标签。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag DecisionTag;

	// 产生该队列条目的源标签。
	// 主战斗意图条目通常记录原始 IntentTag，库存条目通常记录 Inventory ActionTag。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag SourceTag;

	// 条目进入队列时的世界时间。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float EnqueueWorldTimeSeconds = -1.0f;

	int32 PayloadId = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionIntentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "这个配置项对应的意图标签。"))
	FGameplayTag IntentTag;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否要求必须存在有效目标。\nTrue：无目标时，这个意图不参与计算。\nFalse：即使没有目标，这个意图仍可参与计算。"))
	bool bRequireTarget = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "基础欲望值。其他因子都还没加上之前，这个意图先从这里起步。\nBaseDesire > 0：天然更偏向这个意图。\nBaseDesire < 0：天然更排斥这个意图。\n|BaseDesire| 越大，先天倾向越强。"))
	float BaseDesire = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "节奏权重。距离上次执行越久，这部分影响越接近满值。\nCadenceWeight > 0：越久没用越想用。\nCadenceWeight < 0：越久没用越不想用。\n|CadenceWeight| 越大，节奏影响越强。"))
	float CadenceWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "节奏时间，单位秒。系统会先算 TimeSinceLastExecution / CadenceSeconds，再限制到 0 到 1，最后乘以 CadenceWeight。\nCadenceSeconds 越大，节奏影响爬满越慢。\nCadenceSeconds 越小，节奏影响爬满越快。", ClampMin = "0.0", UIMin = "0.0"))
	float CadenceSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "距离因子。原始输入通常是 TargetDistance / SelfAIAttackRange。\n输入 = 1：目标正好在自身攻击距离附近。\n输入 < 1：比攻击距离更近。\n输入 > 1：比攻击距离更远。"))
	FAOAIDecisionResponseCurveFactor DistanceFactor;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "属性区间因子列表。每一项都会先算出“分子 / 分母”的归一化结果，再判断是否落在自己配置的区间内；只有落在区间内才会得分。最后所有成员的结果会直接累加到这个意图的 Desire 上。"))
	TArray<FAOAIDecisionAttributeIntervalFactor> AttributeIntervalFactors;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "最近受伤因子。原始输入是 RecentDamageRatio，也就是最近时间窗内累计掉血 / 最大血量。\n输入越大：最近掉血越猛。\n输入越小：最近越安全。"))
	FAOAIDecisionResponseCurveFactor RecentDamageFactor;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "最近受伤统计时间窗，单位秒。只影响 RecentDamageFactor 的输入。\n数值越大，统计范围越长，结果越平滑。\n数值越小，统计越偏向最近的瞬时受伤。\n小于等于 0：不限制时间窗，会把历史累计受伤都算进去。", ClampMin = "0.0", UIMin = "0.0"))
	float RecentDamageWindowSeconds = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "目标身上的某些状态标签，可以直接给这个意图额外加分或减分。"))
	TArray<FAOAIDecisionTagScoreFactor> TargetStateTagFactors;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "冷却时间，单位秒。在这个时间内再次尝试同一个意图时，会乘上冷却惩罚。\n数值越大，冷却惩罚持续越久。", ClampMin = "0.0", UIMin = "0.0"))
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "冷却惩罚倍率。只会在仍处于冷却期时生效，算法是 Score *= CooldownPenaltyMultiplier。\n0 到 1：冷却期内降分，越接近 0 压制越强。\n= 1：冷却期内不额外惩罚。\n> 1：冷却期内反而强化这个意图。", ClampMin = "0.0", UIMin = "0.0"))
	float CooldownPenaltyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "重复惩罚。算法是 Score -= RepeatPenalty × RepeatedIntentCount。\nRepeatPenalty > 0：越连用越扣分。\nRepeatPenalty < 0：越连用越加分。\n|RepeatPenalty| 越大，连续使用带来的倾向变化越强。"))
	float RepeatPenalty = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "切换奖励。算法是：如果这次选择的不是上一次执行的意图，则 Score += SwitchBonus。\nSwitchBonus > 0：鼓励切换。\nSwitchBonus < 0：惩罚切换，鼓励连续沿用。"))
	float SwitchBonus = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "兜底选择权重。它不直接加到 Score 上，只在所有意图都没分，或者分数非常接近时，决定系统更偏向谁。\n数值越大，越容易在兜底或平分时被选中。"))
	float FallbackSelectionWeight = 0.0f;
};
