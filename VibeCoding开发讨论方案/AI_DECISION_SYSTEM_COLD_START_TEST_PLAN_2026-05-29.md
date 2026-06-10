---
title: AI Decision System Cold Start Test Plan 2026-05-29
date: 2026-05-29
tags:
  - knowledge
  - ai
  - test-plan
  - decision
  - state-tree-ai
aliases:
  - AI 决策系统冷启动测试方案 2026-05-29
  - AI Decision System Cold Start Test Plan 2026-05-29
status: proposed
---

# AI 决策系统冷启动测试方案 2026-05-29

这份文档不是在补一个泛泛的“以后可以测什么”的想法清单，而是把当前这套 AI 决策重构，在真正从 0 开始搭测试时，第一轮到底该怎么起步、先测哪些东西、哪些地方必须分层、哪些地方必须单独看联机权威，正式收成一份可执行方案。

它直接贴着当前已经落下的五个阶段代码写，不再回到抽象讨论。

当前真实代码主链已经很明确：

- `STE_UpdateCombatDecision` 负责把当前战斗事实喂给决策组件，并把结果同步回 `StateTree`
- `UAOAIDecisionComponent` 负责评估、整理、排队、正式提交和执行记忆回写
- `StateTree Condition / Task` 开始优先消费新的 submitted 结果，而不是旧的 pending / selected 结果
- 战斗执行仍然走 `HeroComponent -> AOAbilitySystem`
- 库存执行仍然走 `UAOAIInventoryRuntimeUseLibrary -> QuickBar / Inventory`

这意味着这轮测试不能再按“AI 会不会攻击”“AI 会不会喝药”这种外层现象散着测。真正应该先收口确认的，是这套正式主链有没有成立。

## 第一章 这轮测试真正要回答的不是“能不能跑”，而是“四件事到底是不是真的成立”

如果从冷启动开始做测试，这轮最先要确认的不是行为表现，而是下面四件事。

第一，决策是不是被正确生产出来了。

这里看的不是“角色有没有动”，而是：

- `Evaluator` 有没有把事实真正送进 `UAOAIDecisionComponent`
- `UAOAIDecisionComponent` 有没有真的算出主意图和库存动作
- 这些结果有没有真的进入统一提交链，而不是还停留在旧兼容字段里

第二，统一 FIFO 队列是不是按方案工作了。

这一点很重要，因为这次重构真正新增的正式结构，不是某个 `Condition`，而是“统一标签提交模型”。

所以要确认的不是“分数对不对”，而是：

- 队列有没有去重
- 队列有没有容量上限
- 提交时机是不是受时间窗控制
- 提交后是不是按 `FIFO` 真正出队

第三，`StateTree` 资源消费侧有没有真的切到新接口。

如果这里没有测清，后面非常容易出现一种假通过：

- 角色确实动了
- 资源确实跑了
- 但跑的还是旧 `SelectedIntentTag` 或旧 `PendingInventoryDecisionResult`

这不叫通过，这叫旧链路还在替你兜底。

第四，旧兼容链路还存在的时候，会不会出现双入口打架。

当前阶段 1 到阶段 5 的实现里，旧字段和旧投影还没有物理删除，这本身没有问题，因为这是过渡期结构。

问题在于，如果不专门测，就没人知道下面这些事情是不是已经成立：

- 新链路优先级是不是稳定高于旧链路
- 旧链路现在是不是只剩兼容职责
- 有没有某些地方仍然把旧字段当正式主链在消费

所以这轮测试的真正目标，不是“玩法跑通”，而是先把这四件事钉死。

## 第二章 冷启动时为什么不能一上来就做大而全行为测试

很多系统第一次补测试时，最容易犯的错就是直接做一个“完整场景回归”。

看起来很省事，因为它像是在测真实用户路径。实际上这对当前这套决策重构非常危险。

原因很简单。

当前这套系统里，行为结果和正式主链并不是一回事。

举个最直接的例子。

如果一个 AI 最后成功攻击了目标，你最多只能说明下面这些事情里“至少有一件”成立了：

- 新 submitted 主链成立了
- 旧 `SelectedIntentTag` 兜底了
- 某个资源资产还在吃旧字段
- 某个任务其实是固定命令，不是动态决策结果

也就是说，单看行为表现，你根本不知道系统到底是哪条链在工作。

