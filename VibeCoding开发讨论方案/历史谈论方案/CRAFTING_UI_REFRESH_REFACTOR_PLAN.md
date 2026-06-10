---
title: Crafting UI Refresh Refactor Plan
tags:
  - knowledge
  - crafting-system
  - ui
  - mvvm
  - refactor
aliases:
  - Crafting UI Refresh Refactor Plan
  - 制造系统 UI 与 MVVM 收敛重构方案
---

# 制造系统 UI 与 MVVM 收敛重构方案
更新时间：2026-05-28

这份文档写当前已经落地的制造系统 UI 架构，也写这轮重构为什么要这么改。它不是旧方案备份，也不是泛泛而谈的“未来愿景”，而是给后续接手代码的人一个统一口径。

相关文档：

- [[Crafting System Decisions]]
- [[Crafting System Project Map]]
- [[Crafting System Known Issues]]
- [[Crafting Refactor Handoff 2026-05-27]]

## 第一章 这次真正要解决的不是一个按钮，而是整条链

这轮重构要解决的，其实一直是三类问题叠在一起。

第一类是数据来源不完整。UI 需要的东西不是没地方拿，而是中间层把数据拆碎了，又没有稳定地把真相传上来。最典型的例子就是图标、名字、材料数量、缺料状态这些信息，经常出现有的控件拿到了，有的控件没拿到，或者拿的是过时数据。

第二类是边界错了。原来 Layout、HUD、Widget、ViewModel 都在“顺手刷新一下制造”，结果是每层都像是在帮忙，最后谁都说不清到底谁负责。制造面板只是库存 UI 的一部分，不应该在 Layout 里单独挂一套调度系统。

第三类是刷新时机错了。制造数据不是只在“打开时”和“制造结束时”刷新。只要底层真相变了，UI 就应该重新从 MVVM 拉数据，而不是靠某个中间层猜“这次也许该补一帧”。

这轮方案的核心目标只有一句话：

**制造表继续定义制造规则，`UAOCraftingComponent` 持有制造真相，`UMVVM_Crafting` 直接观察底层，Widget 只消费 MVVM 并向 MVVM 发命令，Layout 不再越权。**

## 第二章 这次锁死了哪些边界

### 2.1 制造表还是制造系统的唯一规则入口

这次没有改掉“制造系统必须读制造表”这件事，反而是把它收得更死。

当前真实口径是：

- 配方规则来自 `UAOPawnData::CraftingRecipeDataTable`
- 解锁等级、未解锁是否可见、排序、材料、产物、基础制造时长，全部都在 `FAOCraftingRecipeRow`
- `AOGameData` 不再持有制造配方表
- 物品总表 `ItemData` 仍然独立存在，只负责 `ItemId -> ItemDefinition`

这件事很重要，因为制造系统和物品系统不是一张表。合并的是两张制造配方表，不是把物品总表也混进来。

### 2.2 `PawnData` 现在只有一个制造表槽位

当前代码已经按这个口径落地：

- `UAOPawnData` 只保留 `CraftingRecipeDataTable`
- 不再有 `CraftingRecipeSourceDataTable`
- 不再有“角色表开配方，再去全局表查详情”的双表链路
- 冷启动排查时，只按单表链路思考

也就是说，现在角色能制造什么、每条配方长什么样，都在这一张角色侧制造表里说清楚。

### 2.3 Widget 不能越过 MVVM 直接命令底层

这条边界也锁死：

- Widget 可以命令 MVVM，这属于正常的 `B -> A`
- Widget 不能直接命令 `UAOCraftingComponent`
- Widget 不能直接操作库存组件
- Widget 不应该自己判断什么时候该刷新底层

UI 要什么数据，就去找 MVVM。MVVM 再去找底层。这才是这里要的 MVVM 口径。

### 2.4 ViewModel 直接观察底层，不要镜像层

这轮方案里最重要的一条，就是不再接受“底层先复制一份快照给 ViewModel 看”的写法。

当前口径是：

- `UMVVM_Crafting` 直接持有被观察的 `UAOCraftingComponent`
- `UMVVM_Crafting` 直接订阅 `UAOCraftingComponent::OnCraftingObservationChanged`
- HUD 只负责把当前 Pawn 上的制造组件接给 MVVM
- 不再在 HUD 或别的中间层再造一套制造数据世界

这样做的意义不是“代码更短”，而是避免数据更新断链。镜像层越多，真相就越容易不同步。

### 2.5 制造列表右键复用现有右键菜单架构，但不再硬绑 `SourceInventory`

这次新增“制作一个 / 制作十个 / 制作全部”时，不新造第二套完全独立的弹出菜单系统，也不把制造动作直接塞回某个 Widget 事件图里。

当前锁定口径是：

- 制造列表项右键，继续复用项目里已经收束好的右键菜单架构
- 这次复用的是“主菜单 ViewModel + 动作项 ViewModel + 菜单 Widget 只消费快照 + 动作统一回流正式入口”这套分层思路
- 但制造右键不再继续依赖 `SourceInventory->GetOrCreateContextMenuViewModel()` 这条库存来源链
- 制造菜单主 ViewModel 改为由当前制造来源自己的 `UAOCraftingComponent` 持有
- 菜单 Widget 和动作项 Widget 继续复用现有 `UMVVM_InventoryItemContextMenu`、`UMVVM_InventoryItemContextAction`、`UAOInventoryItemContextMenuWidget`、`UAOInventoryItemContextActionWidget`
- 这次只是把现有右键菜单架构适配到制造域，不是重做整套菜单框架

也就是说，这次复用的是“右键菜单架构”，不是继续强行复用“普通库存来源链”。

### 2.6 制造右键菜单的打开入口现在怎么定义

这里单独把“打开菜单的主入口”说清楚，避免后续实现时再次摇摆。

当前锁定口径是：

- 制造右键菜单仍然允许通过现有 `AOInventoryUI` 发起打开请求
- 但 `AOInventoryUI` 在这里承担的是“统一 UI 打开入口”的职责，不再承担“库存来源恢复”的职责
- 也就是说，这次保留 `AOInventoryUI` 作为菜单弹出入口，没有问题；真正需要替换的是它背后绑定的来源语义
- 打开制造菜单时，传递的真实来源不再是 `SourceInventory + SlotIndex + ItemInstance`
- 而是 `SourceCraftingComponent + RecipeRowName + ScreenSpacePosition + 当前用于头部显示的快照`
- 菜单真正的上下文持有者是 `UAOCraftingComponent`，不是某个库存组件
- 后续无论是玩家自身制造、工作台制造，还是熔炉制造，只要它们各自有独立的 `UAOCraftingComponent`，这条菜单入口语义都能直接复用

