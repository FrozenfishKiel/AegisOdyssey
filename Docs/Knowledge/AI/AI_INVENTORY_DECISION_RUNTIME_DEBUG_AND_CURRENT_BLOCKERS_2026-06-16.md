---
title: AI Inventory Decision Runtime Debug And Current Blockers 2026-06-16
tags:
  - knowledge
  - ai
  - inventory-decision
  - debug
aliases:
  - AI库存决策运行时调试与当前阻塞点 2026-06-16
---

# AI 库存决策运行时调试与当前阻塞点

更新时间：2026-06-16  
适用范围：当前项目里基于 `STE_UpdateInventoryDecision -> UAOAIDecisionComponent -> UAOAILogicStateTreeComponentBase -> STT_UseResolvedInventoryItem` 的库存决策主链。  
不适用范围：未来尚未落地的库存决策扩展方案、具体某个敌人资产的最终调参结论。

## 1. 当前已经落地的职责边界

这条链现在的正式边界已经比较明确，不能再按旧思路混写。

1. `STE_UpdateInventoryDecision`
   负责收集库存决策事实、候选信息、动作评分，并把“本帧评估结果”缓存到 `UAOAIDecisionComponent`。
2. `UAOAIDecisionComponent`
   负责持有库存评估缓存、统一决策队列、当前已提交库存决策结果、最近已执行记录，以及调试快照输出。
3. `UAOAILogicStateTreeComponentBase`
   只监听“已提交库存决策结果”的变化，并把它转换成 StateTree Event。
4. `STT_UseResolvedInventoryItem`
   只消费当前已提交的库存决策结果，不负责重新评估库存。

当前统一语义是：

- `STE` 负责算
- `UAOAIDecisionComponent` 负责收和提
- `StateTree` 负责消费

## 2. 当前运行时主链

### 2.1 评估层

库存决策评估从 `STE_UpdateInventoryDecision` 进入。  
它会基于当前目标、距离、战术态、生命值比例、耐力比例、最近受击强度、当前武器是否适配距离等事实，评估 `InventoryActionDefinitions` 里的动作。

这里需要区分三层结果：

1. `InventoryDecisionFacts`
   这是库存评估使用的环境事实。
2. `CandidateFactsByActionTag`
   这是每个库存动作当前有没有候选、有没有可执行候选。
3. `CurrentEvaluationInventoryDecisionResult`
   这是评估层在当前帧算出来的最新结果，还不是正式提交给执行层的结果。

### 2.2 提交层

`UAOAIDecisionComponent` 会把库存评估结果纳入统一决策队列。  
当前正式提交给执行层消费的是 `CurrentSubmittedInventoryDecisionResult`，不是 `CurrentEvaluationInventoryDecisionResult`。

这意味着：

- 评估结果存在，不等于执行层此刻已经能消费
- 执行层只能读“已提交结果”
- 不能让 `STT` 反向去猜评估层想做什么

### 2.3 事件桥接层

`UAOAILogicStateTreeComponentBase` 现在监听 `OnSubmittedInventoryDecisionChanged()`。

它只会发两类事件：

- `AI.Event.InventoryDecision.Updated`
- `AI.Event.InventoryDecision.Cleared`

如果事件到来时 StateTree 还没在运行，组件会先把这次事件缓存成 pending，等后续 tick 时再尝试补发。这个改动是为了避免早期出现“树还没启动就发事件”的时序问题。

### 2.4 执行层

`STT_UseResolvedInventoryItem` 的正式职责是：

1. 从当前已提交库存决策结果里拿 `UseCommand`
2. 必要时拿 `ResolvedTarget`
3. 最终落到库存运行时执行库

也就是：

- QuickBar 路径走 QuickBar 激活
- 普通库存路径走库存槽位使用

## 3. 当前调试入口

这轮已经落地了 HUD 宿主 + 全局控制台开关 + 纯 Slate 调试面板方案。

### 3.1 全局控制台指令

- `AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>`
  全局开关当前世界里的 AI 决策 StateTree。
- `AegisOdyssey.AI.SetDebugPanelEnabled <true|false>`
  全局开关 AI 决策调试面板与调试观察链。

### 3.2 调试面板宿主

调试面板挂在 `AAOHUD` 上，由 `UAOHUDViewModelComponent` 驱动 ViewModel，再由 Slate 面板显示。  
这条链明确属于测试/调试入口，不属于主玩法流程。

### 3.3 调试文件日志

当调试面板开启时，会在 `Saved/AIDebug` 下为当前观察会话新建一份按时间命名的 `txt` 文件，并持续追加快照，而不是刷新覆盖。

当前这条链的意义是：

1. 运行时可以看实时面板
2. 会话结束后可以回看整段决策变化
3. 可以确认“有没有评估结果”“有没有提交结果”“当前队列里是什么”

## 4. 当前已经确认的阻塞点

本轮实际运行日志已经证明，当前问题主要不在 `STT_UseResolvedInventoryItem` 消费层，而是在更上游。

当前关键现象是：

- `Has Evaluation Inventory Decision: false`
- `Has Submitted Inventory Decision: false`
- 运行时主要只看到 `AI.Intent.Attack` 和 `AI.Intent.Strafe`

这说明当前更大的问题是：

**库存评估层本身没有稳定产出有效的 `FAOAIInventoryDecisionResult`。**

也就是说，至少在当前测试 AI 上：

1. 要么 `DecisionProfile.InventoryActionDefinitions` 没有有效配置
2. 要么动作定义存在，但所有候选都没有进入“可用候选”
3. 要么候选存在，但最终动作分或候选分没有大于 0，导致评估层没有形成有效结果

## 5. 当前需要记住的判断顺序

以后再遇到“AI 不会主动从库存里拿东西用”，默认按下面顺序排查，不要一上来先盯 `STT_UseResolvedInventoryItem`。

1. 先看测试 AI 的 `DecisionProfile` 是否真的配置了库存动作定义
2. 再看 `STE_UpdateInventoryDecision` 是否算出了 `CurrentEvaluationInventoryDecisionResult`
3. 再看 `UAOAIDecisionComponent` 是否形成了 `CurrentSubmittedInventoryDecisionResult`
4. 再看 `UAOAILogicStateTreeComponentBase` 是否发出了 submitted 事件
5. 最后才看 `STT_UseResolvedInventoryItem` 是否成功消费

## 6. 相关笔记

- [[AI 项目地图]]
- [[AI 已知问题与历史偏差]]
- [[AI Inventory Decision Cold Start Test Plan]]
- [[Item Semantic Tags Shared By AI And Player 2026-06-16]]
