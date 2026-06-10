# AI战斗决策参数调参与计算说明

## 文档目的

这份文档专门回答三件事：

1. 现在这套 AI 战斗决策到底是怎么算分的。
2. 每个参数分别是什么意思，调大会怎样，调小会怎样。
3. 在编辑器里应该怎么配，尤其是距离响应曲线该怎么下手。

这份文档只描述当前已经落地到工程里的实现，不写“以后也许会这样”的空方案。

---

## 一、先说结论

当前这套决策系统的核心流程是：

1. `UpdateCurrentTarget` 先整理战斗事实。
2. `UpdateCombatDecision` 把这些事实喂给 `UAOAIDecisionComponent`。
3. `UAOAIDecisionComponent` 遍历每个 `IntentDefinition`，分别计算 `Desire` 和 `Score`。
4. 分数最高的意图成为当前的 `SelectedIntentTag`。
5. `StateTree` 不负责算分，只负责消费这个结果并进入对应状态。

当前真正的硬条件只有一个：

- `bRequireTarget = true` 时，没有目标就不评估这个意图。

像“攻击距离不够”这种条件，现在已经不是硬排除，而是通过距离因子去做“惩罚或奖励”。

---

## 二、现在有哪些关键数据

## 1. 武器上的 AI 攻击距离

文件：

- [AOWeaponDefinition.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h:20)

字段：

- `AIAttackRange`

作用：

- 只服务 AI 决策。
- 不参与现有普攻、技能、伤害判定等功能链。
- 当前武器切换后，AI 的距离语义也会跟着变。

当前取值来源：

1. 优先取当前武器的 `AIAttackRange`
2. 如果没有武器或取不到，内部回退到 `200.0`

---

## 2. 每个角色自己的决策配置

文件：

- [AOAIDecisionProfile.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h:10)

字段：

- `IntentDefinitions`

作用：

- 每个角色必须自己配一套意图表。
- 现在已经没有默认 Attack / Strafe 自动补全逻辑了。

---

## 3. 运行时事实

文件：

- [STE_UpdateCurrentTarget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp:58)
- [AOAIDecisionComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h:97)

当前每帧会整理这些事实：

- `CurrentTarget`
- `DistanceToTarget`
- `bHasTarget`
- `bIsInAttackRange`

其中：

- `DistanceToTarget = AI 到目标的真实世界距离`
- `bIsInAttackRange = DistanceToTarget <= 当前武器 AIAttackRange`

注意：

- `bIsInAttackRange` 现在主要是事实输出，方便你在 `StateTree` 里参考。
- 它现在不是决策组件里的硬排除条件。

---

## 三、总计算流程

文件：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:58)

每次评估时，流程如下：

### 第 1 步：整理距离比值

公式：

```text
DistanceRatio = TargetDistance / max(1.0, WeaponAIAttackRange)
```

含义：

- `DistanceRatio = 1.0`：刚好在当前武器攻击距离附近
- `DistanceRatio < 1.0`：比武器攻击距离更近
- `DistanceRatio > 1.0`：比武器攻击距离更远

例子：

- 当前武器 `AIAttackRange = 200`
- 实际距离 `100`
- 那么 `DistanceRatio = 0.5`

再比如：

- 当前武器 `AIAttackRange = 1000`
- 实际距离 `800`
- 那么 `DistanceRatio = 0.8`

这就是为什么近战和弓箭可以共用同一套“距离因子结构”，因为它先被归一化了。

### 第 2 步：遍历每个意图

每个 `IntentDefinition` 都会单独计算：

- `Desire`
- `Score`

### 第 3 步：选分数最高的

- 谁的 `Score` 最高，谁就是 `SelectedIntentTag`
- 如果分数并列，优先选 `FallbackSelectionWeight` 更高的
- 如果所有分数都小于等于 `0`，直接走保底选择逻辑

---

## 四、硬条件到底有哪些

文件：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:331)

当前 `CanEvaluateIntent()` 只保留了一个硬条件：

```text
如果 bRequireTarget = true 且当前没有目标
=> 这个意图不参与评估
```

也就是说：

- 没有目标时，像 Attack 这种必须依赖目标的意图可以直接不算
- 但“距离远不远”不会再把攻击意图直接踢掉

