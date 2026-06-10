---
title: Character Inventory Session Unification And Bug1 Analysis 2026-05-29
date: 2026-05-29
tags:
  - knowledge
  - interaction-system
  - inventory-equipment
  - ui
  - multiplayer
  - bug-analysis
status: active
aliases:
  - 角色库存会话统一与 Bug1 排查 2026-05-29
  - Character Inventory Session Unification Bug1
---

# 角色库存会话统一与 Bug1 排查

这篇笔记只收一件事：把这次“角色库存打开后，第一次还能往对象库存里放物品，但后续来回操作异常”的排查结论记清楚。

这不是在补一个零散 bug 记录，也不是在重新写一遍角色库存方案。
这篇文档真正要锁住的是：

当前角色库存交互之所以开始偏离箱子主链，不是因为交互入口没复用箱子，而是因为**打开之后的目标侧会话事实源和 UI 消费路径已经分叉了**。

后面如果不把这个边界重新收紧，角色交易、其他可打开库存对象、甚至后续更多“对象打开对象库存”的需求，都会继续沿着不同路径各长一套，最后就会重新回到“箱子一套、角色一套、交易一套”的老问题上。

## 先把这次问题的核心结论写在前面

当前 Bug1 更像是一个“路径没有统一完”的结构性问题，而不是一个孤立的小逻辑 bug。

更具体一点说：

- 角色库存交互的**打开入口**，确实已经在复用箱子那套交互会话主链。
- 但角色库存交互的**目标侧显示与后续操作链**，并没有继续严格站在箱子那条“单一会话事实源”上。
- 结果就是，表面上看起来像“打开对象库存并拖拽交换”，实际上底下已经不是箱子那种“一条会话链走到底”的模型了。

所以这次问题真正暴露出来的，不只是“某个交换后没刷新”。
它暴露的是：**当前角色目标库存页已经在路径上走偏了。**

## 箱子这条链为什么稳定

之所以这次一直反复拿箱子当对照，不是因为箱子只是一个现成例子，而是因为箱子这条链在结构上其实已经把“对象打开对象库存”这件事收得比较干净了。

箱子当前的特点非常明确：

第一，交互对象只暴露一个正式目标库存，也就是 `ChestInventory`。

第二，服务端打开箱子时，会创建 `UAOContainerInteractionSessionModel`，并把这个 `ChestInventory` 塞进会话模型。

第三，会话模型后面只围绕这一个 `ObservedInventoryComponent` 工作。它会去观察这个组件、提取槽位快照、同步槽位快照、驱动容器 UI 刷新。

第四，容器 UI 也只消费这一份来自会话模型的 `ObservedContainerSlots`。它不是自己回头去对象身上再找别的组件，也不会一边吃会话数据、一边再自己补第二份目标上下文。

把它压成一句话，就是：

**一个交互对象，对应一个正式会话里的目标库存真相源。**

这就是箱子这条链最重要的稳定性来源。

## 角色库存交互现在表面上像箱子，实际上差在哪里

这次排查里最容易让人误判的点，就是“角色库存不是也在创建 `UAOContainerInteractionSessionModel` 吗，为什么还会偏？”

问题不在这里。

角色当前确实已经做到下面这一步了：

- 通过交互入口打开目标角色
- 服务端创建 `UAOContainerInteractionSessionModel`
- 把角色的 `BackPack` 作为容器库存交给会话模型

如果只看到这里，会很容易以为“那这不就是箱子了吗”。

但真正开始分叉的是**后面那半段**。

当前角色库存页并没有继续严格只消费这一个容器会话里的目标库存。
相反，它在目标侧又自己拼了一份显示上下文，直接从目标 Actor 上拿：

- `BackPackComponent`
- `QuickBarComponent`
- `FormalEquipmentInventory`
- `FormalEquipmentManager`
- `SkillComponent`
- `SkillSlotInventory`

也就是说，角色目标页不是“只显示当前会话注册的目标库存事实”，而是变成了“会话给一部分，页面自己再去目标对象上抓更多组件”。

这一步一旦发生，角色交互就已经不再等同于箱子了。