所以这轮测试必须先把链路拆开，先把“决策生产”“统一排队”“submitted 投影”“资源消费优先级”单独看清，再进入更大的行为回归。

这不是形式主义。

这是因为当前代码还处于过渡期，过渡期最怕的就是把“能动”误判成“结构已经切完”。

## 第三章 冷启动测试应该直接分三层，不要把所有事堆在一层里

从 0 开始搭测试时，这里建议直接分成三层。

### 第一层先测组件主链本身

第一层只看 `UAOAIDecisionComponent` 本身，不碰复杂 `StateTree` 资源，不碰大地图，不碰行为表现层。

它要回答的问题只有一个：

**这套统一提交结构本身是不是成立。**

这一层的主要目标包括：

- 决策评估入口是不是稳定可调用
- 队列是不是按方案工作
- submitted 结果是不是按方案更新
- 兼容字段和新字段的优先级是不是符合预期
- reset 后是不是能回到稳定初始态

这一层的价值是最大，也是最应该先补的一层。

因为如果连这层都没稳，后面所有更高层测试看到的失败，都很难定位到底是结构问题、资源问题，还是行为问题。

### 第二层再测 `StateTree` 接线和消费优先级

第二层开始把测试往 `StateTree` 挪，但仍然不进入完整战斗表现。

这里真正要看的，是“新主链有没有被资源消费层真正承认”。

也就是说，这层的重点不是“角色做了什么动作”，而是：

- `STE_UpdateCombatDecision` 有没有把新状态投影出来
- `Condition` 有没有优先匹配 submitted 结果
- `Task` 有没有优先消费 submitted inventory decision
- `StateTree` 事件桥接有没有从旧 pending 事件切到新 submitted 事件

这一层其实是在测“资源接线边界是不是守住了”。

### 第三层最后再做行为回归和联机权威验证

第三层才开始看真实运行时。

这层看的不只是单人行为，还必须把联机权威边界一起纳入。

因为当前方案里，下面这些都已经被正式锁定了：

- 决策生产是服务端权威
- 队列推进是服务端权威
- 随机提交时间窗是服务端权威
- 客户端不复制整条决策队列
- 客户端只消费正式执行结果

这意味着第三层如果只做单机行为回归，不看联机权威，测试是不完整的。

## 第四章 第一层自动化测试，冷启动时应该先补哪几组

这一层建议直接基于已有测试入口扩写，不要从零再建一套风格完全不同的文件。

当前工程里已经有：

- `Source/AegisOdyssey/TestProject/AIDecisionQueueTests.cpp`

这意味着最合理的冷启动方式不是“重写一个新的 TestProject 入口”，而是先把这个文件扩成第一轮正式基础集。

### 4.1 队列基础行为测试

这一组是第一优先级。

至少要覆盖下面这些点：

1. 合法条目可以成功入队。
2. 无效标签不能入队。
3. 重复等价条目不能重复入队。
4. 队列到达容量上限后，新条目被拒绝。
5. 只有 authority 侧允许维护队列。
6. 空队列时 `TrySubmitNextDecision(...)` 返回失败。
7. 时间窗未到时，`TrySubmitNextDecision(...)` 返回失败。
8. 到达允许提交时间后，队首条目按 `FIFO` 出队。
9. 出队后如果队列非空，下一次提交时间会重新安排。
10. 出队后如果队列已空，`NextDecisionSubmitTimeSeconds` 会回到无效值。

这一组的意义不是“证明这个容器能用”，而是证明这次重构最核心的新正式语义，也就是统一 FIFO 提交模型，已经真的成立。

### 4.2 submitted 结果更新测试

这一组是第二优先级。

因为这次系统真正从旧模型切出来，靠的不是队列本身，而是“正式 submitted 结果”。

至少要确认这些事情：

1. `SubmitCurrentDecisionOutputs(...)` 会把当前主意图投影成正式决策条目。
2. 库存结果存在时，`SubmitCurrentDecisionOutputs(...)` 也会尝试生成库存条目。
3. 提交成功后，`CurrentSubmittedDecisionTag` 会更新。
4. 提交成功后，`LastSubmittedDecisionTag` 会更新。
5. 如果本次提交的是库存动作，`CurrentSubmittedInventoryDecisionResult` 会更新。
6. 如果本次提交的不是库存动作，submitted inventory result 会被清空。
7. 队列完全空掉后，当前 submitted 状态是否按当前实现被清空，要单独钉死。
8. `GetCurrentSubmittedInventoryDecisionResult(...)` 和 `GetLastSubmittedInventoryDecisionResult(...)` 的返回语义要测清。

