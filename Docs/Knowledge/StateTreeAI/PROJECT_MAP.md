# StateTree AI 项目地图

更新时间：2026-05-14  
适用范围：`StateTree AI` 运行时链路定位、目标状态追踪、巡逻状态追踪、StateTree 生命周期排查。  
不适用范围：具体 StateTree 资产编辑细节、蓝图图表排布、策划参数设计说明。

## 1. 文档定位

这份文档不是源码摘要，也不是功能说明书。它的作用只有一个：  
当我需要排查 `StateTree AI` 问题时，能在最短时间内回答下面四个问题：

1. 这条链路的运行时状态真相放在哪里。
2. 这些状态由谁写入、由谁消费。
3. StateTree 为什么会启动、停止、重置。
4. 出问题时应该优先钻进哪几个入口，而不是在整个仓库里盲搜。

因此，这份文档优先解决的是“定位成本”，不是“讲全所有实现”。

## 2. 当前场景边界

本知识包当前覆盖的是敌人 AI 的 C++ 运行时主链路，重点包括：

- `AAOAIPlayerBotController` 上维护的目标与巡逻状态
- `UAOStateTreeComponentBase` / `UAOAILogicStateTreeComponentBase` 的生命周期
- Evaluator、Task、Condition 如何消费这些运行时状态
- Possess / UnPossess 时状态与逻辑如何收敛

本轮**不主动覆盖**下面内容：

- 具体某棵 StateTree 资产在编辑器中的节点排布
- 蓝图层面行为图和策划参数配置
- 全部 AI 感知系统的上游实现

原因很简单：第一版知识源要先把最稳定、最常被复用、最容易误判的 C++ 真相源固定下来。

## 3. 我先看哪一层

排查顺序固定成三层，不要跳。

### 3.1 第一层：运行时状态主持有者

先看：

- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.h`
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`

因为当前已确认的三个关键运行时状态都挂在这里：

- `CurrentTarget`
- `PatrolAnchorLocation`
- `PatrolTargetLocation`

只要这三个状态没理解清楚，后面的 StateTree 节点分析通常都会查偏。

### 3.2 第二层：StateTree 生命周期

再看：

- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp`

因为很多看起来像“节点不工作”的问题，根因其实是：

- 树没被设置
- 逻辑没启动
- 组件被停掉后没恢复
- 动态加组件时序导致重启链没走到

### 3.3 第三层：StateTree 消费节点

最后再看：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_FindNearestTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.cpp`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.h`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.cpp`
- `Source/AegisOdyssey/StateTree/Conditions/STC_ActorHasMatchTag.cpp`
- `Source/AegisOdyssey/StateTree/Tasks/STT_GrantPersistentStateTags.cpp`

这些类不是运行时真相源，而是运行时真相的消费者。

## 4. 运行时状态真相源

### 4.1 `CurrentTarget`

当前设计里，目标状态的第一真相源在 `AAOAIPlayerBotController`。

关键接口：

- `GetCurrentTarget()`
- `SetCurrentTarget(AActor* NewTarget)`
- `GetDistanceToCurrentTarget()`

这层设计很重要，因为 `SetCurrentTarget()` 不只是改一个指针，它还同时承担焦点同步：

- `NewTarget != nullptr` 时会 `SetFocus`
- `NewTarget == nullptr` 时会 `ClearFocus`

也就是说，**<u>“当前目标”与“当前 Gameplay 焦点”在这里被绑定成同一个状态边界</u>**。  
如果后续某个 Task 只在局部缓存里换目标，而不经过 `SetCurrentTarget()`，那就会出现“表面上目标变了，但焦点和追击行为还停在旧状态”的问题。

### 4.2 `PatrolAnchorLocation`

这是更稳定的巡逻参考中心。  
在 `OnPossess()` 里，如果控制器还没有 Anchor，就会记录 Pawn 当前坐标作为默认锚点。

这意味着它不是“这一帧想去哪”，而是“这只 AI 当前应围绕哪个家园中心活动”。

### 4.3 `PatrolTargetLocation`

