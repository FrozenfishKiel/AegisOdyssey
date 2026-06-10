---
title: AI Combat Decision Tuning
tags:
  - knowledge
  - ai
  - tuning
  - combat-decision
aliases:
  - AI Combat Decision Tuning
  - AI战斗决策调参与算分说明
---

# AI 战斗决策调参与算分说明

更新时间：2026-05-19  
适用范围：当前 `UAOAIDecisionComponent` 的算分逻辑、参数语义、调参顺序。  
不适用范围：未来未落地的新公式、具体某个敌人最终手感值。

## 1. 当前总流程

当前每轮评估的真实流程是：

1. `STE_UpdateCurrentTarget` 先输出目标事实
2. `STE_UpdateCombatDecision` 把事实喂给 `UAOAIDecisionComponent`
3. `UAOAIDecisionComponent::EvaluateIntent()` 遍历每个 `IntentDefinition`
4. 分别计算 `Desire` 和 `Score`
5. `Score` 最高者成为 `SelectedIntentTag`
6. 如果最高分 `<= 0`，则走 `ResolveFallbackIntentTag()`

## 2. 当前硬条件

当前 `CanEvaluateIntent()` 只保留一个硬条件：

- `bRequireTarget = true` 且当前没有目标时，该意图不参与评估

当前没有把“距离不够”当作硬排除条件。

## 3. 当前真实输入

当前参与决策的输入已经不只距离。

### 3.1 目标和距离

- `CurrentTarget`
- `TargetDistance`
- `SelfAIAttackRange`
- `TargetAIAttackRange`
- `bHasTarget`
- `bIsInAttackRange`

### 3.2 最近受伤

- `RecentDamageRatio`
- `RecentDamageWindowSeconds`
- `RecentDamageFactor`

### 3.3 目标战斗状态标签

通过 `TargetStateTagFactors` 可以直接对目标身上的战斗状态标签加减分。

### 3.4 自身属性区间

通过 `AttributeIntervalFactors` 可以基于属性归一化结果只在特定区间内加分。

## 4. 当前 Desire 计算

当前 `ComputeIntentDesire()` 的核心结构是：

```text
Desire = BaseDesire
       + Cadence 部分
       + DistanceFactor
       + RecentDamageFactor
       + AttributeIntervalFactors 累加
       + TargetStateTagFactors 累加
```

这里最重要的几点是：

1. `Desire` 不是最终选择结果，只是该意图本轮“有多想做”。
2. 距离、最近受伤、属性区间、目标标签都先进入 `Desire`。
3. 当前 `Desire` 的来源已经明显比早期阶段文档更丰富。

## 5. 当前 Score 计算

当前 `ComputeIntentScore()` 的结构是：

```text
Score = Desire

如果仍在冷却期：
    Score *= CooldownPenaltyMultiplier

如果上次执行的就是当前意图：
    Score -= RepeatPenalty * RepeatedIntentCount
否则如果上次执行的是别的意图：
    Score += SwitchBonus

Score = max(0, Score)
```

这意味着：

- `CooldownPenaltyMultiplier` 是乘法
- `RepeatPenalty` 是减法
- `SwitchBonus` 是加法
- 最终分数不允许小于 `0`

## 6. 保底选择逻辑

当前实现存在显式保底逻辑：

- 正常评估阶段先比 `Score`
- 如果所有分数最终都 `<= 0`
- 则由 `FallbackSelectionWeight` 决定保底意图

并且：

- 正常同分比较时，也会优先取 `FallbackSelectionWeight` 更高者

所以 `FallbackSelectionWeight` 不是主评分参数，但也不是完全只在极端情况下才有意义。

## 7. 距离语义

当前距离不是直接用世界距离裸算，而是先归一化：

```text
DistanceRatio = TargetDistance / max(1.0, SelfAIAttackRange)
```

这意味着：

- `1.0` 表示大约处于自己当前武器的 AI 攻击距离附近
- `< 1.0` 表示更近
- `> 1.0` 表示更远

所以近战和远程可以共用同一套距离响应结构，只是武器 `AIAttackRange` 不同。

## 8. ResponseCurveFactor 统一语义

当前 `DistanceFactor` 和 `RecentDamageFactor` 共用同一套响应结构：

- `bEnabled`
- `Weight`
- `InputScale`
- `InputBias`
- `bClampInput`
- `InputClampRange`
- `ResponseCurve`

统一公式是：

```text
EvaluatedInput = RawInput * InputScale + InputBias
可选 Clamp
CurveValue = ResponseCurve(EvaluatedInput)
Contribution = CurveValue * Weight
```

调参时要严格区分：

- 曲线负责“趋势形状”
- `Weight` 负责“影响力度”
- `InputScale / InputBias` 负责“输入空间变形”

## 9. 当前最重要参数语义

### 9.1 `BaseDesire`

表示意图的先天底色，不表示冷却，也不表示距离。

### 9.2 `CadenceWeight` / `CadenceSeconds`

表示这个意图“隔多久又会重新想做”的节奏恢复。

### 9.3 `CooldownSeconds` / `CooldownPenaltyMultiplier`

表示冷却期内是否压分，以及压得多狠。

### 9.4 `RepeatPenalty`

表示连续重复执行同一意图时，每次额外扣多少。

### 9.5 `SwitchBonus`

表示这次若和上次执行意图不同，额外奖励多少切换分。

### 9.6 `FallbackSelectionWeight`

表示当所有意图都没分，或者同分时，系统更偏向谁。

## 10. 推荐调参顺序

当前推荐顺序不是同时乱调所有参数，而是：

1. 先确认武器 `AIAttackRange`
2. 再画 `DistanceFactor` 的曲线形状
3. 再用 `Weight` 调强弱
4. 再调 `Cadence`、`Cooldown`、`RepeatPenalty`、`SwitchBonus`
5. 最后才处理复杂的最近受伤因子、属性区间因子、目标标签因子

## 11. 当前最常见误区

### 11.1 趋势不对却只改 `Weight`

如果曲线形状错了，单调力度只会把错误放大。

### 11.2 武器攻击距离不准却怪曲线不准

当前距离语义全部依赖 `AIAttackRange` 的归一化。

### 11.3 想做冷却却只改 `RepeatPenalty`

`RepeatPenalty` 处理的是连用次数，不是冷却窗口。

### 11.4 想让行为更活就把 `SwitchBonus` 拉得很高

这样容易让动作过度抖动，尤其在评分接近时更明显。

## 12. 当前调试入口

调当前分数时优先看：

- `SelectedIntentTag`
- `SelectedIntentDesire`
- `SelectedIntentScore`
- `ObservedIntentDesire`
- `ObservedIntentScore`
- `LastExecutedIntentTag`
- `RepeatedIntentCount`

如果要看某个没赢的意图，优先填 `ObservedIntentTag`，不要只盯主意图。

## 13. 第一轮提炼来源

这份文档主要从：

- `Notice/HistoryNotice/AI战斗决策参数调参与计算说明.md`

提炼而来，并已和当前 `AOAIDecisionComponent.*`、`AOAIDecisionTypes.h`、`STE_UpdateCombatDecision.cpp` 核对。  
与当前代码不一致的历史说法，已经转移到 [[AI 已知问题与历史偏差]]。
