---
title: 库存右键菜单与MVVM最新收束说明
date: 2026-05-25
tags:
  - inventory
  - ui
  - mvvm
  - context-menu
aliases:
  - 库存右键菜单最新状态
  - Inventory Context Menu MVVM 收束说明
---

# 库存右键菜单与MVVM最新收束说明

这份文档只负责讲一件事：**库存右键菜单这条链，最新到底已经收束成什么样了**。

它不是最早的设计稿，也不是排查日志汇总。它更像是一份“当前真实实现口径”的收束说明，方便后续继续开发、排查和交接时，先把方向站稳。

如果要看更长的历史过程，先读这两份：

- [[角色制造系统首版技术方案]]
- [[HistoryNotice/消耗品与库存右键使用设计方案-现状-方案-实施阶段]]

---

## 第一章 这次收束真正解决的是什么

这次右键菜单改动，核心不是“把菜单弹出来”这么简单。

真正要收的是下面这几个问题：

1. 右键菜单的数据到底从哪里来。
2. 右键菜单的 ViewModel 到底挂在哪。
3. 不同库存组件右键时，菜单上下文到底由谁决定。
4. 菜单 Widget、动作项 Widget、动作项 ViewModel 之间的职责边界到底怎么分。
5. 后续蓝图调样式时，哪些事可以在蓝图做，哪些事不应该继续写进 C++。

这次收束后的核心结论很明确：

**右键谁，就以谁的 `OwnerComponent` 作为来源；菜单主 ViewModel 由那个 `InventoryComponent` 自己持有；菜单 Widget 只消费这份 ViewModel，不再自己推断宿主。**

---

## 第二章 当前真实链路已经变成什么样

### 2.1 唯一入口仍然是 `AOInventoryUI`

当前右键菜单的主入口仍然在：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`

主链路是：

1. `NativeOnMouseButtonDown(...)`
2. `ResolveInventoryItemContextMenuRequest(...)`
3. `RequestOpenInventoryItemContextMenu(...)`
4. `SourceInventory->GetOrCreateContextMenuViewModel()`
5. `ContextMenuViewModel->OpenForInventorySlot(...)`
6. `ActiveContextMenuWidget->SetContextMenuViewModel(...)`
7. `ActiveContextMenuWidget->InitializeForInventorySlot()`
8. `AddToViewport(...)`

这里最重要的不是调用顺序本身，而是现在的语义已经明确了：

- `ResolveInventoryItemContextMenuRequest(...)` 负责把“这次右键到底点中了谁”恢复出来。
- `AOInventoryUI` 负责把这次右键请求翻译成菜单打开请求。
- `InventoryComponent` 负责提供它自己持有的那一份菜单主 ViewModel。
- 菜单 Widget 只负责消费已经准备好的 ViewModel 快照。

也就是说，**UI 不再额外猜宿主，ViewModel 也不再绕路从别的总 ViewModel 上取。**

### 2.2 `InventoryComponent` 现在只做一件事：存菜单主 ViewModel

当前入口在：

- `Source/AegisOdyssey/Inventory/AOInventoryComponent.h`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp`

现有 API 是：

- `UMVVM_InventoryItemContextMenu* GetOrCreateContextMenuViewModel()`

这次收束后，`UAOInventoryComponent` 对这条链的职责被刻意压窄了。

它现在只负责：

1. 保存一份属于自己的 `ContextMenuViewModel`。
2. 在第一次被请求时创建它。
3. 让这份对象的 Outer 直接挂在当前 `InventoryComponent` 上。

它现在**不再负责**：

1. 猜当前玩家应该共用哪个宿主库存。
2. 把别的库存右键请求路由到 `BackPack`。
3. 帮 UI 做来源兜底。

这件事很关键。因为之前一旦做“统一宿主解析”，调试时就很容易出现一种错觉：你右键的是 A，但菜单 ViewModel 实际上挂在 B，上下文一旦漏刷，就会表现成串数据。

这次收束就是把这个隐患先砍掉。

### 2.3 右键谁，就直接命中谁自己的菜单 ViewModel

当前正式口径是：

- 角色物品栏右键，命中那个物品栏所属的 `InventoryComponent`。
- 背包右键，命中背包自己的 `InventoryComponent`。
- 装备栏、快捷栏、技能栏如果也是通过真实来源库存恢复上下文，那也应该命中各自这次请求真正对应的来源组件。

这里的本质不是“每个地方都挂一个很酷的 ViewModel”。

真正的重点是：

**菜单来源必须和这次右键请求的真实来源一致。**

如果来源不一致，第一次看上去也许能用，但一旦切换物品、切换槽位、切换不同库存区域，就会开始出现旧数据残留、动作残留、标题不刷新、图标不刷新这类非常难排查的问题。

