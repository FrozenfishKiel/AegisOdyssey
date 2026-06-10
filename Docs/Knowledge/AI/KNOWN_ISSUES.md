---
title: AI Known Issues
tags:
  - knowledge
  - ai
  - known-issues
aliases:
  - AI Known Issues
  - AI已知问题与历史偏差
---

# AI 已知问题与历史偏差

更新时间：2026-05-19  
适用范围：当前 AI 知识库整理阶段已确认的历史文档偏差、容易误判的入口。  
不适用范围：所有 AI 行为 bug 的完整清单。

## 1. 这份文档记录什么

这份文档当前不主要记录运行时故障，而是先记录：

1. 历史 AI 笔记里哪些说法已经不再等于当前实现
2. 哪些地方最容易让后续整理把过期内容误沉淀进知识库

后续如果再整理到明确的运行时故障案例，再继续补充。

## 2. 已确认的历史偏差

### 2.1 “不填 DecisionProfile 也会自动有默认 Attack / Strafe”

这条不能再当作当前事实使用。

当前代码现状：

- `ApplyDecisionProfileIfNeeded()` 只会在 `DecisionProfile != nullptr` 时覆盖 `IntentDefinitions`
- 当前代码里没有自动补默认 Attack / Strafe 的现行逻辑

因此：

- 后续整理、接手、调试时，默认应先检查 `IntentDefinitions` 是否真的有内容
- 不要假设“不配也能跑”

### 2.2 “当前事实层只是一层简单目标包装”

这条已经过时。

当前事实层除了目标和距离，还包含：

- `SelfAIAttackRange`
- `TargetAIAttackRange`
- `RecentDamageRatio`
- 目标战斗标签状态

如果还按“只有目标和距离”理解，就会漏掉很多当前真实输入。

### 2.3 “当前参数体系主要只围绕距离和节奏”

这条也已经不完整。

当前实现还包括：

- `RecentDamageFactor`
- `AttributeIntervalFactors`
- `TargetStateTagFactors`

所以任何后续知识提炼，如果还把当前系统写成“只有距离曲线 + 冷却惩罚”，都属于知识污染。

### 2.4 “AI当前进度与新会话交接说明”可以直接当现行接线文档

这条也不能直接成立。

这份交接文里仍保留了部分旧说法，例如：

- 仍把 `IdealAttackDistance` 写成配置入口
- 仍写有“不填就走默认 Attack / Strafe”的接法

但当前代码现状是：

- 正式距离入口已经统一到武器 `AIAttackRange`
- 是否真的有意图定义，要以 `DecisionProfile / IntentDefinitions` 现状为准

因此这份文档可以继续当历史交接线索，但不能继续直接当当前规格正文。

## 3. 当前最容易误判的入口

### 3.1 把 `bIsInAttackRange` 当成硬过滤条件

当前不是这样。

当前硬条件只有：

- `bRequireTarget && !bHasTarget`

`bIsInAttackRange` 现在主要是事实输出，不是当前实现中的一票否决条件。

### 3.2 把 StateTree 当成主算分者

当前也不是这样。

当前真实边界是：

- `StateTree` 消费结果
- `UAOAIDecisionComponent` 算分

### 3.3 把 `CommitAIDecisionIntent` 理解成“动作完成后回写”

当前不是。

它是在 `EnterState` 时回写“开始执行了哪个意图”。

如果这里理解错，后面会把冷却、重复惩罚、生效时机全看错。

### 3.4 遇到跨层现象先从外围表现层补丁开刀

这条也容易误判。

对当前这类 AI 联动问题，更稳的默认顺序是：

1. 先找状态持有者
2. 再找转发链
3. 最后找表现消费层

不要默认先：

1. 动属性同步层
2. 在外围补临时显示逻辑
3. 把局部表现问题直接当成单纯 Widget 问题

## 4. 当前整理规则

后续往 `Docs/Knowledge/AI` 里继续提炼时，默认遵守：

1. 历史文档先当线索，不先当事实。
2. 任何涉及“默认行为”的说法都必须核当前代码。
3. 任何涉及“当前硬条件”的说法都必须核 `CanEvaluateIntent()`。
4. 任何涉及“当前输出字段”的说法都必须核 `STE_UpdateCombatDecision.*`。
5. 如果历史文档与当前代码冲突，知识库正文以当前代码为准，冲突内容记在本页。

## 5. 当前关联文档

- [[AI 项目地图]]
- [[AI 决策已锁定设计]]
- [[AI 战斗决策调参与算分说明]]
- [[任务卡与短回合排查协议]]

## 6. 新增识别出的执行链误判点

### 6.1 不要把 `STT_SendCombatCommand` 再写成“持续攻击状态机”

当前不是。

当前更准确的边界是：

1. `STT_SendCombatCommand` 负责一次性输入。
2. `STT_PulseCombatCommand` 负责持续输入脉冲。
3. 连招窗口和能力消费仍由下层现有链路处理。

### 6.2 不要把 `Attack` 状态再写回“单个技能生命周期”

当前更接近：

1. `Attack` 是持续施压阶段。
2. 输入脉冲和控制旋转可在状态内并发。
3. 状态本身不等于某一次技能实例的完整生命周期。

### 6.3 不要把历史走位方案里的“待实现”继续写成当前事实

当前已经有源码入口：

1. `STT_RunEQSSelectLocation`
2. `STT_MoveToLocation`
3. `AOEnvQueryContext_PatrolAnchor`
4. `AAOAIPlayerBotController` 的巡逻锚点与巡逻目标点

因此历史文档里“这些还没正式实现”的说法已经过时。

### 6.4 不要把当前朝向真相简化成“只有 Rotate Task 在起作用”

当前还要同时注意：

1. `AAOAIPlayerBotController::SetCurrentTarget(...)` 仍会 `SetFocus(...)`
2. `ControlRotation` 链和 Focus 链可能同时影响最终手感

如果实测朝向仍然太吸附，不要只盯 `STT_RotateControlTowardTarget`。
