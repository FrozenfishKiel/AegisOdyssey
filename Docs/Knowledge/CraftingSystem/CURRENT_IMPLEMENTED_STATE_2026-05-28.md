---
title: Crafting Current Implemented State 2026-05-28
tags:
  - knowledge
  - crafting-system
  - current-state
  - queue-ui
  - context-menu
  - workbench-reuse
aliases:
  - Crafting Current Implemented State 2026-05-28
  - 制造系统当前已实现状态 2026-05-28
  - 制造系统当前落地状态
  - 制造右键批量与队列UI现状
  - 工作台熔炉烹饪锅制造复用入口
---

# 制造系统当前已实现状态
更新时间：2026-05-28

这份文档只写一件事：**把当前已经真正落地、已经测试通过、以后工作台 / 熔炉 / 烹饪锅 / 其他制造对象必须复用的那部分现状讲清楚。**

相关文档：
- [[Crafting System Decisions]]
- [[Crafting System Project Map]]
- [[Crafting System Known Issues]]
- [[Crafting UI Refresh Refactor Plan]]
- [[Crafting Refactor Handoff 2026-05-27]]

> [!important]
> 以后如果要扩工作台、熔炉、烹饪锅，不要重新发明第二套制造入口、第二套右键菜单链、第二套队列语义。
> 默认应该建立在当前这套 `UAOCraftingComponent -> UMVVM_Crafting -> Widget` 主链之上扩。

## 1. 当前必须先记住的主链

当前真实主链是：

`UAOPawnData::CraftingRecipeDataTable -> UAOCraftingComponent -> UMVVM_Crafting -> Widget`

这条链现在的含义很明确：

- 配方规则仍然来自 `UAOPawnData::CraftingRecipeDataTable`
- `UAOCraftingComponent` 持有制造真相
- `UMVVM_Crafting` 直接观察 `UAOCraftingComponent`
- Widget 只消费 ViewModel，不再自己造制造真相

以后只要你在排工作台、熔炉、烹饪锅的制造接线，先判断自己是不是还在这条主链上。如果已经偏到 UI 临时判断、Inventory 临时拼上下文、或者另一套并行制造请求入口，那方向就已经错了。 ^crafting-main-chain

## 2. 当前已经落地的右键批量制造

当前制造列表里的单条配方，右键后已经支持三条动作：

- `制作一个`
- `制作十个`
- `制作全部`

但要明确，这三条动作只是**目标批量意图**，不是 UI 层循环调用十次单次制作。

当前正式入口是：

- `UMVVM_Crafting::RequestCraftRecipe(...)`
- `UAOCraftingComponent::RequestCraftRecipe(...)`
- `UAOCraftingComponent::TryRequestCraftRecipeOnAuthority(...)`

当前规则已经锁定为：

- `制作一个` 的目标数是 `1`
- `制作十个` 的目标数是 `10`
- `制作全部` 的目标数是“当前真实最多还能做多少个”
- 真正能做几个，统一由底层按 `MaxCraftableCount` 裁剪
- 如果连 `1` 个都做不了，直接失败，并给出明确反馈
- 如果目标数大于真实可做数，但仍然至少能做 `1` 个，就按真实上限尽量做
- 用户可见反馈要明确表达“这次实际开始制作了多少个”，不要让用户误以为点十个就一定排了十个

> [!note]
> 也就是说，右键菜单不是第二套制造系统，它只是另一种发起制造请求的入口。

## 3. 当前队列语义已经改成“一个队列项承载一整批”

这次最重要的收口之一，就是队列语义已经明确改成：

- 一个队列项代表一批同配方制造请求
- 一个批量队列项内部按 `1 个 1 个` 顺序制作
- `MaxQueueSize = 5` 的含义是“最多 5 个批量队列项”
- `制作十个` 如果成功，只占 `1` 个队列格子，不占 `10` 个
- `制作全部` 也是同理，只要被裁剪成一批，就仍然只占 `1` 个格子

当前批量队列项的正式结算规则也已经落地：

- 每完成 `1` 个，就发放 `1` 次产物
- 每完成 `1` 个，`RemainingCraftCount` 递减 `1`
- UI 持续显示剩余次数
- 当前不支持中途取消
- 角色中断或死亡时，当前先按“清空队列且不返还已扣材料”处理

这部分是以后扩工作台、熔炉、烹饪锅时必须继承的默认语义。除非玩法规则整体改变，否则不要在某个子系统里偷偷改成“一个格子只等于一个成品”。 ^crafting-batch-queue

## 4. 当前队列 UI 已经怎么落地

当前队列 UI 已经不是临时显示几条就生成几条，而是固定槽位语义：

