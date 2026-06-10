---
title: Interaction System Project Map
tags:
  - knowledge
  - interaction-system
  - project-map
aliases:
  - Interaction System Project Map
  - 交互系统项目地图
---

# 交互系统项目地图
更新时间：2026-05-19  
适用范围：当前项目里“玩家或 AI 与可交互对象建立会话，并在会话内读写对象数据”的主链，重点覆盖容器/箱子这条已落地样板链。  
不适用范围：单个具体交互对象的玩法细节、美术表现、未来尚未落地的 SmartObject/StateTree 接线方案。

## 1. 这份文档解决什么问题

这份文档只回答下面几件事：

1. 当前交互系统的正式主链是什么。
2. 交互对象、交互会话、UI、对象数据同步分别落在哪里。
3. 哪些层是“规则与状态真相层”，哪些层只是观察或消费层。
4. 后续继续扩展按钮、拉杆、工作台、容器时，先看哪些代码入口。

## 2. 当前正式主链

当前交互系统已经不是“交互能力直接调 UI”这一类临时链路。  
以当前已落地的容器/箱子为例，正式主链是：

**`Interact Ability -> IInteractableTarget / InteractionOption -> Object-side ExecuteInteraction -> InteractionSessionComponent -> SessionModel -> Object State / Inventory -> OwnerOnly Session Snapshot -> UI`**

如果进一步拆开，当前链路可以理解为：

1. `UAOGameplayAbility_Interact` 负责扫描目标、维护可交互选项，并在玩家触发时把事件发给目标对象能力链。
2. 交互对象通过 `IInteractableTarget::GatherInteractionOptions` 暴露标准化交互入口。
3. 目标对象在 `ExecuteInteraction(...)` 中创建或切换会话，而不是让 UI 直接决定对象状态。
4. `UAOInteractionSessionComponent` 持有当前会话，并负责会话复制、交互对象 owner 获取、Mutation 排队与放行。
5. `UAOContainerInteractionSessionModel` 负责把容器对象的当前观察数据整理成会话数据快照。
6. `UAOContainerUI` / `UAOContainerSlot` 只消费会话模型和观察快照，不直接成为对象真相层。

## 3. 当前各层职责

### 3.1 交互入口层

定义位置：

- `Source/AegisOdyssey/Interaction/Abilities/AOGameplayAbility_Interact.*`
- `Source/AegisOdyssey/Interaction/InteractableTarget.h`
- `Source/AegisOdyssey/Interaction/InteractionOption.h`
- `Source/AegisOdyssey/Interaction/InteractionStatics.*`

这一层只回答：

- 当前玩家面前有哪些可交互目标。
- 每个目标暴露了哪些交互选项。
- 玩家最终触发的是哪个交互选项。

它不负责：

- 持有具体对象的长期状态。
- 决定容器内容如何同步给谁。
- 直接修改当前交互对象数据。

### 3.2 交互对象层

当前已确认的样板对象：

- `Source/AegisOdyssey/Interaction/Containers/AOChest.*`

这一层负责：

- 作为世界中的可交互对象本体存在。
- 通过 `IInteractableTarget` 暴露交互入口。
- 在对象侧处理 `CanExecuteInteraction(...)` 和 `ExecuteInteraction(...)`。
- 持有对象自己的规则与状态载体，例如容器库存组件。

对当前箱子来说，`AAOChest` 已经明确不是“UI widget 的附属逻辑”，而是：

**`可交互容器 Actor + IInventoryInterface 提供者 + 观察者会话管理者`**

### 3.3 会话组件层

定义位置：

- `Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.*`

这是当前交互系统运行时总入口之一。  
它负责：

- 持有 `CurrentSessionModel`
- 切换/关闭当前交互会话
- `OwnerOnly` 复制当前会话状态
- 申请/释放当前交互对象 owner
- 统一提交、挂起、放行“修改当前交互对象数据”的 Mutation

当前这层已经具备稳定边界，不应再把具体业务逻辑塞回 UI 或单个请求函数里。

### 3.4 会话模型层

定义位置：

- `Source/AegisOdyssey/Interaction/Session/AOInteractionSessionModel.*`
- `Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.*`

这层负责把“交互对象当前暴露给某个会话的可观察状态”整理成统一模型。  
对容器会话来说，它当前负责：

- 记录当前被观察的交互对象
- 解析出当前容器库存组件
- 维护 `ObservedContainerSlots`
- 生成供 UI 使用的 `ContainerViewModel`
- 在服务端刷新后，把当前快照同步回 `InteractionSessionComponent`

这说明当前正式结构已经是“对象状态 -> 会话观察模型 -> UI 消费”，而不是“对象状态 -> UI 直读”。

