---
title: AI Decisions
tags:
  - knowledge
  - ai
  - decisions
aliases:
  - AI Decisions
  - AI决策已锁定设计
---

# AI 决策已锁定设计

更新时间：2026-05-19  
适用范围：当前已经在代码里落地的 AI 战斗决策结构与协作边界。  
不适用范围：未来扩展方案的全部细节、具体某个敌人资产的调参结果。

## 1. 当前已经锁定的总体方向

当前项目已经锁定的不是“继续在 StateTree 里硬写更多 if”，而是：

**由组件持有运行时决策状态，由数据资产提供意图定义，由 StateTree 消费结果并回写执行记录。**

这意味着后续默认遵守下面几条边界。

## 2. 已锁定边界

### 2.1 决策和执行分层

已经锁定：

- `UAOAIDecisionComponent` 负责算分和保留记忆
- `StateTree` 负责状态组织和行为进入
- 具体攻击、走位、位移等执行仍由 Task / 技能 /输入链路承接

不再建议：

- 在 StateTree Condition 里自己重算攻击分
- 在执行状态里每帧重新决定“现在到底要不要换成另一个意图”

### 2.2 意图必须数据驱动

已经锁定：

- 当前意图配置来自 `DecisionProfile.IntentDefinitions`
- 每个意图单独定义自己的 `BaseDesire`、节奏、距离因子、冷却、重复惩罚、切换奖励、保底权重等

不再建议：

- 在主流程里写死 Attack / Strafe 两个分支的专用字段

### 2.3 距离是影响因素，不是唯一硬门槛

当前代码已经锁定：

- `CanEvaluateIntent()` 的硬条件只保留 `bRequireTarget`
- 距离远近主要通过 `DistanceFactor` 进入 `Desire`

这说明当前正式语义是：

**距离参与评分，但不直接一票否决。**

### 2.4 执行反馈必须在进入状态时回写

已经锁定：

- `CommitAIDecisionIntent` 在 `EnterState` 时调用
- 它回写的是“已开始执行哪个意图”，不是“是否命中”

这是当前冷却、重复惩罚、节奏恢复成立的基础。

### 2.5 决策记忆是否清空必须显式控制

已经锁定：

- `ResetAIDecisionState` 是显式 Task
- 不强绑在所有父状态退出上

原因是：

- 有的状态切换要继承上一轮战斗记忆
- 有的状态切换必须切断上一轮战斗记忆

这个边界不能再被模糊掉。

## 3. 当前设计中已经成立的扩展点

### 3.1 意图扩展

当前架构已经允许：

- `AI.Intent.Attack`
- `AI.Intent.Strafe`
- 后续扩展 `Retreat` / `Reposition` / `Block` / `Pressure`

只要遵循 `Tag -> IntentDefinition -> StateTree -> Commit` 这条链路，就不需要重写主流程。

### 3.2 决策输入扩展

当前已经真实接入的输入不只距离，还包括：

- 最近受伤比例
- 目标战斗标签状态
- 自身属性区间因子

这意味着“决策输入可扩展”已经不是纯概念，而是当前实现特性。

### 3.3 观察输出扩展

当前已经锁定：

- `SelectedIntentTag` 是胜出意图
- `ObservedIntentTag` 允许单独观察某个非胜出意图

这对调试多意图系统非常重要，后续不要再退回只暴露一个主意图结果的做法。

## 4. 第一轮核对后确认的历史偏差

以下内容在历史文档里出现过，但**不能继续当作当前事实使用**：

### 4.1 “不填 Profile 会自动补默认 Attack / Strafe”

当前代码不是这样。

现状是：

- `DecisionProfile == nullptr` 时只是不覆盖 `IntentDefinitions`
- 代码里没有默认自动补全 Attack / Strafe 的当前实现

因此后续必须默认认为：

**每个角色应该明确配置自己的 `DecisionProfile.IntentDefinitions`。**

### 4.2 “当前事实层只包含少量目标事实”

这条已经过时。

当前 `FAOAIDecisionCombatFacts` 里还包含：