- 队列区挂在 `UAOCraftingRecipeListWidget`
- 使用 `QueueListContainer` 承接队列槽位
- 固定槽位数量来自 `UMVVM_Crafting::GetQueueSlotCount()`
- `GetQueueSlotCount()` 最终镜像自 `UAOCraftingComponent::GetMaxQueueSize()`
- 队列区会先创建固定数量槽位，再按当前队列快照填充
- 没有批量制造项时，空槽位依然存在

当前空槽位显示规则：

- 进入 `SetQueueEmptyState()`
- 由 `QueueEmptyPlaceholder` 负责空槽位占位
- `StatusText` 当前明确显示 `空闲中`

当前非空队列项显示规则：

- 显示 `PrimaryOutputDefinition` 对应的图标和名称
- 显示 `RemainingCraftCount / TotalCraftCount`
- 显示状态字样
- `Active` 显示“制作中”
- `Queued` 显示“等待中”
- 显示正式 `ProgressBar`

当前实时进度规则：

- 只有当前 `Active` 队列项做实时进度刷新
- 进度百分比继续复用 `UMVVM_Crafting::GetQueueEntryProgressRatio(...)`
- 实时刷新由 `UAOCraftingRecipeListWidget::NativeTick(...)` 本地驱动
- 还没轮到的 `Queued` 项，进度保持 `0`

这里要特别记住一条已经落地的联机修正：

- 队列开始时间和结束时间仍然由服务端写入
- `UMVVM_Crafting::GetObservedServerWorldTimeSeconds()` 不再直接取客户端本地 `World->GetTimeSeconds()`
- 当前实现优先走 `GameState->GetServerWorldTimeSeconds()`
- 只有拿不到 `GameState` 时才回退本地世界时间

这条改动的目的不是改队列业务规则，而是保证远端客户端看到的进度条，和服务端权威队列时间属于同一时间基准。 ^crafting-queue-server-time-basis

## 4.1 这次 Dedicated Server 进度条问题最后是怎么收掉的

这次实际暴露出来的现象是：

- 远端客户端能看到队列项进入队列
- 但进度条不会立刻推进
- 过一小会才开始动
- 动到一半左右，这一件却已经做完了

最后收出来的根因不是单纯“同步慢”，而是更具体的时基混算：

- `StartServerWorldTimeSeconds / ExpectedFinishServerWorldTimeSeconds` 由服务端写入
- 旧实现里，客户端进度计算却直接用了本地 `World->GetTimeSeconds()`
- 这样就把“服务端时间戳”和“客户端本地时间”混在一起做减法了

当前知识库应该按下面这句话记忆：

**客户端可以本地持续刷新显示，但正式进度计算必须建立在服务端真相和同一服务端时间基准上。**

## 5. 当前配方列表样式和队列样式已经分开配置

这个点必须单独记住，因为它直接影响以后扩新制造对象时的 UI 复用方式。

当前状态不是“配方列表和队列槽位只能共用一个 Widget Blueprint 样式”，而是：

- `RecipeEntryWidgetClass` 只给配方列表用
- `QueueEntryWidgetClass` 只给队列槽位用
- 两边当前仍然共用同一个 C++ 基类 `UAOCraftingRecipeListEntryWidget`
- 但可以挂完全不同的 Widget Blueprint 样式

这意味着以后如果工作台、熔炉、烹饪锅要做不同视觉风格：

- 先优先换 Blueprint 样式
- 仍然沿用当前 C++ 主链和显示语义
- 只有当控件结构和职责完全不同，才考虑再拆新的 C++ 队列条目类

> [!warning]
> 以后如果你发现 `QueueEntryWidgetClass` 明明在蓝图配置了但运行时仍然像空，一定先检查：
> 1. 是否真的是最终运行的那个列表控件蓝图。
> 2. 反射字段新增后是否做过完整编译和编辑器重启。
> 3. 队列条目蓝图里是否真的绑定了 `QueueEmptyPlaceholder`、`StatusText`、`ProgressBar` 等必需控件。

## 6. 当前右键菜单是“复用菜单框架”，不是“复用库存来源链”

当前制造右键菜单已经复用了现有库存右键菜单框架，但复用的层次要说清楚：

复用的是：

- `UMVVM_InventoryItemContextMenu`
- `UMVVM_InventoryItemContextAction`
- `UAOInventoryItemContextMenuWidget`
- `UAOInventoryItemContextActionWidget`

没有继续复用的是“制造菜单一定要从 `SourceInventory` 找来源”这条旧前提。

当前正式口径已经是：

