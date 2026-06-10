---
title: AI Project Map
tags:
  - knowledge
  - ai
  - project-map
aliases:
  - AI Project Map
  - AI项目地图
---

# AI 项目地图

更新时间：2026-05-19  
适用范围：当前项目中基于 `UAOAIDecisionComponent + StateTree` 的战斗决策链路。  
不适用范围：具体某棵 `.uasset` StateTree 资源的编辑器排布、未来尚未落地的新意图方案、历史阶段性假设。

## 1. 这份文档解决什么问题

这份文档不解释某个参数怎么调，也不复述某一篇历史笔记。

它只回答四个问题：

1. 当前 AI 战斗决策的真实运行时结构是什么。
2. 决策事实从哪里来，分数在哪里算，StateTree 消费什么。
3. 如果后续要扩新意图，应该沿哪条链路扩。
4. 如果表现异常，先查哪些代码入口。

## 1.1 当前文档入口怎么读

如果只是想按当前实现快速接手，建议优先顺序固定为：

1. [[AI 项目地图]]
2. [[AI 战斗决策调参与算分说明]]
3. [[AI 决策已锁定设计]]
4. [[AI 已知问题与历史偏差]]
5. [[AI 战斗输入旋转与翻滚执行链]]
6. [[AI Reposition与Patrol框架]]
7. [[任务卡与短回合排查协议]]

其中：

- `AI当前进度与新会话交接说明.md` 更接近阶段性交接文档。
- `AI战斗决策系统现行文档说明.md` 更接近历史导航文档。
- 它们对“先看什么”的整理仍然有价值，但其中的稳定结论现在已经拆进本知识包，不应继续当成最终规格正文。

## 2. 当前真实结构

当前战斗 AI 已经不是“Attack / Strafe 两个 if 加随机子节点”的临时结构。

当前真实结构是三层：

1. 事实层  
   负责整理目标、距离、攻击距离、受伤窗口、目标战斗标签等事实。
2. 决策层  
   由 `UAOAIDecisionComponent` 持有运行时状态，并根据 `IntentDefinitions` 计算每个意图的 `Desire` 和 `Score`。
3. 执行层  
   `StateTree` 只消费决策结果，用条件进入状态，用任务回写“本次已执行哪个意图”。

## 3. 运行时主链路

### 3.1 目标事实入口

先看：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.*`

当前 `STE_UpdateCurrentTarget` 每帧负责输出：

- `CurrentTarget`
- `DistanceToTarget`
- `bIsInAttackRange`
- `bHasTarget`

其中：

- `CurrentTarget` 来自 `AAOAIPlayerBotController::GetCurrentTarget()`
- `DistanceToTarget` 是 Pawn 到目标的真实世界距离
- `bIsInAttackRange` 是 `Distance <= 当前武器 AIAttackRange`

### 3.2 决策入口

再看：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.*`

`STE_UpdateCombatDecision` 的职责是：

1. 把 `UpdateCurrentTarget` 的输出喂给 `UAOAIDecisionComponent::UpdateCombatFacts()`
2. 调用 `EvaluateIntent()`
3. 把结果同步回 StateTree 输出

当前同步出的核心结果包括：

- `SelectedIntentTag`
- `SelectedIntentDesire`
- `SelectedIntentScore`
- `ObservedIntentDesire`
- `ObservedIntentScore`
- `LastExecutedIntentTag`
- `RepeatedIntentCount`

### 3.3 执行层入口

最后看：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.*`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.*`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.*`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.*`

这些节点的边界已经比较清楚：

- `IntentMatches`：只判断当前主意图标签，不自己算分
- `ValueInRange`：只读取意图 `Desire / Score` 或 `RepeatedIntentCount`
- `CommitAIDecisionIntent`：状态进入时回写“本次开始执行的是哪个意图”
- `ResetAIDecisionState`：显式清掉上一轮战斗记忆

## 4. 运行时真相存放在哪里

### 4.1 事实层真相

事实层真相主要在 `FAOAIDecisionCombatFacts`：

- 当前目标
- 当前目标距离
- 自己的 AI 攻击距离
- 目标的 AI 攻击距离
- 最近受伤比例
- 是否在攻击距离内
- 目标是否处于 Preparation / CombatWindow / Combating / Recovery

