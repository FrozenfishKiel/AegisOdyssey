---
title: Character Inventory Interaction Progress 2026-05-29
date: 2026-05-29
tags:
  - progress
  - interaction-system
  - inventory-equipment
  - ui
  - ai
status: in-progress
aliases:
  - 角色库存交互阶段进度 2026-05-29
---

# 角色库存交互阶段进度

这份文档不是知识库正文，只用于记录当前这一轮工作的实际进度、已经确认的结论、以及下一步排查口径。

## 这轮已经收住的结论

### 1. 角色库存交互继续复用既有容器交互链

当前已经明确，角色与角色之间打开库存这件事，本质上仍然是“把角色当成可交互容器对象”。

这意味着：

- 入口继续走普通交互选项
- 会话继续复用既有容器会话语义
- 不额外发明一套“角色专属库存交互体系”

换句话说，这件事在架构上不应脱离箱子链路，只是把“箱子”从静态容器扩成了“角色也可以成为容器目标”。

### 2. 交互弹出的 UI 必须站在 `UAOInteractionSessionWidget` 链上

之前已经确认过一次关键问题：

- 角色交互后如果希望像箱子一样正常弹出并绑定会话
- 外层承载 Widget 必须接到 `UAOInteractionSessionWidget` 这一层

最终收口口径是：

- `UAOLayout_Inventory` 继续承担交互 / 会话承接层职责
- `UAOInventoryPageUI` 继续承担库存页内部显示职责
- 不把库存专属逻辑反向塞进更宽的会话父类里

### 3. 不再让底层子库存 UI 自己判断“看自己还是看目标”

这一轮最大的纠偏，就是把职责重新拉回正确边界。

当前确认的边界是：

- `AOInventoryPage` 这一层负责组织这页库存界面的显示上下文
- 下面的背包、物品栏、正式装备、技能栏等子 UI，只消费上层注入的数据
- 子 UI 不应自己去判断当前是在显示玩家自己，还是目标对象
- 子 UI 不应自己回目标 Actor 身上重新翻组件

否则就会把 MVVM 和会话边界重新打乱，最后越改越深。

## 当前代码侧已确认的事实

### 1. AI 角色相关库存组件在构造时是创建了的

`AAOCharacter` 当前构造函数里已经创建了这些组件：

- `CharacterQuickBar`
- `CharacterBackPackComponent`
- `CharacterFormalEquipmentManagerComponent`
- `CharacterFormalEquipmentSlotInventoryComponent`

所以当前问题不像是“AI 根本没有这些组件”。

### 2. 正式装备槽能显示，不代表背包和物品栏初始化也没问题

已经核到的代码事实是：

- `UAOInventoryComponent` 基类默认 `NumSlots = 0`
- `UAOFormalEquipmentSlotInventoryComponent` 在 C++ 里显式设了 `NumSlots = 5`
- `UAOBackPackComponent` 当前没有看到 C++ 兜底槽位数
- `UAOQuickBarComponent` 当前也没有看到 C++ 兜底槽位数

这意味着一个非常现实的可能：

- 正式装备栏之所以能看见，是因为它自己在代码里就给了槽位
- AI 的背包 / 物品栏如果蓝图默认值没配好，就可能还是 0 槽

## 这轮已经做过但未解决的问题

### 1. 已补过一次默认初始化触发

这轮已经在下面两个组件里补过 `CheckDefaultInitialization()` 触发：

- `UAOBackPackComponent`
- `UAOQuickBarComponent`

这一步的目的，是先排除“初始化链根本没跑到”的可能。

结果是：

- 编译通过
- 用户侧实测后，问题依旧

所以目前不能把问题简单归因成“少调了一次初始化函数”。

### 2. 当前更高概率根因

基于现在已经核到的事实，当前更高概率的根因是：

- AI 使用的角色蓝图、派生蓝图，或者 AI 相关配置链路里
- `CharacterBackPackComponent` / `CharacterQuickBar` 的 `NumSlots` 默认值没有配好

这只是当前高概率判断，不是最终定论。

## 当前阶段最重要的排查口径

下一步优先检查：

1. AI 实际使用的角色蓝图 / Pawn 蓝图里，背包和物品栏组件的 `NumSlots` 是否为 0
2. 是否只有玩家蓝图配了槽位，而 AI 蓝图没有继承到
3. 是否某个数据资产 / 初始化链路会在运行时覆盖这些默认值

## 现阶段建议

> [!note]
> 当前不要继续往 UI 深层乱改。
> 这轮更像是“目标对象的背包 / 物品栏本身就没成功建出有效槽位”，而不是“显示链路不知道怎么显示”。

如果后面确认蓝图默认值确实没配，那么优先先修资源配置或初始化来源，再决定是否需要补 C++ 兜底。

## 相关知识入口

- [[Docs/Knowledge/InteractionSystem/CHARACTER_CONTAINER_UI_AND_AI_INVENTORY_DEBUG_2026-05-29]]
