# AI第四阶段战斗决策系统使用与实现说明

## 文档目的

这份文档不是继续讨论方案，而是把**当前已经落地到工程里的这一套 AI 战斗决策系统**讲清楚：

1. 它现在是怎么设计的
2. 它在代码里已经写了什么
3. 你在 StateTree 里该怎么接
4. 以后如果要加新的欲望/意图，应该从哪里扩
5. 如果运行结果不对，优先该查哪里

这份文档默认读者是：

- 现在正在接这套 AI 的你
- 以后可能重新开一个新 AI 会话继续接手的人
- 后续要做敌人差异化、加新欲望、调攻击欲望的人

---

## 一、这套系统现在到底解决什么问题

我们当前要解决的问题，不是“AI 会不会攻击”，而是：

- AI 不应该只靠 `Try Select Children At Random`
- AI 不应该一会一直攻击，一会一直走位，完全没节奏
- AI 需要有“攻击欲望”“走位欲望”这种可积累、可消耗、可配置的中间层
- StateTree 不应该自己硬算所有逻辑，它应该消费一个已经算好的“决策结果”

所以现在这套系统的核心职责分层是：

1. **事实层**
   - 当前有没有目标
   - 目标是谁
   - 离目标多远
   - 是否进入攻击范围

2. **决策层**
   - 根据事实层，评估多个意图
   - 每个意图都有自己的欲望值和最终得分
   - 最后选一个当前最优意图输出给 StateTree

3. **执行层**
   - StateTree 根据意图进入具体状态
   - 具体状态执行时，再把“我刚刚执行了什么意图”回写给决策层
   - 这样下一拍决策就有记忆，不是每帧都像失忆一样重新抽签

---

## 二、当前最终架构

当前这一套不是“写死 Attack / Strafe 的两个 if”，而是已经改成：

- **组件存运行时状态**
- **Profile 资产存敌人配置**
- **IntentDefinition 定义单个意图的评分规则**
- **StateTree 只消费结果**

也就是说，现在的正式结构是：

### 1. `UAOAIDecisionComponent`

这是 AI 决策运行时核心组件。

它负责：

- 保存事实层数据
- 保存每个意图的运行时欲望/得分/上次执行时间
- 根据当前事实层遍历所有意图并打分
- 选出当前主意图
- 在行为真正执行时记录“刚刚执行了哪个意图”

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`

### 2. `FAOAIDecisionIntentDefinition`

这是“单个意图的配置结构”。

它不是运行时状态，而是规则定义。

例如一个意图会定义：

- 自己的 `IntentTag`
- 是否需要目标
- 是否必须在攻击范围内
- 基础欲望
- 节奏恢复速度
- 距离奖励
- 冷却压制
- 重复惩罚
- 切换奖励
- 保底选择权重

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`

### 3. `UAOAIDecisionProfile`

这是敌人的**正式差异化配置资产**。

以后不同敌人的区别，不应该主要靠改组件源码，而应该主要靠这个资产：

- 理想攻击距离
- 参与评估的意图定义表

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.cpp`

### 4. `FSTE_UpdateCurrentTarget`

这是事实层 Evaluator。

它只负责输出：

- `CurrentTarget`
- `DistanceToTarget`
- `bIsInAttackRange`
- `bHasTarget`

它已经被重新收敛回“目标事实包装”本职，不再混入决策分数。

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`

### 5. `FSTE_UpdateCombatDecision`

这是决策层 Evaluator。

它不自己找目标，而是消费 `UpdateCurrentTarget` 的输出，再把结果写回 StateTree。

它当前输出：

- `SelectedIntentTag`
- `SelectedIntentDesire`
- `SelectedIntentScore`
- `ObservedIntentTag`
- `ObservedIntentDesire`
- `ObservedIntentScore`
- `bHasObservedIntentMetrics`
- `LastExecutedIntentTag`
- `RepeatedIntentCount`

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp`

### 6. StateTree Condition / Task

#### `FSTC_AIDecisionIntentMatches`

作用：

- 判断当前主意图是不是某个 Tag

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp`

#### `FSTC_AIDecisionValueInRange`

作用：

- 查询某个意图当前的欲望/得分是否在某个区间
- 或者当前重复执行次数是否在某个区间

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp`

#### `FSTT_CommitAIDecisionIntent`

作用：

- 当某个状态真正开始执行时，回写“这次我执行的是哪个意图”

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp`

#### `FSTT_ResetAIDecisionState`

作用：

- 显式清空这一轮战斗决策记忆

