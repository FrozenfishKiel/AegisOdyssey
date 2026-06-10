---
title: Skill System Project Map
tags:
  - knowledge
  - skill-system
  - project-map
aliases:
  - Skill System Project Map
  - 技能系统项目地图
---

# 技能系统项目地图

更新时间：2026-05-19  
适用范围：当前项目中 `SkillDefinition -> SkillInstance -> SkillSlot -> SkillComponent -> ASC` 这条技能系统主链，以及已落地的执行结构。  
不适用范围：具体某个技能资源如何配置、所有 UI/蓝图细节、后续尚未整理的交接/BUG 文档。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前技能系统的正式主链是什么。
2. 技能定义、技能来源、技能实例、技能槽、技能执行分别落在哪里。
3. 当前哪些层是“真相层”，哪些层只是投影或观察层。
4. 如果继续扩技能系统，先看哪些代码入口。

## 2. 当前正式主链

当前技能系统已经不是“几个按键直连几个能力”的临时结构。

当前正式主链是：

**`SkillDefinition -> SkillSource -> SkillInstance -> SkillSlot -> SkillComponent -> ASC -> SkillAbility -> ExecutionDefinition`**

其中每一层回答的问题不同。

## 3. 各层职责

### 3.1 `SkillDefinition`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h`

这一层只回答：

- 这是什么技能
- 展示信息是什么
- 主技能标签、技能族、技能组是什么
- 默认 `AbilityClass` 是什么
- 当前挂的是哪个 `ExecutionDefinition`

当前已经确认：

- `SkillDefinition` 是 `UPrimaryDataAsset`
- 它不再通过一个总 `SkillType` 枚举表达技能差异
- 它通过挂接 `ExecutionDefinition` 表达执行结构入口

### 3.2 技能来源层

当前真实来源不等于技能定义本身。

当前技能来源主要由：

- `Source/AegisOdyssey/Inventory/Fragments/AOFragment_SkillSource.*`
- `UAOInventoryItemInstance`

这条链承接。

当前正式语义是：

- 技能载体物品是来源对象
- `SkillDefinition` 是技能静态定义
- 两者不是一回事

### 3.3 `SkillInstance`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Core/AOSkillInstance.h`

这一层回答：

- 当前这个具体技能对象是谁
- 它来自哪个来源物品
- 当前等级/品质是多少
- 当前是否已装配
- 当前装在哪个槽
- 它代表什么冷却身份

当前已经确认：

- `SkillInstance` 是运行时身份对象
- 冷却身份从实例层透出，而不是由槽位承担

### 3.4 `SkillSlot`

当前技能槽运行时关系主要体现在：

- `FAOSkillSlotEntry`
- `FAOSkillSlotViewData`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Components/AOSkillComponent.h`

这一层只表达：

- 第几个槽
- 该槽位输入标签是什么
- 当前装了哪个 `SkillInstance`
- 当前在 ASC 上授予了什么

它不是技能定义本体，也不是技能实例本体。

### 3.5 `SkillComponent`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Components/AOSkillComponent.*`

这是当前技能系统运行时总入口。

它负责：

- 创建和维护 `SkillInstance`
- 维护装配关系
- 把槽位变化翻译成 ASC 授予变化
- 提供输入注入与直接执行入口
- 输出 UI / MVVM 观察快照

这是当前技能系统的主要真相层之一。

### 3.6 `SkillSlotInventoryComponent`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.*`

这一层是库存语义适配层，不是第二个技能系统。

它负责：

- 把技能槽投影成正式库存容器
- 把容器投影结果同步回 `SkillComponent`

当前正式边界是：

- 它不维护技能实例真相
- 不负责冷却
- 不负责 Ability 授予

### 3.7 `SkillGameplayAbility`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.*`

这一层负责把“技能实例语义”和 GAS 能力主链对接起来。

它提供：

- 从 `SourceObject` 反查 `SkillInstance`
- 再反查 `SkillDefinition`
- 再反查 `ExecutionDefinition`
- 冷却标签与冷却应用入口
- 命中结果回送战斗尾链的公共入口

### 3.8 `ExecutionDefinition`

定义位置：

- `Source/AegisOdyssey/SkillSystem/Core/AOSkillExecutionDefinition.*`
- `Source/AegisOdyssey/SkillSystem/Execution/Definitions/*`

这是当前执行结构的正式承载层。

当前已经明确落地的子类包括：

- `UAOSkillProjectileExecutionDefinition`
- `UAOSkillAreaSequenceExecutionDefinition`

这说明技能执行差异已经不再通过一个大枚举表达，而是通过执行定义子类表达。

## 4. 当前真相层和投影层

### 4.1 真相层

当前应视为真相层的有：

- `SkillDefinition`
- `SkillInstance`
- `SkillComponent`
- `ASC` 上真实授予结果
- 具体 `SkillGameplayAbility` / `ExecutionDefinition`

### 4.2 投影层 / 观察层

当前应视为投影或观察层的有：

- `SkillSlotInventoryComponent`
- `FAOSkillSlotViewData`
- `FAOEquippedSkillViewData`
- 技能条 / HUD / MVVM 的显示链

这条边界很重要：

- UI 观察技能真相
- 但不持有第二套技能真相

## 5. 当前输入与触发链

当前已经明确存在两条入口：

### 5.1 Hero 输入链

- `InjectSkillSlotInputCommand`
- `InjectSkillSlotInputCommandByIndex`

这条链用于真实输入桥接，仍然复用 Hero 的统一输入入口。

### 5.2 直接技能执行链

- `ExecuteSkillSlotCommand`
- `ExecuteSkillSlotCommandByIndex`

这条链直接落到 `SkillComponent -> ASC`，不再回灌 Hero 输入广播。

这对 AI / StateTree 触发尤其重要。

## 6. 当前优先阅读顺序

如果后续继续接技能系统，当前推荐阅读顺序是：

1. `AOSkillDefinition.*`
2. `AOSkillInstance.*`
3. `AOSkillComponent.*`
4. `AOSkillSlotInventoryComponent.*`
5. `AOSkillGameplayAbility.*`
6. `AOSkillExecutionDefinition.*`
7. `Execution/Definitions/*`
8. `Execution/AbilityBases/*`
9. `Abilities/*`

## 7. 第一轮提炼来源

本轮主要从下面两篇历史设计文档提炼，并与当前代码核对：

- `Notice/HistoryNotice/技能系统设计方案-技能对象化与槽位装配框架.md`
- `Notice/HistoryNotice/技能系统设计方案-技能执行语义与隐式分类框架.md`

这两篇文档对应的稳定结论，当前已分别沉淀到：

- [[技能系统已锁定设计]]
- [[技能执行语义与结构说明]]
- [[技能系统已知边界与历史偏差]]
