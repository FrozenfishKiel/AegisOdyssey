# AI Decision First Stage Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在不改第二阶段决策生产语义的前提下，先把 `UAOAIDecisionComponent` 落成统一决策队列骨架。

**Architecture:** 这轮只在 `DecisionComponent` 与 `AOAIDecisionTypes` 内新增统一决策项、固定容量 `FIFO` 队列、服务端权威提交节奏和最小观测接口。旧的 `SelectedIntentTag` / `PendingInventoryDecisionResult` 继续保留给现有链路使用，但不再往里面扩新逻辑。

**Tech Stack:** Unreal Engine C++、GameplayTag、Automation Test

---

### Task 1: 第一阶段测试先行

**Files:**
- Create: `Source/AegisOdyssey/TestProject/AIDecisionQueueTests.cpp`
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`

- [ ] 为固定容量 `5`、严格 `FIFO`、满队列丢弃新项、单次只提交一个队头、空队列不提交写 automation tests。
- [ ] 跑测试，先看红。
- [ ] 只补最小生产代码让测试过。
- [ ] 再跑测试确认绿。

### Task 2: 第一阶段骨架收口

**Files:**
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`
- Modify: `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`

- [ ] 新增统一决策项结构与最小观测接口。
- [ ] 把随机提交间隔收成组件内部可配置区间，不写死固定节奏。
- [ ] 确保队列推进和提交只在服务端权威路径成立。
- [ ] 把新队列状态纳入 `ResetDecisionState()`，避免残留旧轮次数据。
