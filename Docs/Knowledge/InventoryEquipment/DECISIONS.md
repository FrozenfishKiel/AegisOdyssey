---
title: Inventory Equipment Decisions
tags:
  - knowledge
  - inventory-equipment
  - decisions
aliases:
  - Inventory Equipment Decisions
  - 库存与装备已锁定设计
---

# 库存与装备已锁定设计

更新时间：2026-05-20  
适用范围：当前库存接收、物品使用、正式装备栏、快捷栏边界里已经锁定为稳定结论的设计。  
不适用范围：所有单个物品蓝图配置细节、所有未来 UI 表现样式。

## 1. 入包语义只认统一入包成功，不认来源玩法名字

已经锁定：

1. 新系统如果想让玩家“获得物品”，应先接 `TryAddInventoryBatchToActor(...)`
2. 不应在采集、拾取、制作等来源系统里各自再写一套“获得提示”逻辑
3. 通知语义跟着“真正进入玩家背包”走，不跟着“采集/拾取/制作”名字走

## 2. 背包是当前玩家主接收容器

已经锁定：

1. `UAOBackPackComponent` 当前是统一入包主接收容器
2. 背包显式打开了获取物品通知出口
3. 快捷栏、正式装备槽、其他投影型库存不应冒充“获得物品”

## 3. 库存内“使用”必须通过实例语义分流

已经锁定：

1. 右键“使用”先由实例的 `CanUseFromInventory(...)` / `TryUseFromInventory(...)` 决定
2. 不应在 UI 侧把消耗品、武器、正式装备混成一套硬编码判断
3. 堆叠消耗由库存组件在服务端统一扣减，不由 UI 自己改数量

## 4. 消耗品默认语义已经固定

已经锁定：

1. 默认 `UAOInventoryItemInstance` 的“使用”语义是消耗品语义
2. 可用性由 `AOFragment_Consumable` 决定
3. 真正效果通过 `EffectsToApply` 对使用者 ASC 生效
4. 成功使用后当前默认消耗 `1` 个

## 5. 正式装备栏是库存语义，不是独立第二套物品系统

已经锁定：

1. 正式装备来源物必须是库存里的真实实例
2. 正式装备栏本身是标准库存投影
3. 装备动作最终仍然复用统一库存交换链
4. UI 不是正式装备真相层

## 6. 正式装备运行时真相层已经锁定

已经锁定：

1. `UAOFormalEquipmentManagerComponent` 负责“角色现在正式穿了什么”
2. `UAOFormalEquipmentSlotInventoryComponent` 只是五槽库存投影
3. 投影变化后，由正式装备管理器把槽位快照翻译回 `EquippedInstance + GrantedHandles`

这条边界后续不能被打穿。

## 7. 正式装备属性授予统一走 `AbilitySetsToGrant`

已经锁定：

1. 正式装备不再在 `AOFragment_FormalEquipment` 里配置属性授予
2. `AOFragment_FormalEquipment` 当前只负责声明唯一 `FormalSlotType`
3. 正式装备授予链统一走 `Definition.AbilitySetsToGrant`
4. 授予和回收都复用 `FAOAbilitySet_GrantedHandles`

这意味着正式装备和武器当前已经收敛到同一套授权结构。

## 8. 正式装备栏固定为五个唯一槽

已经锁定：

1. `Helmet`
2. `Armor`
3. `Gloves`
4. `Necklace`
5. `Boots`

每个正式槽只认自身合法类型，不接受模糊“装备物”概念。

## 9. 正式装备的合法性判断必须收口到管理器

已经锁定：

1. 是否能拖入正式槽，不由 UI 自己猜
2. 正式入口是 `CanAcceptSourceItemForFormalSlot(...)`
3. 至少要校验槽索引、实例类型、定义类型、槽类型匹配
4. 正式装备槽 UI 只能转发这条统一判断，不应在蓝图或控件层复制第二套“头盔能不能进这个槽”的规则
5. 如果后续要改正式装备合法性边界，优先改管理器或正式装备槽库存组件，不要只改某一个 Widget 的局部表现

因此“武器进不去正式装备栏”当前是后端边界，不是 UI 偶然表现。

## 10. 正式装备替换与卸下规则已分开

已经锁定：

1. 替换时，正式装备栏复用统一交换链，旧装备优先回流到来源槽
2. 右键卸下时，不回历史来源槽
3. 右键卸下当前首版规则是退回 `BackPack`
4. 背包无可用槽位时，卸下失败，装备保留在正式槽

## 11. 快捷栏与正式装备栏是两套系统

已经锁定：

1. 武器继续走 `QuickBar + WeaponManager`
2. 正式装备栏只处理长期穿戴槽
3. 两者都可以复用库存实例与 `AbilitySetsToGrant`
4. 但不共享同一个运行时真相组件

## 12. 正式装备槽的 UI 身份信息必须显式下发

已经锁定：

1. 正式装备槽 UI 不应只拿一份“当前槽里装了什么”的物品快照
2. 槽索引、槽类型、槽显示名这类“这个槽是谁”的身份信息也要一起下发
3. 即使槽当前为空，也应稳定显示它是 `Helmet / Armor / Gloves / Necklace / Boots` 里的哪一个
4. 正式装备栏的建槽数量应以正式装备管理器的固定五槽定义为准，而不是临时信任某一帧物品快照数组长度

这条边界的意义是：正式装备栏首先是“五个有语义的长期穿戴槽”，其次才是“这五个槽当前分别装了什么”。

## 13. 装备动画触发边界已经变化

当前代码已经确认：

1. 装备动画触发已改成 `SendGameplayEventToActor(...)`
2. 不再通过“刚授予能力后立即扫描 `ActivatableAbilities`”触发

因此后续不要再把旧的 Scope Lock 排查结论直接写成“当前装备动画仍依赖现授现用能力扫描”。

## 14. 获取物品通知桥接层已经锁定

已经锁定：

1. 世界级广播层是 `UAOInventoryMessageSubsystem`
2. HUD 桥接层是 `UAOHUDViewModelComponent`
3. HUD 聚合根消费层是 `UMVVM_HUD`
4. 蓝图便捷入口是 `UAOMainUI::ConsumePendingInventoryAcquisition()`

所以后续新系统要接“获得物品通知”，应接统一入包链，不应直接接 HUD。
