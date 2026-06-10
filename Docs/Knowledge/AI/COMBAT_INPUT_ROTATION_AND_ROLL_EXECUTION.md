---
title: AI Combat Input Rotation And Roll Execution
tags:
  - knowledge
  - ai
  - combat
  - input
  - rotation
  - roll
aliases:
  - AI Combat Input Rotation And Roll Execution
  - AI战斗输入旋转与翻滚执行链
---

# AI 战斗输入旋转与翻滚执行链

更新时间：2026-05-19  
适用范围：当前项目里 AI 如何通过统一输入链触发攻击/翻滚，如何用控制旋转驱动朝向，以及这些链路如何被 StateTree 消费。  
不适用范围：未来 PPO / LLM 决策接入细节、某个具体敌人资产的节奏调参值、尚未进入源码的理想化输入方案。

## 1. 这篇文档要收住什么

这一轮真正需要收住的，不是“AI 也会攻击”这种结果，而是三条已经进入当前工程真相的执行边界：

1. AI 不直接指定播放哪一招，而是尽量复用玩家输入链。
2. 攻击输入、持续脉冲输入、控制朝向旋转已经拆成独立 Task。
3. 翻滚方向、轻攻击朝向、格挡朝向已经统一依赖 `AController::GetControlRotation()`。

这三条边界决定了后续 AI 战斗表现该往哪里扩，也决定了排查输入/朝向异常时应该先看哪层。

## 2. 当前 AI 战斗执行链的稳定骨架

更稳妥的理解是：

1. 上层决策层决定当前要不要攻击、翻滚、格挡。
2. StateTree Task 负责把这个决策翻译成一次输入、持续输入，或一个持续旋转动作。
3. `UAOHeroComponent` 负责把 AI 输入桥接进和玩家一致的输入消费链。
4. 具体能力、输入缓冲、连招窗口、蒙太奇选择继续由现有 GAS / InputBuffer / Ability 侧负责。

因此当前项目里，AI 战斗执行已经不是“AI 直接调某个技能函数”，而是“AI 像玩家一样发输入，再让既有系统消费”。

## 3. `UAOHeroComponent` 已经是 AI 输入统一桥接入口

当前源码里，`UAOHeroComponent::InjectAbilityInputCommand(...)` 已经明确存在，并且内部直接复用：

1. `Input_AbilityInputTagPressed(...)`
2. `Input_AbilityInputTagStarted(...)`
3. `Input_AbilityInputTagReleased(...)`

这意味着当前正确表述应是：

- AI 侧输入统一先进入 `HeroComponent`
- `HeroComponent` 再按输入类型把信号送进 InputBuffer / 输入委托 / ASC
- `HeroComponent` 自己不承担“这是不是某个技能槽”的业务判断

这条边界很重要，因为它把 AI 输入和玩家输入收口到了同一条内部消费链上。

## 4. AI 注入输入后会主动补一次 `ProcessAbilityInput`

这块是当前工程里非常关键、也非常容易被后续误写掉的事实。

当前 `UAOHeroComponent::InjectAbilityInputCommand(...)` 在检测到 Pawn 是 Bot 后，会额外：

1. 通过 `UAOExtPawnComponent` 找到 `UAOAbilitySystem`
2. 立刻调用一次 `ProcessAbilityInput(...)`

这不是多余逻辑，而是当前 AI 能稳定消费输入的关键补链。

原因很简单：

1. 本地玩家原本有 `PlayerController` 驱动的持续输入泵。
2. AI 没有天然那条本地玩家输入泵。
3. 所以如果只把输入压进缓冲，不主动补这次处理，StateTree 本帧发出的输入就可能不能立刻被 ASC 消费。

因此后续任何“AI 发了输入但像没反应”的问题，都必须优先怀疑这条补链是否被绕开，而不是先怀疑能力本身。

## 5. `STT_SendCombatCommand` 的稳定语义是“一次性按一下”

