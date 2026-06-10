---
title: Interaction System Decisions
tags:
  - knowledge
  - interaction-system
  - decisions
aliases:
  - Interaction System Decisions
  - 交互系统已锁定设计
---

# 交互系统已锁定设计
更新时间：2026-05-19  
适用范围：当前交互系统第一轮深提炼后已经可以当作稳定结论使用的设计边界。  
不适用范围：未来尚未落地的 SmartObject/StateTree 具体接线细节。

## 1. 交互对象负责规则与状态，玩家/AI 只是发起方

这是当前最重要的边界。  
后续无论扩展容器、按钮、拉杆还是工作台，都默认遵守：

**交互对象本体负责规则与状态，玩家或 AI 负责发起交互，UI 只消费标准化数据。**

这意味着：

- 交互对象自己的可用性、占用、锁定、访问条件，不应散落在 UI 或角色侧。
- 玩家与 AI 应尽量共享对象侧交互规则。
- UI 不应变成对象状态的第二写入口。

## 2. 交互主链必须立足现有工程结构，不另起第二套系统

当前已确认的复用方向是：

- 交互入口复用 `Interact Ability + IInteractableTarget + InteractionOption`
- 容器数据复用 `UAOInventoryComponent`
- UI 数据表达复用 `SessionModel -> ViewModel`

因此当前锁定的方向不是：

- 为箱子单独发明一套专用交互入口
- 为容器内容单独发明一套临时数组状态
- 为容器 UI 绕开现有会话组件，再写一套权限等待链

## 3. “访问对象”与“修改对象”必须分层

当前容器样板已经表明：

- `ExecuteInteraction(...)` 负责建立交互会话
- 具体交换、使用、删除等动作属于会话内 mutation 或业务动作

所以当前正式边界是：

- “打开/访问容器”不等于“修改容器数据”
- “建立观察关系”不等于“立即执行库存变更”

这个边界后续对 AI 也同样重要，因为 AI 可能直接调用对象侧业务入口，而不是先打开 UI。

## 4. 容器详细内容默认按会话同步，不按全员广播同步

当前代码已经落地的关键结论是：

**容器详细内容不默认复制给所有相关客户端，而是通过当前会话快照复制给会话拥有者。**

对应事实：

- `UAOInteractionSessionComponent` 的 `ReplicatedSessionState` 使用 `COND_OwnerOnly`
- 容器格子快照保存在 `ReplicatedSessionState.ContainerSlots`
- 客户端通过 `RebuildClientSessionFromReplicatedState()` 重建本地容器会话

这说明历史文档里的“观察者订阅式同步”在当前实现中已经变成了：

- 服务端维护真实观察者
- 对当前会话拥有者复制会话快照
- UI 基于该快照刷新

## 5. 当前 Mutation 权限等待必须统一收口到会话组件

当前已锁定结论是：

**凡是会修改“当前交互对象数据”的动作，都应优先复用 `SubmitCurrentInteractableMutation(...)` 这条统一调度链。**

当前链路已经包含：

- 写权限是否已具备
- 未具备时是否挂起请求
- 何时申请当前对象 owner
- owner 到位后何时放行

这意味着后续新增：

- 容器交换
- 容器内物品使用
- 未来删除、丢弃、整理、拆分、合并

都不应各自再写一套“等待 owner / 队列 / 放行”逻辑。

## 6. 会话模型是对象状态的观察模型，不是第二套权威状态

当前容器会话模型 `UAOContainerInteractionSessionModel` 的正式定位已经明确：

- 它负责整理会话看到的容器快照
- 它负责驱动容器 UI 消费
- 它不是容器数据本体

所以后续扩展时默认遵守：

- 真正对象状态还是在对象侧正式组件里
- SessionModel 只表达“这个会话当前看到了什么”
- 不要把会话模型反向膨胀成对象主状态机

## 7. AI 应复用对象侧正式入口，而不是复用玩家 UI

当前箱子已经提供：

- `TransferItemToInteractorInventory(APawn* InteractingPawn, ...)`

这说明当前正式方向是：

- AI 应走对象侧或对象授权的正式业务入口
- AI 不应依赖玩家容器 UI 才能与对象交互

当前玩家与 AI 的主要区别应是：

- 谁发起交互
- 是否需要 UI 会话

而不应是：

- 一套给玩家的对象规则
- 一套给 AI 的另一套对象规则

## 8. 观察者刷新由对象侧主动驱动

当前容器链已经锁定一个重要设计：

- `UAOContainerInventoryComponent::BroadCastInventoryChange(...)`
- `AAOChest::RefreshObservers()`
- `UAOContainerInteractionSessionModel::RefreshObservedContainer()`

也就是：

**对象数据变更后，由对象侧主动刷新观察者会话，而不是要求 UI 自己轮询容器状态。**

这个方向后续应继续保持。

## 9. 交互系统后续接手默认先沿“对象侧 -> 会话侧 -> 输入链”排查

已经锁定：

1. 如果问题是对象状态、权限等待、观察刷新不对，优先查对象侧和 `InteractionSessionComponent`。
2. 如果问题是按键交互、拾取、句柄没有收进来，优先查 `HeroComponent -> AbilitySystem 输入标签收集链`。

不再建议：

1. 一上来先从 UI 树或蓝图层乱补。
2. 先在 `ProcessAbilityInput(...)` 末端硬补激活，再倒推根因。

这一点后续继续接手交互系统时，默认参考 [[交互系统会话交接与避坑记忆]]。
## 10. Target Inventory Session Rules

2026-05-29 之后，目标库存交互还需要继续遵守一组更具体的规则：

- 目标库存操作的核心不是打开了哪个 UI，而是当前会话是否已经正式拿到目标 mutation 资格。
- 客户端操作目标库存时，必须继续遵守“发请求 -> 等授权 -> 授权到位后放行 mutation”。
- 箱子链路只是成熟样板，不是专用特例；角色库存、工作台库存、交易库存都应尽量复用同类路径。
- UI 只消费会话给出的目标上下文，不再自己拼第二份目标事实源。
- 只要最后还在让客户端直接调目标库存组件旧 RPC，这条链就还没有真正统一。

展开说明见：[[TARGET_INVENTORY_SESSION_RULES_2026-05-29]]
