# AI状态树与战斗输入交接文档

## 文档目的

这份文档不是给玩家看的说明书，而是给下一个接手本项目的 AI / 程序同事看的交接文档。  
目标是让接手者在最短时间内理解：

1. 这个项目当前的 AI StateTree / 战斗输入链路是怎么设计的。
2. 我们最近已经做了哪些改动。
3. 哪些问题已经解决，哪些问题还只是阶段性方案。
4. 后续继续推进时，哪些地方不能想当然，哪些设计约束必须遵守。


## 当前总设计结论

### 1. AI 不直接驱动“第几段连招”，而是驱动“输入”

当前项目已经明确采用统一输入链路：

- 玩家通过外设输入触发能力。
- AI 不再直接广播某个战斗 StateTree 组件，也不再直接“指定播放哪一招”。
- AI 的职责是像玩家一样发输入信号。
- 真正的连招衔接、预输入、输入缓冲、连招窗口，由现有 GAS + 动画 Notify + InputBuffer 体系消费。

这是一条非常重要的架构约束，后续不要再把 AI 战斗逻辑改回“AI 直接驱动技能状态机”的思路。


### 2. `STT_SendCombatCommand` 的职责已经固定

`STT_SendCombatCommand` 的定义已经明确：

- 它代表“一次性按一下键”。
- 就像玩家按下一次鼠标左键，或者按下一次闪避键。
- 它不是“持续攻击状态”，也不是“连招服务任务”。

也就是说：

- 单次动作输入，用它。
- 如果要模拟玩家持续点鼠标，不要修改它的语义，而是另外写“持续输入脉冲 Task”。


### 3. AI 的 Attack 状态不等于“当前技能生命周期”

这是这轮设计里最重要的认知修正。

过去容易误写成：

- 进入 Attack 状态
- 发一次轻攻击输入
- 等当前攻击结束
- 状态退出

但玩家并不是这么操作的。玩家是：

- 处于“持续想攻击”的阶段
- 在这个阶段里不断点鼠标
- 当前这一刀播没播完，和“要不要继续点下一次输入”不是一回事

因此现在 StateTree 的正确理解是：

- `Attack` 状态表达“当前处于攻击施压阶段”
- 在这个状态存活期间，内部可以不断发输入
- 连招窗口、预输入、输入缓冲自己决定这些输入如何被消费


### 4. 旋转和输入必须解耦

后来又引入了一个新的重要设计点：

- “像玩家点鼠标一样发输入”
- 和
- “像玩家转动视角一样逐渐转向目标”

这是两条不同职责，不能耦在一个 Task 里。

因此最终方案是：

- `STT_PulseCombatCommand`：持续输入脉冲
- `STT_RotateControlTowardTarget`：持续控制朝向

两个 Task 在同一个状态里并发运行即可。


## 当前已经完成的工作

---

### 一、动态装备 / 动态战斗组件链路已重构

已经完成一套“装备驱动 GameFeature 行为，但作用域只落在当前装备实例所属角色” 的机制。

核心文件：

- `Source/AegisOdyssey/Equipment/AOEquipmentDefinition.h`
- `Source/AegisOdyssey/Equipment/AOEquipmentInstance.h`
- `Source/AegisOdyssey/Equipment/AOEquipmentInstance.cpp`
- `Source/AegisOdyssey/Equipment/AOWeaponManagerComponent.cpp`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.cpp`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction_AddComponents.h`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction_AddComponents.cpp`

设计结论：

- 没有使用 UE 默认的 `UGameFeatureAction_AddComponents` 来做装备级组件授予。
- 因为默认版本偏向“全局规则 / 按类作用于整个 World”，不适合装备 A 只影响穿着它的这个 Actor。
- 现在改成了“装备实例拥有自己的 FeatureAction 执行与回收逻辑”。

当前效果：

- 装备时，能给当前 Actor 动态添加组件。
- 卸载装备时，能精确移除这次装备添加的组件。
- 适合以后“武器切换 -> 动态挂战斗 StateTree 组件”的路线。


---

### 二、`AOCombatStateTree` 已清理成组件自带资产入口

核心文件：

- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.cpp`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.h`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.cpp`

已经做掉的历史问题：