文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp`

---

## 三、代码里到底是怎么写的

这一部分说“思路”，不是逐行解释。

### 1. 决策组件怎么打分

现在不是写死：

- `AttackDesire`
- `StrafeDesire`

而是：

- 遍历 `IntentDefinitions`
- 每个意图单独算 `Desire`
- 再单独算 `Score`
- 最后选一个当前得分最高的 `SelectedIntentTag`

也就是说，主流程是：

1. 更新事实层
2. 遍历所有意图定义
3. 过滤掉当前不满足前置条件的意图
4. 对每个剩余意图计算：
   - 基础欲望
   - 节奏恢复
   - 距离奖励
   - 进/出攻击范围奖励
   - 冷却压制
   - 重复惩罚
   - 切换奖励
5. 选最高分 Tag 输出
6. 如果所有分数都被压成 0，则按 `FallbackSelectionWeight` 选一个保底意图

关键点：

- 主流程已经和“具体是 Attack 还是 Strafe”解耦
- 真正仍然保留 Attack / Strafe 名字的地方，只在“默认配置”里

### 2. 为什么还保留默认 Attack / Strafe

因为当前项目真实已经落地的业务，只有这两个意图。

所以：

- 架构已经做成可扩展
- 默认数据仍然用当前项目已经存在的两个真实意图初始化

这样是合理的。

不合理的是：

- 主流程里写死两个 if
- Condition 里写死 AttackScore / StrafeScore
- Evaluator 里写死攻击和走位两个输出字段

这些都已经被我拆掉或泛化掉了。

### 3. 为什么要有 `ObservedIntentTag`

因为 StateTree 调试时，你不一定只想看“当前被选中的主意图”。

例如你可能会想看：

- 当前虽然主意图是攻击，但 `Strafe` 分数是多少
- 或者将来你加一个 `AI.Intent.Retreat`，想单独看它这帧为什么没被选中

所以 `UpdateCombatDecision` 里设计了：

- `SelectedIntentTag`：当前真正胜出的意图
- `ObservedIntentTag`：你额外指定要观察的意图

如果 `ObservedIntentTag` 不填，就默认观察当前主意图。

### 4. 为什么执行反馈要在“进入状态时”回写

`CommitAIDecisionIntent` 不是等状态结束才记。

它是在状态**一开始执行**时回写。

原因很简单：

- 我们要的是“这一拍已经选择并开始执行了什么”
- 不是“最后这个动作有没有成功打中”

否则下一拍评分时，重复惩罚和节奏压制就会晚一拍才生效。

### 5. 为什么要有 `ResetAIDecisionState`

因为脱战、回巡逻、切回待机时，如果不显式清理记忆，上一轮战斗留下的：

- `LastExecutedIntentTag`
- `RepeatedIntentCount`
- 上次执行时间

会继续影响下一轮决策。

这在有些状态下是你要的，在有些状态下不是。

所以我没有把它强制绑定在父状态退出里，而是给你一个**显式 Task** 去决定“哪里清，哪里不清”。

---

## 四、你在编辑器里该怎么接

下面是推荐接法。

### 1. 角色 / 敌人蓝图侧

先确认这个敌人身上有：

- `AIDecisionComponent`

它已经在 `AAOCharacter` 构造里默认创建了。

如果你想做敌人差异化：

1. 新建一个 `AO AI Decision Profile`
2. 配置：
   - `IdealAttackDistance`
   - `IntentDefinitions`
3. 把它填到这个敌人的 `AIDecisionComponent.DecisionProfile`

如果你不填：

- 系统会自动用当前默认的 `Attack / Strafe` 两个意图

### 2. StateTree Evaluator 接法

推荐至少挂两个 Evaluator：

#### 第一个：`Update Current Target`

输出：

- `CurrentTarget`
- `DistanceToTarget`
- `bIsInAttackRange`
- `bHasTarget`

#### 第二个：`Update Combat Decision`

把上面四个输出绑定进它的四个输入：

- `CurrentTarget`
- `DistanceToTarget`
- `bIsInAttackRange`
- `bHasTarget`

这样决策层就会在每帧拿到最新事实层。

### 3. 攻击/走位状态怎么进

#### 如果你只想按主意图进状态

攻击状态条件：

- `AI Decision Intent Matches`
  - `ExpectedIntentTag = AI.Intent.Attack`

走位状态条件：

- `AI Decision Intent Matches`
  - `ExpectedIntentTag = AI.Intent.Strafe`

#### 如果你还想加门槛

例如攻击状态不仅要当前意图是攻击，还要求攻击分数足够高：

- `AI Decision Intent Matches`
  - `ExpectedIntentTag = AI.Intent.Attack`
- `AI Decision Value In Range`
  - `ValueType = IntentScore`
  - `IntentTag = AI.Intent.Attack`
  - `MinValue = 0.8`

例如不想让同一种行为连续太多次：

- `AI Decision Value In Range`
  - `ValueType = RepeatedIntentCount`
  - `MaxValue = 2`

### 4. 行为真正开始执行时怎么回写

攻击状态开始时，先放：

- `Commit AI Decision Intent`
  - `ExecutedIntentTag = AI.Intent.Attack`

走位状态开始时，先放：

- `Commit AI Decision Intent`
  - `ExecutedIntentTag = AI.Intent.Strafe`

这样这一拍真正执行了什么，决策组件才能记住。

### 5. 脱战 / 回巡逻怎么清记忆

在下面这类状态里推荐放：

- `Reset AI Decision State`

适合放的位置：

- 丢失目标后进入的脱战状态
- 回巡逻状态
- 强制重置战斗节奏的特殊父状态

---

## 五、以后如果要加新欲望，该怎么做

这里是最重要的扩展说明。

以后你如果要加新的欲望，例如：

- `AI.Intent.Retreat`
- `AI.Intent.Reposition`
- `AI.Intent.Block`
- `AI.Intent.Pressure`

**正常扩展顺序应该是：**

### 第一步：加 GameplayTag

在 `AOGameplayTags` 里增加：

- `AI.Intent.Retreat`

### 第二步：在 `DecisionProfile` 里增加一条 `IntentDefinition`

配置它的：

- `IntentTag`
- 是否要求目标
- 是否要求攻击范围
- 基础欲望
- 节奏恢复
- 冷却
- 重复惩罚
- 切换奖励

### 第三步：在 StateTree 里增加对应状态

例如：

- `Retreat`

然后加：

- `AI Decision Intent Matches(ExpectedIntentTag = AI.Intent.Retreat)`

### 第四步：进入状态时回写

加：

- `Commit AI Decision Intent(ExecutedIntentTag = AI.Intent.Retreat)`

### 第五步：如果需要更细门槛，再用数值 Condition

例如：

- `IntentScore`
- `RepeatedIntentCount`

---

## 六、我建议你优先怎么排查问题

如果后面运行不对，优先按下面顺序看。

### 1. 如果 AI 完全不切状态

先查：

- `UpdateCurrentTarget` 有没有拿到目标
- `UpdateCombatDecision` 有没有输出 `SelectedIntentTag`
- 状态条件里 `ExpectedIntentTag` 是否和真正的 GameplayTag 一致

### 2. 如果 AI 总是固定只进一个状态

先查：

- `DecisionProfile` 里是不是只有一个意图定义
- 另一个意图是不是被 `bRequireAttackRange` / `bRequireTarget` 过滤掉了
- 另一个意图的 `CooldownPenaltyMultiplier` / `RepeatPenalty` / `BaseDesire` 是否配得太低

### 3. 如果 AI 明明脱战了，下一轮还像保留旧节奏

先查：

- 对应脱战/回巡逻状态里有没有放 `Reset AI Decision State`

### 4. 如果你想看某个非主意图为什么没被选中

先查：

- `UpdateCombatDecision.ObservedIntentTag`

把它指定成你要看的那个 Tag，然后看：

- `ObservedIntentDesire`
- `ObservedIntentScore`
- `bHasObservedIntentMetrics`

---

## 七、当前这套方案已经实现到什么程度

对照设计方案，当前代码已经落地的部分是：

### 已实现

- 事实层和决策层拆开
- 决策组件作为持久运行时容器
- StateTree 消费意图标签
- 执行反馈回写
- 显式重置决策记忆
- 敌人差异化配置资产入口
- 数据驱动意图定义
- 泛化的意图数值 Condition

### 还没直接替你做的

- 具体某一棵 `.uasset` StateTree 资源里的蓝图配置接线
- 给你批量创建多个具体敌人的 `DecisionProfile` 资产
- 给 `Retreat / Block / Reposition` 这些新意图补完整的 GameplayTag 和默认状态链

这些不是架构没写，而是属于你项目资源层的下一步接线工作。

---

## 八、关键文件索引

### 架构入口

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`

### 意图定义 / 差异化配置

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.cpp`

### Evaluator

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp`

### Condition

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp`

### Task

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp`

---

## 一句话总结

现在这套系统已经不是“攻击和走位二选一的临时代码”了。

它已经变成：

**事实层提供输入，决策组件遍历意图表打分，StateTree 消费结果，状态执行再反向回写，敌人差异通过 Profile 资产配置。**

以后继续做，不应该再围绕“往 `UpdateScores` 里加 if”这条路扩，而应该围绕：

- 加新意图 Tag
- 配新 IntentDefinition
- 接 StateTree 状态
- 回写执行记录

这条路径继续扩。  
