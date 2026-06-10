---
title: Inventory Context Menu MVVM Convergence
tags:
  - knowledge
  - inventory-equipment
  - ui
  - mvvm
  - context-menu
aliases:
  - Inventory Context Menu MVVM Convergence
  - 库存右键菜单 MVVM 收束说明
---

# 库存右键菜单 MVVM 收束说明
更新时间：2026-05-25

适用范围：库存右键菜单当前正式主链、唯一 ViewModel 来源、菜单层级与蓝图接线风险。  
不适用范围：具体按钮皮肤、右键菜单美术样式、一次性调试日志。

## 1. 当前这份文档解决什么问题

这份文档只回答：

1. 右键菜单到底从哪进入。
2. 菜单主 ViewModel 现在由谁持有。
3. 为什么现在不能再从 `UMVVM_InventoryMenu` 查菜单来源。
4. 菜单本体和动作项各自应该承接什么职责。
5. 后续再接右键菜单时，先看哪些代码。

## 2. 当前唯一主入口仍是 `AOInventoryUI`

优先看：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`

当前右键菜单主链应理解成：

1. `AOInventoryUI` 恢复这次右键请求命中的物品格子上下文。
2. `AOInventoryUI` 生成这次菜单需要的动作决策。
3. `AOInventoryUI` 通过 `SourceInventory->GetOrCreateContextMenuViewModel()` 拿到菜单主 ViewModel。
4. `AOInventoryUI` 把这次完整菜单快照灌入 `UMVVM_InventoryItemContextMenu`。
5. 菜单 Widget 只消费这份 ViewModel。

因此右键菜单到今天为止，唯一主入口仍然是 `AOInventoryUI`，不是库存组件自己弹，也不是 `UMVVM_InventoryMenu` 代发。

## 3. 当前菜单主 ViewModel 唯一来源

优先看：

- `Source/AegisOdyssey/Inventory/AOInventoryComponent.h`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp`

当前正式来源已经收口到：

- `SourceInventory->GetOrCreateContextMenuViewModel()`

这件事对应的正式边界是：

1. 每个 `UAOInventoryComponent` 只持有属于自己的一份 `ContextMenuViewModel`。
2. `UAOInventoryComponent` 在这条链上只负责存和懒创建。
3. `UAOInventoryComponent` 不再负责统一宿主解析。
4. `UAOInventoryComponent` 不再负责统一背包兜底。

所以后续继续接右键菜单时，不要再往库存组件里加“帮你猜来源是谁”的逻辑。

## 4. 右键谁，就用谁自己的来源

当前已锁定的真实口径是：

1. 右键谁，就以这次右键命中的 `OwnerComponent` 作为真实来源。
2. 菜单上下文、菜单 ViewModel、动作执行回流，都应围绕这次真实来源库存组件展开。

这样收口的意义是：避免“右键的是 A，菜单实际挂在 B，切换一次来源后就残留旧快照”这种问题。

## 5. 当前已不是 `UMVVM_InventoryMenu` 在承接右键菜单

这条边界已经正式变化：

1. `UMVVM_InventoryMenu` 现在已经不是右键菜单来源。
2. 如果继续在 `UMVVM_InventoryMenu` 里找 `ContextMenuViewModel`，方向就错了。

右键菜单当前自己的两层 MVVM 是：

1. `UMVVM_InventoryItemContextMenu`
2. `UMVVM_InventoryItemContextAction`

## 6. 当前两层 MVVM 的职责

优先看：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.cpp`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.cpp`

当前职责应理解成：

1. `UMVVM_InventoryItemContextMenu` 负责整份右键菜单快照。
2. `UMVVM_InventoryItemContextAction` 负责单条动作项快照。

也就是：

1. 菜单级别的数据集中收在菜单主 ViewModel。
2. 单动作按钮的数据集中收在动作项 ViewModel。
3. 不要再把一部分状态放 Widget，一部分状态放蓝图临时变量，一部分状态放别的 ViewModel。

## 7. 当前 Widget 层只应消费 ViewModel

优先看：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.cpp`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.cpp`

当前正确理解是：

1. 菜单 Widget 消费 `UMVVM_InventoryItemContextMenu`。
2. 动作项 Widget 消费 `UMVVM_InventoryItemContextAction`。
3. Widget 层不应继续自己重建菜单决策。
4. Widget 层不应继续自己判断库存逻辑。

## 8. 当前动作决策仍集中在 `AOInventoryUI.cpp`

这条边界现在反而很重要：

1. 动作列表目前仍由 `AOInventoryUI.cpp` 集中构建。
2. 这意味着后续要补 `Use / Drop / Split / Equip / Unequip` 一类动作时，优先沿着这条集中决策链扩，而不是在某个按钮蓝图里偷偷写第二套规则。

## 9. 当前蓝图接线和 MVVM 命名的高风险点

当前已经暴露出两个正式风险：

### 9.1 不要在动作项蓝图里重复创建 `MVVM_InventoryItemContextAction`

当前 C++ 侧已经有：

- `MVVM_InventoryItemContextAction`

所以蓝图 `WBP_InventoryContext_Action` 里不要再手动创建同名变量或同名 MVVM Source。否则会直接出现：

1. `There is already a property named 'MVVM_InventoryItemContextAction' ...`
2. `The source 'MVVM_InventoryItemContextAction' was evaluated to be invalid ...`
3. `The viewmodel name could not be found ...`

### 9.2 当前代码里仍存在一条临时兜底路径

当前 `AOInventoryUI.cpp` 里仍保留：

- `/Game/Games/UI/InventoryMenu/Information/WBP_InventoryContext_Action.WBP_InventoryContext_Action_C`

它能帮助调试，但它本身也是风险点，因为路径迁移、改名、宿主菜单蓝图与动作项蓝图概念混用时，都可能让排查变乱。

## 10. 当前继续接手时推荐先看什么

推荐先按这个顺序看：

1. `AOInventoryUI.cpp`
2. `AOInventoryComponent.h/.cpp`
3. `MVVM_InventoryItemContextMenu.h/.cpp`
4. `MVVM_InventoryItemContextAction.h/.cpp`
5. `AOInventoryItemContextMenuWidget.h/.cpp`
6. `AOInventoryItemContextActionWidget.h/.cpp`
7. [[制造系统项目地图]]

最后一条不是多余的。因为制造完成后的统一入库存和后续扩展动作项，会直接跨到库存和制造两个知识包。

