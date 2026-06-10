---
title: Inventory Equipment Known Issues
tags:
  - knowledge
  - inventory-equipment
  - known-issues
aliases:
  - Inventory Equipment Known Issues
  - 库存与装备已知边界与历史偏差
---

# 库存与装备已知边界与历史偏差

更新时间：2026-05-19  
适用范围：当前 `InventoryEquipment` 这一轮深提炼里已经识别出的历史混层、实现边界与后续整理风险。  
不适用范围：完整运行时 bug 清单。

## 1. 当前这组三篇历史文档混了三类不同内容

这一轮来源文档里同时混有：

1. 正式装备设计与落地说明
2. 消耗品使用与统一入包通知方案
3. 快捷栏数字键切槽的 GAS 排查记录

它们都和库存/装备有关，但不应整理成一篇“库存系统总说明”。

因此本轮知识包只先拆成：

- `PROJECT_MAP`
- `DECISIONS`
- `FORMAL_EQUIPMENT_AND_INVENTORY_USE`
- `KNOWN_ISSUES`

## 2. 当前最容易误判的边界

### 2.1 不要把正式装备栏写成角色第二套独立物品系统

当前不是。

已确认当前事实：

1. 正式装备来源物必须是库存真实实例
2. 正式装备槽本身是库存投影
3. 正式装备管理器才是长期穿戴真相层

### 2.2 不要把 `AOFragment_FormalEquipment` 再写回“属性授予入口”

当前不是。

当前已确认：

1. Fragment 只负责 `FormalSlotType`
2. 属性/能力授予统一走 `AbilitySetsToGrant`

### 2.3 不要把“获得物品通知”理解成所有库存变化都会触发

当前不是。

当前已确认：

1. 背包会正式广播
2. 快捷栏不会
3. 正式装备槽不会
4. 世界容器不应冒充玩家获得物品

### 2.4 不要把历史 Scope Lock 排查直接写成“当前仍存在的装备动画故障”

当前代码已经不是当时那条实现。

这轮核对后确认：

1. 历史文档讨论的是“动态授予能力后立刻扫描可激活能力列表”的风险
2. 当前 `TryPlayEquipmentAnimation(...)` 已改成 `SendGameplayEventToActor(...)`
3. 因此那篇文档当前更适合沉淀成“设计边界与误判点”，而不是当前未修复故障

## 3. 当前尚未继续展开、但价值很高的后续主题

以下内容都已经露出价值，但不在本轮正文沉淀范围内：

1. 武器 QuickBar 全链与正式装备栏之间的完整协作边界
2. 正式装备资产的具体蓝图配置清单与验收模板
3. 消耗品之外的更多库存内“使用”语义分类
4. 获取物品通知的具体蓝图 Toast 播放规范
5. 快捷栏装备动画历史问题是否还需要单独沉淀到 `DebugCases`

## 4. 当前整理规则

后续继续往 `Docs/Knowledge/InventoryEquipment` 提炼时，默认遵守：

1. 先区分“统一入包”“库存使用”“正式装备长期穿戴”“快捷栏武器”四层，不要混写。
2. 任何涉及“当前装备授予配置入口”的说法，优先核 `AOFormalEquipmentDefinition.*` 与 `AOFormalEquipmentManagerComponent.*`。
3. 任何涉及“获得物品通知给谁发”的说法，优先核 `ShouldBroadcastInventoryAcquisitionNotifications()` 和 `AOBackPackComponent`。
4. 任何涉及“当前是否仍受 Scope Lock 历史问题影响”的说法，优先核 `AOEquipmentInstance::TryPlayEquipmentAnimation(...)` 当前实现。
5. 不把历史交接/排查记录原样搬进知识库正文。

## 5. 当前新增识别出的一个跨包边界提醒

### 5.1 正式装备来源属性链已经回退到稳定 AttributeSet 来源，但 MMC 联动问题仍未解决

当前已经确认：

1. 正式装备五槽、库存来源物、`AbilitySetsToGrant` 这条装备主链可以继续视为稳定实现方向。
2. 但装备来源属性接进 GAS 之后，最终属性的 `MMC` 派生刷新链仍然没有彻底打通。
3. 这不应再反过来推翻正式装备栏本身的设计边界。

因此后续继续接正式装备系统时，要分清两件事：

1. 正式装备栏本体和库存投影链，是 `InventoryEquipment` 范围。
2. 来源属性变化后为什么最终属性没刷新出来，已经进入 `GameplayFramework / GAS` 排查范围。

当前这一块的长期提醒，默认参考：

- [[GAS MMC 派生属性刷新边界]]
