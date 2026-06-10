# AI决策系统最新进度交接-属性区间与翻滚扩展

## 文档目的

这份文档是写给下一位接手本项目的新 AI 或工程协作者的。

目的不是重新讨论方案，而是快速说明：

- 我们目前在做什么
- 这次会话已经把哪些内容真正写进了 C++
- 哪些旧方案已经被删除，不能再按旧思路理解
- 新 AI 接手时应该先看哪些文件
- 接下来最应该继续做什么

---

## 一、当前我们在做什么

当前这一段工作，核心仍然是 AI 战斗决策系统，但已经从最早的简单 Attack / Strafe 欲望，继续推进到了更泛化、更可配置的阶段。

目前的主线可以概括为三块：

1. AI 战斗意图决策系统
   - 由事实层提供输入
   - 由决策组件统一计算意图
   - 由 StateTree 消费计算结果

2. AI 翻滚接入与方向链路
   - AI 不走增强输入
   - 通过 Send Combat Command 一类的任务向现有输入/能力链路转发标签
   - AI 翻滚方向不是玩家输入绑定，而是内部自行计算并走现有链路

3. 决策参数体系继续泛化
   - 已经不再只围绕生命值写死
   - 已经改成“指定属性 + 分母 + 区间 + 方向 + 可选曲线 + 权重”的通用因子

一句话总结：

当前系统不再是“生命值低就滚”这种写死逻辑，而是“任意属性在任意区间内，以可配置方式影响任意意图”的扩展结构。

---

## 二、这次会话已经落地完成的内容

## 1. 旧的 SelfHealthFactor 已经删除

旧方案里有一个明显偏生命值专用的入口：

- `SelfHealthFactor`
- `SelfHealthRatio`

这一套已经被删除，不能再按旧理解继续扩展。

删除原因：

- 命名太死
- 表达力不够
- 用户已经明确要求不要只为血量服务
- 后续像攻击力、耐力、精力、其他属性都需要同样的入口

---

## 2. 新增了通用属性区间因子序列

当前正式使用的是：

- `FAOAIDecisionAttributeIntervalFactor`
- `TArray<FAOAIDecisionAttributeIntervalFactor> AttributeIntervalFactors`

它位于：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`

每个成员支持：

- `NumeratorAttribute`
- `DenominatorAttribute`
- `ManualDenominator`
- `RangeMin`
- `RangeMax`
- `bHigherScoreNearMin`
- `ResponseCurve`
- `Weight`

这意味着一个意图内部现在可以同时叠加多个属性因子。

例如：

- 当前生命值 / 最大生命值
- 当前攻击力 / 手动分母
- 当前耐力 / 最大耐力

这些都可以作为同一意图的多项输入，最后直接累加。

---

## 3. 当前属性区间因子的实际计算逻辑

实现位置：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`
- 函数：`UAOAIDecisionComponent::EvaluateAttributeIntervalFactor`

当前算法顺序是：

1. 先读取属性分子
2. 再读取属性分母
3. 如果分母属性没配或运行时无效，则回退到 `ManualDenominator`
4. 如果仍无效，则最终按默认分母 `1`
5. 计算：
   - `NormalizedValue = Numerator / Denominator`
6. 如果 `NormalizedValue` 不在 `[RangeMin, RangeMax]` 内
   - 直接返回 `0`
7. 如果在区间内
   - 先算出它在区间里的线性位置 `Alpha`
8. 再根据 `bHigherScoreNearMin` 把它转成“靠近偏好侧的程度” `ProximityRatio`
9. 若配置了 `ResponseCurve`
   - 用 `ProximityRatio` 去采样曲线
10. 若未配置曲线
   - 直接使用 `ProximityRatio`
11. 最终：
   - `FinalScore = CurveOutputOrProximityRatio * Weight`

关键点：

- 曲线横轴吃的不是原始血量值
- 曲线横轴吃的是 `0~1` 的“靠近偏好侧程度”

也就是说：

- `X = 0`：最远离偏好侧
- `X = 1`：最靠近偏好侧

---

## 4. 属性区间因子现在支持可选响应曲线

这是本次会话后半段新增的重点。

用户反馈：

- 纯线性增长太平
- `Weight=8` 可能几乎不触发
- `Weight=9` 又可能明显过头

因此当前已经升级为：

- 区间内先算靠近度
- 再可选过曲线
- 最后乘权重

曲线是可选的，不是硬要求。

如果不配曲线：

- 仍走线性靠近度逻辑

如果配曲线：

- 可以做出前平后陡、前陡后缓、局部高敏感等非线性响应

这一点已经写进代码并编译通过。

---

## 5. 决策组件当前仍保留的其他主要评分入口

虽然生命值专用入口被删了，但当前系统仍保留这些入口：

- `BaseDesire`
- `CadenceWeight`
- `DistanceFactor`
- `AttributeIntervalFactors`
- `RecentDamageFactor`
- `TargetStateTagFactors`
- `CooldownSeconds`
- `CooldownPenaltyMultiplier`
- `RepeatPenalty`
- `SwitchBonus`
- `FallbackSelectionWeight`

也就是说，当前“为什么 AI 选 Roll / Attack / Strafe”，不能只盯一个属性因子看。

尤其要注意：

- `CadenceWeight`
- `BaseDesire`
- `FallbackSelectionWeight`