所以这里的收口不是“把 `AOInventoryUI` 彻底拿掉”，而是：

- 保留它作为统一打开入口
- 去掉它对 `SourceInventory` 的硬前提
- 把菜单上下文真正挂回到 `UAOCraftingComponent`

### 2.7 批量制造请求不是 UI 层循环点十次单次入队

这次右键菜单虽然会出现：

- `制作一个`
- `制作十个`
- `制作全部`

但这三条动作都不应在 UI 层展开成“循环调用十次 `RequestEnqueueRecipe()`”。

当前锁定口径是：

- 右键菜单动作只表达“目标批量意图”
- 最大能做多少、这次实际能做多少、是否连一个都做不了，都由 `UAOCraftingComponent` 负责判定
- `制作一个` 的目标数是 `1`
- `制作十个` 的目标数是 `10`
- `制作全部` 的目标数是“当前在制造真相下的最大可制造数”
- 如果目标数大于当前真实可制造数，但仍然能做至少 `1` 个，就按真实上限尽量做
- 如果当前连 `1` 个都做不了，就直接按“无法制作 / 材料不足”处理

也就是说，这三条动作的区别是“目标上限不同”，不是“是否允许尽量做”的规则不同。

### 2.8 `MaxQueueSize = 5` 以后按“最多五个批量队列项”理解

这次还要把队列语义明确改掉。

旧理解是：

- 一个队列项只代表一次单配方制作

这次锁定的新理解是：

- 一个队列项代表同一配方的一批制作
- `MaxQueueSize = 5` 表示最多同时排五个“批量制造队列项”
- `制作十个` 如果成功入队，应只占一个队列格子，而不是十个
- `制作全部` 也是同理，只要这一批最终决议成一个批量队列项，就仍然只占一个队列格子

这样做的目的，是把“队列里排了几批事”与“这一批内部要做多少次”分开，避免 UI 和底层继续把“十次制作”误当成“十个队列项”。

### 2.9 队列窗口按“固定槽位 + 有批次才填充”理解

这轮队列显示不再临时按当前队列长度现生成几条，也不单独给 UI 再配一套数量。

当前锁定口径是：

- 队列窗口预先摆出固定数量的队列槽位
- 这个固定数量直接跟底层 `MaxQueueSize` 对齐
- 空槽位只显示占位底板，不显示物品信息和进度
- 只有真正有制造批次进入时，才把对应槽位填充成非空队列项
- 队列显示顺序继续按当前底层队列数组顺序，不额外做一套独立排序规则

也就是说，这里展示的是“最多可以同时排几批制造”的固定窗口，不是“当前有几批就生成几个可变数量控件”的口径。

### 2.10 队列项显示内容按“Definition + 剩余/总数 + 状态 + 进度条”理解

这轮队列 UI 不再为队列项镜像一套额外物品显示字段，继续沿用当前制造系统已经在用的 `Definition` 语义。

当前锁定口径是：

- 每个非空队列项继续提供 `PrimaryOutputDefinition`
- UI 要什么图标、名字或别的物品显示信息，就继续从这个 `Definition` 上取
- 数量显示至少包括：
  - `RemainingCraftCount`
  - `TotalCraftCount`
- 不额外显示 `CompletedCraftCount`
- 状态字样至少区分：
  - `制作中`
  - `等待中`
- 队列项里保留正式的进度条控件，最终就是把实时百分比送给 `UProgressBar::SetPercent(...)`

这里的重点是：队列显示层继续消费制造真相里已经存在的数据，不再在 Widget 层重新定义一套“队列物品显示模型”。

### 2.11 实时进度条按“结构刷新走 MVVM，百分比刷新走同一服务端时间基准上的本地显示”理解

这轮最容易走偏的不是队列数据本身，而是“实时进度条到底怎么动”。

当前锁定口径是：

- 队列增删、状态切换、剩余数量变化，继续走现有 `UMVVM_Crafting::RefreshObservationData()` 和 `GetQueueList()` 快照刷新
- 当前 `Active` 队列项的进度条，不要求底层每帧广播
- 进度条可以在客户端本地持续刷新显示，但它的百分比计算不能把“服务端写入的开始/结束时间”与“客户端本地 `World->GetTimeSeconds()`”混着用
- 计算输入继续复用当前制造真相已经提供的：
  - `StartServerWorldTimeSeconds`
  - `ExpectedFinishServerWorldTimeSeconds`
  - `ResolvedDurationSeconds`
- `StartServerWorldTimeSeconds` 和 `ExpectedFinishServerWorldTimeSeconds` 的语义继续按服务端权威时间戳理解
- 当前推荐继续复用 `UMVVM_Crafting::GetQueueEntryProgressRatio(...)` 这条统一口径，但这条口径内部必须建立在客户端可对齐的服务端时间基准上，而不是直接拿本地世界时间做减法
- 如果权威队列快照和权威时间戳还没复制到当前客户端，进度条可以先保持 `0` 或停留在等待显示，不要提前按本地时钟驱动正式进度
- `Queued` 但还没轮到的项，进度条保留控件，但百分比保持 `0`
- 只有当前 `Active` 项需要持续实时刷新

也就是说，这轮不是让服务端每帧推 UI 百分比，而是要把“服务端权威制造时间戳”和“客户端显示时使用的时间基准”彻底对齐，再把实时感收在显示层里解决。

### 2.12 联机职责边界按“服务端决定制造事实，客户端负责发起请求与显示结果”理解

这次 Dedicated Server 下暴露出来的问题，本质上不是 `ProgressBar` 控件问题，而是制造联机职责边界必须写死。

当前锁定口径是：

- 客户端负责发起制造请求、打开右键菜单、显示反馈和展示观察到的制造结果
- 客户端不负责最终裁决这次是否成功、实际开始时间是多少、什么时候完成、还剩多少次
- 服务端负责真正的制造事实：
  - 是否接受这次请求
  - 实际可制造数量
  - 队列项何时进入 `Active`
  - `StartServerWorldTimeSeconds`
  - `ExpectedFinishServerWorldTimeSeconds`
  - `RemainingCraftCount`
  - 每完成 1 个时的产物发放与后续批次推进
- 客户端可以提前显示“已开始制作”这类轻量反馈，但不能提前认定队列时间事实
- 队列正式显示和进度条正式推进，都应以当前客户端已经观察到的权威队列快照为准

换句话说，这轮必须把职责边界收成一句话：

**状态归服务端，显示归客户端，但客户端显示正式进度时必须建立在服务端真相和同一服务端时间基准上。**

## 第三章 当前代码已经落地到什么程度

### 3.1 制造配方已经收成单表模型

当前已落地的关键代码点：

- [AOPawnData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.h)
- [AOPawnData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp)
- [AOCraftingRecipeTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h)