## 这里真正偏掉的不是 UI 好不好看，而是事实源被拆开了

如果只是从界面层看，这种写法很容易让人觉得只是“页面多显示了几块区域”。

但它真正危险的地方，不在于页面布局，而在于**事实源开始分裂**。

箱子那条链里，谁是目标库存、谁能被观察、谁是正式 mutation 对象，这几个问题答案都非常统一。

角色这条链里，现在变成了两套口径同时存在：

一套口径是会话模型里的 `ObservedInventoryComponent`。
它是当前交互系统正式持有的“目标容器库存”。

另一套口径是页面自己组出来的目标侧显示上下文。
它会额外把目标角色身上的其他库存组件也拉进来显示。

问题恰恰就在这里：

**显示出来了，不等于它已经成为这次会话正式注册和正式观察的一部分。**

这件事一开始可能不明显，但一旦进入“拖拽交换、再次交换、放回原库存、使用、刷新、多人同步”这些有状态的操作，就会开始出问题。

## 这次 Bug1 为什么会表现成“第一次可以，后面不对”

这类症状其实很符合“第一次碰巧踩中正式链，后续开始碰到分叉链”的特征。

当前更像是这样一件事：

第一次把玩家物品拖进目标角色库存时，落点刚好是当前会话真正认的那个目标背包组件，所以交换能成功。

但后面继续从目标侧拿东西出来、再交换回去、或者在目标侧内部做新的拖拽时，UI 看到的目标区域和 mutation 权限链真正认的“当前目标容器”已经不再完全是同一个东西。

于是就会出现一种特别典型的错位感：

- 画面上对象库存是打开的
- 页面上也能看到目标区块
- 甚至第一次操作还能成功
- 但后续来回操作开始不稳定，或者根本不走正确结果

这不是普通意义上的“按钮没绑好”。
这更像是：**一次会话里，显示链和操作链对“目标到底是谁”这件事已经说了两种话。**

## 真正应该统一的地方，不是“多显示几个组件”，而是“谁有资格进入这次会话”

这次讨论里一个很关键的判断是对的：

后面不应该让 Widget 自己去决定“我该看哪个组件”，也不应该让页面自己继续扩展“角色目标页想显示什么就自己去目标身上抓什么”。

真正应该统一的，是会话模型。

更准确地说，后续应该把口径改成：

**会话模型是这次目标库存上下文的唯一持有者。**

也就是说，一次打开对象库存的会话里：

- 目标侧有哪些库存组件可以被这次会话观察
- 哪些组件允许被这次会话修改
- 哪些组件需要进入复制快照
- UI 该显示哪些目标区块

这些问题都应该先在 SessionModel 层决定。

UI 不再自己回头 `FindComponentByClass`。
UI 只负责消费 SessionModel 注入的那份上下文。

这才是真正的统一路径。

## 为什么这里必须把“动态观察组件”收在 SessionModel，而不是 UI

表面上看，“会话模型动态观察组件”和“页面自己多抓几个组件显示出来”，都像是在让角色库存支持多面板。

但这两者本质完全不一样。

如果动态观察发生在 UI 层，那么 UI 只是知道“现在目标身上还有这些组件”。
可它并不知道：

- 这些组件是不是当前会话正式允许操作的组件
- 这些组件是不是已经被绑定了变化通知
- 这些组件是不是进入了当前 owner-only 快照
- mutation 权限链该不该把这些组件当成当前交互对象数据的一部分

换句话说，UI 层最多只能做“看起来像是知道更多东西”，但它不掌握这次会话真正的系统级语义。

而如果动态观察收在 SessionModel 层，情况就不一样了。

SessionModel 可以统一决定：

- 这次目标对象暴露哪些库存组件
- 每个组件属于什么显示分区
- 哪些组件要注册观察
- 哪些组件要进同步快照
- 哪些组件允许进入 mutation

然后 UI 只消费这份已经成型的上下文。

这时“多个目标面板”才仍然属于**同一条正式会话链的扩展**，而不是多条路径凑在一个页面上。

## 后续应该统一成什么样的主线