当前它的定位应该保持收敛：

1. 它负责发送一次输入命令。
2. 它不是持续攻击服务。
3. 它不是连招状态机。
4. 它也不是专门服务某一个技能的特判 Task。

更准确地说，它表达的是：

- AI 在这一刻，像玩家一样按了一次输入。

如果要做单次闪避、单次格挡起手、单次喝药、单次技能释放，这个 Task 仍然是稳定入口。

## 6. `STT_SendCombatCommand` 的等待模式已经不是早期粗糙版本

历史文档里最容易过期的，就是把等待模式理解成“发一次输入，然后靠固定超时等技能播完”。

当前工程源码已经更细：

1. 它会先解析 `AbilitySystemComponent`
2. 会记录发送前匹配 `InputTag` 的 Spec
3. 会区分发送前已经处于激活态的旧 Spec
4. 会通过 `DynamicSpecSourceTags` 匹配输入来源，而不是只看静态 `AssetTags`

这说明当前更稳妥的表述应是：

- 等待模式在尝试识别“本次输入真正新激活出来的能力”，而不是笼统等待某个老能力或任意匹配 Tag 的技能结束。

因此这块应该继续被写成“精确识别本次输入触发能力的等待链”，而不是“带超时的输入任务”。

## 7. `STT_PulseCombatCommand` 解决的是“持续想攻击”而不是“单次输入”

这条边界也已经很明确。

当前 `STT_PulseCombatCommand`：

1. 进入状态后保持 `Running`
2. 内部维护下一次脉冲时间
3. 到点再次调用 `InjectAbilityInputCommand(...)`
4. 不直接判断是否应该切出当前状态

它当前支持的关键配置也已经落地：

1. `bSendImmediatelyOnEnter`
2. `PulseIntervalMin / PulseIntervalMax`
3. `DurationMin / DurationMax`
4. `MaxPulseCount`

并且还保留了运行时输出：

1. `PulseCount`
2. `ElapsedTime`
3. `DurationLimit`
4. `bLastPulseSucceeded`

因此当前最稳妥的理解不是“Attack 状态重进很多次”，而是：

- Attack 状态保持活着，脉冲 Task 在状态内部像玩家一样继续按键。

## 8. 当前攻击态更适合被理解为“持续施压阶段”

这不是纯概念，而是和当前 Task 设计直接对应的结论。

因为现在已经有：

1. `STT_PulseCombatCommand` 负责继续按键
2. `STT_RotateControlTowardTarget` 负责持续转向

所以当前 `Attack` 状态更准确的语义是：

- AI 当前处于持续施压、持续尝试输入的阶段

而不是：

- 这个状态等于某一次技能的完整生命周期

后续如果再把 `Attack` 状态写回“进一次、打一刀、等结束、退出来”，就等于把当前已经拆开的职责又揉回去了。

## 9. 旋转职责已经独立收口到 `STT_RotateControlTowardTarget`

当前这条链已经很清楚：

1. 目标方向先被解析成 `DesiredRotation`
2. 默认只持续修 Yaw，Pitch 可选
3. 通过 `RotationSpeed * RotationSpeedMultiplier` 计算有效转向速度
4. 最终改的是 `AIController->SetControlRotation(...)`

这说明当前稳定设计不是：

- 直接硬改 Actor 朝向

而是：

- 像玩家一样维护自己的控制朝向，再让能力和动作系统消费它

这也是为什么它更像“AI 在转鼠标”，而不是“AI 在瞬间锁脸”。

## 10. 翻滚、轻攻击、格挡的朝向语义已经统一成 `AController`

这点已经被当前源码明确证实：

1. `UAT_WaitRotateToDirection` 读取的是 `AController`
2. `GA_LightAttack` 读取的是 `AController->GetControlRotation()`
3. `GA_Block` 读取的是 `AController->GetControlRotation()`
4. `GA_Roll` 在方向蒙太奇选择时也读取 `AController->GetControlRotation()`

