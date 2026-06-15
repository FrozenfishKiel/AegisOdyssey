---
title: Item Semantic Tags Shared By AI And Player 2026-06-16
tags:
  - knowledge
  - inventory-equipment
  - gameplay-tags
  - ai
aliases:
  - AI与玩家共享的物品语义标签 2026-06-16
---

# AI 与玩家共享的物品语义标签

更新时间：2026-06-16  
适用范围：当前库存定义层里供 AI 查询与玩家侧分类共同使用的物品语义标签。  
不适用范围：具体某个道具蓝图已经如何展示给玩家、最终 UI 文案分类样式。

## 1. 当前共享语义标签机制已经存在

这次不是从零新造一套标签系统。  
当前项目里本来就已经有“物品语义标签”这层数据位。

核心入口是：

1. `UAOInventoryItemDefinition::SemanticTags`
2. `UAOInventoryItemDefinition::HasSemanticTag(...)`
3. `FAOAIInventoryItemQuery::SemanticTag`
4. AI 运行时库存查询时会用 `ItemDefinition->HasSemanticTag(Query.SemanticTag)` 做筛选

这意味着：

- AI 已经可以按物品语义标签检索库存
- 这套标签也可以作为玩家侧查看或后续分类的统一语义来源
- 不应该再为 AI 单独发明一套“只给 AI 看”的物品类别字段

## 2. 本轮新增的全局物品语义标签

本轮已经把下面 5 个语义标签正式补进全局 Gameplay Tags：

- `Item.Semantic.Weapon.Sword`
- `Item.Semantic.Weapon.Spear`
- `Item.Semantic.Consumable.HealthPotion`
- `Item.Semantic.Consumable.ManaPotion`
- `Item.Semantic.Consumable.Food`

它们的作用不是直接驱动 UI，而是先把项目里“这是什么类型的物品”统一收口成稳定语义。

## 3. 当前命名约定

这批标签当前按下面层级组织：

- `Item.Semantic.Weapon.*`
- `Item.Semantic.Consumable.*`

这个约定的好处是后续扩展时不会乱：

- 新武器类型可以继续挂在 `Item.Semantic.Weapon`
- 新消耗品类型可以继续挂在 `Item.Semantic.Consumable`

例如后续继续扩：

- `Item.Semantic.Weapon.Axe`
- `Item.Semantic.Weapon.Bow`
- `Item.Semantic.Consumable.StaminaPotion`

## 4. 当前真实落地状态

需要明确区分“代码里已经定义标签”和“内容资源已经开始使用标签”。

当前已经完成的是：

1. 全局 Gameplay Tag 已定义
2. AI 查询链已经能消费这类标签
3. 物品定义层已经有 `SemanticTags` 容器

当前还没有自动完成的是：

1. 具体物品资产不会因为代码里新增了标签就自动带上这些标签
2. 需要后续给具体 `ItemDefinition` 资源手动配置 `SemanticTags`
3. 如果玩家侧 UI 要显示这种类别，也需要决定显示层是否直接读取这些语义标签

## 5. 当前对 AI 的意义

对 AI 来说，这层语义标签的意义很直接：

1. 决策层可以说“我要找一把剑”，而不是绑定某个具体物品资产名
2. 同类物品可以共用一套决策查询条件
3. 后续资产替换时，AI 查询逻辑不需要跟着改代码

这条思路和当前 AI 库存决策架构是一致的：

- 决策层写语义
- 运行时查询层去匹配具体库存项
- 执行层只消费最终解析结果

## 6. 当前对玩家侧的意义

对玩家侧来说，这层标签目前更像“共享语义底座”。

当前已经能确定的是：

1. 这套标签不是 AI 私有数据
2. 后续玩家查看、筛选、分类时，可以复用同一套物品语义
3. 但当前玩家侧是否已经把这些标签直接显示出来，需要按具体 UI 方案决定

## 7. 相关笔记

- [[Inventory Equipment Decisions]]
- [[AI 项目地图]]
- [[AI 库存决策运行时调试与当前阻塞点 2026-06-16]]