如果把这次结论继续往前推，后面比较稳的统一方向其实已经很清楚了。

不是把角色库存继续当特例补丁修，而是把“打开对象库存”这件事正式抽成统一模式：

第一步，交互系统命中对象，并创建正式会话。

第二步，SessionModel 不再只持有一个 `ObservedInventoryComponent`，而是持有一份“目标库存上下文”。

第三步，这份上下文里明确列出：

- 当前对象暴露给这次会话的目标库存组件集合
- 每个组件对应的显示区块语义
- 每个组件是否允许 mutation
- 每个组件是否需要快照同步

第四步，SessionModel 统一注册这些组件的变化通知，统一决定何时刷新快照。

第五步，UI 页面只按 SessionModel 注入的上下文去显示，不再额外自行查目标组件。

第六步，所有 mutation 请求都必须先校验：目标组件是否属于当前 Session 注册集合。

这样一来，箱子只是“目标库存组件集合里只有一个组件”的特例。
角色则是“同一套模型下，目标库存组件集合里有多个组件”的扩展。

路径仍然是一条，不会分成两条甚至更多。

## 这件事为什么对后面的角色交易系统尤其重要

如果现在不把这条路径统一回来，后面角色交易系统几乎一定会继续长偏。

因为角色交易天然不是只看一个背包格子。
它会更容易涉及：

- 双方多个库存分区
- 当前会话可见和可操作范围
- 目标库存变化如何同步
- UI 上多个区块如何保持同一个会话事实源

如果角色库存打开这一层现在还是“入口统一了，但目标页自己再抓一套”，那角色交易只会把这种分叉进一步放大。

到时候很容易出现下面这种局面：

- 箱子有箱子的目标上下文
- 角色库存有角色库存的目标上下文
- 角色交易又长出第三套“交易专用目标上下文”

这正是现在最需要提前避免的方向。

所以这次 Bug1 的价值不只是修一个问题。
它其实是在提前提醒我们：

**如果还想把后续“对象打开对象库存”继续收口成一条正式主链，现在就必须把会话模型重新扶正。**

## 这次先记住的结论

这次最该记住的，不是某个函数名，也不是哪一个 Widget 先动过。

真正要记住的是下面这句话：

**角色库存交互之所以开始偏离箱子，不是因为交互入口没复用箱子，而是因为打开后的目标侧事实源没有继续统一收口在 SessionModel。**

只要这个边界不重新收回去，后面无论是继续补角色库存、做角色交易，还是接其他可打开库存对象，都会继续出现“表面统一、内部各走各路”的问题。

所以当前更稳的方向不是继续在页面层补判断，而是：

**把“目标侧有哪些库存组件属于这次会话”这件事，正式收回 SessionModel。**

这才是后面整条路径重新统一的起点。

## 相关代码入口

## 方案补记：这次收口不是另起一套系统

这里要特别记一下，这次方案不是要在现有工程上再另起一套“泛化的库存面板框架”。
也不是要新加一个 `PanelId`，或者先打一层很大的“未来支持很多库存类型”的抽象。

这次的出发点其实很具体：

- 工程里已经有 `UAOContainerInteractionSessionModel`
- 箱子链路已经在用它
- 角色打开库存的入口也已经走到它这里了

所以后面真正该做的，不是换一套新系统，而是把它现在没有收紧的那一半补齐。
也就是说，不是新做一个角色库存会话模型，而是把“目标侧有哪些库存组件属于本次会话、哪些组件进入观察、哪些组件允许 mutation、哪些组件需要进入复制快照”这些职责，正式收回到现有的 `UAOContainerInteractionSessionModel` 和 `UAOInteractionSessionComponent` 这条主链里。

这才是基于现有设计继续统一路径，而不是换个地方继续写死。

## 方案补记：后面真正要收紧的边界

如果把这次讨论压成最具体的落地方向，后面真正要收口的边界主要有四个。

第一个边界，是“哪些目标库存组件属于这次会话”。
现在箱子链路因为实际上只有一个 `ObservedInventoryComponent`，所以这个问题被单组件特例掩盖了。可一旦角色目标页要展开多个分区，这件事就不能继续让每个 Widget 自己决定。后面应该是 SessionModel 先给出正式结果：哪些组件属于本次会话注册集合，哪些不属于就不能混进来。

