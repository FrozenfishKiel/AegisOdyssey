# AI当前进度与新会话交接说明

## 文档目的

这份文档不是新的设计方案，而是给后续新开的 AI 会话做交接。

目标只有一个：

- 让新的 AI 在最短时间内知道我们现在做到哪了
- 知道哪些设计已经讨论完并已经落地
- 知道哪些地方只是方案，哪些地方已经写进工程
- 知道下一步应该先看什么、先查什么、先接什么

---

## 一、当前我们在做什么

当前这条 AI 开发线，核心是两部分：

1. 前面阶段已经完成了：
   - 战斗标签的进入/退出管理
   - 巡逻 / 走位阶段的基础能力
   - 基于 StateTree + EQS 的巡逻/走位链路梳理

2. 当前最新落地的是第四阶段：
   - 把 AI 在战斗中的“攻击 / 走位”从纯随机，升级成可配置的决策系统
   - 引入“意图 / 欲望 / 评分”这一层
   - 最终仍然由 StateTree 消费结果并执行具体状态

也就是说，我们现在不是在推翻原本架构，而是在**保留当前 StateTree 架构的前提下，给它补一层更灵活、更可扩展的战斗决策系统**。

---

## 二、已经讨论定稿并落地的关键结论

### 1. 不再把战斗决策写死成 Attack / Strafe 两个 if

已经明确不采用这种方式：

- `AttackScore`
- `StrafeScore`
- `if Attack > Strafe`

原因很简单：

- 扩展性差
- 后续加更多欲望会越来越臃肿
- 新 AI 接手时很容易继续把逻辑堆死

现在已经改成：

- 用 `IntentDefinitions` 描述每个“意图”
- 用 Tag 表达最终被选中的意图
- 用运行时组件保存事实、记忆、评分结果

### 2. 事实层和决策层已经拆开

这个结论已经确定并落地：

- `UpdateCurrentTarget` 只负责目标事实
- `UpdateCombatDecision` 只负责读取事实并做决策评估

不要再把一堆战斗评分重新塞回 `UpdateCurrentTarget`。

### 3. 决策记忆不能只靠父状态退出时清理

这个结论也已经明确：

- 不能假设一定是某个父状态统一清理
- 某些子状态也可能主动清理记忆
- 所以清理必须做成显式 Task

现在已经有单独的重置任务，供 StateTree 自己决定什么时候清。

### 4. 当前设计必须贴合项目现状，不能架空未来输入

这是本轮实现最重要的约束之一，后续新 AI 必须继续遵守：

- 可以预留扩展口
- 不能把未来可能会有的信号，当成现在已经存在
- 不能为了“理论上更智能”而架空项目当前调用链

例如前摇、后摇、破绽窗口、受压等级、强化学习输入等，都可以留入口，但**当前没有稳定数据源时，不要假装已经接好了**。

---

## 三、已经写进工程里的内容

### 1. 决策组件

已新增：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp`

职责：

- 保存当前目标事实
- 保存各个意图的运行时欲望 / 分数 / 上次执行时间
- 评估所有意图并选出当前意图
- 记录“最近一次真正执行了哪个意图”

### 2. 决策定义与配置资产

已新增：

- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h`
- `Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.cpp`

作用：

- `FAOAIDecisionIntentDefinition`：定义单个意图的规则
- `UAOAIDecisionProfile`：给不同敌人做差异化配置

已经不是只能改源码调参数了，后续敌人的“攻击欲望风格”优先走 Profile 资产。

### 3. StateTree Evaluator

已新增或重构：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCombatDecision.cpp`

其中：

- `UpdateCurrentTarget`：输出 `CurrentTarget / DistanceToTarget / bIsInAttackRange / bHasTarget`
- `UpdateCombatDecision`：输出 `SelectedIntentTag / SelectedIntentScore / SelectedIntentDesire` 等决策结果

### 4. StateTree Condition / Task

已新增：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionIntentMatches.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Conditions/STC_AIDecisionValueInRange.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_CommitAIDecisionIntent.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_ResetAIDecisionState.cpp`

作用分别是：

- `IntentMatches`：判断当前选中的意图 Tag 是否匹配
- `ValueInRange`：判断某个意图的 Desire / Score / RepeatedIntentCount 是否在范围内
- `CommitAIDecisionIntent`：状态真正执行时，把执行结果回写到决策组件
- `ResetAIDecisionState`：显式清理决策记忆

### 5. Gameplay Tag

已新增：

- `AI.Intent.Attack`
- `AI.Intent.Strafe`

对应文件：