现在 `FAOCraftingRecipeRow` 已经承载：

- `UnlockLevel`
- `bVisibleBeforeUnlock`
- `SortOrder`
- `DisplayName`
- `BaseCraftDurationSeconds`
- `MaterialEntries`
- `OutputEntries`

旧的 `AOCraftingRecipeSourceTypes.h` 已经移除，不应该再作为当前方案的一部分继续出现在分析链里。

### 3.2 底层真相已经收回到 `UAOCraftingComponent`

当前底层主入口是：

- [AOCraftingComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h)
- [AOCraftingComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp)

这层现在负责：

- 从角色的单张制造表读取配方
- 根据 `ItemId` 去 `AOGameData` 的物品总表解析 `ItemDefinition`
- 组装配方列表、详情、队列三类 ViewData
- 观察角色可参与制造的库存组件
- 在队列变化和库存变化时统一广播 `OnCraftingObservationChanged`
- 处理制造请求、扣料、入队、出队和完成

所以现在的真相链应该理解成：

`UAOPawnData::CraftingRecipeDataTable -> UAOCraftingComponent -> UMVVM_Crafting -> Widget`

### 3.3 `AOGameData` 现在只保留物品总表职责

当前相关代码：

- [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
- [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)

现在这里的职责很单纯：

- 提供 `ItemCatalogDataTable`
- 支持 `ItemId -> ItemDefinition`
- 不再提供制造配方表

这也是这轮排查里一个关键修正点。之前 `OutRecipeRow` 找不到，根因之一就是制造表口径被放错地方了。

### 3.4 MVVM 已经改成直接观察底层

当前主文件：

- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp)

这里已经落地的点有：

- `SetObservedCraftingComponent()` 直接绑定底层制造组件
- `RefreshObservationData()` 每次都重新从当前底层真相拉完整数据
- `SetSelectedRecipeRowName()` 会刷新详情区
- `RequestEnqueueRecipe()` 无论成功失败，都会请求后回拉一次真相
- `HandleObservedCraftingObservationChanged()` 直接响应底层广播

所以 `RefreshObservationData()` 现在不是“只在开始和结束走一次”的函数，它是 ViewModel 的统一重拉入口。只要观察源变了，或者底层广播真相变化了，它都应该会被触发。

### 3.5 当前队列显示所需的数据其实已经大半落地

当前队列相关真相已经不只是“有没有排队”，而是已经带出了显示层真正要用到的核心字段。

当前关键位置是：

- [AOCraftingObservationTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h)
- [AOCraftingComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp)
- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp)

当前 `FAOCraftingQueueEntryViewData` 已经提供：

- `QueueEntryId`
- `RecipeRowName`
- `PrimaryOutputDefinition`
- `State`
- `ResolvedDurationSeconds`
- `StartServerWorldTimeSeconds`
- `ExpectedFinishServerWorldTimeSeconds`
- `TotalCraftCount`
- `RemainingCraftCount`
- `CompletedCraftCount`

同时 `UMVVM_Crafting` 已经提供：

- `GetQueueList()`
- `GetQueueEntryRemainingSeconds(...)`
- `GetQueueEntryProgressRatio(...)`
- `GetActiveQueueRemainingSeconds()`
- `GetActiveQueueProgressRatio()`

所以这轮真正缺的不是再去补底层队列真相，而是把这些真相接到正式的队列显示 Widget 上。

### 3.6 UI 刷新逻辑已经尽量收回 C++

当前 Widget 相关入口：

- [AOCraftingWidgetBase.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h)
- [AOCraftingWidgetBase.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp)
- [AOCraftingRecipeListWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.h)
- [AOCraftingRecipeListWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp)
- [AOCraftingRecipeDetailWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.h)
- [AOCraftingRecipeDetailWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.cpp)

当前已经明确的是：

- 蓝图不再负责制造刷新时机
- 蓝图不再负责把图标、名字、数量这些显示逻辑补在事件图里
- `UAOCraftingRecipeListEntryWidget` 承担了配方项、材料项、产物项三种显示模式
- 详情区的材料和产物也复用 `UAOCraftingRecipeListEntryWidget`
- 队列显示最终也复用 `UAOCraftingRecipeListEntryWidget`，不再额外新增独立的队列项 Widget 类
- 队列区挂载在 `UAOCraftingRecipeListWidget` 内部，由它自己的 `QueueListContainer` 承接固定槽位显示
- 固定槽位数不由蓝图手工预摆，而是运行时按 `UAOCraftingComponent::MaxQueueSize` 循环创建

这里要特别记一笔：`UAOCraftingRecipeListEntryWidget` 不是独立文件，它定义在 `AOCraftingRecipeListWidget.h/.cpp` 里。后续接手时不要再按旧习惯去找单独的 `AOCraftingRecipeListEntryWidget.*` 文件。

### 3.7 这次新增需求会落在哪几层

这次“制造列表项右键后弹出制作一个 / 十个 / 全部”会明确落在下面几层：

第一层是右键动作决议层：

- 继续由 `AOInventoryUI` 那条现有右键菜单主链承接
- 但动作项不再只局限于普通 `Use / Close`
- 制造列表项右键时，要能生成制造专用动作集合

第二层是制造 ViewModel / 请求层：

- `UMVVM_Crafting` 不再只承接单次 `RequestEnqueueRecipe(FName)`
- 要能承接“按目标批量请求制造”的正式入口
- 左键选中和详情刷新链继续保持原状
- 右键菜单动作只是另一种发起制造请求的入口，不是另一套制造状态机

第三层是制造底层队列层：

- `UAOCraftingComponent` 不再只支持“单次入队一条配方”
- 队列项本身要能承载批量次数
- 入队即按“本批实际数量”一次性扣料
- 当前批次做完后，再切到下一个批量队列项

所以这次不是只补一个菜单动作，而是要把“右键入口语义”和“队列项语义”一起对齐。

## 第四章 现在的数据是怎么往上走的

### 4.1 配方数据怎么变成 UI 可用数据

当前主链是这样的：

1. `UAOCraftingComponent` 从 `UAOPawnData::CraftingRecipeDataTable` 拿到配方行
2. 配方行里的材料和产物通过 `ItemId` 表达
3. `UAOCraftingComponent` 通过 `AOGameData::ItemCatalogDataTable` 把 `ItemId` 解成 `ItemDefinition`
4. `UAOCraftingComponent` 组装 `FAOCraftingRecipeListEntryViewData`、`FAOCraftingRecipeDetailViewData`、`FAOCraftingQueueEntryViewData`
5. `UMVVM_Crafting` 拉取这些 ViewData 并缓存
6. Widget 每次只从 MVVM 读当前数据，不再自己补查底层

