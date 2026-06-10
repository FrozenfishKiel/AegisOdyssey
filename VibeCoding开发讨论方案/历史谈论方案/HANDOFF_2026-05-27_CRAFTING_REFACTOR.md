---
title: Crafting Refactor Handoff 2026-05-27
tags:
  - knowledge
  - crafting-system
  - handoff
aliases:
  - Crafting Refactor Handoff 2026-05-27
  - 制造系统重构交接 2026-05-27
---

# 制造系统重构交接 2026-05-27
更新时间：2026-05-27

这份文档是给下一个 AI 或下一个接手人看的。目标不是复述所有历史讨论，而是快速让后来者知道现在代码改到了哪、哪些东西已经定了、接下来别在哪些地方再走回头路。

配套文档：

- [[Crafting UI Refresh Refactor Plan]]
- [[Crafting System Decisions]]
- [[Crafting System Project Map]]
- [[Crafting System Known Issues]]

## 第一章 这轮已经改了什么

### 1.1 制造配方入口已经改成单表

已经完成：

- `UAOPawnData` 只保留 `CraftingRecipeDataTable`
- `CraftingRecipeSourceDataTable` 已退出当前架构
- `FAOCraftingRecipeRow` 已合并承载解锁、排序、材料、产物、时长
- `AOGameData` 不再持有制造配方表，只保留物品总表

核心文件：

- [AOPawnData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.h)
- [AOPawnData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp)
- [AOCraftingRecipeTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h)
- [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
- [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)

### 1.2 MVVM 观察链已经改成直连底层

已经完成：

- `UMVVM_Crafting` 直接观察 `UAOCraftingComponent`
- HUD 只负责绑定当前 Pawn 的制造组件
- Layout 不再作为制造刷新调度器
- Widget 不再通过蓝图逻辑接管刷新时机

核心文件：

- [AOCraftingComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h)
- [AOCraftingComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp)
- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp)
- [AOHUDViewModelComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/AOHUDViewModelComponent.cpp)

### 1.3 UI 显示逻辑已经尽量收回 C++

已经完成：

- 蓝图不再承担读图标、名字、计数、状态这些刷新逻辑
- `UAOCraftingRecipeListEntryWidget` 统一处理配方项、材料项、产物项三种显示模式
- 详情区材料和产物项复用同一个条目类

相关文件：

- [AOCraftingRecipeListWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.h)
- [AOCraftingRecipeListWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp)
- [AOCraftingRecipeDetailWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.h)
- [AOCraftingRecipeDetailWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.cpp)

## 第二章 当前真实架构怎么记

只记这一条就够了：

`PawnData 单表 -> CraftingComponent 真相 -> MVVM 直连观察 -> Widget 只消费`

如果后面有人再把分析链写成：

- source 表开放配方
- GameData 表给详情
- HUD 镜像一层
- Layout 再补一层刷新

那就是又回到旧方案了。

## 第三章 当前最重要的调试入口

### 3.1 查配方有没有进系统

先查：

1. `UAOPawnData::GetCraftingRecipeDataTable()`
2. `UAOCraftingComponent::GetOwnerCraftingRecipeTable()`
3. `UAOCraftingComponent::FindRecipeRow()`

如果这里拿不到配方，不要再去找 `CraftingRecipeSourceDataTable` 或 `AOGameData` 里的制造表，因为当前架构里已经没有这条链了。

### 3.2 查 UI 为什么没刷新

按这个顺序查：

1. `UAOCraftingComponent::NotifyCraftingObservationChanged()`
2. `UMVVM_Crafting::HandleObservedCraftingObservationChanged()`
3. `UMVVM_Crafting::RefreshObservationData()`
4. `UAOCraftingRecipeListWidget::HandleCraftingViewModelChanged()`
5. `UAOCraftingRecipeDetailWidget::HandleCraftingViewModelChanged()`
6. `UAOCraftingRecipeListEntryWidget::RefreshRecipeDisplay() / RefreshMaterialDisplay() / RefreshOutputDisplay()`

### 3.3 查 `CraftRecipe ... returned false`

这条日志不是命令没执行，而是底层校验失败。

优先排查：

1. `RecipeRowName` 是否真是当前单表中的行名
2. 当前 Pawn 吃到的 `PawnData` 是否对
3. 配方里的 `ItemId` 是否都能解出 `ItemDefinition`
4. 当前可参与制造的库存里是否真有材料

## 第四章 当前还没解决干净的问题

### 4.1 中文乱码风险还在

当前代码和文档里都出现过乱码，所以后续如果看到中文显示不对，不要只查绑定逻辑，也要查文件编码和字符串字面量。

### 4.2 观察数据结构还没完全收口

当前虽然已经改成直接传 `ItemDefinition`，但 `FAOCrafting*ViewData` 仍然偏多，命名里也还留着 `PrimaryOutputDefinition` 这种偏旧语义字段。

### 4.3 蓝图壳子仍然需要配置正确

逻辑已经尽量收回 C++，但蓝图壳子仍要保证：

- 容器控件存在
- 绑定控件名对
- `RecipeEntryWidgetClass`、`MaterialEntryWidgetClass`、`OutputEntryWidgetClass` 配对正确

## 第五章 后续继续改时不要再踩的坑

不要再做这些事：

- 不要把制造表再塞回 `AOGameData`
- 不要把单表重新拆成双表
- 不要再让 Layout 专门替制造补刷新
- 不要再让 HUD 复制一层制造快照
- 不要再让 Widget 直接命令底层

只要这五条守住，后续继续清结构、补 UI、补测试，方向就不会再歪。

## 第六章 当前状态一句话总结

这轮不是把所有制造问题都做完了，但已经把最核心的架构方向纠正过来了。

现在最重要的不是再临时补一层刷新，而是继续沿着“单表、底层真相、MVVM 直连、Widget 只消费”的方向，把剩下的显示问题、编码问题和结构收敛问题继续做干净。