第二个边界，是“UI 消费会话，而不是反过来自组会话”。
`AOInventoryPageUI` 和各个子面板后面都不该再回头 `FindComponentByClass`。它们只能消费 SessionModel 正式注入的目标上下文，而不是按“我想显示什么就去目标身上抓什么”的方式继续长分叉路径。

第三个边界，是“mutation 的合法目标范围”。
现在 `SubmitCurrentInteractableMutation(...)` 已经把 active session、current interactable、owner 获取这些事情托管起来了，但它还没有真正回答第四个问题：当前 mutation 指向的目标库存组件，是否属于本次 session 正式注册的目标集合。只要这个边界不补上，就会继续出现“UI 上看得到，但在会话语义上并不属于当前交互目标”的错位。

第四个边界，是“复制和快照也必须跟着会话注册集合走”。
现在 `UAOInteractionSessionComponent` 复制给客户端的 `ReplicatedSessionState` 里，核心仍然只有 `ContainerSlots` 这类单容器快照。这说明当前复制模型本质上还是按“单一 container session”在想问题。只要后面的正式会话已经不止一个目标组件，复制快照和客户端重建逻辑也必须一起跟着扩展，否则服务端知道的是一套，客户端恢复出来的还是另一套。

## 方案补记：这套思路有没有把单机、服务器、客户端一起考虑进去

有，而且正因为不能只看单机，所以这次才必须把路径收回到会话模型。

单机现在之所以有时看起来“还能工作”，本质上是因为它没有完整经历“请求 owner、等待权限、owner-only snapshot、客户端重建”这一整条链路。在这种情况下，就算 UI 额外抓了一些目标组件，也可能因为都在本地执行而暂时不明显。

但到了 server / client 环境，事情就不是这样了。当前库存交互真正托底的主链，已经明确是下面这几块：

- `SubmitCurrentInteractableMutation(...)`
- `HasCurrentInteractableMutationAuthority()`
- `RequestAcquireCurrentInteractableOwner()`
- `ReplicatedSessionState`
- `RebuildClientSessionFromReplicatedState()`

这意味着，对联机场景来说，“当前会话到底正式承认哪些目标组件”不是一个 UI 展示问题，而是权限边界问题。

所以这套方案已经把单机 / 服务器 / 客户端的差异一起纳入考虑了，而且结论很明确：

- 单机能跑，不代表路径就是对的
- 只要服务端正式会话集合和客户端重建集合不一致，后面就会继续出现显示正常但操作异常、或者操作成功但同步不完整的问题
- 这种问题不能靠 UI 层补刷新或补判断，只能靠 session 注册集合统一收紧

## 方案补记：这套思路有没有把其他多人同步问题一起考虑进去

也有，但这里要说准确一点：这次方案不是已经把所有多人同步细节都实现完了，而是先把多人同步赖以成立的事实源统一起来。

这点非常关键。
如果会话自己都没有正式定义“哪些组件属于当前交互目标”，那后面所有多人同步问题都会继续悬空：

- 哪些区域应该有 snapshot
- 哪些区域应该在 OnRep 后重建
- 哪些区域应该 owner-only
- 哪些变更应该排队等权限
- 哪些目标组件变化需要驱动 UI 统一刷新

这些问题的前提，都是先要有同一份“当前 session 注册的目标组件集合”。

所以这次方案实际上已经把后面的多人网络同步问题一起纳入考虑了，只是它不是先在外层堆同步补丁，而是先把根收紧。

这里还要额外记一个当前阶段的风险：
如果后面只扩展了服务端 session 的注册集合，但 `ReplicatedSessionState` 仍然只复制 `ContainerSlots` 这一类单容器快照，那设计依然是不完整的。因为客户端重建会话时，恢复出来的仍然只是“单 container 事实”，而不是完整的目标库存上下文。

所以这次记录下来的最终结论是：