这里要把“下方材料需求列表”这件事单独说清楚。

当前口径不是给材料区再单独造一份真相，而是继续让它消费当前选中配方的详情快照。

- 左键点击制造列表项，本质上只是在 `UMVVM_Crafting` 里切换 `SelectedRecipeRowName`
- `RefreshObservationData()` 会基于这个选中行名重新拉取 `FAOCraftingRecipeDetailViewData`
- 下方材料需求列表始终消费 `SelectedRecipeDetail.MaterialEntries`
- 每个材料条目都应同时带出 `ItemId`、`ItemDefinition`、`RequiredCount`、`OwnedCount`、`bSatisfied`

也就是说，材料区不是独立系统，它属于“当前选中配方详情真相”的一部分。

### 4.2 UI 现在主要依赖 `Definition` 取显示信息

当前观察数据结构里，材料、产物和主产物都已经直接带 `UAOInventoryItemDefinition*`。

这意味着：

- 图标应该从 `Definition` 上的 `AOFragment_InventoryIcon` 取
- 名字应该从 `Definition` 上的显示字段取
- 后续 UI 还要更多物品信息，也优先继续从 `Definition` 取

这一层的设计意图已经对了。它避免了“今天为了图标加一个字段，明天为了稀有度再加一个字段”的不断扩展。

不过这层还没有完全收干净。现在结构体里仍然保留了 `PrimaryOutputDefinition` 这样的命名，语义上偏“主产物”，而不是完全抽象成“直接传定义对象后，UI 自己决定读什么”。这不影响当前链路工作，但属于后续还可以继续收口的点。

### 4.3 当前选中配方和材料需求列表怎么约定

这次补充一个明确口径：

- 打开制造面板后，如果当前没有人工点击任何配方，继续沿用 `UMVVM_Crafting` 现有的默认首项自动选中逻辑
- 默认首项一旦建立，下方材料需求列表就应该自动显示这条首项配方的 `MaterialEntries`
- 用户后续左键点击其他制造栏位时，不新增第二套“材料区选中状态”，而是继续只切换 `SelectedRecipeRowName`
- 如果用户再次左键点击当前已经选中的同一条配方，也按一次显式刷新处理，而不是静默忽略
- 材料需求列表不单独缓存、不单独直连底层、不单独维护网络状态

这样做的目的，是避免“列表选中了一条、详情区还挂着另一条、材料区又是第三条”的三套状态并存。

另外再补一条显示边界：

- 对“未解锁但允许可见”的配方，只要它能出现在制造列表里，左键选中后就应照常显示完整材料需求和当前已有数量

### 4.4 当前材料需求列表的数据来源怎么定义

下方材料需求列表的数据来源固定为：

`UAOPawnData::CraftingRecipeDataTable -> UAOCraftingComponent::BuildRecipeDetailViewData() -> UMVVM_Crafting::SelectedRecipeDetail.MaterialEntries`

其中每个材料条目的数据来源继续按下面这条链理解：

1. 配方行从角色当前 `CraftingRecipeDataTable` 中读取
2. 配方材料仍然只通过 `ItemId` 表达
3. `UAOCraftingComponent` 通过 `AOGameData::ItemCatalogDataTable` 把 `ItemId` 解析成 `ItemDefinition`
4. `RequiredCount` 来自当前选中配方的材料需求
5. `OwnedCount` 来自当前角色允许参与制造的全部库存范围聚合统计
6. `bSatisfied` 由 `OwnedCount` 是否满足 `RequiredCount` 得出

所以材料需求列表显示的不是“资源层静态表数据”，而是“当前选中配方 + 当前库存真相”共同生成的观察快照。

### 4.5 右键批量制造动作的数据应该怎么理解

这次右键菜单里的三条制造动作，不是额外维护一份独立菜单真相，而是继续建立在当前制造真相之上。

当前应按下面这条链理解：

1. 用户右键的是“当前制造列表中的某一条配方项”
2. 右键菜单动作只携带“这条配方是谁 + 目标数量语义是什么”
3. 真正的可制造数量计算仍回到 `UAOCraftingComponent`
4. `UAOCraftingComponent` 根据当前材料池、当前配方、当前队列容量语义，算出这次实际可入队数量
5. 如果实际可入队数量大于 `0`，就生成一个批量队列项
6. 如果实际可入队数量等于 `0`，就按失败处理，并让上层回拉当前真相

也就是说，右键菜单动作本身不是制造真相，只是制造真相的一个发起入口。

这里再锁死两条材料行口径：

- 同一条配方里如果 `MaterialEntries` 出现多个相同 `ItemId`，材料需求列表按配方表原样逐行显示，不在制造系统层提前合并
- 如果某个材料 `ItemId` 能进配方，但当前没有成功解析到 `ItemDefinition`，制造系统仍按“一个 `ItemId` 对应一个材料格子”继续传递该材料行；由于没有 `Definition`，UI 侧自然拿不到图标或其他定义信息，这里不额外再发明第二套兜底显示逻辑

## 第五章 刷新时机现在应该怎么理解

这部分是最容易被误解的，所以单独写清楚。

制造数据刷新不是某个面板按钮的局部逻辑，而是“底层真相一变，MVVM 就应该重新拉数据”。

当前应该按下面几类入口去理解。

### 5.1 观察的制造组件换了，要立刻重拉

当前入口：

- [AOHUDViewModelComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/AOHUDViewModelComponent.cpp) 的 `BindCraftingObservationSource()`
- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp) 的 `SetObservedCraftingComponent()`

这类场景包括：

- Pawn 重建
- 重新 Possess
- HUD 重新绑定到新的 `UAOCraftingComponent`

这时应该立即调用 `RefreshObservationData()`，因为观察源本身已经变了。

### 5.2 制造队列变了，要广播后重拉

当前底层入口在 `UAOCraftingComponent`：

- 入队成功
- 队列复制更新
- 活动项完成
- 队列清空

这些地方最终都应该回到 `NotifyCraftingObservationChanged()`，再由 MVVM 统一 `RefreshObservationData()`。

### 5.3 库存变了，也要广播后重拉

当前底层入口：

- `BindObservedInventorySources()`
- `HandleObservedInventoryChanged()`

这类变化直接影响：

- 是否缺料
- 材料拥有数
- 配方可制造状态
- 详情区材料满足状态

所以库存变化本身就是制造真相变化，不能再让 HUD 或 Layout 补一层“顺手刷新”。

### 5.4 用户切换配方，也要刷新详情

当前入口：

- `UMVVM_Crafting::SetSelectedRecipeRowName()`

这里刷新的是“当前选中配方对应的详情真相”，不是去改底层规则。

对这次需求来说，要再补一句明确口径：