### 3.5 对象数据层

当前容器样板的对象数据层：

- `Source/AegisOdyssey/Interaction/Containers/AOContainerInventoryComponent.*`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.*`

这里的关键事实是：

- 容器内容没有另起一套临时数组结构。
- 当前容器内容继续复用了正式的 `UAOInventoryComponent` 体系。
- `UAOContainerInventoryComponent` 在库存变更时会反向通知 `AAOChest::RefreshObservers()`

因此当前容器系统是“交互层复用库存系统”，不是“为了交互再发明第二套容器数据结构”。

### 3.6 UI 消费层

定义位置：

- `Source/AegisOdyssey/UI/Common/Inventory/AOContainerUI.*`
- `Source/AegisOdyssey/UI/Common/Inventory/AOContainerSlot.*`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.*`

这一层负责：

- 绑定当前玩家的 `InteractionSessionComponent`
- 监听当前会话变化
- 如果当前会话是容器会话，则重绑定 `UAOContainerInteractionSessionModel`
- 用 `ObservedContainerSlots` 重建容器格子
- 在需要修改当前交互对象数据时，通过 `SubmitCurrentInteractableMutation(...)` 提交请求

它不应负责：

- 自己判断何时拥有交互对象写权限
- 自己维护等待队列
- 直接成为容器状态权威层

## 4. 当前真相层与观察层

### 4.1 真相层

当前应视为真相层的有：

- 交互对象本体，例如 `AAOChest`
- 对象持有的正式数据组件，例如 `UAOContainerInventoryComponent`
- 会话运行时控制入口 `UAOInteractionSessionComponent`
- 对象侧 `CanExecuteInteraction(...)` / `ExecuteInteraction(...)`

### 4.2 观察层 / 消费层

当前应视为观察层或消费层的有：

- `UAOContainerInteractionSessionModel`
- `FAOObservedInventorySlot`
- `UMVVM_InventoryMenu`
- `UAOContainerUI`
- `UAOContainerSlot`

这条边界非常重要：

- 会话模型和 UI 可以表达对象当前对这个会话暴露出来的状态。
- 但它们不是第二套容器真相。

## 5. 当前同步模型

当前容器交互链已经明确采用两层同步：

### 5.1 世界对象状态

例如：

- 箱子 Actor 是否存在
- 箱子是否可交互
- 箱子 owner 是否已切到当前交互者

这类状态继续走 Actor / Component 的正常网络模型。

### 5.2 会话级详细内容快照

例如：

- 当前容器格子列表
- 格子中的实例与堆叠数

这部分当前不默认广播给所有相关客户端，而是：

**由服务端会话模型整理成快照，再通过 `UAOInteractionSessionComponent` 以 `OwnerOnly` 方式复制给当前会话拥有者。**

这就是当前容器详细内容同步的正式结构。

## 6. 当前推荐阅读顺序

如果后续继续接交互系统，当前推荐阅读顺序是：

1. `Interaction/Abilities/AOGameplayAbility_Interact.*`
2. `Interaction/InteractableTarget.h`
3. `Interaction/AOInteractionSessionComponent.*`
4. `Interaction/Session/AOInteractionSessionModel.*`
5. `Interaction/Session/AOContainerInteractionSessionModel.*`
6. `Interaction/Containers/AOChest.*`
7. `Interaction/Containers/AOContainerInventoryComponent.*`
8. `UI/Common/Inventory/AOContainerUI.*`
9. `UI/Common/Inventory/AOContainerSlot.*`
10. `UI/Widgets/Inventory/AOInventoryUI.*`

## 7. 第一轮提炼来源

本轮主要从下面三篇历史文档提炼，并与当前代码核对：

- `Notice/HistoryNotice/交互对象系统工作方案与协作基线.md`
- `Notice/HistoryNotice/交互系统Mutation权限等待与统一调度设计说明.md`
- `Notice/HistoryNotice/联机交互箱子设计方案-玩家共享可见-AI可用-UI与同步策略.md`

当前已沉淀出的稳定结果分别落在：

- [[交互系统已锁定设计]]
- [[交互系统Mutation与容器同步]]
- [[交互系统已知边界与历史偏差]]

## 8. 当前已补出的交接 / 长期记忆主题

除了主骨架、Mutation 与容器同步链，当前这一包还应继续关联一条“长期记忆与排查顺序”主题：

- [[交互系统会话交接与避坑记忆]]

它解决的问题不是对象状态真相本身，而是：

1. 后续接手者先要继承哪些硬约束。
2. 哪些错误方向已经被明确否定。
3. 如果再次出现交互/拾取异常，先从哪条输入链和会话链排查。
