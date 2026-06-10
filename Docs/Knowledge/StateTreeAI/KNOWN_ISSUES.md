# StateTree AI 已知问题与高频误判点

更新时间：2026-05-14  
适用范围：`StateTree AI` 排查时最常见的误判入口、历史型故障形态、反复出现的诊断偏差。  
定位：这份文档记录的是“哪里最容易查偏”，不是“当前生效规则”。

## 1. 文档目标

这份文档存在的价值，不是为了留档，而是为了让下次同类问题进来时，能够少走一轮弯路。

我只记录下面三类内容：

- 很容易被误判的表面现象
- 当前代码里已经能确认的风险边界
- 下次再遇到时应该先查什么，而不是先改什么

## 2. 问题一：StateTree 看起来没反应，不一定是节点错了

### 现象

- AI 没有进入预期行为
- 条件像是没触发
- MoveTo 像是没执行
- Evaluator 像是没更新

### 常见误判

- 直接去改某个 Task
- 直接怀疑某个 Condition 判断式

### 当前已知事实

- `UAOStateTreeComponentBase` 默认不自动启动
- `AAOAIPlayerBotController::OnPossess()` 里有手动 `RestartLogic()` 链路
- `UAOAILogicStateTreeComponentBase` 可能通过 `DefaultStateTree` 兜底配置树

### 正确起手

先查三件事：

1. 组件有没有拿到有效 StateTree 资产。
2. `OnPossess()` 后有没有真正执行重启。
3. 组件是不是在生命周期边界被 `StopLogic()` 停掉了。

只要这三件事没查清，改节点基本都属于盲修。

## 3. 问题二：目标问题经常被错判成移动问题

### 现象

- AI 不追目标
- AI 追错目标
- 切目标后还在追旧目标
- 目标已经无效，但表现还像有目标

### 常见误判

- 上来就改 `STT_MoveToTarget`

### 当前已知事实

- 目标真相在 `AAOAIPlayerBotController::CurrentTarget`
- `AAOEnemyBotController::SetSenseResultActor_Implementation` 会直接写 `SetCurrentTarget`
- `STT_FindNearestTarget` 也会直接写 `SetCurrentTarget`
- `STT_MoveToTarget` 只是目标消费者，不是目标真相源

### 正确起手

排查顺序必须是：

1. 目标是谁写进去的。
2. 写进去后 Controller 当前值是不是对。
3. `STE_UpdateCurrentTarget` 有没有把它正确投影出来。
4. 最后才看 MoveTo 是否按新目标重建请求。

## 4. 问题三：巡逻问题很容易被查成“随机移动问题”

### 现象

- AI 脱战后回错位置
- 巡逻点像在乱跳
- AI 看起来围绕错误中心活动

### 常见误判

- 把 Anchor 和 Target 当成同一类位置变量

### 当前已知事实

- `PatrolAnchorLocation` 是稳定中心
- `PatrolTargetLocation` 是本轮实际目标点
- `OnUnPossess()` 会清 Patrol Target，不会自动清 Patrol Anchor
- `AOEnvQueryContext_PatrolAnchor` 会从 Controller 读取 Patrol Anchor

### 正确起手

先问自己：  
这次错的是“家在哪”，还是“这一轮要去哪”。

如果这两个问题不分开，后面的 EQS、移动、脱战逻辑都会混起来。

## 5. 问题四：绑定缺失会伪装成条件逻辑错误

### 现象

- 距离条件始终不成立
- 某个分支永远进不去
- 条件代码看起来没问题，但表现始终不对

### 常见误判

- 直接怀疑 `STC_TargetWithinDistanceRange` 内部比较式

### 当前已知事实

- `STC_TargetWithinDistanceRange` 依赖 `TargetActor`
- 同时依赖 `DistanceToTarget`
- `TargetActor == nullptr` 时会直接按 false 处理

### 正确起手

先确认：

1. Evaluator 有没有产出对应值。
2. StateTree 绑定有没有把值喂到实例数据。
3. 最后才看 Min / Max 配置是否有问题。

## 6. 问题五：目标移动和目标切换是两类不同问题

### 现象

- 同一个目标移动时跟踪不稳定
- 目标换人后仍追旧目标

### 常见误判

- 把这两种都称为“MoveTo 不稳定”

### 当前已知事实

- `bTrackMovingGoal` 处理的是“同一目标在动”
- `bTrackTargetActorChanges` 处理的是“目标 Actor 引用变了”

### 正确起手

先给问题分类，再定修法。  
这两类问题的根因和调参点不是一套。

## 7. 问题六：Tag 问题不一定是“没这个 Tag”

### 现象

- 某个 `ActorHasMatchTag` 条件结果和预期不一致

### 常见误判

- 直接断言目标对象没有这个 Tag

### 当前已知事实

- `STC_ActorHasMatchTag` 会从多条路径解析 ASC
- 读取对象可能是 `PlayerState`、`Pawn`、`Controller` 或 `AOExtPawnComponent`

### 正确起手

先确认当前节点到底在从哪个对象读 ASC，再确认该对象是否具有目标 Tag。

## 8. 当前误判点汇总

当前高频误判点集中在：

- 把 StateTree 生命周期问题误判成节点逻辑问题
- 把目标真相源问题误判成 MoveTo 问题
- 把 Patrol Anchor 和 Patrol Target 混成同一类状态
- 把输入绑定缺失误判成条件实现错误
- 把目标移动和目标切换混成同一类跟踪问题
- 把 ASC 解析路径问题误判成目标对象没有 Tag
