---
title: Interaction System Known Issues
tags:
  - knowledge
  - interaction-system
  - known-issues
aliases:
  - Interaction System Known Issues
  - 交互系统已知边界与历史偏差
---

# 交互系统已知边界与历史偏差
更新时间：2026-05-19  
适用范围：当前交互系统第一轮深提炼中已经识别出的文档混层、实现边界和后续整理风险。  
不适用范围：完整运行时 bug 列表。

## 1. 当前文档混层点

### 1.1 历史交互文档同时混有协作基线、设计原则、样板对象方案

这一批历史文档里，至少混了三类内容：

- 交互系统协作基线
- Mutation 统一调度设计
- 箱子/容器样板对象的多人同步方案

这些内容都重要，但不应继续混写成一篇“交互总文档”。  
本轮已经把它们拆成：

- [[交互系统项目地图]]
- [[交互系统已锁定设计]]
- [[交互系统Mutation与容器同步]]

### 1.2 容器样板对象很容易被误写成“交互系统全部真相”

当前箱子只是第一条已落地样板链。  
它能说明：

- 会话结构
- 容器数据复用方式
- 观察者刷新方向

但它不等于：

- 所有交互对象未来都必须长成“带库存的箱子”

后续按钮、拉杆、工作台的系统骨架应复用当前主链，但对象侧状态模型可以不同。

## 2. 当前最容易误判的实现边界

### 2.1 不要再把容器详细内容当成“默认广播给所有相关客户端”

当前代码不是这样。  
已确认的当前事实是：

- `ReplicatedSessionState` 走 `COND_OwnerOnly`
- 容器详细格子快照放在 `ContainerSlots`
- 客户端会重建自己的本地容器会话

因此当前知识库里如果再写“所有附近玩家都会默认同步完整容器内容”，就会污染知识库。

### 2.2 不要把 `SubmitCurrentInteractableMutation(...)` 写成“还只是计划中的公共链”

当前这条公共链已经存在于代码中，并且 `AOInventoryUI` 已经在使用。  
所以当前应表述为：

- 统一 Mutation 调度链已落地

而不是：

- 未来准备这么做

### 2.3 不要把 UI 当作容器状态真相层

当前不是：

- `AOContainerUI` 和 `AOContainerSlot` 不持有权威状态
- 它们消费的是会话模型和观察快照

如果后续整理文档时把容器 UI 写成“容器系统主状态层”，就会把设计边界写反。

### 2.4 不要把玩家路径和 AI 路径写成两套对象规则

当前箱子已经存在：

- 玩家通过会话 + UI 访问
- AI 或无 UI 调用方通过对象侧正式入口访问

这说明当前正确理解是：

- 规则尽量共用
- 入口路径可以不同

## 3. 当前尚未继续展开、但价值很高的后续主题

以下内容已经露出稳定结构，但本轮没有继续深提炼：

1. `InteractionSystem` 的进度交接文档与长期避坑清单
2. 非容器类交互对象如何接入当前会话主链
3. 容器内删除、丢弃、整理、拆分、合并等动作是否已全部切到统一 Mutation 调度
4. AI / StateTree / SmartObject 如何正式接到当前交互对象主链
5. 会话关闭、owner 释放、异常中断时的边界行为

这些内容后续适合再开新一轮深提炼。

## 3.1 当前已确认的交接/避坑误判点

### 3.1.1 不要把“当前会话快照已到”当成“当前对象 owner 已到”

这条很容易误判。

当前统一等待链存在的原因正是：

1. 会话复制状态可能先到
2. 但对象 `Owner` 还没真正同步到当前玩家

因此当前是否具备 mutation authority，仍要以：

- `InteractableActor->GetOwner() == SessionOwnerActor`

为准，而不是以“UI 已经打开”或“会话快照已到”为准。

### 3.1.2 不要把 `SubmitCurrentInteractableMutation(...)` 理解成“容器专用补丁”

当前不是。

它已经是会话内修改当前交互对象数据的统一调度入口，后续应继续扩给更多对象和动作，而不是把它理解成“为箱子临时加的一段等待逻辑”。

### 3.1.3 不要把拾取问题直接归因为 `ProcessAbilityInput(...)` 尾部没跑

当前这条也不能先默认成立。

历史排查已经明确提醒过：

1. 问题可能更早出在 `InputTag -> AbilityHandle` 收集链
2. `GatherAbilityHandlesForInputTag(...)` 为空时，后面输入泵当然也不会消费

所以后续排查交互/拾取异常，不要直接跳过前面的输入标签收集阶段。

## 4. 当前整理规则

后续继续往 `Docs/Knowledge/InteractionSystem` 提炼时，默认遵守：

1. 先区分“交互主链”“容器样板链”“AI 后续接线”三层，不要混写。
2. 任何涉及“当前是否已经有统一 Mutation 等待链”的说法，优先核 `AOInteractionSessionComponent.*`。
3. 任何涉及“容器详细内容同步给谁”的说法，优先核 `ReplicatedSessionState` 和 `AOContainerInteractionSessionModel.*`。
4. 任何涉及“观察者如何刷新”的说法，优先核 `AAOChest::RefreshObservers()` 与 `UAOContainerInventoryComponent::BroadCastInventoryChange(...)`。
5. 不把箱子样板直接扩写成所有交互对象的未来事实。
6. 任何涉及“当前拾取/交互按键为什么失效”的说法，优先先核 `HeroComponent -> AbilitySystem 输入标签收集链`，再看输入泵末端。