这是当前一轮巡逻真正要去的点。  
它比 Anchor 更短命，也更容易被状态流转覆盖。

`OnUnPossess()` 会清理 Patrol Target，而不会顺手清空 Patrol Anchor。  
这两个变量如果混着理解，脱战回位、回家巡逻、EQS 寻点这几类问题就会非常难查。

## 5. 目标写入链路

### 5.1 感知结果进入 Controller

我先看 `Source/AegisOdyssey/Character/Enemies/AOEnemyBotController.cpp`。

当前已确认的关键点非常直接：

- `AAOEnemyBotController::SetSenseResultActor_Implementation`
- 内部会调用 `SetCurrentTarget(SenseResultActor)`

这说明当前目标不是在某个 Evaluator 里凭空算出来的，而是先被写回 Controller，再由 StateTree 读取。

这个顺序决定了排查方式：

1. 先确认目标有没有写进 Controller。
2. 再确认 StateTree 有没有读到它。
3. 最后才看具体节点为什么没按预期转。

### 5.2 主动搜目标写回 Controller

我再看 `STT_FindNearestTarget.cpp`。

这个 Task 的职责不是“做完整攻击决策”，而是：

- 搜索候选目标
- 选最近的目标
- 调用 `AIController->SetCurrentTarget(NearestTarget)`

因此它和感知链路的共同点在于：  
**都会把最终目标真相收敛到 `AAOAIPlayerBotController::CurrentTarget` 上。**

这就是当前知识包里最重要的一条设计理解：  
不要把“目标状态”理解为散落在多个节点中的局部事实，它在当前实现里是有统一落点的。

## 6. StateTree 生命周期链路

### 6.1 `UAOStateTreeComponentBase`

这是项目通用的 StateTree 组件基类。

当前最关键的行为只有两个：

- `bStartLogicAutomatically = false`
- 组件默认启用复制

这已经足够说明一个排查原则：  
如果树没跑，默认不能假设“它应该自己跑起来”。

### 6.2 `UAOAILogicStateTreeComponentBase`

这个类是在 AI 侧对通用基类再包一层。

当前已确认的职责：

- 在 `InitializeComponent()` 之前调用 `ApplyDefaultStateTreeIfNeeded()`
- 如果还没配置树且 `DefaultStateTree != nullptr`，就 `SetStateTree(DefaultStateTree)`
- 在 `UninitializeComponent()` 和 `EndPlay()` 里执行 `StopLogic`
- `FullReset()` 当前只做 `InstanceData.Reset()`

这里的重点不是“它有没有很多逻辑”，而是它给出了一套生命周期约束：

- 树资产允许来自默认配置
- 组件销毁和结束播放时会显式停逻辑
- reset 当前是轻量 reset，不等于完整重新建模

### 6.3 Possess 后的重启链路

`AAOAIPlayerBotController::OnPossess()` 里有一段非常关键的补偿逻辑：

- 扫描 Pawn 身上所有 `UAOStateTreeComponentBase`
- 如果组件存在且已经拿到 StateTree 资产，就 `RestartLogic()`

代码注释已经把原因说得很明白：

- 这些 StateTree 组件可能是在 `SetPawnData()` / `GameFeature AddComponent` 阶段动态加上去的
- 所以它们可能错过更早的 PossessedBy 时机

这意味着这条重启链路不是“可有可无的小优化”，而是当前 AI 运行时能不能重新接起来的一条补偿入口。

## 7. StateTree 消费链路

### 7.1 `STE_UpdateCurrentTarget`

这个 Evaluator 的定位非常清晰，它不是做决策，而是做投影：

- 从 Controller 读取 `CurrentTarget`
- 计算 `DistanceToTarget`
- 计算 `bIsInAttackRange`
- 输出 `bHasTarget`

同时它还会尝试从武器定义读取 AI 攻击距离，如果拿不到，退回默认值 `200.0f`。

所以遇到“距离判断很怪”的问题时，要先拆成两个问题：

1. Target 有没有读对。
2. 距离阈值到底来自默认值还是武器定义。

### 7.2 `STT_MoveToTarget`