- 这套思路已经把库存操作在单机、服务器、客户端三种环境下的差异一起考虑进来了
- 也已经把后面的多人网络同步问题一起考虑进来了
- 但它现在仍然是“统一事实源的方案记录”，不是“所有同步实现都已经做完”
- 后面正式开工时，不能只改 UI 显示，必须把 session 注册、mutation 路由、复制快照和 client rebuild 当成同一件事一起改

## 方案补记：当前必须清理掉的几类不符合项

到这里还要再记一件非常实际的事。
这次不是只把方向说清楚就算完，当前代码里有哪些东西已经明显不符合这套方案，也必须先点出来。否则后面一开工，很容易又在旧路径上继续补。

### 第一类：UI 仍然在自己组目标侧上下文

这一类是最该先清理的，因为它直接破坏“SessionModel 是唯一目标事实源”这个前提。

现在 [`AOInventoryPageUI.cpp`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryPageUI.cpp) 里，`BuildTargetInventoryDisplayContext()` 仍然在自己拼一份目标上下文。表面上它已经不是直接对目标 Actor 做 `FindComponentByClass` 了，但它仍然在页面层决定：

- 目标页要拿哪些组件
- 这些组件怎么被组合成 `TargetInventoryDisplayContext`
- 子面板最终消费的是哪一套页面拼出来的上下文

这件事本身就不符合方案。
因为按我们现在定下来的边界，页面层不该拥有“目标页应该观察哪些目标组件”的决定权。页面层只能消费 SessionModel 已经正式注册好的结果，而不是自己再组一层目标视图上下文。

换句话说，`BuildTargetInventoryDisplayContext()` 这种函数的存在形式本身，就是后面要清理的对象。

### 第二类：SessionModel 里还在直接从 Actor 身上临时找组件

这类问题在当前阶段更隐蔽，但也更关键。

现在 [`AOContainerInteractionSessionModel.cpp`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.cpp) 里，下面这些接口都还在通过 `Interactable->FindComponentByClass<...>()` 临时找目标组件：

- `GetObservedQuickBarComponent()`
- `GetObservedFormalEquipmentSlotInventory()`
- `GetObservedFormalEquipmentManager()`
- `GetObservedSkillComponent()`
- `GetObservedSkillSlotInventory()`

这说明当前的 SessionModel 其实还没有真正“持有目标组件注册集合”，它只是：

- 正式持有了一个 `ObservedInventoryComponent`
- 然后对其他目标组件继续做按需查询

这仍然不符合方案。
因为这本质上还是“只有背包是正式会话成员，其他东西只是顺手从 Actor 身上再拿”。只要还是这种写法，QuickBar、FormalEquipment、Skill 这些目标组件就依然没有真正进入会话注册、观察、复制和 mutation 语义。

所以这一类要清理的不是某一个函数名，而是这整个模式：
**SessionModel 不允许继续用 `FindComponentByClass` 充当目标会话注册。**

### 第三类：单容器思维还写死在 SessionModel 的数据结构里

这个问题不清掉，后面很容易出现“看起来已经支持多块目标库存了，实际上底层还是单容器”。

现在 [`AOContainerInteractionSessionModel.h`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h) 和对应 cpp 里，核心状态仍然围着这几个名字转：

- `ObservedInventoryComponent`
- `ObservedContainerSlots`
- `GetCurrentContainerInventory()`
- `ApplyObservedSlotsSnapshot(...)`

这套命名和结构本身没有错，它对箱子是成立的。
但如果后面目标已经不是“只有一个正式容器”，那这些结构继续作为唯一核心，就会把实现再次拖回“多面板 UI，单容器底座”的老路。

这里不是说一定要为了抽象而大改命名，而是要明确：只要一个会话正式承认的目标组件集合已经不止一个，SessionModel 的内部状态就不能继续只让一个 `ObservedInventoryComponent` 代表全部目标事实。

所以这一类要清理的，是“单容器状态就是全部目标上下文”的默认前提。

### 第四类：复制状态仍然只承认单一 container snapshot

这一类是最直接的联机隐患。

现在 [`AOInteractionSessionComponent.cpp`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.cpp) 里，`UpdateReplicatedStateFromCurrentSession()` 复制出去的核心内容仍然是：

