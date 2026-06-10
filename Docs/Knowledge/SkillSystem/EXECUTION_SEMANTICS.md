---
title: Skill Execution Semantics
tags:
  - knowledge
  - skill-system
  - execution
aliases:
  - Skill Execution Semantics
  - 技能执行语义与结构说明
---

# 技能执行语义与结构说明

更新时间：2026-05-19  
适用范围：当前技能系统如何表达执行差异、为什么不用总 `SkillType`、当前代码已经落到哪一步。  
不适用范围：每个具体技能资源的完整配置教程。

## 1. 当前已锁定结论

当前最重要的结论是：

**程序层不再依赖一个总的显式技能类型枚举来驱动整个技能系统。**

取而代之的是：

**通过 `ExecutionDefinition` 子类和具体 `SkillGameplayAbility` 子类表达执行差异。**

## 2. 为什么当前不应该回到总 `SkillType`

历史方案给出的理由是对的，当前代码也已经按这个方向落地：

- 一个真实技能往往不是单一类别
- “像投射物”“像范围技”是人类沟通分类
- 程序真正需要的是执行结构

当前代码中的直接证据是：

- `UAOSkillDefinition` 当前只有 `ExecutionDefinition`
- 头文件注释已明确“不再回答属于哪种执行枚举”

这说明“总枚举驱动全系统”已经不是当前正式路线。

## 3. 当前真正的执行结构落点

### 3.1 静态执行入口

在 `UAOSkillDefinition` 上：

- `AbilityClass`
- `ExecutionDefinition`

共同决定技能执行入口。

### 3.2 执行定义基类

`UAOSkillExecutionDefinition` 当前承载的是：

- 统一战斗尾链效果配置
- 统一 `GameplayCue` 配置入口
- 统一调试绘制配置

也就是说，执行定义不是“随便挂一些参数”，而是正式的执行结构承载对象。

### 3.3 已落地的执行定义子类

当前已经落地：

- `UAOSkillProjectileExecutionDefinition`
- `UAOSkillAreaSequenceExecutionDefinition`

这两者分别代表：

- 投射体执行链
- 区域序列执行链

## 4. 当前父类和子类的边界

### 4.1 `UAOSkillGameplayAbility` 负责什么

当前父类负责：

- 从 `SourceObject` 读取 `SkillInstance`
- 读取 `SkillDefinition`
- 读取 `ExecutionDefinition`
- 统一冷却入口
- 统一技能命中结果回送战斗尾链
- 统一 `GameplayCue` 触发入口

### 4.2 `UAOSkillGameplayAbility` 不再默认负责什么

当前父类不再默认内建：

- 播动画
- 等单个事件
- “激活后自动按一个统一模板执行”

这意味着具体技能释放语义必须留在具体技能或具体执行基类里。

## 5. 当前代码已经证明的执行语义思路

虽然历史方案讨论的是“目标获取 / 原点 / 投递 / 命中采集 / 结算 / 生命周期”这套语义框架，但当前代码并不是空概念。

当前已经能在执行定义里看到这些维度的真实落点。

### 5.1 投射体执行定义

`UAOSkillProjectileExecutionDefinition` 已经显式承载：

- 原点配置 `SpawnOrigin`
- 是否跟随 Avatar 朝向
- 飞行速度
- 最大飞行距离
- 碰撞半径
- 爆炸半径
- 生命周期

这已经是完整的“投递 + 命中收集范围 + 生命周期”配置结构。

### 5.2 区域序列执行定义

`UAOSkillAreaSequenceExecutionDefinition` 已经显式承载：

- 波次间隔
- 波次数量
- 前方逻辑区域中心
- 逻辑大圆半径
- 单次影响半径
- 区域中心偏移

这已经是完整的“阶段/波次 + 区域结构 + 局部命中范围”配置结构。

## 6. 当前命中后统一尾链

`UAOSkillGameplayAbility` 当前提供：

- `RouteSkillEffectApplicationFromRuntimeActor()`
- `ApplySkillEffectsToTargets()`

这说明当前结构已经明确：

- 各技能自己负责执行和命中采集
- 命中结果再统一回送战斗尾链

这个边界很重要，因为它避免了：

- 每个技能自己重新发明一套战斗结算链

## 7. 当前 `GameplayCue` 边界

当前执行定义和技能父类已经留出正式 `GameplayCue` 入口：

- 执行定义上有 `CueConfig`
- 父类上有 `ExecuteSkillCue()`

这说明表现同步也开始被拉回正式入口，而不是继续散写在具体技能里。

## 8. 当前第一轮深提炼后的结论

这轮最重要的核对结论有三条：

1. 历史方案里“不要做总 `SkillType`”这条，不只是观点，当前代码已经按这个方向落地。
2. 技能执行结构已经正式收束到 `ExecutionDefinition` + `SkillGameplayAbility` 子类体系。
3. 技能父类已经主动收紧边界，不再默认包办动画和释放时序，这对后续扩技能非常关键。

## 9. 当前仍需后续继续整理的部分

这轮只沉淀了执行结构骨架，还没有展开整理：

- 火球术案例的完整执行链
- 火山喷发案例的完整执行链
- 动画事件 / GameplayEvent 如何与具体技能对点
- 状态树如何触发技能槽

这些后续适合从交接文档和案例文档里再开新一轮整理，不应在这一轮混进主骨架文档。