这正符合你前面定下来的设计思路：

- 距离是影响因素
- 不是决定因素

---

## 五、Desire 是怎么计算的

文件：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:286)

当前公式：

```text
CadenceAlpha = clamp(TimeSinceLastExecution / max(很小值, CadenceSeconds), 0, 1)

Desire = BaseDesire
       + CadenceAlpha * CadenceWeight
       + DistanceFactorContribution
```

可以把它理解成三部分：

1. 基础想法
2. 节奏恢复
3. 距离响应

---

## 六、Score 是怎么计算的

文件：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:297)

当前公式：

```text
Score = Desire

如果仍在冷却期内：
    Score *= CooldownPenaltyMultiplier

如果上一次执行的就是同一个意图：
    Score -= RepeatPenalty * RepeatedIntentCount
否则如果上一次执行的是别的意图：
    Score += SwitchBonus

Score = max(0, Score)
```

注意顺序：

1. 先拿到 `Desire`
2. 再做冷却乘法
3. 再做重复惩罚或切换奖励
4. 最后不允许低于 `0`

这意味着：

- `CooldownPenaltyMultiplier` 是乘法
- `RepeatPenalty` 和 `SwitchBonus` 是加减法

---

## 七、每个参数是什么意思

文件：

- [AOAIDecisionTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h:65)

## 1. `IntentTag`

作用：

- 这个意图最后输出给 `StateTree` 的标签。

常见例子：

- `AI.Intent.Attack`
- `AI.Intent.Strafe`

---

## 2. `bRequireTarget`

作用：

- 是否必须有目标才允许评估这个意图。

建议：

- 攻击、追击、绕圈这类依赖敌人的意图，一般开
- 脱战、回位、待机这类不一定依赖目标的意图，可以按需要关

---

## 3. `bRequireAttackRange`

作用：

- 这是旧语义保留字段。
- 当前已经不建议把它当作“距离不够就不准算”的硬门槛。

当前理解：

- 可以先忽略它的决策意义
- 真正的距离倾向，请优先通过 `DistanceFactor` 表达

---

## 4. `BaseDesire`

作用：

- 这个意图的基础欲望值。

调大效果：

- 这个意图天生更容易赢

调小效果：

- 这个意图更依赖其他因子才能赢

建议理解：

- 它像“角色性格底色”
- 不是距离
- 不是冷却
- 不是节奏

---

## 5. `CadenceWeight`

作用：

- 距离上一次执行越久，额外恢复多少欲望。

公式位置：

```text
Desire += CadenceAlpha * CadenceWeight
```

调大效果：

- 这个动作“憋一会儿之后更想做”

调小效果：

- 这个动作不太靠“等时间恢复”

---

## 6. `CadenceSeconds`

作用：

- 节奏恢复需要多长时间恢复满。

公式位置：

```text
CadenceAlpha = clamp(TimeSinceLastExecution / CadenceSeconds, 0, 1)
```

例子：

- `CadenceSeconds = 2`
- 距离上次执行 `1` 秒
- 那么 `CadenceAlpha = 0.5`

调大效果：

- 恢复更慢

调小效果：

- 恢复更快

---

## 7. `CooldownSeconds`

作用：

- 这个意图的冷却判定时间。

注意：

- 它不是“冷却期内绝对不能选”
- 它只是决定“是否进入冷却惩罚区间”

判断方式：

```text
TimeSinceLastExecution < CooldownSeconds
```

---

## 8. `CooldownPenaltyMultiplier`

作用：

- 仍在冷却期时，对分数乘上的系数。

例子：

- 当前 `Desire = 1.2`
- 还在冷却期
- `CooldownPenaltyMultiplier = 0.25`
- 那么冷却后变成 `0.3`

常见用法：

- `1.0`：等于没冷却惩罚
- `0.5`：冷却期内砍半
- `0.2`：冷却期内强烈压制
- `0.0`：冷却期内直接清零

---

## 9. `RepeatPenalty`

作用：

- 连续重复执行同一意图时，每次额外扣多少分。

公式：

```text
Score -= RepeatPenalty * RepeatedIntentCount
```

例子：