- 左键点击制造栏位，只切换 `SelectedRecipeRowName`
- `UMVVM_Crafting` 随后统一执行 `RefreshObservationData()`
- `SelectedRecipeDetail` 跟着切到新配方
- 下方材料需求列表随 `SelectedRecipeDetail.MaterialEntries` 一起切换
- 如果左键点中的是当前已经选中的同一条配方，也仍然按一次显式刷新处理

也就是说，材料需求列表的刷新不是单独补的一条 UI 支线，而是“选中配方详情刷新”这条既有主链的一部分。

### 5.5 用户请求制造后，要区分“本地反馈刷新”和“权威队列真相回拉”

当前入口：

- `UMVVM_Crafting::RequestEnqueueRecipe()`

因为请求失败也会改变 UI 认知。比如用户以为可制造，结果底层校验失败，UI 这时也应该重新拿一次当前真实状态。

但这次联机问题说明，还要再补一层口径：

- 客户端发出制造请求后，可以立即刷新本地反馈文案
- 但这次立即刷新，不应被当成“权威队列时间已经到位”的信号
- 队列里到底有没有正式进入一批制造、当前批次的开始时间和结束时间是什么，仍然要等服务端权威队列快照复制回来
- 如果当前客户端此时还没拿到权威开始/结束时间，进度条可以先停在 `0`，不要先按本地世界时间推进

对这次右键批量制造再补一句明确口径：

- 无论是 `制作一个`、`制作十个` 还是 `制作全部`
- 无论最终实际入队了 `1` 个、`N` 个，还是 `0` 个失败
- 请求返回后都要统一回拉一次制造真相

这样列表可制造状态、详情材料拥有数、以及队列区显示的“当前批次数量”才会一起回到同一个最新口径。

但 Dedicated Server 下还要再加一句：

- 真正驱动进度条正式开始的，不是这次本地立刻回拉本身，而是后续到达客户端的权威队列观察快照

### 5.6 右键菜单动作不是第二套制造入口链

这次虽然新增右键菜单入口，但它不是第二套制造系统。

当前锁定口径是：

- 左键选中、详情刷新、材料刷新，继续走现有制造 MVVM 主链
- 右键菜单只负责表达动作意图，并把请求回流到正式制造入口
- 右键菜单不自己计算最大可制造数
- 右键菜单不自己判断材料是否足够
- 右键菜单不自己维护批量队列状态

也就是说，右键菜单只是“另一种发起方式”，不是“另一份制造真相”。

### 5.7 Layout 不再是制造刷新入口

这轮方案明确去掉了下面这些旧思路：

- 打开 Layout 就主动替制造面板刷新
- Layout 监听交互会话变化后替制造补刷
- HUD 额外监听库存和队列，再转发制造刷新

原因不是“这些入口永远不会触发刷新”，而是它们不应该成为制造系统自己的主刷新源。

### 5.8 左键选中不是网络同步状态，材料真相才是

这次还要专门锁死一个多人边界：

- `SelectedRecipeRowName` 是本地 UI 观察状态，不是需要复制给其他客户端的游戏状态
- 左键选中了哪条配方，不需要 RPC，也不需要专门做服务端权威同步
- 真正需要可信的是 `OwnedCount`、`bSatisfied`、`bCanEnqueue` 这些由当前制造真相和库存真相推导出来的结果
- 这些结果继续由当前客户端观察到的 `UAOCraftingComponent` 与库存同步状态重新构建

换句话说，网络层不负责“同步你现在 UI 正在看哪条配方”，网络层负责的是“让你当前看到的配方材料状态建立在正确的底层真相上”。

## 第六章 冷启动调试现在该怎么查

### 6.1 先确认自己在查单表架构，不是在查旧方案

如果现在又看到：

`CraftRecipe Rope_Craft`

然后日志是：

`CraftRecipe request for 'Rope_Craft' on BP_Anny_C_0 returned false`

第一反应不应该是去找“旧 source 表”和“旧全局 recipe 表”。

现在正确的排查顺序是：

1. 当前角色实际吃到的 `PawnData` 是谁
2. 这个 `PawnData` 上的 `CraftingRecipeDataTable` 是否配置正确
3. 目标 `RecipeRowName` 是否真的是这张表里的行名
4. 这条配方里的材料和产物 `ItemId` 是否能通过 `AOGameData::ItemCatalogDataTable` 解析到 `ItemDefinition`
5. 当前角色可参与制造的库存集合里是否真的有足够材料

也就是说，`returned false` 的含义是“正式入队前的校验没过”，不是“控制台命令没执行”。

### 6.2 看图标、名字、详情不对时的排查链

如果你现在看到的问题是：

- 列表图标没出来
- 名字不对
- 详情区材料或产物没刷新
- 明明切了配方但控件没变

优先按这条链查：

1. `UAOCraftingComponent::BuildRecipeListViewData()`
2. `UAOCraftingComponent::BuildRecipeDetailViewData()`
3. `UMVVM_Crafting::RefreshObservationData()`
4. `UAOCraftingRecipeListWidget::HandleCraftingViewModelChanged()`
5. `UAOCraftingRecipeDetailWidget::HandleCraftingViewModelChanged()`
6. `UAOCraftingRecipeListEntryWidget::SetEntryData() / SetMaterialData() / SetOutputData()`
7. `UAOCraftingRecipeListEntryWidget::RefreshRecipeDisplay() / RefreshMaterialDisplay() / RefreshOutputDisplay()`

如果 `RefreshOutputDisplay()` 根本没进，优先先查“这个输出项 Widget 有没有被创建出来”，而不是直接怀疑图标资源。

### 6.3 看乱码时不要只盯运行时

这轮代码和文档里已经出现过一次明显的乱码问题。它有两种可能来源：

- 文件编码不一致
- `FText::FromString(TEXT("..."))` 里的中文文本本身已经损坏

所以后续只要再看到 UI 中文乱码，不要只查 Widget 绑定，还要同时检查：

- 对应 `.h/.cpp/.md` 文件编码
- 当前字面量字符串在源码里是不是已经坏了

### 6.4 查右键批量制造为什么和预期不一致

如果你现在看到的问题是：

- 右键没有出现 `制作一个 / 制作十个 / 制作全部`
- 点了 `制作十个` 却只入队了一次单次制作
- 点了 `制作全部` 结果占了多个队列格子
- 菜单显示能做十个，但最后一个都没入队

优先按这条顺序查：

1. 这次右键命中的是否真是制造列表项，而不是普通库存槽位
2. 右键菜单动作决议链是否真的生成了制造专用动作，而不是仍然只走 `Use / Close`
3. 动作回流后是否真的进入了正式制造请求入口，而不是 UI 层循环调用旧的单次入队
4. `UAOCraftingComponent` 是否按当前材料真相算出了“本次实际可制作数”
5. 最终是否生成的是“一个批量队列项”，而不是多个单次队列项

