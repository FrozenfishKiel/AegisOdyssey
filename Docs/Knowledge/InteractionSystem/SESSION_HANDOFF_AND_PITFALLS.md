---
title: Interaction Session Handoff And Pitfalls
tags:
  - knowledge
  - interaction-system
  - handoff
  - pitfalls
aliases:
  - Interaction Session Handoff And Pitfalls
  - 交互系统会话交接与避坑记忆
---

# 交互系统会话交接与避坑记忆

更新时间：2026-05-19  
适用范围：当前交互系统里“统一交互 + 会话层 + 容器 UI + owner 权限等待 + 输入链排查顺序”这几类已经验证过的长期记忆。  
不适用范围：所有未来对象类型的完整功能清单、所有单次排查细节原文。

## 1. 这份文档记录什么

这份文档不替代正式主骨架文档。

它主要记录三类东西：

1. 当前已经稳定下来的交互系统硬约束。
2. 已经踩过、后续不要再重走的错误方向。
3. 当前若再次出现交互/拾取异常，推荐从哪条链开始排查。

## 2. 当前已经稳定下来的硬约束

### 2.1 箱子只是样板，不是专用主链

当前交互系统的目标从一开始就不是“把箱子做通就结束”，而是：

1. 箱子
2. 工作台
3. 铁砧
4. 商店
5. AI 可交互对象
6. 其他任何需要打开 UI 或修改对象数据的世界对象

因此当前默认规则是：

- 不围绕“箱子专用判断”扩主链。

### 2.2 会话层只解决“当前谁在和对象交互”

会话层当前正式职责是：

1. 标记当前玩家正在交互哪个对象。
2. 管理当前会话模型。
3. 在修改对象数据前协调 owner / authority。

它不是：

1. UI 树检索器。
2. 对象类型分发器。
3. 第二套对象真相层。

### 2.3 UI 既是展示层，也是标准化操作请求入口

当前已经成立的方向是：

1. UI 读取会话模型和观察快照。
2. UI 发起标准化数据操作请求。
3. 服务端处理权威逻辑。
4. 结果再通过会话快照回到 UI。

### 2.4 修改当前交互对象数据必须先过统一 Mutation 链

当前默认入口是：

- `SubmitCurrentInteractableMutation(...)`

以及：

- `ExecuteOrQueueCurrentInteractableMutation(...)`

后续凡是：

1. 交换
2. 使用
3. 删除
4. 丢弃
5. 整理
6. 拆分
7. 合并

只要是在“当前交互对象会话内修改对象数据”，默认都不应再绕开这条链另起一套权限等待逻辑。

## 3. 当前已经验证过的关键现状

### 3.1 owner-only 会话快照已经是现行结构

当前已确认：

1. `ReplicatedSessionState` 走 `COND_OwnerOnly`
2. `ContainerSlots` 放在会话快照里
3. 客户端通过 `RebuildClientSessionFromReplicatedState()` 重建本地容器会话

因此不要再把“所有相关客户端默认共享容器详细内容”写成当前事实。

### 3.2 AI 的非 UI 正式入口已经有雏形

当前箱子对象已经提供：

- `TransferItemToInteractorInventory(APawn* InteractingPawn, ...)`

这说明当前正式方向是：

1. 玩家可以通过 UI / 会话访问。
2. AI 或无 UI 调用方应走对象侧正式入口。

### 3.3 统一交互能力当前是“对象优先 + 旧链回退”

`UAOGameplayAbility_Interact::TriggerInteractionByIndex(...)` 当前顺序已核实为：

1. 组装统一交互事件数据
2. 优先 `UInteractionStatics::TryExecuteInteraction(...)`
3. 对象侧没接住时，再回退到旧能力事件链

所以当前不能误写成：

1. 旧链已经完全下线
2. 所有交互对象都只剩最终形态

## 4. 已经踩过、后续不要再重走的方向

### 4.1 不要再围绕对象类型扩玩家侧判断

不要再写：

1. 这是箱子
2. 这是工作台
3. 这是按钮
4. 玩家自己决定打开哪套专用 UI

当前稳定方向是：

1. 玩家负责发起交互。
2. 对象自己决定行为和 UI。

### 4.2 不要再通过遍历 UI 树找当前容器

当前已经明确否掉：

1. 遍历 Layout 找 UI
2. 根据 UI 类型反推当前对象

当前应继续围绕：

1. `InteractionSessionComponent`
2. `SessionModel`
3. 当前交互对象本体

来理解“谁正在和谁交互”。

### 4.3 不要再用初始化/重置数据层掩盖 UI 问题

这类问题如果根因在：

1. 会话层
2. 同步时机
3. 观察刷新

就不该回头通过重置对象数据或重新初始化库存来止血。

### 4.4 不要再靠额外布尔锁掩盖双触发或权限时序问题

如果一次交互会双触发，优先问的是：

1. 为什么存在两条并行主链
2. 为什么一次输入走了两条路

而不是先加：

1. 临时锁
2. 临时门闩
3. 某类对象专用布尔状态

## 5. 当前拾取链仍是需要警惕的高风险入口

这两篇文里最重要的非结构性提醒是：

当前交互系统不只要看容器会话，还要继续警惕“统一交互接入过程中把原本稳定的拾取链改坏”这条风险。

因此如果后续再出现：

1. 客户端按 `E` 不拾取
2. 单机或服务器重复进包
3. 某阶段交互完全不进输入句柄数组

当前更推荐的排查顺序是：

1. `UAOHeroComponent::Input_AbilityInputTagPressed`
2. `UAOHeroComponent::Input_AbilityInputTagStarted`
3. `UAOAbilitySystem::AbilityInputTagPressed`
4. `UAOAbilitySystem::AbilityInputTagStarted`
5. `UAOAbilitySystem::GatherAbilityHandlesForInputTag(...)`
6. 最后再看 `ProcessAbilityInput(...)`

也就是说，先查“句柄为什么没收进来”，再查“输入泵为什么没消费”。

## 6. 当前关联文档

- [[交互系统项目地图]]
- [[交互系统已锁定设计]]
- [[交互系统Mutation与容器同步]]
- [[交互系统已知边界与历史偏差]]
