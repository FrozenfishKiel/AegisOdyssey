---
title: AI Reposition And Patrol Framework
tags:
  - knowledge
  - ai
  - reposition
  - patrol
  - eqs
  - navigation
aliases:
  - AI Reposition And Patrol Framework
  - AI Reposition与Patrol框架
---

# AI Reposition 与 Patrol 框架

更新时间：2026-05-19  
适用范围：当前项目里 AI 战斗换位与脱战巡逻的底层执行框架、上下文划分，以及哪些部分已经进入源码事实。  
不适用范围：具体某棵 StateTree 资产最终排布、某个敌人最终手感参数、未来还没接入的战术层策略。

## 1. 先把历史方案和当前代码分开

`AI走位与巡逻设计方案.md` 的长期价值，不在于它判断了哪些文件已经存在，而在于它先把走位问题拆成了：

1. 选点
2. 移动
3. 朝向

这个拆法今天仍然成立。  
但它里面“EQS 选点 / MoveToLocation / Patrol 还没正式实现”的说法，已经不能再照搬成当前事实。

因为当前源码里已经存在：

1. `STT_RunEQSSelectLocation`
2. `STT_MoveToLocation`
3. `AOEnvQueryContext_CurrentTarget`
4. `AOEnvQueryContext_PatrolAnchor`
5. `AAOAIPlayerBotController` 上的巡逻锚点与巡逻目标点

所以这轮整理的关键不是复述旧方案，而是把“仍然有效的架构边界”和“已经落地的当前工程真相”收束到一起。

## 2. 当前最稳的总抽象：`Reposition` 和 `Patrol`

这条历史判断仍然值得保留。

更稳妥的理解仍然是：

1. 战斗中的换位、侧移、绕位、短暂拉开，都更适合归到 `Reposition`
2. 脱战或待机时围绕 Home 区域的小范围游走，更适合归到 `Patrol`

这样做的好处是：

1. 不把战斗走位局限在 `Strafe`
2. 不把巡逻和战斗换位做成两套完全无关的系统
3. 可以让两者共享底层执行件，只在上下文和切换条件上分化

## 3. 当前底层职责拆分仍应坚持“选点 + 移动 + 朝向”

这也是当前源码已经在兑现的方向。

### 3.1 选点

当前选点层已经有明确入口：

1. `STT_RunEQSSelectLocation`
2. `AOEnvQueryContext_CurrentTarget`
3. `AOEnvQueryContext_PatrolAnchor`

这说明当前项目里，“去哪”已经开始通过 EQS 与上下文来表达，而不是继续写死在某个移动 Task 里。

### 3.2 移动

当前移动层至少已经分成两类：

1. `STT_MoveToTarget`
2. `STT_MoveToLocation`

这条边界很关键，因为它明确区分了：

1. 跟随某个 Actor
2. 走向某个位置点

这也是走位与巡逻以后能共享底层的前提。

### 3.3 朝向

当前朝向层已经有独立收口：

1. `STT_RotateControlTowardTarget`

它不和 `MoveTo` 混写，也不和攻击输入脉冲混写。  
这一点和历史方案的方向是对的，而且当前代码已经明显按这个边界在组织。

## 4. `PatrolAnchor` 和 `PatrolTargetLocation` 的分层必须单独记住

这块是当前工程里最值得沉淀的项目化认识之一。

`AAOAIPlayerBotController` 当前已经明确维护两类位置：

1. `PatrolAnchorLocation`
2. `PatrolTargetLocation`

它们不是近义词。

更稳妥的理解是：

1. `PatrolAnchorLocation` 是稳定中心  
   用来表达脱战后、回家后、巡逻时应围绕哪个 Home 区域展开。
2. `PatrolTargetLocation` 是本轮具体目的地  
   用来表达“这一次巡逻/换位真正要走到哪里”。

如果把这两个概念混着写，巡逻、脱战回位、EQS 选点、换位移动这几类问题就会被混成一团。

## 5. 当前 `PatrolAnchor` 已经不是纯设想

当前控制器源码已经表明：

1. `OnPossess(...)` 首次接管 Pawn 时，会记录默认锚点
2. `ResetPatrolAnchorLocationToPawn()` 会把锚点重置到当前 Pawn 位置
3. `AOEnvQueryContext_PatrolAnchor` 就是当前 EQS 读取 Patrol 中心的正式上下文入口

并且 `AOEnvQueryContext_PatrolAnchor` 里还专门处理了“没显式配置锚点时先退回自身位置”的兜底逻辑。

因此 Patrol 现在已经不是“连中心点都没有的纯方案层”，而是已经开始有稳定上下文真相。

## 6. 当前 `MoveToTarget` 的职责已经比较稳

它当前的稳定职责不是“通吃一切移动”，而是：

