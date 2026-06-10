---
title: Inventory Exchange And Formal Equipment Status 2026-05-27
date: 2026-05-27
tags:
  - knowledge
  - inventory-equipment
  - interaction-system
  - formal-equipment
  - drag-drop
aliases:
  - 库存交换与正式装备现状 2026-05-27
  - 拖拽交换语义收口 2026-05-27
status: implemented
---

# 库存交换与正式装备现状 2026-05-27

更新范围：这篇笔记只记录 2026-05-27 这一轮已经实际落地、已编译验证通过的内容。  
不讨论后续 AI 决策层，也不扩展到蓝图拖拽 `Payload` 边界问题。

> [!summary]
> 这一轮真正落地的核心不是“再发明一套库存交换逻辑”，而是把现有交换链的两侧语义收口，并把会继续误导接手者的命名清掉。

## 1. 这轮到底改了什么

这轮实际完成了四件事：

1. 统一了库存交换主链对“两侧”的理解。
2. 收口了 `AAOChest` 这类容器入口里会误导人的 `Source / Target` 命名。
3. 收口了正式装备管理里把“库存物品”误写成 `SourceItem` 的命名。
4. 修复了 `UAOFormalEquipmentSlotUI` 里历史损坏的 `TEXT(...)` 字符串，恢复正式装备槽 UI 的可编译状态。

## 2. 当前锁定的交换两侧语义

当前代码里，库存交换主链的两侧语义已经锁成下面这套：

- `DraggedInventory / DraggedSlotIndex`：被拖起的那一侧。
- `DropInventory / DropSlotIndex`：当前落点那一侧。

这套语义当前已经对齐到：

- `UAOInventoryUI::RequestExchangeBetweenInventories(...)`
- `UAOInventoryComponent::ExecuteExchangeRequest(...)`
- `UAOInventoryComponent::WhenItemExchange(...)`

这意味着后续谁再接拖拽链、右键换位链、正式装备替换链，都不应该再把“来源侧 / 目标侧”写成抽象的 `Source / Target` 后再靠脑补理解。

## 3. `SourceContainer` 现在应该怎么理解

这轮顺手把一个长期容易带偏人的点也收口了：`SourceContainer` 不是拖拽两侧语义。

在下面这些 UI 槽位控件里：

- `UAOContainerSlot`
- `UAOSkillSlotUI`
- `UAOFormalEquipmentSlotUI`

`SourceContainer` 的真实含义都是：

**这个槽位自己隶属于哪个真实库存组件。**

它回答的是“这个槽是谁家的”，不是“这次拖拽里谁是来源侧”。  
真正的拖拽两侧，还是要回到 `DraggedInventory / DropInventory` 这一组参数上理解。

## 4. `AAOChest` 当前为什么改名

这轮把 `AAOChest` 里的这两个函数改成了更直白的命名：

- `TransferChestSlotToInventorySlot(...)`
- `TransferInventorySlotToChestSlot(...)`

原因不是功能变化，而是语义收口。

旧名字：

- `TransferItemToInventory(...)`
- `TransferItemFromInventory(...)`

虽然单看不一定错，但在多轮接手后，很容易继续把这里的“箱子内 / 箱子外”误读成拖拽 `Source / Target`，甚至把参数方向接反。

当前这两个新名字只表达一件事：

- 一侧是箱子槽位。
- 另一侧是箱子外部库存槽位。

它们只是对象语义里的“箱子内 / 箱子外”，不是拖拽层里的 `Dragged / Drop`。

## 5. 正式装备管理这一轮收口了什么

这轮把正式装备管理中几组容易混淆的名字改成了更准确的领域语义：

- `CanAcceptInventoryItemForFormalSlot(...)`
- `RequestEquipInventoryItemToSlot(...)`
- `EquipInventoryItemToSlot(...)`
- `FindOwningInventoryContainingItem(...)`

这里的重点不是“改名好看”，而是避免把“库存里的真实物品实例”继续误写成 `SourceItem`，然后和拖拽来源侧混在一起。

当前正式装备管理的含义应该这样理解：

1. 它判断的是“某件库存物品能不能进入某个正式槽”。
2. 它在真正执行装备时，会先反查这件物品当前到底属于哪个真实库存。
3. 最终仍然回落到统一库存交换主链，而不是自己另写一套正式装备迁移 RPC。

## 6. `UAOFormalEquipmentSlotUI` 当前已知状态

这轮确认并修复了 `UAOFormalEquipmentSlotUI` 的一个历史代码损坏问题：

- 右键菜单字符串字面量损坏，导致 `卸下 / 关闭` 无法编译。
- 正式装备槽标签字符串字面量损坏，导致 `头盔 / 护甲 / 手套 / 项链 / 靴子` 无法编译。

当前这个文件已经被重新整理到可编译状态，槽位文案现状是：

- `头盔`
- `护甲`
- `手套`
- `项链`
- `靴子`
- 默认兜底：`正式装备`

右键菜单文案现状是：

- `卸下`
- `关闭`

## 7. 当前没有改什么

这轮有几个边界是明确没动的：

1. 没有扩展蓝图拖拽 `Payload` 的创建和传递边界。
2. 没有开始 AI 自主决策使用库存的行为逻辑层。
3. 没有去重写技能系统里本来就属于“技能来源物”领域语义的 `SourceItem` 命名。
4. 没有因为这轮语义收口去发明第二套库存交换链。

> [!warning]
> 后续如果再排拖拽问题，先分清是在排“槽位归属语义”还是在排“拖拽两侧语义”。这两件事不是一回事。

## 8. 这轮落地文件

本轮直接落地并已编译通过的核心文件是：

- `Source/AegisOdyssey/Interaction/Containers/AOChest.h`
- `Source/AegisOdyssey/Interaction/Containers/AOChest.cpp`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.cpp`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.cpp`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.cpp`
- `Source/AegisOdyssey/UI/Common/Inventory/AOContainerSlot.h`
- `Source/AegisOdyssey/UI/Common/Inventory/AOContainerSlot.cpp`

## 9. 验证结论

2026-05-27 这一轮已经重新编译过：

- `AegisOdysseyEditor Win64 Development`

结果是：

- `Succeeded`

## 10. 建议挂回的旧知识链

这篇笔记应该和下面几篇一起看：

- [[Docs/Knowledge/InventoryEquipment/FORMAL_EQUIPMENT_AND_INVENTORY_USE|正式装备栏与库存使用链说明]]
- [[Docs/Knowledge/InventoryEquipment/DECISIONS|库存与装备已锁定设计]]
- [[Docs/Knowledge/InventoryEquipment/PROJECT_MAP|库存与装备项目地图]]
- [[Docs/Knowledge/InteractionSystem/MUTATION_AND_CONTAINER_SYNC|Interaction Mutation 与容器同步]]
- [[Docs/Knowledge/InteractionSystem/PROJECT_MAP|交互系统项目地图]]

## 11. 一句话版结论

当前正式装备、容器拖拽、普通库存交换，底层都还是同一条库存交换主链。  
这轮做的事情，是把这条主链的两侧语义和外围入口命名收干净，避免后续再有人把参数方向和对象语义看反。