定义位置：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`

### 4.2 决策运行时真相

决策运行时真相主要在 `UAOAIDecisionComponent`：

- `IntentRuntimeStates`
- `SelectedIntentTag`
- `LastExecutedIntentTag`
- `RepeatedIntentCount`
- `PendingActionDirection`

这里才是当前每个意图真实的 `Desire / Score / LastExecutedTime` 持有者。

### 4.3 配置真相

当前配置真相在 `UAOAIDecisionProfile::IntentDefinitions`：

- `DecisionProfile` 是 `UPrimaryDataAsset`
- `IntentDefinitions` 是每个角色真正参与评估的意图表

定义位置：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h`

## 5. 当前扩展路径

如果要增加新的战斗意图，正确顺序是：

1. 先新增 `GameplayTag`
2. 再在 `DecisionProfile` 里新增一条 `IntentDefinition`
3. 再在 StateTree 里接对应状态和 `IntentMatches`
4. 状态开始执行时接 `CommitAIDecisionIntent`
5. 必要时再加 `ValueInRange` 做细门槛

也就是说，当前扩展主线已经是：

**Tag -> IntentDefinition -> StateTree 状态 -> Commit 回写**

而不是：

**往某个 Evaluator 里继续加 Attack / Strafe 特判**

## 6. 当前优先排查顺序

遇到“AI 不按预期决策”时，排查顺序固定为三层。

### 6.1 第一层：事实有没有进来

先查：

- `CurrentTarget` 是否有效
- `DistanceToTarget` 是否正常
- `bHasTarget` / `bIsInAttackRange` 是否符合当前场景

### 6.2 第二层：决策有没有算出来

再查：

- `SelectedIntentTag`
- `SelectedIntentDesire`
- `SelectedIntentScore`
- `ObservedIntentDesire`
- `ObservedIntentScore`

### 6.3 第三层：StateTree 有没有正确消费

最后查：

- `ExpectedIntentTag` 是否配对
- `CommitAIDecisionIntent` 是否真正进入执行状态时触发
- `ResetAIDecisionState` 是否放在正确的脱战/回巡逻节点

## 7. 第一轮提炼来源

这份地图当前主要从下面三篇历史笔记提炼，并已用当前代码核对：

- `Notice/HistoryNotice/AI第四阶段战斗决策系统使用与实现说明.md`
- `Notice/HistoryNotice/AI第四阶段战斗决策与攻击欲望设计方案.md`
- `Notice/HistoryNotice/AI战斗决策参数调参与计算说明.md`
- `Notice/HistoryNotice/AI当前进度与新会话交接说明.md`
- `Notice/HistoryNotice/AI战斗决策系统现行文档说明.md`

其中更偏“当前真实实现”的部分，已经并入 [[AI 决策已锁定设计]] 和 [[AI 战斗决策调参与算分说明]]。  
其中更偏“当前读图顺序”和“历史说法清理”的部分，已经并入本页与 [[AI 已知问题与历史偏差]]。  
历史文档里与当前代码不一致的地方，单独记录在 [[AI 已知问题与历史偏差]]。

## 8. 当前已补齐的两条执行侧主题

除了战斗意图算分主链，当前 AI 包里还应明确再分出两条执行侧主题：

1. [[AI 战斗输入旋转与翻滚执行链]]
2. [[AI Reposition与Patrol框架]]

它们解决的问题不是“哪个意图分更高”，而是：

1. 当前 AI 如何像玩家一样发输入、持续发输入、持续调整控制朝向。
2. 当前走位/巡逻底层执行件已经落到什么程度，哪些高层行为仍需继续核资产与运行时链。

## 9. 当前已从历史过程文中拆出的协作侧主题

除了运行时结构和执行链，当前 AI 包里还关联一条“怎么稳定协作排查”的方法侧主题：

1. [[任务卡与短回合排查协议]]

它解决的不是运行时算分，而是：

1. 中等复杂度以上、跨消息/观察/消费链的问题，第一轮该怎样喂给 AI。
2. 后续回合怎样按“分析 -> 收窄 -> 实施 -> 验证 -> 沉淀”短回合推进，避免重新滑回自然聊天。