- `RepeatPenalty = 0.2`
- 当前这是连续第 3 次执行同一意图
- 那么本次会额外扣 `0.6`

适合用来避免：

- 连打
- 连续横移
- 一种动作霸屏

---

## 10. `SwitchBonus`

作用：

- 如果这次候选意图和上次执行意图不同，就给切换奖励。

效果：

- 鼓励动作切换
- 让行为更活

注意：

- 它不是“强制切换”
- 只是给换动作一小段额外分数

---

## 11. `FallbackSelectionWeight`

作用：

- 当所有意图最终分数都小于等于 `0` 时，用它决定保底选谁。

文件位置：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:341)

适合做什么：

- 保底偏向 `Strafe`
- 保底偏向 `Chase`
- 保底偏向某种更安全的行为

不适合做什么：

- 它不是正常评估阶段的主权重
- 它只在“大家都没分”时接管

---

## 八、距离响应因子怎么理解

文件：

- [AOAIDecisionTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h:11)
- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:266)

当前距离因子结构叫：

- `FAOAIDecisionResponseCurveFactor`

它不是“攻击距离专用结构”，而是一个通用的“输入 -> 曲线 -> 贡献值”结构。

现在只是先把它接到了距离上。

---

## 1. 总公式

```text
RawInput = DistanceRatio

EvaluatedInput = RawInput * InputScale + InputBias

如果 bClampInput = true：
    EvaluatedInput = clamp(EvaluatedInput, InputClampRange.X, InputClampRange.Y)

CurveValue = ResponseCurve(EvaluatedInput)

DistanceFactorContribution = CurveValue * Weight
```

最后这部分会加进 `Desire`。

---

## 2. `bEnabled`

作用：

- 是否启用这个因子。

注意：

- 关掉后，这整个因子完全不参与计算。

---

## 3. `Weight`

作用：

- 曲线输出值最终有多大影响力。

最重要的理解：

- 曲线决定形状
- `Weight` 决定力度

例子：

- 曲线采样结果是 `0.8`
- `Weight = 2.0`
- 最终贡献就是 `1.6`

再比如：

- 曲线采样结果还是 `0.8`
- `Weight = 0.25`
- 最终贡献只有 `0.2`

建议：

- 调“趋势”先改曲线
- 调“强弱”先改 `Weight`

---

## 4. `InputScale`

作用：

- 在采样曲线前，先对输入做缩放。

公式：

```text
EvaluatedInput = RawInput * InputScale + InputBias
```

机械理解：

- `InputScale > 1`：输入变化会被放大
- `InputScale < 1`：输入变化会被压小

建议：

- 大多数时候先保持 `1.0`
- 只有你明确觉得“同一条曲线反应太快或太慢”时再改它

---

## 5. `InputBias`

作用：

- 在采样曲线前，对输入做整体平移。

机械理解：

- `InputBias > 0`：同样的原始输入，会去采样更靠右的位置
- `InputBias < 0`：同样的原始输入，会去采样更靠左的位置

建议：

- 大多数时候先保持 `0.0`
- 如果你只是想把“最佳点”整体往前或往后挪，再考虑它

---

## 6. `bClampInput`

作用：

- 是否在采样前把输入限制在一个固定区间。

适合场景：

- 你只想调 `0~2` 这段距离比值
- 不想让特别离谱的超远距离把曲线采样跑飞

建议：

- 距离因子一般建议开

---

## 7. `InputClampRange`

作用：

- 输入限制范围。

常见建议：

- `0 ~ 2`

含义：

- `0` 代表贴脸
- `1` 代表正好在武器攻击距离
- `2` 代表超过武器攻击距离 100%

---

## 8. `ResponseCurve`

作用：

- 真正决定“什么距离更想做这件事”的核心形状。

你可以把横轴理解成：

```text
X = 当前距离 / 当前武器攻击距离
```

纵轴理解成：

```text
Y = 这个距离给该意图带来的原始倾向值
```

最后再乘 `Weight`。

---

## 九、怎么画距离曲线

## 1. 近战攻击曲线

目标：

- 距离过远时，不太想立刻攻击
- 接近合适距离时，攻击意图上升
- 太贴脸时可以按手感决定是否略降

推荐思路：