这一组测试真正解决的问题是：

后面资源层到底在消费什么。

如果这组没测清，后面任何 submitted 相关逻辑都没有基础。

### 4.3 决策匹配优先级测试

这组用例一定要单独列出来，不能混在别的行为里顺手带过。

因为当前过渡期最容易出问题的点，就是“到底谁才是当前正式主链”。

至少要确认：

1. `MatchesCurrentDecisionTag(...)` 在有 `CurrentSubmittedDecisionTag` 时优先匹配 submitted。
2. 没有 submitted 但队列有条目时，优先匹配队首条目。
3. submitted 和队列都没有时，才回退到旧 `SelectedIntentTag`。
4. 输入无效 tag 时稳定返回 false。

这组用例本质上不是功能补丁测试，而是在给“新主链优先级”立法。

### 4.4 reset 和清理语义测试

这组经常容易被忽略，但实际上很重要。

因为这次重构已经把更多运行时状态集中进了 `UAOAIDecisionComponent`：

- 队列
- 下一次提交时间
- submitted decision
- submitted inventory result
- pending inventory result
- tactical state
- runtime desire / score

所以 `ResetDecisionState()` 必须专门测。

至少要确认：

1. 队列被清空。
2. submitted decision 被清空。
3. pending inventory decision 被清空。
4. submitted inventory decision 被清空。
5. `RepeatedIntentCount` 和执行记录会回到初始值。
6. tactical state 会回到初始值。
7. runtime state 的 desire / score 会回到 0。

如果这组没测，后面很容易在行为层看到“上一场状态残留”的假问题。

## 第五章 第二层自动化测试，要专门确认 `StateTree` 已经优先消费新主链

这一层的重点不是让 AI 在场景里做出完整行为，而是确认现在的资源消费侧到底在用谁。

### 5.1 Evaluator 投影测试

这里重点看的是：

- `STE_UpdateCombatDecision` 是否每帧把组件里的新状态同步到实例数据

至少要确认下面这些字段的同步语义：

- `CurrentQueuedDecisionTag`
- `CurrentSubmittedDecisionTag`
- `LastSubmittedDecisionTag`
- `DecisionQueueCount`
- `NextDecisionSubmitTimeSeconds`
- `CurrentSubmittedInventoryDecision`
- 旧 `PendingInventoryDecision`

这里一定要把“新字段”和“旧兼容字段”一起测。

因为这轮不是“旧字段已经没了”，而是“旧字段还在，但它们不该再承担正式主链职责”。

测试目标就是把这个边界测清楚。

### 5.2 战斗意图匹配 Condition 测试

这部分对应：

- `STC_AIDecisionIntentMatches`

这里要确认的不是“这个条件能返回 true”，而是它为什么返回 true。

至少要验证：

1. submitted decision 存在时，匹配来自 submitted。
2. submitted 不存在但队列有头部条目时，匹配来自 queue head。
3. 两者都没有时，才会回退到旧 `SelectedIntentTag`。
4. `bInvert` 语义不被这轮重构破坏。

这是整个战斗资源消费层最重要的一组接线确认。

### 5.3 库存决策匹配 Condition 测试

这部分对应：

- `STC_AIPendingInventoryDecisionMatches`

虽然类型名还带着 `Pending`，但当前正式语义已经不是“只看 pending”了。

所以必须专门确认：

1. submitted inventory decision 存在时，优先消费 submitted。
2. submitted 不存在时，才回退到旧 pending。
3. `CoordinationFilter` 是否仍按有效结果工作。
4. `ExpectedActionTag` / `ExpectedCandidateTag` 是否按有效结果工作。
5. `bRequireResolvedTarget` 是否按有效结果工作。
6. `bRequireAdditiveInventoryWindow` 是否仍只消费战术窗口输出，而不是自己重新做决策。

这组测试的真正作用，是证明库存行为资源没有偷偷继续把旧 pending 当主入口。

### 5.4 库存执行任务输入优先级测试

