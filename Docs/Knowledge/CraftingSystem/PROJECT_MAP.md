---
title: Crafting System Project Map
tags:
  - knowledge
  - crafting-system
  - project-map
aliases:
  - Crafting System Project Map
  - 制造系统项目地图
---

# 制造系统项目地图
更新时间：2026-05-27

这份文档是给后续接手代码的人看的。重点不是把所有文件都列一遍，而是告诉你现在这套制造系统的真实主链在哪，应该先看哪几个文件。

相关文档：

- [[Crafting UI Refresh Refactor Plan]]
- [[Crafting System Decisions]]
- [[Crafting System Known Issues]]
- [[Crafting Current Implemented State 2026-05-28]]
- [[Crafting Refactor Handoff 2026-05-27]]

## 1. 当前主链先怎么理解

先把当前主链记住：

`UAOPawnData::CraftingRecipeDataTable -> UAOCraftingComponent -> UMVVM_Crafting -> Widget`

这条链的意思是：

- 角色侧制造表定义配方规则
- 制造组件持有底层真相
- ViewModel 直接观察底层
- Widget 只消费 ViewModel

如果你从一开始就按这个口径看代码，很多旧问题会自动少一半。

## 2. 先看哪些文件

### 2.1 配方定义和角色入口

先看这些文件：

- [AOPawnData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.h)
- [AOPawnData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp)
- [AOCraftingRecipeTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h)

这三处要看清楚两件事：

- 角色现在只有一张制造表
- 配方行里已经包含解锁、排序、材料、产物和时长

### 2.2 底层制造真相

再看：

- [AOCraftingComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h)
- [AOCraftingComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp)

这里是当前制造系统最重要的文件。

优先看这些函数：

- `BuildRecipeListViewData()`
- `BuildRecipeDetailViewData()`
- `BuildQueueViewData()`
- `RequestEnqueueRecipe()`
- `TryEnqueueRecipeOnAuthority()`
- `FindRecipeRow()`
- `BindObservedInventorySources()`
- `HandleObservedInventoryChanged()`
- `NotifyCraftingObservationChanged()`

看完这些，基本就能知道配方是怎么被读取、怎么被判定、怎么变成 UI 数据的。

### 2.3 物品定义解析

再看：

- [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
- [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)

这里现在只负责物品总表。

如果你在查图标、名字、Definition 为空之类的问题，就要回到这里确认 `ItemId` 能不能正常解到物品定义。

### 2.4 MVVM 接底层的地方

再看：

- [MVVM_Crafting.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h)
- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp)
- [AOHUDViewModelComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/AOHUDViewModelComponent.cpp)

这里主要看三件事：

- HUD 怎么把当前 Pawn 上的制造组件交给 MVVM
- MVVM 怎么订阅底层变化
- MVVM 怎么统一重拉列表、详情和队列

### 2.5 Widget 怎么消费数据

最后看：

- [AOCraftingWidgetBase.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h)
- [AOCraftingWidgetBase.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp)
- [AOCraftingRecipeListWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.h)
- [AOCraftingRecipeListWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp)
- [AOCraftingRecipeDetailWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.h)
- [AOCraftingRecipeDetailWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.cpp)

这里要重点记住一件事：

`UAOCraftingRecipeListEntryWidget` 定义在 `AOCraftingRecipeListWidget.h/.cpp` 里，不是单独文件。它现在同时承载：

- 配方列表项
- 材料项
- 产物项

## 3. 当前数据结构怎么分层

当前观察数据定义在：

- [AOCraftingObservationTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h)

这里现在主要有：

- `FAOCraftingRecipeListEntryViewData`
- `FAOCraftingRecipeDetailViewData`
- `FAOCraftingMaterialViewData`
- `FAOCraftingOutputViewData`
- `FAOCraftingQueueEntryViewData`

这些结构体还没完全收干净，但当前已经有一个正确方向：它们直接带 `ItemDefinition`，UI 不再依赖一堆为显示专门复制出来的字段。

## 4. 当前刷新链怎么找

如果你是在查“为什么没刷新”，按这条顺序看：

1. `UAOCraftingComponent` 有没有广播 `OnCraftingObservationChanged`
2. `UMVVM_Crafting` 有没有接到广播并执行 `RefreshObservationData()`
3. `AOCraftingWidgetBase` 有没有收到 ViewModel 变化通知
4. 列表或详情 Widget 有没有重建子项
5. 子项 Widget 的显示函数有没有被调用

当前不推荐再从 Layout 往回推，因为 Layout 已经不应该是制造刷新源。

## 5. 当前推荐阅读顺序

如果你完全没看过这套系统，建议按这个顺序读：

1. `AOPawnData.h/.cpp`
2. `AOCraftingRecipeTypes.h`
3. `AOCraftingObservationTypes.h`
4. `AOCraftingComponent.h/.cpp`
5. `AOGameData.h/.cpp`
6. `MVVM_Crafting.h/.cpp`
7. `AOHUDViewModelComponent.cpp`
8. `AOCraftingWidgetBase.*`
9. `AOCraftingRecipeListWidget.*`
10. `AOCraftingRecipeDetailWidget.*`

按这个顺序看，先抓规则入口，再抓底层真相，再看 UI 消费链，理解成本最低。

## 6. 当前最容易读错的地方

这里单独列一下，避免后续再绕回旧口径。

- 不要再找 `CraftingRecipeSourceDataTable`
- 不要再找 `AOGameData` 里的制造配方表
- 不要把 HUD 理解成制造中转刷新层
- 不要把 Layout 理解成制造面板专属调度器
- 不要把 `UAOCraftingRecipeListEntryWidget` 误以为是独立文件

只要这五件事不再读错，当前代码结构基本就不会看偏。