如果最后表现成“材料明明够做 6 个，但点制作十个直接失败”，优先怀疑的不是菜单文字，而是底层仍按“必须完全满足目标数才允许入队”的旧逻辑在跑。

### 6.5 程序员阅读导航

这部分只给后续接手的人提供排查顺序，不改设计结论。

如果你要确认“制造右键菜单为什么会弹出、为什么不再依赖 `AOInventoryUI` 外层宿主”，先按这个顺序看：

1. `AOCraftingRecipeListWidget` 的右键事件入口，确认它是在制造列表项本体上发起菜单，而不是继续转交给普通库存槽位。
2. `UMVVM_Crafting` 的菜单 ViewModel 持有关系，确认菜单上下文来源是当前 `UAOCraftingComponent`，不是 `SourceInventory`。
3. `AOInventoryUI` 的打开请求链，确认它只保留“统一弹窗入口”职责，不再承担来源恢复职责。
4. `UMVVM_Crafting::RequestCraftRecipe()`，确认三条菜单动作最终都回流到正式制造请求入口。
5. `UAOCraftingComponent` 的入队与队列构建逻辑，确认批量语义是“一个批量队列项承载一批制作”，不是多个单次队列项。

如果你要快速判断“问题是在菜单弹出层、请求层，还是队列层”，优先用下面的切面切开：

- 菜单没弹出，先看右键命中对象是不是制造列表项，再看菜单动作决议链是否真的生成了制造专用动作集合。
- 菜单弹出了但只有 `Use / Close`，先看菜单 ViewModel 持有者是不是仍在走库存来源链。
- 菜单动作点了没反应，先看动作是否真的回流到了 `UMVVM_Crafting::RequestCraftRecipe()`。
- 菜单动作能回流，但队列表现不对，先看 `UAOCraftingComponent` 是否按批量次数构建了一个队列项，而不是拆成多个单次项。

### 6.6 右键批量制造排查入口

这部分专门对应本轮新增的三条动作和批量队列语义。出现问题时，优先按“入口 -> 动作 -> 回流 -> 队列”顺序排查，不要先怀疑文案。

优先检查这几个点：

1. 右键命中的是不是制造列表项本体，而不是普通库存槽位。
2. 右键菜单动作决议链是否真的生成了 `制作一个 / 制作十个 / 制作全部`。
3. 菜单动作回流后，是否真的进入了 `UMVVM_Crafting::RequestCraftRecipe()`，而不是停留在 UI 层循环调用。
4. `UAOCraftingComponent` 是否按当前材料真相算出了本次实际可入队批次数。
5. 最终生成的是不是一个批量队列项，而不是多个单次队列项。

如果结果表现成“菜单看得到，但队列里排法不对”，优先怀疑的是队列项语义没有对齐，而不是菜单入口本身失效。

### 6.7 查队列显示和实时进度条为什么不对

如果你现在看到的问题是：

- 队列窗口没有预先显示固定数量槽位
- 有批次进队后，槽位没有填充物品信息
- `Active` 项进度条不动
- 当前批次完成 1 个后，剩余数量没及时减少
- `Queued` 项和 `Active` 项显示混乱

优先按这条顺序查：

1. 队列窗口是不是按 `MaxQueueSize` 预生成了固定数量槽位，而不是按当前队列长度临时生成。
2. 非空槽位是不是直接消费 `FAOCraftingQueueEntryViewData`，而不是在 Widget 层再造一套镜像字段。
3. 队列项物品显示是不是继续从 `PrimaryOutputDefinition` 取图标、名字和其它物品信息。
4. 当前 `Active` 项的进度条百分比，是不是走 `UMVVM_Crafting::GetQueueEntryProgressRatio(...)` 这一条统一口径。
5. 当前实现是不是把“结构刷新”和“实时百分比刷新”分开了：
   - 队列增删、状态切换、剩余数量变化走 `RefreshObservationData()`
   - `Active` 项进度条本地持续刷新
6. `GetQueueEntryProgressRatio(...)` 内部使用的“当前时间”是不是和 `StartServerWorldTimeSeconds / ExpectedFinishServerWorldTimeSeconds` 属于同一服务端时间基准，而不是直接使用客户端本地 `World->GetTimeSeconds()`。
7. 客户端请求制造后，那次立即 `RefreshObservationData()` 有没有被误当成权威队列时间已到，导致 UI 提前开始用错误时基推进进度。
8. 当前批次完成 1 个后，UI 是不是重新消费了最新 `QueueList`，从而让 `RemainingCraftCount` 立即减 1，并让下一轮单件进度重新开始。

如果结果表现成下面这种 Dedicated Server 远端客户端现象：

- 队列项已经看得到
- 进度条要等一小会才开始动
- 动到一半左右，这一件却已经做完了

优先怀疑的不是 `ProgressBar` 控件本身，也不是单纯的“同步慢”，而是：

- 服务端权威时间戳已经写进队列
- 客户端却在用本地世界时间参与进度计算
- 再叠加一次“请求后立刻本地刷新”带来的早刷新观感

## 第七章 当前推荐的冷启动手测步骤

这部分写给没有参与这轮改造的人，按顺序走就行。

### 7.1 先准备资源真相

开始前先确认：

1. 当前角色 `PawnData` 上配置了 `CraftingRecipeDataTable`
2. 目标配方行真实存在于这张表
3. 配方里的材料和产物 `ItemId` 都能在物品总表里找到
4. 对应 `ItemDefinition` 上已经有可用的名字和图标 Fragment
5. 当前 Pawn 身上确实挂了 `UAOCraftingComponent`

### 7.2 冷启动打开背包，先看首屏列表

动作：

1. 进入游戏
2. 确认角色初始化完成
3. 打开背包
4. 直接看制造列表，不要先点别的

预期：

- 制造列表能直接出现
- 列表项能显示主产物名字
- 有图标资源的项会直接显示图标
- 如果列表非空，下方材料需求列表会默认显示首条配方的材料需求
- 如果有可见但未解锁配方，状态应正确

如果不对，先查：

- `UAOHUDViewModelComponent::BindCraftingObservationSource()`
- `UMVVM_Crafting::SetObservedCraftingComponent()`
- `UMVVM_Crafting::RefreshObservationData()`

### 7.3 切换配方，看详情区是否完整刷新

动作：

1. 选择一条配方
2. 再切到另一条配方
3. 来回切几次

预期：