- `InteractableActor`
- `SessionWidgetClass`
- `ContainerSlots`

`RebuildClientSessionFromReplicatedState()` 恢复客户端会话时，走的也是：

- `InitializeContainerSession(ReplicatedSessionState.InteractableActor, nullptr)`
- `ApplyObservedSlotsSnapshot(ReplicatedSessionState.ContainerSlots)`

这说明当前 client rebuild 逻辑承认的仍然只是“一个 interactable + 一份 container slots snapshot”。

这与方案是直接冲突的。
因为如果服务端正式会话里已经注册了多个目标组件，而客户端重建时仍然只恢复 `ContainerSlots`，那客户端拿到的就不是同一份目标事实源。

所以这一类必须清理的，不只是字段名，而是这整个假设：
**复制状态不能继续只承认单一 container 快照。**

### 第六类：会话层只托管了权限申请，但没有托管正式执行入口

这一类遗漏非常关键，而且这次已经被实际日志直接打出来了。

当前角色库存交互在页面层和 session 路由层，看起来已经开始把 target-side 操作收进
`SubmitCurrentInteractableMutation(...)` 这条链了。
表面上像是：

- session 知道当前 interactable 是谁
- session 知道当前 mutation 是否命中了 target-side inventory
- session 也会先做 `RequestAcquireCurrentInteractableOwner()`

但如果 mutation 真正落地执行时，最后仍然回退到：

- `UAOInventoryComponent::ExecuteExchangeRequest(...)`
- `DraggedInventory->WhenItemExchange(...)`

那这条链的网络语义本质上还是旧的 inventory component RPC 语义，而不是正式的 session / interactable
语义。

这件事在单机或本地 listen server 上很容易被掩盖，但一到真正 client / server：

- 客户端最终还是在对目标角色身上的 `InventoryComponent` 直接发 `WhenItemExchange`
- 这个组件所属 Actor 并不一定对当前客户端拥有合法 owning connection
- UE 会直接报：
  `UNetDriver::ProcessRemoteFunction: No owning connection ... Function WhenItemExchange will not be processed`

也就是说，问题并不只是“owner 没申请到”，而是：

**当前实现只把“权限申请”接进了 session，但没有把“最终服务器执行入口”也一起收进 session。**

这正是为什么这里必须反复回到“角色操作箱子那套已经成熟的网络执行链”。
后面修正时不允许在这里另起一套新框架，也不允许为了绕过问题再造新的泛化 mutation 系统。
正确要求应该写死成下面这几条：

- 角色库存交互的网络执行入口，必须严格对齐现有“角色操作箱子”已成熟的权限执行链
- session 层不只负责识别 target-side inventory、请求 owner、排队 pending mutation
- session 层还必须负责把正式 mutation 提交到已有成熟入口，不能在最后退回 `InventoryComponent` 组件 RPC
- 只要一条 target-side 交换链最终仍然调用客户端侧 `InventoryComponent->WhenItemExchange`，就说明它还没有真正对齐箱子方案

换句话说，这次方案此前漏掉的一点必须补清楚：

**会话统一不只包括“谁属于当前 session”和“谁来拿权限”，还包括“谁代表这次 session 正式把交换提交到服务器执行”。**

## 方案补记：这次网络执行链修正的重点、范围、边界与约束

这部分要单独写清楚，避免后面开工时又把“看懂问题”和“重新发明一套东西”混在一起。

### 改动重点

这次真正要补的重点，不是继续扩 UI，也不是继续扩 target-side component 注册表本身，而是补齐下面这一段：

- 角色库存交互已经接入 session 的 target 注册与 owner 申请链
- 但真正的服务器执行入口仍然可能退回 `UAOInventoryComponent::ExecuteExchangeRequest(...)`
- 而 `ExecuteExchangeRequest(...)` 最后又会落回 `InventoryComponent->WhenItemExchange(...)`
- 这会把整条 target-side 交换重新拉回“组件 RPC 语义”，而不是“session / interactable 正式执行语义”

