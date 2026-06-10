---
title: Character Container UI And AI Inventory Debug 2026-05-29
date: 2026-05-29
tags:
  - knowledge
  - interaction-system
  - inventory-equipment
  - state-tree-ai
  - ui
  - debug
status: active
aliases:
  - 角色作为容器 UI 与 AI 库存排查 2026-05-29
---

# 角色作为容器 UI 与 AI 库存排查

这份笔记只记录当前已经确认过的技术事实和边界，避免后面继续把问题重新写回“箱子专属逻辑”或者“UI 自己硬找目标组件”的老路。

## 一、角色库存交互当前的正式理解

### 1. 角色可以被视为可交互容器对象

当前这轮已经正式锁定：

- 角色打开角色库存，不是新发明第二套库存系统
- 它仍然属于现有容器交互体系的扩展
- 箱子只是第一个跑通链路的对象，不是唯一语义

因此后续如果再做角色库存、尸体库存、调试开放敌人库存，本质都应继续站在同一条容器交互主链上。

### 2. 交互打开 UI 的承接层必须是 `UAOInteractionSessionWidget` 链

之前角色交互打不开 UI 的一个关键原因，已经确认不是“角色不能当容器”，而是：

- 交互推出来的界面如果不在 `UAOInteractionSessionWidget` 体系里
- `UAOHUDLayout` 就不会按既有容器会话链去绑定当前 SessionModel

因此当前正确边界是：

- `UAOLayout_Inventory` 负责交互会话承接
- `UAOInventoryPageUI` 负责库存页内容组织

不要为了角色库存交互，把更宽的会话父类改成库存专用父类。

## 二、这轮 UI / MVVM 纠偏后的边界

### 1. `UAOInventoryPageUI` 才是这页库存界面的显示上下文组织层

这轮最重要的收口，不是“某个控件终于显示出来了”，而是职责重新摆正了。

当前正式口径：

- 这页库存界面显示谁、显示哪一组库存数据，应该由库存页这一层组织
- 下面的背包、物品栏、正式装备、技能栏子 UI，只负责消费上层注入的显示上下文

也就是说：

- 子 UI 不应该自己判断“我是玩家区还是目标区”
- 子 UI 不应该自己反查当前目标 Actor
- 子 UI 不应该自己去目标身上抓组件

### 2. 不要把“取谁的数据”下沉成各子 UI 的硬编码分支

这一点后面非常容易再次跑偏，必须单独记清楚。

错误方向是：

- 每个子库存 Widget 自己判断 self / target
- 每个子库存 Widget 自己查 Backpack / QuickBar / FormalEquipment / Skill
- 最后越写越像“每个面板都内置一套目标解析器”

正确方向是：

- 上层先组织好这页界面的显示上下文
- 子面板只绑定被注入的那份上下文

这才符合当前项目一直在强调的 MVVM 口径。

## 三、AI 背包 / 物品栏不显示问题的当前结论

### 1. 组件存在，不等于已经有可显示槽位

当前已经核到 `AAOCharacter` 构造时确实创建了：

- `UAOQuickBarComponent`
- `UAOBackPackComponent`
- `UAOFormalEquipmentManagerComponent`
- `UAOFormalEquipmentSlotInventoryComponent`

所以问题至少不在“AI 根本没有这些组件”这一层。

### 2. 当前更像是槽位初始化 / 默认值问题

当前已经确认：

- `UAOInventoryComponent` 基类默认 `NumSlots = 0`
- `UAOFormalEquipmentSlotInventoryComponent` 显式把 `NumSlots` 设成了 `5`
- `UAOBackPackComponent` 当前没有看到 C++ 兜底槽位数
- `UAOQuickBarComponent` 当前也没有看到 C++ 兜底槽位数

这会直接导致一个现象：

- 正式装备槽能显示，因为它自带槽位数
- 背包和物品栏如果资源默认值没配，运行时就可能还是 0 槽

### 3. 已尝试过初始化链补救，但当前没有证明问题在这里

这轮已经尝试在：

- `UAOBackPackComponent`
- `UAOQuickBarComponent`

里补 `CheckDefaultInitialization()` 的触发。

结果是：

- 编译通过
- 实测后问题仍在

因此当前不能把结论写成“少调用初始化导致 UI 不显示”，最多只能说：

- 这条路径已经排过一轮
- 它不是当前唯一解释，且实测没有直接解决问题

## 四、当前最高优先级的排查方向

下一步建议优先核：

1. AI 实际使用的角色蓝图 / Pawn 蓝图里，背包和物品栏组件的 `NumSlots` 默认值
2. 玩家蓝图与 AI 蓝图之间，是否只有玩家那套配了槽位
3. 是否存在运行时初始化链把资源侧默认值重新覆盖回 0

> [!warning]
> 在这个问题没查清之前，不建议继续把症状强行解释成更深层的 UI / MVVM 设计问题。
> 当前更像是底层目标库存本身没有形成有效显示数据。

## 五、这份笔记要防止的两个常见误判

### 1. 不要再把“角色库存交互”写回角色专属第二体系

角色库存交互仍然是容器交互的一个对象扩展，不是新的并行系统。

### 2. 不要再把“目标库存不显示”直接写成子 UI 应该自己去目标身上找数据

如果最后真是目标对象根本没槽位，UI 写再深也只是把问题掩住，不是解决问题。

## 六、关联文档

- [[CHARACTER_INVENTORY_INTERACTION_DESIGN_2026-05-28]]
- [[CHARACTER_INVENTORY_INTERACTION_DESIGN_SUPPLEMENT_2026-05-28]]
- [[../InventoryEquipment/INVENTORY_MAINLINES_2026-05-28]]
- [[../StateTreeAI/AI_INVENTORY_USE_AND_DECISION_STATUS_2026-05-27]]
