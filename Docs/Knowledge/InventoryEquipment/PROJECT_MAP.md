---
title: Inventory Equipment Project Map
tags:
  - knowledge
  - inventory-equipment
  - project-map
aliases:
  - Inventory Equipment Project Map
  - 库存与装备项目地图
---

# 库存与装备项目地图

更新时间：2026-05-19  
适用范围：当前项目里“统一入包、库存内右键使用、快捷栏装备、正式装备栏长期穿戴”这几条已经落地的库存与装备主链。  
不适用范围：采集系统本体、交互系统完整会话主链、所有具体 HUD 动画表现细节。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前玩家物品是怎么进入库存的。
2. 当前库存里的“使用”动作会分流到哪几条正式链。
3. 正式装备栏的运行时真相和库存投影分别落在哪里。
4. 快捷栏武器链和正式装备栏为什么是两套系统。

## 2. 当前主线不是一条，而是四条彼此衔接的链

当前 `InventoryEquipment` 组真正已经落地的是下面四条链：

1. 统一入包链
2. 库存内右键使用链
3. 快捷栏装备链
4. 正式装备栏长期穿戴链

它们互相有关联，但不应混写成“一套库存万能链”。

## 3. 统一入包链

优先看：

- `Source/AegisOdyssey/Inventory/AOInventoryStatics.*`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.*`
- `Source/AegisOdyssey/Inventory/AOBackPackComponent.*`

当前正式语义是：

1. 外部系统先把结果翻译成 `FAOInventoryReceiveBatch`
2. 再统一走 `UAOInventoryStatics::TryAddInventoryBatchToActor(...)`
3. 由角色身上的库存组件按优先级挑选接收者
4. 当前玩家主接收容器是 `UAOBackPackComponent`

这意味着“获得物品”语义当前默认锚定在背包，不锚定在快捷栏、正式装备栏或世界容器。

## 4. 库存内右键使用链

优先看：

- `Source/AegisOdyssey/Inventory/AOInventoryComponent.*`
- `Source/AegisOdyssey/Inventory/AOInventoryItemInstance.*`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.cpp`

当前正式链路是：

1. UI 右键菜单先问 `CanUseFromInventory(...)`
2. 点击“使用”后走 `TryUseItemAtSlot(...)`
3. 服务端真正执行 `TryUseFromInventory(...)`
4. 如果返回了消耗数量，再扣减堆叠并广播库存变化

这条链当前至少承接两类语义：

1. 普通消耗品：默认 `UAOInventoryItemInstance` 直接根据 `AOFragment_Consumable` 应用效果并消耗 1 个
2. 可装备物：具体子类改写 `CanUseFromInventory(...)`，把“使用”重定向成装备请求

## 5. 快捷栏武器链

优先看：

- `Source/AegisOdyssey/Equipment/AOQuickBarComponent.*`
- `Source/AegisOdyssey/Equipment/AOWeaponManagerComponent.*`
- `Source/AegisOdyssey/Equipment/AOEquipmentInstance.*`

当前正式语义是：

1. `QuickBar` 是装备/快捷使用槽，不是统一入包口
2. 快捷栏切槽会触发 `OnItemUse / OnItemUnUse`
3. 武器装备真相在 `UAOWeaponManagerComponent`
4. 武器属性授予走 `EquipmentDefinition -> AbilitySetsToGrant`

这条链和正式装备栏共享“库存里先有真实实例”这个前提，但不共享运行时真相层。

## 6. 正式装备栏长期穿戴链

优先看：

- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.*`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentInstance.*`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.*`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.*`
- `Source/AegisOdyssey/Inventory/Fragments/AOFragment_FormalEquipment.*`

当前正式主链可以概括成：

**正式装备定义 -> 正式装备实例 -> 正式装备槽库存投影 -> 正式装备运行时真相 -> ASC 授予结果**

其中：

- `UAOFormalEquipmentDefinition` 负责静态定义
- `UAOFormalEquipmentInstance` 是库存里真实移动的实例
- `UAOFormalEquipmentSlotInventoryComponent` 是五槽库存投影
- `UAOFormalEquipmentManagerComponent` 才是“角色当前穿了什么”的真相层

## 7. 正式装备栏当前固定骨架

当前已经锁定的槽位骨架是五个唯一槽：

1. `Helmet`
2. `Armor`
3. `Gloves`
4. `Necklace`
5. `Boots`

这一槽位顺序当前不只是 UI 排列，也是正式装备库存投影与运行时真相同步的索引基础。

## 8. 当前 ViewModel 与 UI 承接层

优先看：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.*`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.*`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.*`

当前正式装备栏已经不是“UI 直接扒组件状态”。

当前承接方式是：

1. `FormalEquipmentSlotInventoryComponent` 更新 `FormalEquipmentList`
2. `AOFormalEquipmentBarUI` 监听 `OnFormalEquipmentListChangedDynamic`
3. 单槽 Widget 消费槽位快照并负责右键卸下/拖拽判定

所以正式装备栏当前已经进入“库存投影 -> ViewModel -> Widget”这条观察链。

## 9. 当前继续排查时的阅读顺序

如果后续继续整理或排查，当前推荐顺序是：

1. `AOInventoryStatics.*`
2. `AOInventoryComponent.*`
3. `AOInventoryItemInstance.*`
4. `AOBackPackComponent.*`
5. `AOFormalEquipmentDefinition.*`
6. `AOFormalEquipmentInstance.*`
7. `AOFormalEquipmentSlotInventoryComponent.*`
8. `AOFormalEquipmentManagerComponent.*`
9. `AOFormalEquipmentBarUI.*`
10. `AOFormalEquipmentSlotUI.*`
11. `AOQuickBarComponent.*`
12. `AOWeaponManagerComponent.*`

## 10. 本轮提炼来源

本轮主要从下面三篇历史文档提炼，并结合当前代码核对：

- `Notice/HistoryNotice/正式装备栏系统方案锁定说明-2026-05-14.md`
- `Notice/HistoryNotice/消耗品与库存右键使用设计方案-现状-方案-实施阶段.md`
- `Notice/HistoryNotice/快捷栏数字键切换导致装备动画不播放-GAS Scope Lock问题排查说明.md`

沉淀后的稳定文档分别是：

- [[库存与装备已锁定设计]]
- [[正式装备栏与库存使用链说明]]
- [[库存与装备已知边界与历史偏差]]