- `Source/AegisOdyssey/AOGameplayTags.h`
- `Source/AegisOdyssey/AOGameplayTags.cpp`

后续新增更多战斗意图时，要继续走这条路，不要再退回字符串硬编码。

### 6. 角色接入

已修改：

- `Source/AegisOdyssey/Character/AOCharacter.h`
- `Source/AegisOdyssey/Character/AOCharacter.cpp`

当前 `AAOCharacter` 上已经挂了 `AIDecisionComponent`。

---

## 四、已经写好的说明文档，新的 AI 先看这些

按推荐顺序阅读：

1. `Notice/AI状态树与战斗输入交接文档.md`
   - 这是更早期的大交接文档，能帮助理解前面几轮 AI / StateTree / 战斗输入为什么这么搭

2. `Notice/AI走位与巡逻设计方案.md`
   - 这是走位 / 巡逻阶段的设计记录
   - 包含 EQS、巡逻、走位状态的设计背景

3. `Notice/AI第四阶段战斗决策与攻击欲望设计方案.md`
   - 这是第四阶段的设计方案文档
   - 讲的是为什么要做欲望层、评分层、选择层

4. `Notice/AI第四阶段战斗决策系统使用与实现说明.md`
   - 这是已经落地代码的“怎么用、怎么接、怎么扩展”说明
   - 如果新 AI 要继续接第四阶段，优先看这个

5. `Notice/AI当前进度与新会话交接说明.md`
   - 也就是你现在看到的这份文档

---

## 五、当前这套系统怎么接

这里写给后续 AI，也写给后续工程师。

### 1. 角色侧

确认敌人角色实例上有 `AIDecisionComponent`。

当前它已经在 `AAOCharacter` 中创建。

### 2. 决策 Profile

如果要做敌人差异化：

1. 新建一个 `AO AI Decision Profile`
2. 配置：
   - `IdealAttackDistance`
   - `IntentDefinitions`
3. 把它填给敌人身上的 `AIDecisionComponent.DecisionProfile`

如果不填：

- 系统会走当前默认的 `Attack / Strafe` 两个意图定义

### 3. StateTree Evaluator 接法

推荐至少挂两个 Evaluator：

1. `Update Current Target`
   - 负责输出当前目标事实

2. `Update Combat Decision`
   - 输入绑定前者输出
   - 负责输出当前被选中的意图与评分

### 4. StateTree 状态怎么消费它

例如：

- 攻击子状态前挂 `AI Decision Intent Matches`
  - `ExpectedIntentTag = AI.Intent.Attack`

- 走位子状态前挂 `AI Decision Intent Matches`
  - `ExpectedIntentTag = AI.Intent.Strafe`

也可以再配合：

- `AI Decision Value In Range`

例如：

- 只有当攻击意图分数高于某阈值时，才允许进入攻击
- 或者当重复执行次数过高时，限制连续选择

### 5. 状态执行反馈

在真正的攻击状态、走位状态开始执行时，挂：

- `Commit AI Decision Intent`

例如：

- 攻击状态填 `AI.Intent.Attack`
- 走位状态填 `AI.Intent.Strafe`

作用是把“这次真的执行了什么”回写给决策系统，供下一轮评分使用。

### 6. 脱战 / 重置

在明确需要清理战斗记忆的地方挂：

- `Reset AI Decision State`

例如：

- 丢失目标后切回待机
- 完整脱战
- 回巡逻

不要假设所有清理都必须放在某个固定父状态退出中。

---

## 六、已经验证过什么

当前 C++ 已经成功编译通过。

验证命令：

- `D:\UE_5.6\Engine\Build\BatchFiles\Build.bat AegisOdysseyEditor Win64 Development D:\UE_Project\AO\AegisOdyssey\AegisOdyssey\AegisOdyssey.uproject -WaitMutex -NoHotReloadFromIDE`

也就是说：

- 反射宏
- 模块依赖
- 新增类型
- StateTree 节点声明

至少在编译层面已经通了。

注意，这不等于编辑器里的 StateTree 资产已经全部配完。

---

## 七、当前还没替你做完的部分

这些地方是“后续继续做”，不是“当前没实现就算失败”。

### 1. 编辑器资源层接线

目前最需要继续推进的是：

- 把新的 Evaluator / Condition / Task 真正接进对应的 StateTree 资源
- 逐个敌人创建或调整 `AO AI Decision Profile`
- 根据敌人类型把参数调出差异

### 2. 更多意图

当前正式接入的意图只有：

- `AI.Intent.Attack`
- `AI.Intent.Strafe`