- 标题切到对应主产物
- 材料区刷新拥有数和需求数
- 产物区刷新名字、图标和数量
- 按钮可用状态跟着当前配方变化
- 材料区始终跟随当前选中配方，不会停留在上一条配方的数据上
- 即使重复点击当前已选中的同一条配方，也会重新拉一次当前详情真相

如果不对，先查：

- `UMVVM_Crafting::SetSelectedRecipeRowName()`
- `UAOCraftingComponent::BuildRecipeDetailViewData()`
- `UAOCraftingRecipeDetailWidget::RebuildMaterialEntryWidgets()`
- `UAOCraftingRecipeDetailWidget::RebuildOutputEntryWidgets()`

### 7.4 改背包材料，验证缺料状态是否活的

动作：

1. 选中一条当前缺料的配方
2. 记下材料拥有数和缺料状态
3. 增加材料
4. 再减少材料

预期：

- 不需要重新开关面板
- 缺料状态会跟着变
- 列表和详情区对同一条配方的判断一致

如果不对，先查：

- `UAOCraftingComponent::BindObservedInventorySources()`
- `UAOCraftingComponent::HandleObservedInventoryChanged()`
- `UAOCraftingComponent::NotifyCraftingObservationChanged()`
- `UMVVM_Crafting::HandleObservedCraftingObservationChanged()`

### 7.5 点制造，验证成功和失败都能回到最新真相

动作：

1. 选择一条可制造配方
2. 点击制造
3. 观察材料数、按钮状态、队列状态
4. 再对一条故意缺料的配方点击制造

预期：

- 成功时会扣料并入队
- 失败时不会入队，但 UI 仍会回拉当前真实状态
- 队列和详情区不会各说各话

如果不对，先查：

- `UMVVM_Crafting::RequestEnqueueRecipe()`
- `UAOCraftingComponent::RequestEnqueueRecipe()`
- `UAOCraftingComponent::TryEnqueueRecipeOnAuthority()`
- `UAOCraftingComponent::BuildQueueViewData()`

### 7.6 右键弹出菜单，验证入口和宿主解耦

动作：

1. 在制造列表里右键一条配方项。
2. 观察是否弹出制造专用菜单，而不是库存槽位菜单。
3. 关闭菜单后，再从 `AOInventoryUI` 以同一入口重新打开一次。
4. 确认菜单仍能正常弹出，且没有把来源语义重新绑回库存宿主。

预期：

- 右键命中的是制造列表项本体，菜单可以直接弹出。
- 菜单不是依赖 `AOInventoryUI` 里的 `SourceInventory` 来恢复来源。
- 重新打开时，菜单上下文仍然来自当前 `UAOCraftingComponent`。

如果不对，先查：

- 右键命中对象是不是制造列表项，而不是普通库存槽位。
- 菜单 ViewModel 是否仍在走库存来源链。
- `AOInventoryUI` 是否只承担统一弹出入口，而没有重新承担来源恢复职责。

### 7.7 右键制作一个，验证只回流一次正式制造入口

动作：

1. 在制造列表里右键一条配方。
2. 点击 `制作一个`。
3. 先在“完全缺料”的情况下测一次。
4. 再在“刚好够做一个”的情况下测一次。

预期：

- 完全缺料时，不会入队。
- 会明确表现为“无法制作 / 材料不足”。
- 刚好够做一个时，会生成一个批量队列项。
- 这个批量队列项的实际批次数应为 `1`。
- UI 层不应出现循环调用单次入队的痕迹。
- 最终回流点应是 `UMVVM_Crafting::RequestCraftRecipe()`。

如果不对，先查：

- 菜单动作是否真的回流到了 `UMVVM_Crafting::RequestCraftRecipe()`。
- `UAOCraftingComponent` 是否把这次请求识别为单次批量语义。
- 队列是否只新增了一个批量队列项。

### 7.8 右键制作十个，验证目标批次与实际入队批次的区别

动作：

1. 在制造列表里右键同一条配方。
2. 点击 `制作十个`。
3. 分别在“可做 0 个 / 3 个 / 10 个 / 大于 10 个”四种材料条件下验证。

预期：

- 可做 `0` 个时，直接失败，不入队。
- 可做 `3` 个时，应成功入队一个批量队列项，实际批次数为 `3`。
- 可做 `10` 个时，应成功入队一个批量队列项，实际批次数为 `10`。
- 可做大于 `10` 个时，应成功入队一个批量队列项，实际批次数仍为 `10`。
- 菜单动作只表达“目标十个”，不是 UI 层循环十次单次入队。
- 如果真实可做数小于十，最终语义应按底层真实上限收敛。

如果不对，先查：

- `UMVVM_Crafting::RequestCraftRecipe()`。
- `UAOCraftingComponent` 是否按批量次数计算实际可入队数量。
- 队列项是否仍然被拆成多个单次项。

### 7.9 右键制作全部，验证一个队列格子承载整批

动作：

1. 在制造列表里右键一条当前可做多次的配方。
2. 点击 `制作全部`。
3. 观察队列区显示。
4. 再连续给别的配方追加几批制造，直到接近 `MaxQueueSize`。

预期：

- `制作全部` 成功时，只新增一个批量队列项。
- 不会按“做多少次就占多少格”去挤满队列。
- 当已经排了 5 批不同的制造队列项后，再新增第 6 批才应被视为队列已满。
- 当前批次完成前，队列区显示的仍应是“这批剩余多少次 / 当前批正在跑哪一次”的同一队列项语义，而不是拆成一串单次条目。

如果不对，先查：

- 菜单动作是否只是表达批量意图。
- `UAOCraftingComponent` 是否生成了一个批量队列项。
- 队列 UI 是否还在按旧的单次队列项思维渲染。

### 7.10 队列窗口默认态，验证固定槽位不是按当前长度临时生成

动作：

1. 冷启动进入游戏并打开制造面板。
2. 在还没有任何制造批次入队时，直接观察队列窗口。
3. 记录当前可见的槽位数量。
4. 再入队 1 批制造，观察槽位总数是否保持不变。

预期：

- 队列窗口会预先显示固定数量槽位。
- 固定数量直接跟 `MaxQueueSize` 对齐，不需要 UI 额外配置。
- 空槽位只显示占位底板，不显示物品信息、数量和进度。
- 有批次进入后，只填充对应槽位内容，不改变总槽位数。

如果不对，先查：

- 队列显示层是不是按当前 `QueueList.Num()` 临时创建控件。
- 是否错误地给 UI 再配了一套独立槽位数量。
- `UAOCraftingRecipeListWidget` 下是否已经挂好了 `QueueListContainer`，并且它复用了 `UAOCraftingRecipeListEntryWidget` 作为固定队列槽位。

### 7.11 队列项填充态，验证物品信息和数量口径

动作：