- `SelfAIAttackRange`
- `TargetAIAttackRange`
- `RecentDamageRatio`
- 多个目标战斗状态标签布尔量

### 4.3 “当前调参只围绕距离和基础欲望”

这条也已经不完整。

当前实现还已经接入：

- `RecentDamageFactor`
- `AttributeIntervalFactors`
- `TargetStateTagFactors`

## 5. 后续整理时必须遵守的 AI 包内规则

1. 知识库正文只能写当前代码已成立的结论。
2. 历史方案里的扩展设想只能写成“未来入口”，不能写成“当前事实”。
3. 凡是涉及“默认行为”“自动补全”“硬条件”的说法，都必须先核代码再写。
4. 任何新提炼的 AI 文档，都应优先链接到 [[AI 项目地图]]、[[AI 战斗决策调参与算分说明]]、[[AI 已知问题与历史偏差]]。

## 6. AI 战斗执行继续默认走统一输入链

已经锁定：

1. AI 战斗侧优先通过 `UAOHeroComponent::InjectAbilityInputCommand(...)` 复用玩家输入链。
2. Bot 注入输入后，要继续保留主动补一次 `ProcessAbilityInput(...)` 的行为。
3. 后续不要再把 AI 战斗执行改回“直接点名播放某个技能或连招段”的主叙事。

## 7. 持续攻击、控制旋转、目标跟随继续拆成独立件

已经锁定：

1. `STT_SendCombatCommand` 只代表一次性输入。
2. `STT_PulseCombatCommand` 负责持续输入脉冲。
3. `STT_RotateControlTowardTarget` 负责持续控制朝向。
4. `STT_MoveToTarget / STT_MoveToLocation` 负责导航移动。

不再建议：

1. 把这些职责重新揉成一个大 Task。
2. 用状态重入去模拟持续攻击。
3. 把旋转逻辑塞回输入 Task 或 MoveTo Task。

## 8. 当前朝向语义继续以 `ControlRotation` 为真相入口

已经锁定：

1. `GA_LightAttack`、`GA_Block`、`GA_Roll` 与 `AT_WaitRotateToDirection` 当前都依赖 `AController::GetControlRotation()`。
2. 这使 AI 与玩家可以共享同一套方向语义。
3. 任何后续 AI 攻击/翻滚方向问题，默认先核 `ControlRotation` 链，而不是先核角色朝向。

## 9. 当前走位与巡逻继续按“选点 + 移动 + 朝向”三层写

已经锁定：

1. EQS / 上下文负责“去哪”。
2. `MoveToTarget / MoveToLocation` 负责“怎么去”。
3. `RotateControlTowardTarget` 负责“怎么看”。
4. `PatrolAnchorLocation` 和 `PatrolTargetLocation` 必须分开建模。

因此后续不要再把普通 `Reposition / Patrol` 默认写回一套大 Ability，或写成一个包办所有职责的大黑盒。

## 10. 当前 AI 文档层级已经锁定

已经锁定：

1. `AI当前进度与新会话交接说明.md` 继续作为历史交接来源保留。
2. `AI战斗决策系统现行文档说明.md` 继续作为历史导航来源保留。
3. 当前真正的现行正文入口以 [[AI 项目地图]]、[[AI 战斗决策调参与算分说明]]、[[AI 决策已锁定设计]]、[[AI 已知问题与历史偏差]] 为准。

这意味着：

1. 后续接手时可以用历史交接文找线索。
2. 但凡历史交接文与当前代码或知识包冲突，默认以当前代码和知识包正文为准。

## 11. AI 联动排查默认继续沿主链收窄

已经锁定：

1. 跨 AI / 战斗 / 消息 / 观察 / UI 消费链的问题，优先先找“谁持有状态、谁转发、谁消费”。
2. 不默认先碰属性同步层。
3. 不默认先在外围加临时表现兜底。

后续如果要继续组织这类协作，默认优先复用 [[任务卡与短回合排查协议]]，而不是重新靠自然聊天把范围慢慢长出来。