所以这次改动的真正重点应当写死成一句话：

**把 target-side inventory mutation 的最终服务器执行入口，对齐到现有“角色操作箱子”已经成熟的权限执行链。**

### 改动范围

这次允许改动的范围，要严格限制在“把角色库存交互对齐到箱子现有成熟网络解法”这件事本身：

- `UAOInteractionSessionComponent`
- `UAOContainerInteractionSessionModel`
- 角色库存交互当前命中的 mutation 路由入口
- 与“角色操作箱子”那条成熟执行链直接对应的现有交互入口

如果某处改动已经超出“让角色库存复用箱子成熟权限执行链”这个范围，就说明方向开始跑偏了。

### 边界

这次必须守住下面这些边界：

- 不新起一套通用 inventory transaction 框架
- 不为了这次问题新造泛化的 mutation 总线
- 不把策略继续下放到子 UI / 槽位 / 面板各自判断
- 不把“箱子现成方案”改造成另一套新方案，再要求角色库存去追它
- 不把问题解释成单纯 UI 刷新或单纯 owner acquire 时序问题

要明确：

- 这次要复用的是“角色操作箱子”已经存在且成熟的正式执行链
- 不是借箱子这个词，再重新设计一套看起来更抽象的新体系

### 约束

后面真正落代码时，必须同时满足下面这些约束：

- 角色库存 target-side 交换一旦进入 session 路由，就不能在最后退回客户端侧 `InventoryComponent->WhenItemExchange`
- 只要日志里还会出现 `No owning connection ... WhenItemExchange will not be processed` 这一类 target-side 组件 RPC 报错，就说明修正没有完成
- session 层要统一持有的不只是 target 注册集合和 owner 申请权，还包括正式 mutation 提交权
- 最终执行路径必须能被解释成“角色库存这次只是复用了箱子已有成熟链路”，而不是“角色库存又有了自己的一套特例执行口”

把这几条压成最后一句要求就是：

**这次修正只允许做“对齐已有成熟方案”，不允许做“借着修 Bug 再发明新体系”。**

## 方案补记：这次还漏掉的一层，不只是执行入口，还包括权限获取时机

这一点必须单独补记，因为它就是这次反复出现 `No owning connection` 的根因之一。  
前面那一轮讨论里，我们已经明确了一个问题：**不能让 target-side 交换最终又退回 `InventoryComponent->WhenItemExchange` 这种旧的组件 RPC 语义。**  
但到这里还不够，因为这只能解决“最后怎么执行”，还没有解决“这次会话什么时候正式拿到可变更权限”。

当前角色库存这条链里，`StartSession(...)` 做的事情仍然主要只是：

- 创建当前会话
- 激活 `SessionModel`
- 同步 OwnerOnly 的会话快照

它**没有像成熟箱子链那样，把“打开后马上进入可变更权限获取流程”这一层也一起对齐**。  
结果就是：

- 会话已经开了
- 目标侧 UI 也已经能显示
- 但当客户端第一次真正开始改目标库存时，权限获取和正式执行还在临时补救

这就会导致两类后果混在一起：

- 一类是“第一次勉强成功，后续来回操作开始不稳定”
- 另一类是“最后又掉回无 owning connection 的旧 RPC 报错”

所以这次方案要补清楚：

**会话统一不只包括 SessionModel 统一、target 注册统一、mutation 提交统一，还包括权限获取时机也必须统一。**

换句话说，后面落代码时不能只修“最终服务器执行入口”，还必须把下面这件事一起对齐：

- 打开目标库存会话以后，后续所有 target-side 可变更操作都必须站在已经对齐好的权限获取链上
- 不能继续靠“操作发生时再临时碰碰运气申请 owner，再看最后能不能发出去”

如果这层不补上，就算交换入口写对了，后面仍然会继续出现：

- 会话已经打开，但 mutation 资格还没稳定进入正式链
- 某些操作还能走，某些操作又退回旧链
- 联机时继续出现 owner / owning connection 类问题

这里要明确，**这不是额外发明一层新权限系统**。  
这次要做的仍然只有一件事：**把角色库存这条链，对齐到项目里“角色操作箱子”已经跑通的成熟权限获取时机。**

