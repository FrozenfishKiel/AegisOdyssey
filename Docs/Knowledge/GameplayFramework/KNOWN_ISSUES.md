---
title: Gameplay Framework Known Issues
tags:
  - knowledge
  - gameplay-framework
  - known-issues
  - gas
aliases:
  - Gameplay Framework Known Issues
  - GameplayFramework 已知边界与历史偏差
---

# GameplayFramework 已知边界与历史偏差

更新时间：2026-05-19  
适用范围：当前 `GameplayFramework` 这一轮深提炼里已经识别出的历史误判点、混层风险和后续整理边界。  
不适用范围：完整 GameplayFramework bug 清单。

## 1. 这三篇历史文档不能直接当当前事实原样迁移

原因不是它们没价值，而是它们混了三种层次：

1. 引擎机制解释
2. 当时的项目排查结论
3. 对当前项目的落地建议

因此这一轮没有把历史原文搬进知识库，而是拆成：

- `PROJECT_MAP`
- `DECISIONS`
- `GAS_REPLICATION_AND_EFFECT_CONTEXT`
- `KNOWN_ISSUES`

## 2. 当前最容易误判的三条边界

### 2.1 不要把 `ReplicateYes` 写成“Ability 内所有字段自动同步”

当前不能这样写。

已确认当前事实：

1. `GA_Sprint` 是 `ReplicateYes`
2. 但它的普通 C++ 成员当前没有显式复制声明

### 2.2 不要把“客户端属性值变了”写成“UI 变化通知一定自动触发”

当前不能这样写。

已确认当前事实：

1. 属性复制仍要走 `OnRep_*`
2. 当前项目 UI 依赖的业务事件，是项目自己在 `OnRep_*` 里广播出来的

### 2.3 不要把“自定义 EffectContext 教程已经写过”理解成“项目里可能还没正式接线”

当前不能这样写。

已确认当前事实：

1. `DefaultGame.ini` 已注册 `UAOAbilitySystemGlobals`
2. `AOHealthAttributeSet` 里已把 `FAOGameplayEffectContext` 当正式必备上下文
3. `CombatManager -> ExecCal -> AttributeSet` 已形成实际消费链

## 3. 当前历史偏差里最重要的一条

历史文档里的“Health 客户端回调正常但 Vigor 客户端回调失败”在当时有排查价值，  
但本轮代码核对后确认：

1. `AOCombatAttributeSet::OnRep_Vigor`
2. `AOCombatAttributeSet::OnRep_MaxVigor`

现在都已经补上了 `GAMEPLAYATTRIBUTE_REPNOTIFY(...)` 和自定义广播。

所以这条历史结论当前只能保留为：

- 一次有价值的 GAS 复制回调边界案例

不能再写成：

- 当前项目仍存在的正式缺陷

## 4. 当前仍未进入本轮正文、但后续很值得继续整理的主题

以下内容都在 `GameplayFramework` 大类里，但本轮仍没有全部展开：

1. 更完整的 Ability 输入层与 ActivationPolicy 体系
2. 如果后续出现更多自定义 `GameplayEffectContext` 用法，是否需要单独整理“上下文字段治理规则”
3. 如果未来真的引入 `SmartObject`，还需要单独整理正式的项目接线文档
4. 如果后续出现更多 StateTree 事件消费节点，还需要单独整理 C++ 侧消费地图

## 5. 当前新增识别出的两个高风险误判点

### 5.1 不要把 `FCombatStateTreeInputEvent` 写成“当前同时保存 Tag 和 InputType”

当前不能这样写。

已确认当前事实：

1. 结构体当前只有 `InputType` 成员
2. 事件 Tag 走的是 `FStateTreeEvent.Tag`
3. 构造函数里接收 `InInputTag`，但没有把它落成成员

### 5.2 不要把 `SmartObject` 历史研究稿写成“项目已正式接入”

当前不能这样写。

已确认当前事实：

1. 插件未启用
2. 模块未引入
3. 源码未接线
4. 内容资产未建

因此它当前最多只能被当成“后续候选接线方案”。

## 6. 当前整理规则

后续继续往 `Docs/Knowledge/GameplayFramework` 提炼时，默认遵守：

1. 先区分“引擎机制解释”“当前项目真实接线”“历史排查记录”三层，不要混写。
2. 任何涉及 `ReplicationPolicy` 的说法，都要再核具体类有没有显式属性复制。
3. 任何涉及客户端属性变化通知的说法，都要再核 `OnRep_*` 是否做了手动广播。
4. 任何涉及 `EffectContext` 的说法，都要同时核“全局工厂是否注册”“谁写字段”“谁消费字段”，不能只看结构体声明。
5. 任何涉及 `StateTree` 事件负载的说法，都要区分 `Event.Tag` 和 `Payload`，不要照历史示例想当然。
6. 任何涉及 `SmartObject` 的说法，都要先核工程插件、模块、源码引用和资产四层，确认它是否真的进入项目。
7. `StateTreeAI` 的 AI 运行时真相和这里的框架层边界继续分包维护，不混回一篇。

## 7. 当前新增识别出的一个高风险误判点

### 7.1 不要把“MMC 没算出来”直接写成“GAS 不支持来源属性派生链”

当前不能这样写。

这轮已经确认：

1. 当前问题更准确的描述，是“来源属性已经变化，但派生属性刷新链没有按预期打通”。
2. 这和“MMC 根本不能做这件事”不是一回事。
3. 当前已经否掉了把问题继续往“动态增删 AttributeSet 拓扑协调”方向扩的做法。
4. 当前也已经否掉了给 `PawnData` 再加第二条属性集配置旁路的做法。

因此后续继续沉淀这块时，默认参考：

- [[GAS MMC 派生属性刷新边界]]

不要再把阶段性交接里的动态拓扑修法直接写成现行框架结论。

### 7.2 不要把 `WeaponInstance` 当成静态武器蓝图本体

当前不能这样写。

这轮武器表现链已经明确把运行时武器表现路由收口到 `WeaponInstance`，后续要找的是当前生效武器实例，而不是直接把静态武器蓝图当最终生效对象。