- 取消了额外的 `DefaultStateTree` 恢复逻辑。
- 统一使用组件模板上自带的 StateTree 资产配置入口。
- `AOCombatStateTree` 构造时已设置自动启动。
- 动态添加的 StateTree 组件在合适时机会自动启动 / 重启。

注意：

- `AOCombatStateTree` 和 `AOCombatLocomotionStateTree` 都会监听 `HeroComponent` 和 `InputBufferComponent` 的输入事件，并把它们转成 StateTreeEvent。
- 这也是为什么后面 AI 只要走统一输入链，战斗 StateTree 就能像玩家一样收到输入。


---

### 三、AI 自定义 `MoveTo` Task 已实现

核心文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.cpp`

背景：

- 当前 AI 没有用 `StateTreeAIComponent` 那套默认 Schema。
- 因此不能直接吃 UE 默认的 MoveTo Task。

当前实现特征：

- 支持从绑定的目标变量取目标。
- 若未绑定，则回退到 `AAOAIPlayerBotController::CurrentTarget`。
- 使用 `UAITask_MoveTo` 驱动。
- 支持动态目标变化时重启追踪。


---

### 四、距离 Condition 已实现

核心文件：

- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.h`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.cpp`

用途：

- 判断当前目标是否进入某个距离区间。
- 用于决定“是否可以从追击切到攻击”之类的分支。

设计要点：

- 这是 Condition，不是把距离判断硬塞进 `MoveTo` Task 里。
- 这样更符合后续扩展，也便于不同技能、不同攻击方式各自有不同的距离策略。


---

### 五、AI 输入链路已改成与玩家统一

核心文件：

- `Source/AegisOdyssey/Character/AOHeroComponent.h`
- `Source/AegisOdyssey/Character/AOHeroComponent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.cpp`

已经完成的关键点：

#### 1. 新增 `UAOHeroComponent::InjectAbilityInputCommand`

这个函数现在是 AI 输入统一入口。

作用：

- 直接复用玩家现有的输入处理函数
  - `Input_AbilityInputTagPressed`
  - `Input_AbilityInputTagStarted`
  - `Input_AbilityInputTagReleased`

也就是说，AI 发出来的输入，和玩家输入最终走的是同一条内部链路。


#### 2. Bot 注入输入时会立刻补一次 `ProcessAbilityInput`

原因：

- 玩家控制器原本会在 `AOPlayerController::PostProcessInput` 里每帧处理能力输入。
- AI 没有这条天然路径。

因此在 `InjectAbilityInputCommand` 里，如果检测到当前 Pawn 是 Bot，会立即调用一次能力输入处理，让 AI 输入不至于只进缓冲但不被消费。


#### 3. `STT_SendCombatCommand` 现在只代表“一次性按一下”

它目前支持两种语义：

- `bWaitForAbilityCompletion = false`
  - 发一次输入，立即成功
- `bWaitForAbilityCompletion = true`
  - 发一次输入，然后等待这次命令真正触发出来的能力结束


#### 4. 等待模式已经做过多轮修正

这个 Task 之前踩过几个坑，现在的设计要点必须记住：

- 不能用固定超时去硬判定“技能还没激活就算失败”。
- 不能只看 `AssetTags`，要看 `DynamicSpecSourceTags`。
- 不能把“同一个 InputTag 下本来就在播的旧能力”误判成“本次输入触发的新能力”。

因此现在的等待模式内部做了这些事：

- 发送输入前，记录所有匹配 `InputTag` 的 Spec。
- 同时记录发送前已经处于 Active 的那批 SpecHandle。
- 后续只等待“本次输入后新激活出来”的那个能力。
- 同一个 SpecHandle 再次激活时，也能重新被识别。


#### 5. ASC 查询优先级已经统一成 `PlayerState` 优先

这个项目里，ASC 查询不能想当然地从 Pawn 本体硬拿。

当前 `ResolveAbilitySystemComponent()` 的优先级是：

1. `PlayerState`
2. `Pawn->GetPlayerState()`
3. `Controller->PlayerState`
4. `UAOExtPawnComponent`
5. 最后才兜底 `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CommandTarget)`

这个约定是用户明确要求过的，后续不要改回去。


---

### 六、持续输入脉冲 Task 已实现

核心文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_PulseCombatCommand.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_PulseCombatCommand.cpp`

这个 Task 是为了解决一个核心问题：