这部分对应：

- `STT_UseResolvedInventoryItem`

当前代码里，它的输入优先级已经明确了：

1. `CurrentSubmittedInventoryDecision`
2. `PendingInventoryDecision`
3. `DecisionComponent->GetCurrentSubmittedInventoryDecisionResult(...)`
4. `DecisionComponent->GetPendingInventoryDecisionResult(...)`
5. 固定 `UseCommand`

这一组必须专门测。

因为如果这组优先级不清，后面任何“库存动作执行成功”的现象，都不能证明执行层是在消费新主链。

### 5.5 submitted inventory 事件桥接测试

这部分对应：

- `UAOAILogicStateTreeComponentBase`

这里要确认的是：

1. 组件绑定的是 `OnSubmittedInventoryDecisionChanged()`
2. 不再靠旧 `OnPendingInventoryDecisionChanged()`
3. 有 submitted 结果时发 `AI_Event_InventoryDecision_Updated`
4. 结果被清空时发 `AI_Event_InventoryDecision_Cleared`

这组测试本质上是在测：`StateTree` 事件桥接层有没有真正转向新的正式主链。

## 第六章 第三层不只是手工行为回归，还必须把联机权威一起纳入

如果只看代码结构，这套系统最容易让人误判的一点就是：

“既然现在提交是统一队列了，那先在单机里跑通就行，联机以后再说。”

这个判断不对。

因为这次方案本身就已经把联机权威边界写进结构了。

所以第三层不是附加项，而是正式必测层。

### 6.1 单机行为回归先看什么

单机先看的不是所有玩法，而是最能证明主链真的接上了的那几条行为。

建议第一轮先只选下面几类：

1. 一个纯战斗主意图行为。
   例如攻击或走位。
2. 一个库存消耗行为。
   例如喝药。
3. 一个库存装备/切换行为。
   例如切武器或选择 `QuickBar` 项。

这三类足够了。

因为它们正好覆盖了：

- submitted 战斗标签消费
- submitted inventory result 消费
- 旧正式执行入口复用

这里不要贪多。

冷启动最怕的不是漏，而是一次测太宽，最后谁都说不清问题到底在哪。

### 6.2 联机权威验证必须单独看

这一层要明确检查下面这些事情：

1. 决策队列是否只在服务端推进。
2. 客户端是否不会本地自发生成一套 submitted 结果。
3. 客户端是否只看到服务端正式执行后的表现结果。
4. 库存动作是否仍然遵守正式库存权威入口。
5. 提交时间窗是否不会在客户端本地另跑一套随机节奏。

这里真正要看的不是“客户端上看起来有没有动作”，而是：

- 决策真相是不是只有服务端有
- 客户端是不是只消费正式结果

这条边界如果不单独测，后面非常容易因为“看起来联机也能动”而误判成结构已经对了。

## 第七章 冷启动时程序员手上必须先准备好的东西

从 0 开始搭这套测试，先不要急着写 case。

先把下面这些准备好。

### 7.1 一个干净的自动化测试入口

当前最适合直接扩的是：

- `Source/AegisOdyssey/TestProject/AIDecisionQueueTests.cpp`

第一步先把这个文件升级成“AI 决策基础自动化测试入口”，不要再到处散建零碎测试文件。

后面如果测试量确实长大，再按主题拆文件。

但冷启动第一轮不建议先分散。

### 7.2 一个最小 AI 决策测试 Pawn / Controller 场景

不要一上来就在正式大地图里测。

原因很简单：

- 正式地图干扰源太多
- 行为树、感知、其他 AI、掉落物、动画资源都会把问题搅混

建议单独准备一个最小测试场景，只放下面这些最小对象：

- 一个 AI Pawn
- 一个目标 Pawn 或 Dummy
- 必要的 `StateTree` 资源
- 必要的 `DecisionProfile`
- 能触发库存执行的最小库存配置

冷启动第一轮的目标不是体验玩法，而是把结构链路钉死。

### 7.3 一份固定的阅读和定位顺序

后续谁来接这套测试，都不要从资源往下猜。

默认阅读顺序应该固定成：

1. `STE_UpdateCombatDecision`
2. `UAOAIDecisionComponent`
3. `STC_AIDecisionIntentMatches`
4. `STC_AIPendingInventoryDecisionMatches`
5. `STT_UseResolvedInventoryItem`
6. `AOAILogicStateTreeComponentBase`