以后完全可以扩展：

- `AI.Intent.Retreat`
- `AI.Intent.Reposition`
- `AI.Intent.Block`
- `AI.Intent.Feint`

但要注意：

- 先补 Tag
- 再补 Profile 定义
- 再补 StateTree 消费状态
- 再决定是否需要额外事实输入

### 3. 更高级的战斗信号

例如：

- 前摇窗口
- 后摇窗口
- 压制状态
- 格挡风险
- 体力压力

这些方向在设计上已预留思路，但**不要让新的 AI 误以为当前工程已经有完整输入链**。

---

## 八、后续新 AI 最容易犯的错误

### 1. 又把泛化结构改回硬编码

例如重新搞回：

- `AttackScore`
- `StrafeScore`
- `bShouldAttack`

如果只是临时调试可以理解，但不要再把总架构写回去了。

### 2. 把未来输入当成当前输入

如果工程里还没有：

- 明确的破绽窗口标签
- 明确的前后摇事实
- 明确的体力风险输入

那就不要在决策核心里写成“现在已经依赖这些信号”。

### 3. 把事实层和决策层重新搅在一起

`UpdateCurrentTarget` 已经被收敛回“目标事实包装”职责。

不要为了图省事再把大量决策分数塞回去。

### 4. 只看方案，不看真实调用链

后续如果继续实现：

- 必须先读当前 C++ 文件
- 必须先看已有 StateTree 节点是怎么声明和输出的
- 必须确认项目里真实存在的数据来源

不能只看文档就直接续写。

---

## 九、新 AI 建议的接手顺序

如果后续你重新开一个 AI，让它按这个顺序开始：

1. 先读：
   - `Notice/AI第四阶段战斗决策系统使用与实现说明.md`
   - `Notice/AI当前进度与新会话交接说明.md`

2. 再读关键代码：
   - `AOAIDecisionComponent`
   - `AOAIDecisionTypes`
   - `STE_UpdateCombatDecision`
   - `STC_AIDecisionIntentMatches`
   - `STT_CommitAIDecisionIntent`

3. 再确认正在接的 StateTree 资源，看看编辑器侧现在接到哪一步

4. 最后再决定是：
   - 继续接资源
   - 继续加新意图
   - 继续补新的事实输入

---

## 十、给新 AI 的一句话总结

当前这套系统已经从“攻击 / 走位纯随机”升级成了“基于事实 + 意图定义 + 运行时评分 + StateTree 消费结果”的可扩展战斗决策系统，C++ 主体已经落地并编译通过；后续重点不再是推翻方案，而是基于现有调用链继续接资源、调参数、补新意图、补真实可用的事实输入。
## 十一、交互与功能框架的总约束

这一条虽然最初是围绕当前交互系统讨论出来的，但它本质上不是“箱子功能约束”，而是后续继续开发 AI、交互对象、UI 框架时都必须遵守的总原则。

### 1. 这不是某个对象的临时功能，而是一套框架能力

- 交互不是某个对象的功能补丁，而是一套可复用、可扩展、可迁移的框架能力
- 不要因为当前示例对象是箱子、工作台、按钮、拉杆，就把玩家侧、AI 侧或 UI 侧逻辑写死成对象类型分支
- 新增功能时，优先抽象统一入口、对象侧语义、会话承载、反向操作链，而不是围绕某个示例对象单独堆专用逻辑

### 2. 职责边界必须稳定

- 玩家客户端或 AI 侧只负责发起交互、承载结果、消费结果，不负责硬编码判断“这是哪一种对象”
- 具体对象自己决定这次交互的语义，例如：
- 是否直接执行动作
- 是否打开 UI
- 打开什么 UI
- 是否建立持续会话
- 是否允许反向操作数据
- 承载层不识别业务类型，只承载对象提供的会话、数据和界面

### 3. 这条约束的影响范围不只限于箱子

- 以后做工作台、商店、机关、可控制 AI、可观察 AI、AI 打开背包、AI 取用容器，表面上对象不同，但都必须优先接入统一交互框架
- 不允许为了实现某一个当前需求，就重新引入一套平行链路、重复能力或面向对象边界错误的临时系统

### 4. 后续所有类似宽泛功能的开发，都要先检查这四件事

- 是否具备扩展性
- 是否具备灵活性
- 是否具备复用性
- 是否会因为当前示例对象而把整体框架写窄

一句话总结：

**不要把“当前在做的示例对象”误认为“系统的真实边界”。真实边界应该是框架边界，而不是箱子、工作台、商店、AI 这类具体对象名字。**