它们都可能在属性因子为 0 时，仍然让某个意图被选中。

---

## 6. 关于“满血也一直滚”的一次重要排查结论

用户测试时发现：

- 给翻滚意图配了属性区间曲线
- 但满血时 AI 仍一直滚

我们已经确认：

- 如果区间是 `0.2 ~ 0.8`
- 满血按 `1.0`
- 那么属性区间因子本身会直接返回 `0`
- 连曲线都不会参与

所以这类问题不能直接归因于属性区间因子。

更可疑的来源是：

- `BaseDesire`
- `CadenceWeight`
- `SwitchBonus`
- `FallbackSelectionWeight`
- `RepeatPenalty`

新 AI 后续如果继续追这个问题，要优先查总分构成，而不是只查血量/属性因子。

---

## 7. 中文注释与蓝图提示已经继续补充

当前这一批相关结构和组件头里，已经有一轮新的中文 ToolTip 与注释补充。

用户非常在意：

- 注释必须是中文
- 不要抽象词
- 要明确方向含义
- 要明确正负和左右的意义

后续继续扩展时必须保持这个风格。

---

## 三、这次会话涉及的重要文件

## 核心决策结构

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.cpp`

## StateTree 决策相关

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp`

## AI 翻滚/方向链路相关

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.*`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_PlayRollAnimation.*`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CalculatePlanarDirection.*`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.*`
- `Source/AegisOdyssey/AbilitySystem/Abilities/GA_Roll.*`

注意：

本次文档重点不是重新讲翻滚系统全部细节，但新 AI 如果后续继续处理翻滚触发、翻滚决策或翻滚方向，必须把上面这条链一起看。

---

## 四、新 AI 接手时建议先看的文档

按优先级建议：

1. `Notice/AI第四阶段战斗决策系统使用与实现说明.md`
   - 先理解第四阶段决策系统主结构

2. `Notice/AI当前进度与新会话交接说明.md`
   - 先知道更早一批工作做到了哪

3. `Notice/AI战斗决策参数调参与计算说明.md`
   - 这个和当前调参直接相关

4. 本文档
   - 用来补上这次会话新增的属性区间因子、响应曲线、翻滚相关最新状态

5. `Notice/AI状态树与战斗输入交接文档.md`
   - 如果要继续处理输入转发、StateTree 消费行为、翻滚/攻击触发链路，需要一起看

---

## 五、当前明确删除或废弃的旧思路

新 AI 接手时不要再按下面这些旧理解继续扩展：

## 1. 不要恢复 SelfHealthFactor

它已经被通用属性区间因子替代。

如果以后想让“生命值”继续参与决策，应当：

- 在 `AttributeIntervalFactors` 里新增一项
- 分子配当前生命值
- 分母配最大生命值

而不是重建一个生命值专用入口。

## 2. 不要把属性区间逻辑重新写回单一血量逻辑

当前用户要求很明确：

- 这不是只为血量服务的设计
- 是为“任意指定属性”服务的设计

## 3. 不要把曲线当作原始属性值曲线

当前实现里：

- 曲线横轴不是原始属性值
- 曲线横轴是区间内“靠近偏好侧的程度”

如果未来要支持“直接对原始归一化值采样”的模式，可以新增模式开关，但不要误以为当前已经是那个逻辑。

---

## 六、当前编译状态

这次会话内，当前修改已经成功编译通过。

验证命令：

- `D:\UE_5.6\Engine\Build\BatchFiles\Build.bat AegisOdysseyEditor Win64 Development D:\UE_Project\AO\AegisOdyssey\AegisOdyssey\AegisOdyssey.uproject -WaitMutex -NoHotReloadFromIDE`

当前这份交接文档对应的代码状态，是“已编译通过”的状态。

---

## 七、新 AI 接手后的建议工作顺序

建议按下面顺序继续：

1. 先读本文档和旧交接文档
2. 再读 `AOAIDecisionTypes` 和 `AOAIDecisionComponent`
3. 明确当前属性区间因子和曲线的真实计算链
4. 再去看蓝图/StateTree 资产当前怎么配的
5. 再判断下一步是：
   - 继续调参
   - 继续补调试输出
   - 继续扩意图
   - 继续扩事实层输入

---

## 八、如果新 AI 要继续排查“为什么总是滚”

优先排查顺序建议如下：

1. 先看翻滚意图的 `BaseDesire`
2. 再看翻滚意图的 `CadenceWeight`
3. 再看 `FallbackSelectionWeight`
4. 再看 `SwitchBonus`
5. 再看 `RepeatPenalty`
6. 最后才回头看某个单独属性区间因子的贡献

原因：

属性区间因子只是总分来源之一，很多“老是滚”的现象其实不是它单独造成的。

---

## 九、给下一位 AI 的一句话总结

当前这套 AI 战斗决策系统已经从“血量专用因子 + 简单 Attack/Strafe 欲望”推进到了“通用属性区间因子序列 + 可选区间内响应曲线 + StateTree 消费 + AI 翻滚/输入链已接通”的阶段。

后续重点不该是推翻结构，而应该是：

- 基于现有结构继续调参与验证
- 必要时补更细的调试输出
- 继续在事实层和意图层上做扩展
- 始终保持中文注释、可配置、可扩展、不要回退到写死逻辑