这条顺序很重要。

因为这轮系统的正式真相，就是沿这条线串起来的。

## 第八章 真正从 0 开始执行时，推荐的落地顺序

这里不写成“可以任选其一”，而是直接给推荐顺序。

### 第一步，先把第一层自动化测试补透

优先完成：

- 队列基础测试
- submitted 结果测试
- 决策匹配优先级测试
- reset 清理测试

如果这一步没做完，先不要进入大地图行为验收。

### 第二步，再补第二层 `StateTree` 接线测试

优先完成：

- Evaluator 投影测试
- intent condition 优先级测试
- inventory condition 优先级测试
- inventory task 输入优先级测试
- submitted inventory event bridge 测试

这一步的目标是回答：

资源层到底有没有真的承认新主链。

### 第三步，再做单机最小行为回归

只挑最小样例：

- 一个战斗行为
- 一个喝药行为
- 一个切换装备或切武器行为

先证明新主链已经能驱动真实执行层。

### 第四步，最后做联机权威验证

这一步必须单独记录：

- 哪些行为只在服务端做真相
- 客户端最终看到了什么
- 有没有出现客户端自发决策

不要把这部分揉进单机手测里一句带过。

## 第九章 这轮最小可交付测试集，至少应该包含什么

如果按“第一轮就够开始稳定回归”的标准收口，这轮最小测试集建议至少包含下面这些。

### 自动化最小集

1. 队列入队 / 去重 / 容量 / 出队节流
2. submitted decision 更新
3. submitted inventory decision 更新
4. `MatchesCurrentDecisionTag()` 优先级
5. `STC_AIDecisionIntentMatches` 优先级
6. `STC_AIPendingInventoryDecisionMatches` 优先级
7. `STT_UseResolvedInventoryItem` 输入优先级
8. `ResetDecisionState()` 清理语义

### 手工最小集

1. 单机下一个战斗主意图行为
2. 单机下一个库存消耗行为
3. 单机下一个库存切换行为

### 联机最小集

1. Listen Server 下 AI 仍然正常决策
2. Client 只看到正式结果，不本地自发决策
3. 库存执行仍然走正式权威入口

## 第十章 这轮验收口径应该怎么写，才不会把“结构过渡期”误判成“系统已经完全收尾”

这轮测试通过，不等于整套 AI 决策系统已经最终收尾。

它真正应该确认的是下面这些事：

1. `Evaluator -> DecisionComponent -> Queue -> Submitted Result -> StateTree Consumption` 这条正式主链已经成立。
2. 新 submitted 结果已经在资源消费层取得优先级。
3. 旧字段仍然存在，但当前只承担兼容职责，不再是正式主入口。
4. 战斗执行链和库存执行链都还在复用原正式执行入口，没有因为重构长出第二套执行系统。
5. 联机权威边界没有被破坏，客户端没有偷偷长出本地独立决策语义。

反过来说，下面这些事不应该在这轮测试通过时被误写成“已经完成”：

- 所有 AI 行为都已经调到足够智能
- 所有资源资产都已经完全切完旧字段
- 所有联机表现层可视化都已经补齐
- 所有库存协同语义都已经最终收束

这轮的验收问题不是“AI 是不是已经变聪明了”，而是：

**这套正式决策提交结构，是不是真的已经站住了。**

## 第十一章 接下来真正开写测试代码时，第一批建议直接落哪几项

如果现在马上开始写，我建议第一批直接开下面这些：

1. 在 `AIDecisionQueueTests.cpp` 里补 `submitted decision` 和 `MatchesCurrentDecisionTag` 相关测试。
2. 单独补一组 `submitted inventory result` 和 `OnSubmittedInventoryDecisionChanged()` 相关测试。
3. 补 `STT_UseResolvedInventoryItem` 的输入优先级测试。
4. 补 `STC_AIDecisionIntentMatches` 和 `STC_AIPendingInventoryDecisionMatches` 的新优先级测试。

这一批写完以后，才值得进入后面的最小场景手测。

因为到这一步，你至少已经能回答一句非常关键的话：

当前 AI 决策系统，不只是“还能跑”，而是“新的正式主链已经被自动化测试钉住了”。