- AI 想持续攻击时，不能靠“重进同一个 Attack 状态很多次”来模拟连点。
- StateTree 状态本身不应该用重入去模拟玩家持续点鼠标。

因此这个 Task 的职责是：

- 进入状态后持续 `Running`
- 内部维护下一次输入脉冲时间
- 到点就再次调用 `InjectAbilityInputCommand`

它不是专门服务普攻，而是一个通用输入模式：

- `STT_SendCombatCommand` = 按一次
- `STT_PulseCombatCommand` = 在当前状态里持续按很多次

#### 当前支持的配置

- `InputTag`
- `InputType`
- `bSendImmediatelyOnEnter`
- `InitialDelayMin / InitialDelayMax`
- `PulseIntervalMin / PulseIntervalMax`
- `DurationMin / DurationMax`
- `MaxPulseCount`

#### 当前输出

- `PulseCount`
- `ElapsedTime`
- `DurationLimit`
- `bLastPulseSucceeded`

#### 当前设计约束

- 它不等待技能结束。
- 它不判断“当前是否应该切走攻击态”。
- 它只负责“状态还活着时，像玩家一样再按一次键”。

也就是说，状态退出还是由外部 Condition / Transition 主导。  
`DurationMin/Max` 和 `MaxPulseCount` 只是内部可选收尾条件。


#### 重要调参结论

用户实测发现：

- 如果 `PulseIntervalMin / Max` 太慢，AI 会经常只打第一段，然后错过连招窗口。

这不是逻辑错，而是节奏参数问题。  
如果要压连招窗口，间隔需要明显比默认值更快。


---

### 七、控制朝向旋转 Task 已实现

核心文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_RotateControlTowardTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_RotateControlTowardTarget.cpp`

这个 Task 的出现，是为了让 AI 不再“硬锁玩家”，而是更像玩家自己转鼠标。

#### 设计原则

- 不直接转 Actor 面向目标。
- 不把旋转逻辑塞进输入脉冲 Task。
- 统一通过 `Controller->ControlRotation` 来表达“AI 的虚拟视角/控制朝向”。

这点非常关键，因为：

- 玩家本质上是转摄像机/ControlRotation 再发攻击。
- AI 虽然没有摄像机，但完全可以维护自己的 `ControlRotation`。

因此这个 Task 的职责是：

- 每 Tick 获取目标方向
- 平滑调整 `Controller->ControlRotation`
- 通过速度和容差来控制“像玩家转鼠标”的感觉

#### 当前支持的参数

- `TargetActor`
- `RotationSpeed`
- `RotationSpeedMultiplier`
- `YawTolerance`
- `AimOffsetYaw`
- `bUsePitch`
- `bContinuous`
- `MaxDuration`

#### 参数含义

- `RotationSpeed`
  - 基础转向速度，单位是每秒多少度
- `RotationSpeedMultiplier`
  - 外部倍率，用来做“蓄力时转慢”这类限制
- `YawTolerance`
  - Yaw 误差小于这个值，就认为基本对准了
- `bContinuous`
  - 为 `true` 时持续追踪
  - 为 `false` 时进入容差即结束


---

### 八、玩家和 AI 的朝向语义已做统一

核心文件：

- `Source/AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_LightAttack.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_Block.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Roll.cpp`

以前这些代码只认 `APlayerController`。

已经改成：

- 统一认 `AController`
- 统一读取 `Controller->GetControlRotation()`

这意味着：

- 玩家还是原来的行为
- AI 现在也能走同一套朝向语义

这是后面让 AI 攻击方向、格挡方向、翻滚方向和玩家统一的基础。


## 当前推荐的 StateTree 使用方式

### 攻击态

不要再用“进入攻击态 -> 发一次输入 -> 等技能结束 -> 状态退出”的老思路。

更推荐：

- 同一个 `Attack` 状态里并发运行
  - `STT_PulseCombatCommand`
  - `STT_RotateControlTowardTarget`

这样做的含义是：

- 一个 Task 负责像玩家一样持续点键
- 一个 Task 负责像玩家一样持续转鼠标

而不是让状态本身承担所有行为。


### 单次指令

以下场景优先继续使用 `STT_SendCombatCommand`：

- 单次闪避
- 单次格挡起手
- 单次喝药
- 单次技能释放
- 某些确实需要等待能力结束的动作


## 当前仍然需要注意的问题

### 1. `SetFocus` 仍然存在

文件：

- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`

