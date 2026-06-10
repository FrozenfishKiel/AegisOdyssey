# StateTree AI 运行规则

更新时间：2026-05-14  
适用范围：`StateTree AI` 运行时规则、状态边界、输入约定。  
不适用范围：策划层数值规则、蓝图资产编辑规则、感知系统所有上游来源。

## 1. 文档目标

这份文档不解释“为什么这样设计”，它只做一件事：  
把当前已经能从代码中确认的运行规则写死，避免每次排查都靠记忆和口头解释。

一旦代码和这份文档冲突，处理顺序应该是：

1. 先确认是不是文档过期。
2. 如果文档没过期，以这里的规则为当前约束。
3. 如果规则已经变化，先更新文档，再继续让 AI 或人基于它工作。

## 2. 状态主持有规则

### 2.1 `CurrentTarget` 由 Controller 主持有

当前规则：

- `CurrentTarget` 挂在 `AAOAIPlayerBotController`
- 读取应优先通过 `GetCurrentTarget()`
- 变更应优先通过 `SetCurrentTarget()`

这条规则的含义是：  
**目标不是某个 Task 的局部私有状态，而是跨多个 StateTree 节点共享的控制器级运行时状态。**

### 2.2 巡逻相关状态也由 Controller 主持有

当前规则：

- `PatrolAnchorLocation` 挂在 `AAOAIPlayerBotController`
- `PatrolTargetLocation` 挂在 `AAOAIPlayerBotController`

这意味着巡逻问题优先看 Controller，而不是优先看 EQS 或某个单独节点。

## 3. 目标写入与清理规则

### 3.1 统一目标写入口

当前规则：

- 所有需要更新当前目标的逻辑，优先走 `SetCurrentTarget()`

因为 `SetCurrentTarget()` 当前不只是写指针，还同时承担：

- `SetFocus(NewTarget, EAIFocusPriority::Gameplay)`
- `ClearFocus(EAIFocusPriority::Gameplay)`

所以这里实际维护的是一个“目标 + 焦点”的联合边界。

### 3.2 统一目标清理边界

当前规则：

- `OnUnPossess()` 会清 `CurrentTarget`
- `OnUnPossess()` 会清 Gameplay Focus

因此：

- 脱离控制、死亡、重生、销毁后出现追旧目标、看向旧目标、条件仍然判定有目标时，优先检查 `OnUnPossess()` 边界是否走通

## 4. 巡逻状态规则

### 4.1 `PatrolAnchorLocation` 是稳定参考中心

当前规则：

- `OnPossess()` 首次接管 Pawn 时，如果还没有 Anchor，会记录 Pawn 当前位置
- 后续可以通过 `SetPatrolAnchorLocation()` 或 `ResetPatrolAnchorLocationToPawn()` 更新

它表达的是“这只 AI 当前应围绕哪里活动”，不是这轮巡逻的即时目的地。

### 4.2 `PatrolTargetLocation` 是本轮即时目的地

当前规则：

- 可以通过 `SetPatrolTargetLocation()` 设置
- 可以通过 `ClearPatrolTargetLocation()` 清空
- `OnUnPossess()` 会主动清空 Patrol Target

这说明它是短生命周期状态，不应被当成长期巡逻中心使用。

## 5. StateTree 生命周期规则

### 5.1 基类默认不自动启动

当前规则：

- `UAOStateTreeComponentBase` 默认 `bStartLogicAutomatically = false`

因此：

- StateTree 逻辑能否跑起来，不应依赖“组件自动开始”这一假设

### 5.2 AI StateTree 组件允许默认树兜底

当前规则：

- `UAOAILogicStateTreeComponentBase` 在初始化前会尝试 `ApplyDefaultStateTreeIfNeeded()`
- 如果当前没设置树，且 `DefaultStateTree != nullptr`，会 `SetStateTree(DefaultStateTree)`

因此：

- 排查树来源时，必须同时看显式配置与 `DefaultStateTree`

### 5.3 组件卸载与结束播放时会停逻辑

当前规则：

- `UninitializeComponent()` 会 `StopLogic`
- `EndPlay()` 会 `StopLogic`

因此：

- 遇到“树为什么停了”的问题，不要只看行为层，要回看组件生命周期

### 5.4 Possess 后需要显式重启逻辑

当前规则：

- `AAOAIPlayerBotController::OnPossess()` 会扫描 Pawn 上的 `UAOStateTreeComponentBase`
- 如果组件已持有有效树，会调用 `RestartLogic()`

这不是可选优化，而是当前动态加组件链路下的重要补偿机制。

## 6. Evaluator 输入投影规则

### 6.1 `STE_UpdateCurrentTarget` 是目标状态投影器

当前规则：

- 从 Controller 读取 `CurrentTarget`
- 产出 `CurrentTarget`
- 产出 `DistanceToTarget`
- 产出 `bIsInAttackRange`
- 产出 `bHasTarget`

因此：

- 这个 Evaluator 的职责是把 Controller 真相映射成 StateTree 可消费输入
- 它不是目标选择器，也不是攻击策略器

### 6.2 AI 攻击距离允许来自武器定义

当前规则：

- 优先尝试从当前武器定义读取 AI Attack Range
- 拿不到时退回默认值 `200.0f`

因此：

- “为什么距离判定和我体感不一致”时，要先确认攻击距离来源

## 7. Task / Condition 输入规则

### 7.1 `STT_FindNearestTarget` 可以直接写回目标真相

当前规则：

- 找到目标后会调用 `AIController->SetCurrentTarget(NearestTarget)`

所以它不是一个只输出局部变量的辅助节点，而是能改控制器级状态的节点。

### 7.2 `STT_MoveToTarget` 允许 fallback 到 Controller 当前目标

当前规则：

- 如果实例数据中已绑定 `TargetActor`，优先用显式绑定
- 如果没绑定，则回退到 `AAOAIPlayerBotController::GetCurrentTarget()`

因此：

- 修移动问题时，不能默认目标来源只有一种

### 7.3 目标 Actor 变化和目标位置变化是两套规则

当前规则：

- `bTrackMovingGoal` 处理“同一目标在移动”
- `bTrackTargetActorChanges` 处理“目标对象换了”

因此：

- 这两类问题不能混着下结论，也不能共用一套修法

### 7.4 距离条件依赖正确输入绑定

当前规则：

- `STC_TargetWithinDistanceRange` 依赖 `TargetActor`
- 同时依赖 `DistanceToTarget`
- `TargetActor == nullptr` 时会直接按 false 处理，再与 `bInvert` 组合

因此：

- 看到条件不成立时，先查输入绑定，再查距离阈值

### 7.5 Tag 条件依赖 ASC 解析路径

当前规则：

- `STC_ActorHasMatchTag` 会尝试从 `PlayerState`、`Pawn`、`Controller`、`AOExtPawnComponent` 等路径解析 ASC

因此：

- “没有这个 Tag” 和 “从错误对象上读了 Tag” 是两类不同问题

## 8. 默认排查层级

当前系统的默认排查层级为：

1. `AAOAIPlayerBotController` 运行时状态
2. StateTree 组件生命周期与启动链
3. Evaluator / Task / Condition 的消费逻辑
