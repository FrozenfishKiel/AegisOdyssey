---
title: StateTree Events And SmartObject Boundary
tags:
  - knowledge
  - gameplay-framework
  - statetree
  - smartobject
aliases:
  - StateTree Events And SmartObject Boundary
  - StateTree 事件与 SmartObject 接线边界
---

# StateTree 事件与 SmartObject 接线边界

更新时间：2026-05-19  
适用范围：当前项目里 `StateTree` 事件注入链、事件负载边界、以及 `SmartObject` 是否已经进入工程这一层框架事实。  
不适用范围：敌人 AI 决策值、追击/巡逻状态真相、具体 StateTree 资产排布和策划参数。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前项目里的 `StateTree` 输入事件到底是怎么被送进去的。
2. `FStateTreeEvent.Tag` 和 `Payload` 在当前项目里分别承载什么。
3. 当前项目有没有真的接入 `SmartObject`。
4. 如果后续要接 `SmartObject`，当前工程已经具备哪些前提。

## 2. 当前 `StateTree` 事件链已经落地

优先看：

- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.*`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.*`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.*`
- `Source/AegisOdyssey/Character/AOHeroComponent.*`
- `Source/AegisOdyssey/Character/AOInputBufferComponent.*`

当前已经确认的正式链路是：

1. `UAOStateTreeComponentBase` 作为项目基类存在，默认 `bStartLogicAutomatically = false`，并启用复制
2. `UAOCombatStateTree` 和 `UAOCombatLocomotionStateTree` 在 `InitializeComponent()` 里监听 `HeroComponent` 与 `InputBufferComponent` 的输入委托
3. 委托回调统一进入 `CallStateTreeToSentEvent(...)`
4. 组件内部构造 `FStateTreeEvent`
5. 最后通过 `SendStateTreeEvent(Event)` 把事件送进运行中的树

这说明历史文档里的“StateTree 事件系统”在当前项目里不是纯教程，而是已经被用于战斗/输入侧的正式接线。

## 3. 当前事件语义怎么分层

优先看：

- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.cpp`

当前代码里，事件语义被拆成两层：

1. `FStateTreeEvent.Tag`
2. `FStateTreeEvent.Payload`

当前实际写法是：

1. `Event.Tag = InTargetTag`
2. `Payload.InitializeAs<FCombatStateTreeInputEvent>(FCombatStateTreeInputEvent(InTargetTag, InInputType))`
3. `Event.Payload = FConstStructView(Payload)`

但要特别注意一个当前事实：

`FCombatStateTreeInputEvent` 现在只真正保存了 `InputType`。  
它的构造函数虽然接收 `InInputTag`，但结构体内部并没有把 Tag 存成成员字段。

因此当前正确理解是：

1. 输入标签语义走 `Event.Tag`
2. 输入类型语义走 `Payload.InputType`

不要把这套当前实现误写成“Payload 里完整复制了一份 Tag + InputType”。

## 4. 当前这套事件链更像“统一注入入口”，不是完整 C++ 事件消费框架

本轮代码核对里，我没有在项目 C++ 层看到这类直接消费：

- `GetCurrentEvent()`
- `Payload.GetPtr<FCombatStateTreeInputEvent>()`

至少在当前代码面上，已经明确存在的是“发送侧”和“负载结构定义”。

这意味着当前稳定结论应写成：

1. 项目已经有正式的 `StateTree` 事件注入入口
2. 但这轮没有证实项目在 C++ 侧普遍用 `GetCurrentEvent()` 直接解析这份负载
3. 事件的实际消费更可能落在 StateTree 资产运行时匹配和现有节点流程里

如果后续要继续查 AI 运行时消费链，应转到 [[StateTree AI 项目地图]]，不要把这份框架笔记当成完整 AI 行为图地图。

## 5. 当前 `StateTree` 相关基础设施已经具备哪些事实

除了事件注入链，本轮还核对到几个和后续框架接线有关的前提：

1. `GameplayStateTreeModule` 和 `StateTreeModule` 已进入 `.uproject` 和 `Build.cs`
2. `UAOAILogicStateTreeComponentBase` 支持 `DefaultStateTree` 自动挂载
3. `AAOAIPlayerBotController::OnPossess()` 里有针对动态添加 `StateTree` 组件的 `RestartLogic()` 补偿链
4. `UAOEquipmentFeatureAction_AddComponents` 里也有对动态加上来的 `StateTree` 组件执行 `RestartLogic()` 的补偿
5. 项目已经有正式的 `FSTT_MoveToLocation` 通用任务，可读取 `GoalLocation` 并驱动 `UAITask_MoveTo`

这些都不是 SmartObject，但它们决定了项目以后若要接新的“世界资源交互层”，当前基础并不空白。

## 6. 当前 `SmartObject` 还没有进入工程

这一点在代码和工程配置层都已经核过。

当前已确认：

1. `AegisOdyssey.uproject` 里没有启用 `SmartObjects` 或 `GameplayBehaviorSmartObjects`
2. `AegisOdyssey.Build.cs` 里没有 `SmartObjectsModule` 或 `GameplayBehaviorSmartObjectsModule`
3. `Source/` 和 `Config/` 里没有当前项目对 `SmartObject` API 的引用
4. `Content/` 下也没有检出 `SmartObject` 相关资产

所以当前项目事实只能写成：

**`SmartObject` 目前仍是未接线的框架候选，不是当前运行时主链的一部分。**

## 7. 历史 `SmartObject` 文档当前应该怎么用

那篇历史文档有价值，但当前更适合被当成：

1. UE5 `SmartObject` 机制学习资料
2. 面向当前工程的接线研究稿
3. 后续如果真的要接 `SmartObject` 时的起步参考

它当前不应被写成：

1. 项目已经存在 `USmartObjectComponent`
2. 已经有 `SmartObjectDefinition`
3. AI 已通过 `Find -> Claim -> Move -> Use -> Release` 正式跑通世界资源交互

这些在当前工程里都还没有成立。

## 8. 当前工程如果未来接 `SmartObject`，更匹配哪条方向

> [!note]
> 这一节是从历史研究文档和当前工程结构推出的“后续接线候选方向”，不是当前已经落地的事实。

基于当前代码结构，更自然的方向是：

1. 让 `SmartObject` 负责“找资源 + 占资源 + 给出 SlotTransform”
2. 继续复用现有 `StateTree`
3. 继续复用现有 `FSTT_MoveToLocation`
4. 不要在第一步就把 `GameplayBehaviorSmartObjects` 整套行为层也一起引进来

原因很直接：

1. 项目已经有 `StateTree` 组件体系
2. 项目已经有动态组件重启补偿链
3. 项目已经有通用移动任务
4. 当前真正缺的是“世界资源查询/占用层”，不是“再起一套完整行为流”

## 9. 本轮提炼来源

本轮主要从下面两篇历史文档提炼，并结合当前代码核对：

- `Notice/HistoryNotice/StateTree事件系统笔记.md`
- `Notice/HistoryNotice/UE5智能对象SmartObject从介绍到基础案例实现.md`

这轮沉淀后的稳定结论是：

1. `StateTree` 事件注入在当前项目里已经正式存在
2. 当前事件语义边界是 `Event.Tag` 承载输入标签，`Payload.InputType` 承载输入类型
3. `SmartObject` 目前还未进入工程依赖和运行时主链