---

## 第三章 这次 MVVM 是怎么分层的

### 3.1 当前有两层 ViewModel

现在这套右键菜单，ViewModel 层已经拆成了两层：

1. `UMVVM_InventoryItemContextMenu`
2. `UMVVM_InventoryItemContextAction`

对应文件是：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.cpp`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.cpp`

这两层分工现在已经比较清楚了。

### 3.2 菜单主 ViewModel 负责“整份右键上下文”

`UMVVM_InventoryItemContextMenu` 现在负责承接整份菜单快照。

它承接的内容包括：

1. 当前来源库存组件。
2. 当前来源槽位索引。
3. 当前命中的物品实例。
4. 当前菜单显示位置。
5. 当前头部显示文本和图标。
6. 当前动作项 ViewModel 列表。
7. 当前菜单是否可见。
8. 当前动作请求最终该回到哪个 `InventoryUI` 去执行。

这意味着一个很重要的架构结论：

**菜单主 ViewModel 不只是“给 UI 显示几个字段”，而是这一整次右键菜单请求的统一承载体。**

这也是为什么后续如果要继续补功能，最好继续沿着这层做，而不是在 Widget 各处零散补逻辑。

### 3.3 动作项 ViewModel 负责“单个动作项”

`UMVVM_InventoryItemContextAction` 负责的是菜单里每一条动作。

它只承接：

1. 当前动作标签文本。
2. 当前动作是否可点击。
3. 当前这条动作对应的原始决议数据。
4. 点击后如何把执行请求回交给父菜单 ViewModel。

这层的好处很直接：

- 菜单 Widget 不需要自己拼一堆按钮语义。
- 动作项 Widget 不需要自己直接碰库存逻辑。
- 后续动作项增多时，也不会逼着菜单主 Widget 自己手写一大坨按钮状态代码。

### 3.4 Widget 层现在应该只做消费，不做业务决议

当前对应文件是：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.cpp`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.cpp`

这两层 Widget 现在的职责应该这样理解：

- 菜单主 Widget 负责接菜单主 ViewModel，刷新头部和动作项容器。
- 动作项 Widget 负责接动作项 ViewModel，转发点击。

它们现在都**不应该**继续承担这些事：

1. 自己推导当前物品能不能用。
2. 自己拼动作列表。
3. 自己直接修改库存。
4. 自己硬编码 UI 样式。
5. 自己在 C++ 里搭一套备用 Slate 样式按钮。

用户这轮已经明确要求，不要再自己在 C++ 里造 UI 样式和结构，这条边界后续要继续遵守。

---

## 第四章 当前动作决议是怎么来的