1. 尝试解析一个目标 Actor
2. 如果外部没绑定，就回退到 `AAOAIPlayerBotController::CurrentTarget`
3. 用 `UAITask_MoveTo` 执行追踪
4. 在目标变化时刷新或重启追踪

也就是说，它当前更接近“追目标 Actor 的执行件”，而不是“所有位移逻辑都往里塞的大容器”。

## 7. 当前 `MoveToLocation` 已经是真实存在的“走向点位”执行件

这点直接推翻了历史方案里“还建议新增”的那部分描述。

当前工程里，`STT_MoveToLocation` 已经存在并负责：

1. 读取位置目标
2. 生成 `FAIMoveRequest`
3. 用 `UAITask_MoveTo` 执行
4. 作为独立 StateTree Task 复用

这说明当前代码已经明确承认：

- “追 Actor” 和 “走向位置” 应当拆成两个移动执行件

后续知识库不能再把这条写成未来设想。

## 8. 当前 `Run EQS Select Location` 也已经是正式入口

历史文档里把它写成建议新增，但现在源码已经给出：

1. `STT_RunEQSSelectLocation` 负责跑 EQS
2. `QueryTemplate`、`RunMode`、命名参数等作为输入
3. 结果位置可写入父状态或全局共享参数

这说明当前项目在 StateTree 层面已经具备比较明确的“先求点、再走过去”的基础执行模式。

因此这条知识应升级成当前事实：

- EQS 选点已经不是纸面方案，而是已经进入实际工程能力面。

## 9. 但完整的高层 `Reposition / Patrol` 状态组织仍未完全收口

这里要把边界收紧，避免另一种过度表述。

虽然底层件已经存在，但目前还不能轻率写成：

- 当前项目已经完整接好一整套成熟的 `Reposition / Patrol` 高层行为树

更准确的说法应是：

1. 底层执行件已经进入工程事实
2. Patrol 的稳定中心、目标点、EQS 上下文、位置移动入口都已经出现
3. 但高层状态组织、切换策略、节奏策略是否在所有敌人资产上完整接通，仍需继续按资产和运行时链验证

## 10. `Reposition` 不应该默认升级成 GAS 技能

历史方案这里的判断仍然有效。

普通战斗换位的核心仍然是：

1. 去哪里
2. 怎么走
3. 走时看向哪里

这些本质上仍属于：

1. 决策层
2. 导航层
3. 朝向层

因此普通换位仍更适合走框架链，而不是一开始就技能化。

只有在下面这些情况下，才更适合升格成能力：

1. 需要 Root Motion
2. 需要冷却/消耗
3. 需要无敌帧、霸体或强动作生命周期
4. 需要玩家与 AI 共用完全同一套战斗位移动作

## 11. 当前最稳的推进原则仍然是“小闭环优先”

从当前代码状态反推，最稳的方向仍然是：

1. 保持选点、移动、朝向三层继续拆开
2. 让 `Run EQS Select Location -> MoveToLocation -> RotateControlTowardTarget` 这条链先闭环
3. 再按战斗 `Reposition` 和脱战 `Patrol` 去组织上层状态

不要再回退成：

1. 一个超大 Task 包办所有事
2. 先技能化所有普通位移
3. 或把巡逻和战斗换位写成两套完全平行的系统

## 12. 当前真正有价值的项目化借鉴点

如果只保留对后续最有用的结论，这一轮最值得沉淀的是：

1. Patrol 中心和 Patrol 目标必须分开建模
2. Actor 跟随和位置点移动必须分开建模
3. EQS 负责“去哪”，MoveTo 负责“怎么去”，Rotate 负责“怎么看”
4. 普通 Reposition / Patrol 先走框架，不默认升格成能力
5. 高层状态组织是否完整接通，要继续用当前资产和运行时链验证，不能只看底层件存在就直接宣告完成

## 13. 适用范围与不适用范围再收一次

### 13.1 适用范围

1. 解释当前项目中 Reposition / Patrol 的底层执行骨架。
2. 解释 Patrol 锚点、Patrol 目标点、EQS 上下文的真实边界。
3. 解释为什么普通走位不应默认做成能力。
4. 为后续 StateTree 资产接线与问题排查提供稳定概念基线。

### 13.2 不适用范围

1. 不能据此断言所有敌人都已完整接通成熟 Patrol 行为。
2. 不能把底层执行件存在，直接等同于高层状态组织已经全部完成。
3. 不能把未来战术层意图、完整节奏调参和具体资产接线细节混入本文。

## 14. 关联文档

- [[AI 项目地图]]
- [[AI 战斗输入旋转与翻滚执行链]]
- [[AI 决策已锁定设计]]
- [[StateTree AI 项目地图]]