```text
X: 0.0   0.4   0.8   1.0   1.2   1.6   2.0
Y: 0.2   0.5   0.9   1.0   0.7   0.2   0.0
```

含义：

- 在武器攻击距离附近最想打
- 超过攻击距离越多，越不想硬打
- 但不是直接取消攻击资格

---

## 2. 远程攻击曲线

目标：

- 太近时不想射
- 中远距离更想射
- 特别远时再慢慢掉下来

推荐思路：

```text
X: 0.0   0.3   0.6   0.9   1.1   1.5   2.0
Y: 0.0   0.1   0.6   1.0   0.9   0.5   0.1
```

含义：

- 近了不舒服
- 拉开后更愿意输出

---

## 3. Strafe 曲线

目标：

- 当距离不理想时更倾向走位或调整
- 当已经特别理想时，走位欲望可以低一些

一个常见思路是让它和攻击曲线形成互补：

### 近战 Strafe

```text
X: 0.0   0.5   1.0   1.3   1.8   2.0
Y: 0.4   0.3   0.1   0.4   0.9   1.0
```

解释：

- 太远时更想调整位置
- 正好能打时，走位意图让一让攻击

### 远程 Strafe

```text
X: 0.0   0.3   0.8   1.0   1.5   2.0
Y: 1.0   0.8   0.3   0.2   0.5   0.8
```

解释：

- 太近时很想拉扯
- 正常输出位时降低走位冲动

---

## 十、推荐调参顺序

不要一口气同时改十个参数，推荐按这个顺序来：

### 第 1 步：先定武器距离

先给每把武器定一个靠谱的 `AIAttackRange`。

因为当前所有距离归一化都基于它。

如果这一步错了：

- 曲线怎么画都会别扭

---

### 第 2 步：只调距离曲线形状

先暂时把下面这些保持简单：

- `BaseDesire`
- `CadenceWeight`
- `CooldownPenaltyMultiplier`
- `RepeatPenalty`
- `SwitchBonus`

先只看：

- 攻击在什么距离更想发生
- 走位在什么距离更想发生

---

### 第 3 步：再用 `Weight` 调力度

如果趋势已经对了，但影响不够明显：

- 调 `Weight`

不是先乱改 `InputScale` 和 `InputBias`。

---

### 第 4 步：再调节奏

如果你发现：

- 角色总是连打
- 角色动作切换不自然
- 某个动作沉默太久

这时再去调：

- `CadenceWeight`
- `CadenceSeconds`
- `CooldownSeconds`
- `CooldownPenaltyMultiplier`
- `RepeatPenalty`
- `SwitchBonus`

---

## 十一、一个完整例子

下面给一个近战敌人的起步配置思路。

### Attack

- `BaseDesire = 0.4`
- `CadenceWeight = 0.5`
- `CadenceSeconds = 1.2`
- `CooldownSeconds = 0.8`
- `CooldownPenaltyMultiplier = 0.35`
- `RepeatPenalty = 0.15`
- `SwitchBonus = 0.05`
- `FallbackSelectionWeight = 0.2`

距离因子建议：

- `bEnabled = true`
- `Weight = 1.0`
- `InputScale = 1.0`
- `InputBias = 0.0`
- `bClampInput = true`
- `InputClampRange = (0, 2)`
- 曲线峰值放在 `0.8 ~ 1.0`

### Strafe

- `BaseDesire = 0.25`
- `CadenceWeight = 0.3`
- `CadenceSeconds = 0.8`
- `CooldownSeconds = 0.2`
- `CooldownPenaltyMultiplier = 0.8`
- `RepeatPenalty = 0.08`
- `SwitchBonus = 0.1`
- `FallbackSelectionWeight = 0.8`

距离因子建议：

- `bEnabled = true`
- `Weight = 0.8`
- `InputScale = 1.0`
- `InputBias = 0.0`
- `bClampInput = true`
- `InputClampRange = (0, 2)`
- 曲线在 `1.4` 之后逐渐升高

这样做的常见结果是：

- 离得合适时偏攻击
- 离得过远时偏走位/调整
- 不是硬切
- 而是自然倾向变化

---

## 十二、StateTree 里应该怎么接

当前推荐结构：