当前动作生成入口仍然在：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`

具体是：

- `BuildInventoryItemContextActions(...)`

现在的正式实现很简单：

1. 如果当前槽位、实例、来源库存有效。
2. 如果 `Entry.Instance->CanUseFromInventory(...)` 返回真。
3. 就生成一条 `Use`。
4. 如果动作列表不为空，再统一补一条 `Close`。

所以当前真实语义是：

- 没有任何动作时，菜单不会打开。
- 当前只正式支持 `Use + Close` 这一组最小动作集。
- `Close` 由基类统一补，不要求每个子类菜单自己追加。

这套逻辑现在虽然简单，但它已经有一个很重要的优点：

**动作决议集中在一处。**

后续如果要继续扩 `Drop`、`Split`、`Equip`、`Unequip`、`MoveToQuickBar` 这类动作，最好继续在这条决议链上补，而不是在各个 Widget 或蓝图按钮上各写一套。

---

## 第五章 当前蓝图接线必须知道的几个事实

### 5.1 菜单主蓝图类当前有一个临时路径兜底

当前 `AOInventoryUI.cpp` 里保留了一个临时兜底：

- `/Game/Games/UI/InventoryMenu/Information/WBP_InventoryContext_Action.WBP_InventoryContext_Action_C`

只有在 `InventoryItemContextMenuClass` 为空时，它才会尝试加载这条路径。

这里必须明确两件事。

第一，这个行为是**按当前用户明确要求**保留的，目的是调试期间别因为类没配上导致整条菜单链完全打不开。

第二，这条路径的名字看起来更像动作项蓝图，而不是菜单宿主蓝图，所以它本身是一个明显的风险点。

也就是说，当前代码层面虽然能兜底，但长期来看，最好还是在蓝图配置上把宿主菜单类和动作项类彻底配清楚，不要长期依赖这个兜底。

### 5.2 动作项蓝图类不是自动猜的

菜单里按钮为什么能不能显示，除了动作决议，还取决于：

- `ActionEntryWidgetClass`

这意味着：

1. 菜单主蓝图需要显式配置动作项 Widget 类。
2. 动作项 Widget 蓝图至少要把主按钮之类的最小绑定接好。
3. 如果没配好，现象可能不是“动作逻辑错了”，而是“动作项压根没可视化出来”。

### 5.3 菜单 UI 不应该再依赖 C++ 手写样式

这次已经明确收口：

- 不再自己在 C++ 里搭默认 Slate 按钮样式。
- 不再自己在 C++ 里决定按钮排版细节。
- 不再自己在 C++ 里硬塞默认视觉布局。

代码只负责：

1. 提供 ViewModel。
2. 提供动作项容器刷新。
3. 提供菜单显示位置。
4. 提供最小交互闭环。

至于皮肤、布局、尺寸、自适应、文字样式，应该回到蓝图去做。

---

## 第六章 这次必须记住的两个坑

### 6.1 `MVVM_InventoryItemContextAction` 命名冲突问题

当前动作项 Widget 这边，C++ 已经有一个属性名：

- `MVVM_InventoryItemContextAction`

而且 `AOInventoryItemContextActionWidget.cpp` 这边也按这个名字做 MVVM 源注册。

所以蓝图 `WBP_InventoryContext_Action` 里，**不要再手动创建一个同名的变量或同名的 MVVM Source**。

否则就会报这类错误：

- `There is already a property named 'MVVM_InventoryItemContextAction' in scope ...`
- `The viewmodel name could not be found ...`
- `The source 'MVVM_InventoryItemContextAction' was evaluated to be invalid ...`

这类报错的本质不是“动作逻辑错了”，而是**蓝图侧同名定义把 C++ 侧约定冲突掉了**。

### 6.2 首次显示、重复打开、切换物品时最容易混的不是逻辑，而是旧快照残留

这轮我们已经遇到过一次典型问题：

- 第一次右键 A 正常。
- 第二次右键 B 时，ViewModel 还残留 A 的内容。

后来收束后，这条风险的根因已经更清楚了：

1. 菜单 ViewModel 来源不唯一时，最容易串。
2. 右键来源和持有 ViewModel 的组件不一致时，最容易串。
3. Widget 复用但没有在打开前重新灌完整快照时，最容易串。
4. 动作项容器复用但旧动作没清干净时，最容易串。

所以后续如果再遇到“第一次对，切一下就错”的问题，优先查的方向应该是：

1. 这次 `SourceInventory` 到底是谁。
2. 拿到的 `ContextMenuViewModel` 到底挂在哪个 `InventoryComponent` 上。
3. `OpenForInventorySlot(...)` 是否真的在打开前被重新调用。
4. `InitializeForInventorySlot()` 和动作项刷新链有没有完整走到。

不要一上来先怀疑蓝图文本绑定。

---

## 第七章 这条链当前的明确边界

当前右键菜单系统已经明确了这些边界。

### 7.1 `InventoryComponent` 不做宿主解耦逻辑

库存组件在这条链上只负责存一份菜单 ViewModel。

它不负责：

1. 统一找角色背包当宿主。
2. 推断当前右键请求来自哪个系统。
3. 替外部做来源修正。

### 7.2 谁是来源，由右键请求自己给出来

当前正式口径是：

**右键谁，来源就从这次右键请求自身的 `OwnerComponent` 和槽位上下文恢复。**

不要再补“猜一层”“兜一层”“反推一层”的额外来源逻辑。

### 7.3 菜单主 ViewModel 是唯一收口点

后续要继续补头部信息、动作项、可点击态、阻断文案、扩展动作，都应该优先走：

- `UMVVM_InventoryItemContextMenu`

不要再把一部分状态放 Widget，一部分状态放别的临时缓存，一部分状态放蓝图自造变量。

### 7.4 代码不再替蓝图做 UI 样式决策

代码负责逻辑和数据。

蓝图负责视觉。

这条边界现在已经很明确，后续继续遵守。

---

## 第八章 后续如果还要继续接，建议先从哪几处看

如果下一个 AI 或工程继续接这块，建议先看下面这些文件。

第一组是入口和来源：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.h`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp`

第二组是菜单主 ViewModel：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.cpp`

第三组是动作项 ViewModel 和动作项 Widget：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h`
- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.cpp`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.cpp`

第四组是菜单宿主 Widget：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.h`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.cpp`

---

## 第九章 一句话结论

当前库存右键菜单这条链，已经从“菜单数据来源分散、宿主推断混乱、Widget 和逻辑层纠缠”的状态，收束成了：

**由 `SourceInventory` 直接提供唯一菜单主 ViewModel，`AOInventoryUI` 统一发起请求，菜单 Widget 全量消费 MVVM 快照，蓝图专注样式，代码专注上下文和动作执行链。**

后续继续扩功能时，不要再把这条链重新打散。