- 制造右键菜单主 ViewModel 挂在 `UAOCraftingComponent`
- 入口仍然允许通过 `AOInventoryUI` 发起打开
- 但 `AOInventoryUI` 在这里是统一弹出入口，不是制造上下文真相宿主
- 菜单真正上下文是 `SourceCraftingComponent + RecipeRowName + ScreenSpacePosition`
- 不是 `SourceInventory + SlotIndex + ItemInstance`

所以以后做工作台、熔炉、烹饪锅时，默认思路应该是：

- 每个制造源自己持有自己的 `UAOCraftingComponent`
- 右键菜单主 ViewModel 也由这个制造源的 `UAOCraftingComponent` 持有
- 菜单动作最终继续回流到 `UMVVM_Crafting::RequestCraftRecipe(...)`

而不是重新把语义硬绑回库存来源链。 ^crafting-context-menu-reuse

## 7. 以后扩工作台 / 熔炉 / 烹饪锅时应该怎么复用

当前这套实现，已经明确不是只服务“玩家自身制造”。

以后只要某个对象也想成为制造源，默认复用口径应该是：

1. 这个对象自己也拥有一个 `UAOCraftingComponent`
2. 这个制造组件自己决定配方表、材料来源和速度加成口径
3. `UMVVM_Crafting` 继续观察“当前被打开的那个制造组件”
4. 右键菜单继续以“制造组件 + 配方行名”为真实上下文
5. 队列 UI 继续吃 `GetQueueSlotCount()`、`GetQueueList()`、`GetQueueEntryProgressRatio(...)`

也就是说，未来不是给工作台另造一套“工作台专用制造 UI 主链”，而是让工作台成为新的制造源，继续走当前这套统一制造域主链。 ^future-station-reuse

## 8. 当前最重要的排查入口

以后如果新会话要排制造系统，优先从下面这些关键词进：

### 8.1 查右键批量制造

- `UMVVM_Crafting::RequestCraftRecipe`
- `UAOCraftingComponent::RequestCraftRecipe`
- `UAOCraftingComponent::TryRequestCraftRecipeOnAuthority`
- `UMVVM_InventoryItemContextMenu::ExecuteResolvedAction`
- `UAOCraftingComponent::GetOrCreateCraftingContextMenuViewModel`

### 8.2 查队列 UI 为什么没显示

- `QueueListContainer`
- `QueueEntryWidgetClass`
- `QueueEmptyPlaceholder`
- `StatusText`
- `UMVVM_Crafting::GetQueueSlotCount`
- `UAOCraftingComponent::GetMaxQueueSize`

### 8.3 查批量队列语义为什么不一致

- `MaxQueueSize`
- `RemainingCraftCount`
- `TotalCraftCount`
- `HandleActiveCraftingFinished`
- `BuildQueueViewData`

### 8.4 查 Dedicated Server 远端客户端进度条为什么怪

- `UMVVM_Crafting::GetObservedServerWorldTimeSeconds`
- `UMVVM_Crafting::GetQueueEntryRemainingSeconds`
- `UMVVM_Crafting::GetQueueEntryProgressRatio`
- `UAOCraftingComponent::StartNextQueuedEntry`
- `UAOCraftingComponent::HandleActiveCraftingFinished`

如果现象表现成“队列看得到，但进度条要等一下才开始走，且只走半截这一件就完成了”，优先怀疑的是时间基准混用，而不是先怀疑 `ProgressBar` 控件或队列本身没推进。

## 9. 当前必须避免的误解

- 不要再把右键菜单理解成第二套制造入口链
- 不要再把 `MaxQueueSize = 5` 理解成“最多做 5 个成品”
- 不要再把“制作十个”理解成“UI 循环调用十次单次制作”
- 不要再把空槽位显示为空白当成“队列没创建”，先看 `QueueEmptyPlaceholder`
- 不要再把 `AOInventoryUI` 理解成制造右键菜单的真相宿主
- 不要再把未来工作台扩展理解成“另起一套专用制造系统”
- 不要再把服务端写入的队列时间戳和客户端本地世界时间混着算进度

## 10. 当前最值得先看的文件

- [AOCraftingComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h)
- [AOCraftingComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp)
- [MVVM_Crafting.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h)
- [MVVM_Crafting.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp)
- [AOCraftingRecipeListWidget.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.h)
- [AOCraftingRecipeListWidget.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp)
- [MVVM_InventoryItemContextMenu.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.cpp)
- [AOInventoryUI.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h)
- [AOInventoryUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp)

## 11. 一句话收口

当前制造系统已经不再只是“角色面板里点一下制作”的零散功能，而是已经收成了一条可复用主链：

**统一制造源、统一 ViewModel、统一右键动作回流、统一批量队列语义、统一固定槽位队列 UI。**

以后扩工作台、熔炉、烹饪锅，默认是在这条链上接，不是重新起炉灶。