## 方案补记：BUG2 不是 UI 接错库存，而是目标侧操作串回了自用链

这一点也必须提前写死，避免后面又把问题误判成“目标面板绑错组件”。  
当前用户已经明确复测过：

- BUG2 不是“目标物品栏其实显示的是玩家自己的库存”
- 目标侧显示本身可以是对的
- 但在操作目标侧物品栏时，会莫名其妙触发玩家自身那条 `TryUseItemAtSlot(...)` / 自用链

所以 BUG2 的正式排查边界要锁成下面这几条：

第一，不再从“是不是 UI 绑错库存组件”这个方向继续空转。  
这不是本次主要矛盾。

第二，要完整追 target-side `Use` 行为链，而不是只看右键菜单字面表现。  
重点要查的是：

- `UAOInventoryUI::RequestUseInventoryItem(...)`
- `ShouldRouteInventoryUseThroughInteractionSession(...)`
- `UAOInventoryComponent::TryUseItemAtSlot(...)`
- `ServerTryUseItemAtSlot(...)`
- `UserPawn` 最终是怎么解析的

第三，要把“显示谁的数据”和“最后以谁的身份执行 Use”当成同一个问题看。  
如果目标侧 UI 显示的是目标库存，但最后 `Use` 仍然落到：

- 玩家自己身上的组件
- 或者玩家自己默认解析出来的 `UserPawn`

那本质上还是同一类问题：**目标侧显示链和正式执行链没有完全对齐到同一个会话上下文。**

第四，BUG2 的修法也必须沿用这次总方案的同一条边界：

- 不给每个子 UI 单独加目标/自用判断
- 不在槽位控件里补特判
- 不新增 panel id、widget id、对象类型分支
- 只允许把目标侧 `Use` 行为重新收回正式会话链

把这一节压成一句话就是：

**BUG2 不是“UI 绑错”，而是“目标侧操作仍然有一部分执行语义偷偷回到了玩家自用链”。**

### 第五类：子面板仍然保留本地 fallback 取件路径

这类东西现在看起来像是“只是兜底”，但从统一路径角度看，它其实也是残留分叉。

例如 [`AOBackPackUI.cpp`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOBackPackUI.cpp) 和 [`AOQuickBarUI.cpp`](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOQuickBarUI.cpp) 里，仍然能看到对 `OwningPawn->FindComponentByClass<...>()` 这种本地 fallback 的依赖。

对自背包页来说，这种路径短期内不一定有问题，因为 self context 本来就是本地拥有者。
但从“统一打开库存对象路径”的角度，它会继续培养一种坏习惯：Widget 自己也可以去推断该看谁、该拿谁。

所以后面正式清理时，要把原则写死：

- self context 可以保留本地构建
- target context 不允许保留 widget fallback
- 任何 target-side panel 都不允许在 session 之外重新推断目标组件

### 这一节最后压成一句话

当前最该清理的，不是某个零散 bug 判断，而是下面这几种不符合方案的旧习惯：

- 页面层自组 target context
- SessionModel 临时从 Actor 身上找“顺手组件”
- 底层状态仍把单容器当成全部目标事实
- 复制状态仍只承认单一 `ContainerSlots`
- target 子面板保留本地 fallback 取件路径

这些东西只要还留着，后面无论是角色库存、工作台库存、交易库存，还是别的可打开库存对象，都会继续长出“入口像统一了，内部其实又各走各路”的分叉。

- [AOChest.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/Containers/AOChest.cpp)
- [AOCharacter.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp)
- [AOContainerInteractionSessionModel.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h)
- [AOContainerInteractionSessionModel.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.cpp)
- [AOInteractionSessionComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.cpp)
- [AOInventoryPageUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryPageUI.cpp)
- [AOInventoryUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp)
- [AOContainerUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOContainerUI.cpp)
- [AOContainerSlot.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOContainerSlot.cpp)
- [AOBackPackUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOBackPackUI.cpp)
- [AOQuickBarUI.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Common/Inventory/AOQuickBarUI.cpp)