1. `UpdateCurrentTarget`
2. `UpdateCombatDecision`
3. `AttackSelector`
4. `PulseAttack`
5. `Strafe`

### `AttackSelector`

职责：

- 只负责根据当前结果选择子状态
- 自己不负责执行动作

建议：

- `Selection Behavior = TrySelectChildrenInOrder`

### `PulseAttack`

进入条件：

- `AI Decision Intent Matches`
- `ExpectedIntentTag = AI.Intent.Attack`

建议 Task：

1. `Commit AI Decision Intent`
2. 攻击执行 Task

### `Strafe`

进入条件：

- `AI Decision Intent Matches`
- `ExpectedIntentTag = AI.Intent.Strafe`

建议 Task：

1. `Commit AI Decision Intent`
2. 走位执行 Task

关键原则：

- 用 `SelectedIntentTag` 决定“进入哪个子状态”
- 不要在执行状态里用 `On Tick` 每帧反复看标签来切动作

---

## 十三、这套系统还包含哪些辅助节点

如果只看“算分公式”，这套系统还不算完整。

当前整套系统除了决策组件本体，还包含下面这些给 `StateTree` 用的辅助节点。

## 1. `AI Decision Intent Matches`

文件：

- [STC_AIDecisionIntentMatches.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.h:10)
- [STC_AIDecisionIntentMatches.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp:11)

作用：

- 判断“当前主意图是不是某个标签”。

典型用法：

- `ExpectedIntentTag = AI.Intent.Attack`
- `ExpectedIntentTag = AI.Intent.Strafe`

它适合做：

- 选择层分支进入条件

它不适合做：

- 执行中的每帧重切状态

---

## 2. `AI Decision Value In Range`

文件：

- [STC_AIDecisionValueInRange.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.h:10)
- [STC_AIDecisionValueInRange.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp:11)

作用：

- 查询某个意图当前的 `Desire`
- 查询某个意图当前的 `Score`
- 查询当前连续重复执行次数 `RepeatedIntentCount`

当前支持的 `ValueType`：

- `IntentDesire`
- `IntentScore`
- `RepeatedIntentCount`

典型用法：

- 攻击分数必须高于某个门槛才允许进某个强攻击状态
- 连续同一意图次数太多时，不允许继续进这个状态

例子：

```text
ValueType = IntentScore
IntentTag = AI.Intent.Attack
MinValue = 0.8
```

含义：

- 只有当攻击意图当前分数不低于 `0.8` 时，这个条件才成立

再比如：

```text
ValueType = RepeatedIntentCount
MaxValue = 2
```

含义：

- 连续重复执行次数超过 `2` 后，这个条件不成立

---

## 3. `Commit AI Decision Intent`

文件：

- [STT_CommitAIDecisionIntent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.h:10)
- [STT_CommitAIDecisionIntent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp:12)

作用：

- 某个状态一开始真正执行时，把“本次执行的是哪个意图”回写给决策系统

注意：

- 它回写的是“开始执行了什么”
- 不是“最后命中了什么”

这个回写非常重要，因为下面这些运行时量都依赖它：

- `LastExecutedIntentTag`
- `RepeatedIntentCount`
- `LastExecutedTime`

如果不放它：

- 冷却和重复惩罚都不会按你预期工作

---

## 4. `Reset AI Decision State`

文件：

- [STT_ResetAIDecisionState.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.h:9)
- [STT_ResetAIDecisionState.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp:11)

作用：

- 显式清掉这一轮战斗留下的决策记忆

它会清掉：

- `SelectedIntentTag`
- `LastExecutedIntentTag`
- `RepeatedIntentCount`
- 各意图运行时记录的 `Desire / Score / LastExecutedTime`

适合放在哪里：

- 丢失目标后的脱战状态
- 回巡逻状态
- 你明确想切断上一轮战斗记忆的状态

不建议乱放在什么地方都执行。

因为一旦重置：

- 节奏恢复
- 冷却记忆
- 重复惩罚

都会一起被清空。

---

## 5. `Debug Print On Screen`

文件：

- [STT_DebugPrintOnScreen.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/StateTree/Tasks/STT_DebugPrintOnScreen.h:12)
- [STT_DebugPrintOnScreen.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/StateTree/Tasks/STT_DebugPrintOnScreen.cpp:23)