当前 `SetCurrentTarget()` 里还会：

- `SetFocus(NewTarget, EAIFocusPriority::Gameplay);`

这意味着：

- 即便已经有了 `STT_RotateControlTowardTarget`
- AI 控制器本身仍可能通过 UE 默认 Focus 体系参与转向

如果后面实测发现：

- 朝向仍然太“吸附”
- 不像人为转鼠标

优先怀疑这里，而不是怀疑新旋转 Task。

本轮改动没有强删 `SetFocus`，是出于降低风险考虑。


### 2. `Notice` 目录下中文注释在某些终端会显示乱码

代码里已经尽量补了中文注释。  
但如果某些工具链默认编码不一致，PowerShell 输出可能会看到乱码。  
这不代表源码实际编码坏了，更多是终端显示问题。


### 3. 当前并没有完整接入 PPO / LLM

但现在的结构已经为此留好了公共接口位置：

- 持续输入节奏：以后可由随机规则换成模型输出
- 旋转速度倍率：以后可由模型给出
- 是否继续攻击：以后可由模型决定

而下层输入 Task 不需要跟着重写。


## 下一位 AI 接手时的建议顺序

### 如果继续调 AI 连招手感

优先看：

- `STT_PulseCombatCommand`
- 当前 StateTree 里 `PulseIntervalMin / Max`
- 动画 Notify 的连招窗口时长
- InputBuffer 是否正常消费

不要第一时间怀疑 `STT_SendCombatCommand`。


### 如果继续调 AI 朝向手感

优先看：

- `STT_RotateControlTowardTarget`
- `RotationSpeed`
- `RotationSpeedMultiplier`
- `YawTolerance`
- `AAOAIPlayerBotController::SetFocus`


### 如果继续做“蓄力时旋转变慢”

推荐方向：

- 不要在 AI 侧单独写死特判
- 让旋转 Task 读取与玩家一致的状态约束
- 最简单的是外部给 `RotationSpeedMultiplier`
- 更统一的是让 GA / Tag 系统也能对旋转速度做修正


### 如果继续做 PPO / LLM 接入

推荐不要直接改底层输入 Task，应该改上层决策输出：

- 是否继续攻击
- 下一次输入什么时候发
- 当前旋转倍率是多少

然后继续复用：

- `STT_PulseCombatCommand`
- `STT_RotateControlTowardTarget`
- `STT_SendCombatCommand`


## 当前关键文件索引

### 输入链路

- `Source/AegisOdyssey/Character/AOHeroComponent.h`
- `Source/AegisOdyssey/Character/AOHeroComponent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_PulseCombatCommand.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_PulseCombatCommand.cpp`

### 旋转链路

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_RotateControlTowardTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_RotateControlTowardTarget.cpp`
- `Source/AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h`
- `Source/AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_LightAttack.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_Block.cpp`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Roll.cpp`

### StateTree / 战斗组件

- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.cpp`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.h`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.cpp`

### AI 目标与移动

- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.h`
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.cpp`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.h`
- `Source/AegisOdyssey/StateTree/Conditions/STC_TargetWithinDistanceRange.cpp`

### 装备驱动动态组件

- `Source/AegisOdyssey/Equipment/AOEquipmentDefinition.h`
- `Source/AegisOdyssey/Equipment/AOEquipmentInstance.h`
- `Source/AegisOdyssey/Equipment/AOEquipmentInstance.cpp`
- `Source/AegisOdyssey/Equipment/AOWeaponManagerComponent.cpp`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.cpp`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction_AddComponents.h`
- `Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction_AddComponents.cpp`


## 当前阶段总结

到这一步为止，项目在 AI 战斗输入这块已经完成了一个比较清晰的统一框架：

- AI 和玩家共用同一条输入消费链
- 攻击不是“技能脚本化驱动”，而是“输入驱动”
- 旋转和输入被拆成两个低耦合 Task
- 装备系统也已经能支撑后续动态挂战斗组件

这意味着后续继续做：

- 更自然的 AI 连招
- 更像人的旋转
- 更复杂的武器战斗树
- PPO / LLM 决策接入

都已经有了可继续推进的基础，不需要再推翻当前整体架构。