这个 Task 的职责是把目标转成 `FAIMoveRequest`，并驱动 `UAITask_MoveTo`。

当前有两个特别容易误判的点：

第一，`TargetActor` 并不一定完全依赖外部绑定。  
如果没显式绑定，它会 fallback 到 `AAOAIPlayerBotController::GetCurrentTarget()`。

第二，代码里区分了两种变化：

- `bTrackMovingGoal`：同一目标在移动
- `bTrackTargetActorChanges`：目标 Actor 本身换了

这两个语义如果混了，排查“追不上移动目标”和“切目标后还追旧目标”时就会一直绕圈。

### 7.3 `STC_TargetWithinDistanceRange`

这个条件判断本身很直白，但它有一个非常重要的前提：

- 必须拿到有效的 `TargetActor`
- 必须拿到正确的 `DistanceToTarget`

如果 `TargetActor == nullptr`，当前实现会直接按 false 处理，再和 `bInvert` 组合。  
因此它特别容易制造一种假象：  
看起来像 Min / Max 配置不对，实际上是输入根本没绑定进来。

### 7.4 `STC_ActorHasMatchTag`

这个条件在排查时不能只看 Tag 名字，还要看 ASC 解析路径。

当前实现会优先尝试从这些对象上找 ASC：

- `PlayerState`
- `Pawn`
- `Controller`
- `AOExtPawnComponent`
- 最后退回 `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)`

也就是说，问题不一定出在“有没有这个 Tag”，也可能出在“当前这个节点实际是从哪个对象读 Tag”。

## 8. 当前最小主链路

如果把“AI 找到目标并开始追击”压成最小闭环，当前可以按下面理解：

1. 感知或搜索逻辑找到候选目标。
2. 目标写回 `AAOAIPlayerBotController::CurrentTarget`。
3. `STE_UpdateCurrentTarget` 把 `CurrentTarget` 和距离信息投影成可绑定数据。
4. `STC_TargetWithinDistanceRange` 等条件根据这些数据决定分支。
5. `STT_MoveToTarget` 读取目标并发起或更新移动任务。

这条链路里，Controller 是状态真相源，Evaluator 是状态投影层，Task / Condition 是消费层。

## 9. 当前最小巡逻链路

如果问题是巡逻相关，当前最小链路要换一种看法：

1. `OnPossess()` 首次确定是否初始化 `PatrolAnchorLocation`
2. 巡逻相关逻辑读取 Anchor 作为稳定中心
3. 某一轮具体巡逻行为写入或读取 `PatrolTargetLocation`
4. `OnUnPossess()` 退出控制时清掉 Patrol Target，避免短命目标点污染下轮逻辑

因此，巡逻类问题必须先问自己：  
我现在查的是“家在哪”，还是“这轮要去哪”。

## 10. 最容易误判的 6 个位置

### 10.1 把 Task 当成真相源

很多人会先改 `MoveTo` 或距离条件，但目标的第一真相源其实在 Controller。

### 10.2 假设树会自动启动

当前基类默认 `bStartLogicAutomatically = false`，所以“树没跑”不是异常情况，而是必须显式接上的行为。

### 10.3 忽略 `OnPossess()` 里的补偿重启

如果动态加组件时序变了，这里就是第一排查入口。

### 10.4 把 Patrol Anchor 和 Patrol Target 当成一个变量

这会直接把巡逻、脱战回位、EQS 寻点三类问题混成一团。

### 10.5 只盯条件公式，不查输入绑定

`STC_TargetWithinDistanceRange` 这类节点特别容易在“输入根本没值”的情况下伪装成逻辑错误。

### 10.6 把“目标移动”和“目标切换”混成同一类问题

一个看 `bTrackMovingGoal`，一个看 `bTrackTargetActorChanges`，修法不是同一套。

## 11. 当前关键代码入口

- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.h`
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`
- `Source/AegisOdyssey/Character/Enemies/AOEnemyBotController.cpp`
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_FindNearestTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.cpp`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.h`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.cpp`
- `Source/AegisOdyssey/StateTree/Conditions/STC_ActorHasMatchTag.cpp`