作用：

- 在屏幕上打印一个可绑定的调试值

当前支持的值类型：

- 字符串
- `GameplayTag`
- `Name`
- `Object`
- `Float`
- `Int`
- `Bool`

推荐用途：

- 在 `Global Task` 里看 `SelectedIntentTag`
- 看 `LastExecutedIntentTag`
- 看某个 `Score`
- 看 `RepeatedIntentCount`

一个 Task 只负责一条信息。

如果你要同时看多个值：

- 就在 `Global Task` 里放多个这个 Task

---

## 十四、为什么屏幕上标签会疯狂切

原因通常不是打印 Task 出错，而是结构接法不对。

如果你把：

- `SelectedIntentTag`

拿去做执行状态里的 `On Tick` 切换依据，那么每次重评估时它都可能变化，于是状态会来回抖。

再加上进入执行状态后，`CommitExecutedIntent()` 会立刻回写执行记录并重新评估：

- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:110)

所以抖动会更明显。

正确接法不是：

- 在执行状态里每帧看标签换动作

而是：

- 回到选择层时，再根据当前标签选下一次动作

---

## 十五、调参时最常见的误区

### 误区 1：曲线形状不对，却一直改 `Weight`

问题：

- 趋势不对时，改力度没有意义

正确做法：

- 先改曲线形状
- 再改 `Weight`

### 误区 2：武器攻击距离不准，却怪曲线不准

问题：

- 当前所有距离语义都是相对当前武器距离算的

正确做法：

- 先确认 `AIAttackRange` 是否合理

### 误区 3：想做“冷却”，却只改 `RepeatPenalty`

问题：

- `RepeatPenalty` 是针对连续重复次数
- `CooldownPenaltyMultiplier` 才是冷却期压制

### 误区 4：想要动作更活，却把所有意图 `SwitchBonus` 都拉很高

问题：

- 会让角色过度跳动作

正确做法：

- 小幅加
- 结合 `RepeatPenalty` 一起看

---

## 十六、排查顺序

如果结果不符合预期，按这个顺序查：

1. 先查当前武器 `AIAttackRange` 对不对
2. 再查 `DistanceToTarget` 是否正常
3. 再查 `DistanceRatio` 大概落在哪个区间
4. 再查对应意图的曲线在这个区间输出什么值
5. 再看 `Weight` 后贡献有多大
6. 再看是否被冷却乘法压低
7. 再看是否被重复惩罚扣低
8. 再看是否被别的意图用更高分抢走
9. 最后再看是否走了 `FallbackSelectionWeight`

---

## 十七、当前代码参考

关键文件：

- [AOAIDecisionTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h:11)
- [AOAIDecisionComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h:50)
- [AOAIDecisionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:58)
- [AOAIDecisionProfile.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h:10)
- [STE_UpdateCurrentTarget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp:58)
- [STE_UpdateCombatDecision.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp:39)
- [STC_AIDecisionIntentMatches.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp:11)
- [STC_AIDecisionValueInRange.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp:11)
- [STT_CommitAIDecisionIntent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp:12)
- [STT_ResetAIDecisionState.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp:11)
- [STT_DebugPrintOnScreen.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/StateTree/Tasks/STT_DebugPrintOnScreen.cpp:23)

---

## 十八、文档状态

当前推荐你优先看这两份：

1. [AI当前进度与新会话交接说明.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI当前进度与新会话交接说明.md)
2. [AI战斗决策参数调参与计算说明.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI战斗决策参数调参与计算说明.md)

其中：

- 交接文档负责讲“现在项目做到哪了”
- 本文负责讲“当前实现是怎么算、怎么调的”

如果你再去翻更早的阶段文档，请以这两份现行文档为准。

---

## 十九、最后给一句实战建议

如果你现在只想把一只敌人调顺，不要一开始就追求“参数特别科学”。

最有效的做法通常是：

1. 先把武器 `AIAttackRange` 定准
2. 先把 Attack 和 Strafe 的距离曲线画出明显分工
3. 先让行为趋势对
4. 再用冷却、重复惩罚、切换奖励去修手感

先把“方向对”做出来，再去磨“质感”。