因此当前知识库里更准确的写法应该是：

- 这些能力的方向基础已经不是“只认玩家控制器”
- 而是“统一认控制器的 `ControlRotation`”

这正是 AI 和玩家共享方向语义的关键落点。

## 11. AI 翻滚已经进入统一链，而不是外挂动作

从当前源码看，翻滚相关入口至少已经包括：

1. `STT_SendCombatCommand`
2. `STT_PlayRollAnimation`
3. `STT_CalculatePlanarDirection`
4. `GA_Roll`

因此翻滚当前更适合被理解为：

1. 上层先决定要不要滚
2. StateTree 任务负责发翻滚输入、补方向数据或播放对应动作
3. `GA_Roll` 继续在能力层选择方向蒙太奇并消费输入数据

这说明翻滚不是另起一条“AI 自己的特制动作链”，而是已经尽量接入统一输入与方向体系。

## 12. 当前走位/巡逻执行层已经有比历史方案更完整的底层件

这轮核对里一个很关键的纠偏是：

历史方案文档还把下面这些写成“后续建议实现”：

1. `Run EQS Select Location`
2. `MoveToLocation`
3. Patrol EQS 上下文

但当前源码已经存在：

1. `STT_RunEQSSelectLocation`
2. `STT_MoveToLocation`
3. `AOEnvQueryContext_CurrentTarget`
4. `AOEnvQueryContext_PatrolAnchor`
5. `AAOAIPlayerBotController` 上的 `PatrolAnchorLocation` 与 `PatrolTargetLocation`

所以这块不能再写成“纯方案层”。  
更准确的表述是：

- 当前走位/巡逻底层执行件已经开始落地，但上层完整 Reposition / Patrol 状态组织仍需继续收束。

## 13. `MoveToTarget` / `MoveToLocation` / `EQS` 的职责边界已经比历史文档更清楚

当前源码已经体现出比较稳定的拆分：

1. `STT_RunEQSSelectLocation` 负责求位置
2. `STT_MoveToLocation` 负责走向位置
3. `STT_MoveToTarget` 负责跟随目标 Actor
4. `STT_RotateControlTowardTarget` 负责持续盯向目标

这说明当前应继续坚持：

- 选点、移动、朝向分层

而不是重新写回一个“大一统走位 Task”。

## 14. `SetFocus` 仍然是当前朝向手感的重要干扰项

当前 `AAOAIPlayerBotController::SetCurrentTarget(...)` 里仍然会：

1. 设置 `CurrentTarget`
2. 调 `SetFocus(NewTarget, EAIFocusPriority::Gameplay)`

所以当前项目的朝向真相不能被写成“完全只由 `STT_RotateControlTowardTarget` 决定”。

更准确的说法是：

1. 当前已经有独立的控制旋转 Task
2. 但 AIController 默认 Focus 体系仍然在参与朝向
3. 如果实测出现过强吸附感，首先要核 `SetFocus(...)`

## 15. 适用范围与不适用范围再收一次

### 15.1 适用范围

1. 解释 AI 当前如何复用玩家输入链。
2. 解释持续攻击为什么不该再靠状态重入模拟。
3. 解释旋转链、翻滚方向链、控制朝向链当前如何收口。
4. 解释走位/巡逻执行层哪些底层件已经真实落地。

### 15.2 不适用范围

1. 不能把 PPO / LLM 接入写成当前工程已落地事实。
2. 不能把完整 Patrol / Reposition 高层状态组织写成已经全部收口完成。
3. 不能把历史方案里“待实现”的说法继续当成当前源码真相。

## 16. 关联文档

- [[AI 项目地图]]
- [[AI 决策已锁定设计]]
- [[AI 战斗决策调参与算分说明]]
- [[AI Reposition 与 Patrol 框架]]