1. 对同一配方发起 1 批制造。
2. 观察第一个非空槽位。
3. 再追加 1 到 2 批不同配方制造。
4. 对照每个非空槽位显示的物品信息和数量。

预期：

- 每个非空槽位的物品信息继续来自 `PrimaryOutputDefinition`。
- 槽位能显示当前主产物图标和名字。
- 数量显示至少包括：
  - `RemainingCraftCount`
  - `TotalCraftCount`
- 不额外要求显示 `CompletedCraftCount`。
- 状态字样至少能区分 `制作中` 和 `等待中`。

如果不对，先查：

- 队列项显示是不是继续沿用 `Definition` 语义。
- 数量是不是直接消费了 `RemainingCraftCount` 和 `TotalCraftCount`。
- 状态字样是不是由 `EAOCraftingQueueEntryViewState` 决定。

### 7.12 实时进度条，验证 Active 项在联机下也按完整时长推进

动作：

1. 入队至少 2 批制造，确保有 1 个 `Active` 项和至少 1 个 `Queued` 项。
2. 盯住当前 `Active` 槽位的进度条持续观察。
3. 同时观察后面的 `Queued` 槽位进度条。
4. 等当前 `Active` 批次完成 1 个后，继续观察下一轮单件制作。

预期：

- 只有当前 `Active` 项的进度条持续实时变化。
- `Queued` 项保留进度条控件，但百分比保持 `0`。
- `Active` 项进度条最终是把实时百分比送给 `UProgressBar::SetPercent(...)`。
- 在 Dedicated Server 远端客户端视角下，当前 `Active` 项不应出现“先停一下再开始走”的明显空档。
- 在 Dedicated Server 远端客户端视角下，当前 `Active` 项应基本按这一件的完整制造时长走完，不应出现“只走到一半左右，这一件就已经完成”的表现。
- 当前批次完成 1 个后：
  - `RemainingCraftCount` 立即减 1
  - 进度条重置
  - 下一轮单件制作进度立刻重新开始

如果不对，先查：

- 进度条百分比是不是走 `UMVVM_Crafting::GetQueueEntryProgressRatio(...)`。
- 当前实现是不是把“结构刷新”与“进度条实时刷新”分开了。
- `Active` 项是不是有本地持续刷新机制，而不是只等底层广播。

### 7.6 右键制作一个，验证“连一个都做不了就直接失败”

动作：

1. 在制造列表里右键一条配方
2. 点击 `制作一个`
3. 先在“完全缺料”的情况下测一次
4. 再在“刚好够做一个”的情况下测一次

预期：

- 完全缺料时，不会入队
- 会明确表现为“无法制作 / 材料不足”
- 刚好够做一个时，会生成一个批量队列项
- 这个批量队列项的实际批次数应为 `1`

### 7.7 右键制作十个，验证“目标十个但按真实上限尽量做”

动作：

1. 在制造列表里右键同一条配方
2. 点击 `制作十个`
3. 分别在“可做 0 个 / 3 个 / 10 个 / 大于 10 个”四种材料条件下验证

预期：

- 可做 `0` 个时，直接失败，不入队
- 可做 `3` 个时，应成功入队一个批量队列项，实际批次数为 `3`
- 可做 `10` 个时，应成功入队一个批量队列项，实际批次数为 `10`
- 可做大于 `10` 个时，应成功入队一个批量队列项，实际批次数仍为 `10`

重点不是按钮名字，而是“目标十个，但实际按当前真相截断到可做上限”这条规则是否成立。

### 7.8 右键制作全部，验证“一个队列格子承载整批”

动作：

1. 在制造列表里右键一条当前可做多次的配方
2. 点击 `制作全部`
3. 观察队列区显示
4. 再连续给别的配方追加几批制造，直到接近 `MaxQueueSize`

预期：

- `制作全部` 成功时，只新增一个批量队列项
- 不会按“做多少次就占多少格”去挤满队列
- 当已经排了 5 批不同的制造队列项后，再新增第 6 批才应被视为队列已满
- 当前批次完成前，队列区显示的仍应是“这批剩余多少次 / 当前批正在跑哪一次”的同一队列项语义，而不是拆成一串单次条目

## 第八章 这轮之后还剩哪些问题

### 8.1 文档口径之前是旧的，这次已经统一，但后续仍要继续守住

这次文档已经统一到单表架构。但后续任何新增说明、测试笔记、知识库条目，只要又把制造系统写回“双表链路”，就会把下一个接手的人重新带偏。

### 8.2 观察数据结构还没有完全收干净

当前方向已经正确，但还有继续收敛空间：

- `FAOCrafting*ViewData` 仍然偏多
- `PrimaryOutputDefinition` 这种命名还带着旧列表语义
- 详情区和列表区仍然分成多层 ViewData 结构

这不妨碍当前代码工作，但它们确实还是后续可以继续清理的对象。

### 8.3 中文乱码问题仍然是当前遗留风险

这一点必须单独留档。

当前代码里已经能看到一部分中文字面量存在乱码迹象，所以后续再接手 UI 完善时，要把“显示逻辑问题”和“编码问题”分开查，不能混成一个问题。

### 8.4 这轮之后，“单次队列项思维”必须退出分析链

这次如果把右键批量制造接进来，后续最容易反弹的误解就是：

- 觉得 `制作十个` 理应占十个队列格子
- 觉得 `制作全部` 应该等价于一连串单次入队
- 觉得 UI 层循环调十次旧接口也算完成需求

这些都属于旧思路。只要这次正式改成“一个队列项承载一批制作”，后续分析、排查和文档口径都必须一起切过去。

## 第九章 这轮方案的结论

这轮不是在补一个制造按钮，也不是在给蓝图补几个事件。

这轮真正完成的是把制造系统重新收回到一条清楚的链上：

- 制造规则只认角色侧单张制造表
- 物品信息只认物品总表
- 底层真相只认 `UAOCraftingComponent`
- UI 数据观察只认 `UMVVM_Crafting`
- Widget 只做显示和命令，不再接管底层
- Layout 不再越权插手制造刷新

在这个基础上，这次继续往前推的新增口径也很明确：

- 制造列表项右键继续复用现有右键菜单主链
- 菜单动作只表达批量制造意图，不自己计算制造真相
- 批量制造不再展开成多个单次队列项
- 队列上限以后按“最多五个批量制造队列项”理解
- 制造队列、开始时间、结束时间、剩余次数继续由服务端持有权威真相
- 客户端可以本地持续刷新进度条显示，但正式进度计算必须使用可对齐的服务端时间基准，不能再把服务端时间戳和客户端本地世界时间混算

后面不管是继续收结构体、继续补控件、继续修图标名字，还是继续做工作台上下文，这条边界都不要再打散。
