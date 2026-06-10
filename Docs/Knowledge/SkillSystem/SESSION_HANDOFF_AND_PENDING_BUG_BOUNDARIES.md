---
title: Skill System Session Handoff And Pending Bug Boundaries
tags:
  - knowledge
  - skill-system
  - handoff
  - debugging
aliases:
  - 技能系统交接与遗留问题边界
  - 技能系统当前进度与遗留BUG提炼
---

# 技能系统交接与遗留问题边界

更新时间：2026-05-19  
适用范围：从 `Notice/HistoryNotice/技能系统当前进度与遗留BUG交接说明-2026-05-09.md` 里提炼出来、并且回到当前源码后仍然成立的技能系统运行时入口、遗留问题边界与排查起点。  
不适用范围：把历史交接文里的阶段口径直接当当前事实；具体某个技能资源如何配；已经需要进蓝图逐格验证的 UI 细节。

## 1. 这篇文档解决什么问题

这篇文档不重复讲整套技能系统设计。  
它只回答三件事：

1. 那篇交接文里哪些“当前已经落地”的说法，今天回到源码后仍然成立。
2. 哪些历史 BUG 适合继续保留为“排查边界”，但不能写成“已修复事实”。
3. 如果后续继续接这块，应该先顺着哪几条正式入口看，而不是重新从长文交接里捞信息。

相关正式骨架请先看：

- [[PROJECT_MAP]]
- [[DECISIONS]]
- [[EXECUTION_SEMANTICS]]
- [[KNOWN_ISSUES]]

## 2. 当前仍然成立的正式入口

### 2.1 技能触发当前已经分成两条链

当前源码里已经稳定存在两条入口：

1. `InjectSkillSlotInputCommand(...) / InjectSkillSlotInputCommandByIndex(...)`
2. `ExecuteSkillSlotCommand(...) / ExecuteSkillSlotCommandByIndex(...)`

这两条链不是重复实现，而是语义不同：

- `InjectSkillSlotInputCommand*` 先把技能槽语义翻译成 `Hero` 输入桥，继续复用玩家输入主链。
- `ExecuteSkillSlotCommand*` 直接走 `SkillComponent -> ASC -> ProcessAbilityInput(...)`，明确服务 `StateTree / AI`，不再回灌 `Hero` 输入广播。

这意味着后续如果出现“玩家按键正常，但 `StateTree` 触发不对”或反过来的问题，第一步不是怀疑技能资源，而是先确认到底走的是哪条正式链。

### 2.2 `StateTree` 直接触发技能槽命令已经是当前事实

当前 `Source/AegisOdyssey/StateTree/Tasks/STT_TriggerSkillSlotCommand.cpp` 已经正式通过：

- `UAOSkillComponent::ExecuteSkillSlotCommandByIndex(...)`

来直接触发技能槽命令。

因此“`StateTree` 要不要直接发技能槽命令”已经不是待设计问题，而是当前项目事实。  
后续如果 `StateTree` 技能触发异常，优先沿：

- `STT_TriggerSkillSlotCommand`
- `AOSkillComponent::ExecuteSkillSlotCommandByIndex(...)`
- `UAOAbilitySystem::AbilityInputTagPressed/Started/Released(...)`
- `UAOAbilitySystem::ProcessAbilityInput(...)`

这条链往下查。

### 2.3 技能槽库存投影层当前承担客户端本地壳层职责

当前 `UAOSkillSlotInventoryComponent` 里已经明确：

- `bAllowUnifiedInventoryIntake = false`
- `InitializeParams()` 会把 `NumSlots` 对齐到 `SkillComponent`
- `InitializeOrRefreshInventorySlots()` 会把本地投影条目补齐到技能槽数量
- `HandleChangeInitState(...)` 里客户端与服务端都会先补本地投影壳
- 只有把投影结果翻译回 `SkillComponent` 运行时真相这一步仍坚持只由 authority 执行

因此当前不能把 `SkillSlotInventoryComponent` 简化理解成“纯服务端容器”。  
它在客户端本地也承担一层关键职责：先让 UI 和统一库存交换主链看见“这里确实有这么多个合法技能槽目标”。

### 2.4 技能槽拖拽落点已经有正式 C++ 入口

当前 `AOSkillBarUI` / `AOSkillSlotUI` 这条链已经正式落了下面几个边界：

- `AOSkillBarUI` 构建技能槽时显式注入 `ObservedSlotIndex`
- `UAOSkillSlotUI` 持有 `ObservedSlotIndex` 与 `SourceContainer`
- `CanAcceptDraggedSourceSlotForThisSkillSlot(...)` 先回到 `SkillComponent` 做合法性判断
- `RequestEquipDraggedSourceSlotToThisSkillSlot(...)` 再走统一库存交换请求

这意味着“技能槽落点完全靠蓝图自己猜当前是第几个格子”不再是当前正确理解。  
如果后续还出现“只有第一个技能槽能放”的现象，优先怀疑的是蓝图 `OnDrop` 有没有真正调用到当前格子实例自己的正式入口。

## 3. 这篇交接文里哪些内容不能直接写成当前事实

### 3.1 “遗留 BUG 已经修到哪一步”不能直接当现状

这篇交接文的价值很高，但其中不少句子属于：

- 当时那轮改到了哪
- 当时最怀疑哪个根因
- 当时编译通过但玩家反馈仍有问题

这些信息适合保留成“排查历史”和“误判边界”，不适合直接写成当前知识库正文里的运行时事实。

### 3.2 输入去重那轮尝试不能写成“当前已有这套机制”

交接文里记录过一轮围绕 `ObservedPressedInputTags` 和 `ShouldForwardStateTreeInputEvent(...)` 的去重思路。  
但回到当前源码核对后，可以确认：

- `UAOStateTreeComponentBase.cpp` 当前没有这套通用去重实现
- `AOCombatStateTree` 和 `AOCombatLocomotionStateTree` 仍然都在直接订阅 `Hero` 与 `InputBuffer` 广播，并直接 `SendStateTreeEvent(...)`

因此这段历史材料当前应保留成“曾经尝试过的修法”和“排查输入重复时的背景线索”，不能写成“项目现在已经统一靠这套去重门闩解决”。

### 3.3 “客户端补齐技能槽壳层”也不能直接等于 bug 已经关单

交接文里把“客户端也先补齐技能槽库存投影空槽”作为一个重要修法记录下来。  
当前源码确实已经能看到这条修法落地，但这不等于“拖拽只能落到第一个技能槽”的问题已经被项目级验证彻底关单。

稳定能写进知识库的只有：

- 这是当前已经存在的实现边界。
- 如果还出问题，排查时不能再把 `SkillSlotInventoryComponent` 当成“和本地落点合法性无关”的层。

## 4. 当前仍值得保留的两个遗留问题定义

### 4.1 遗留问题一：技能输入进入 `StateTree` 后的重复消费语义

当前更准确的问题定义不是“技能一定会被真正激活很多次”，而是：

- `StateTree` 相关输入事件在按住周期内是否被重复消费
- `Hero` 原始输入广播和 `InputBuffer` 重放广播是否同时进入多个 `StateTree` 组件
- `Trigger / Start / Release` 在 `Hero / InputBuffer / StateTree / ASC` 四层之间是否仍然没有完全统一语义

因此后续排查这类问题时，优先入口应是：

1. `AOHeroComponent`
2. `AOInputBufferComponent`
3. `AOCombatStateTree`
4. `AOCombatLocomotionStateTree`
5. `AOAbilitySystem`

而不是上来就在某个技能 Ability 里加拦截。

更完整的跨包风险提示见：

- [[KNOWN_ISSUES]]
- [[HANDOFF_DOCUMENT_BOUNDARY_AND_EXTRACTION_RULES|交接文档边界规则]]

### 4.2 遗留问题二：技能槽拖拽落点身份是否在 UI 层丢失

当前更准确的问题定义也不是“统一库存交换主链一定错了”，而是：

- 当前落点 UI 实例是否真的在用自己的 `ObservedSlotIndex`
- 蓝图 `OnDrop` 是否还残留旧索引或固定第一格引用
- 请求进入 `ExecuteExchangeRequest(...)` 之前，目标容器与目标索引是否已经在 UI 层被写错

因此这类问题的排查顺序应优先是：

1. `AOSkillBarUI`
2. `AOSkillSlotUI`
3. 技能槽蓝图 `OnDrop`
4. `AOSkillSlotInventoryComponent`
5. `AOInventoryComponent::ExecuteExchangeRequest(...)`

而不是一开始就重写一套新的技能槽拖拽系统。

## 5. 后续继续接这篇交接文时的推荐顺序

建议按下面顺序接：

1. 先用 [[PROJECT_MAP]] 和 [[DECISIONS]] 确认技能系统当前正式边界。
2. 再看这篇文档，判断交接文里的哪部分是“当前事实”，哪部分只是“历史排查记录”。
3. 如果要查 `StateTree` 输入重复，转到 `SkillSystem + StateTreeAI` 共同排查，不要把它只当技能包内问题。
4. 如果要查技能槽拖拽落点，先核 UI 实例身份与蓝图落点接线，再往库存交换主链下钻。

## 6. 本轮来源

本轮主要来源：

- `Notice/HistoryNotice/技能系统当前进度与遗留BUG交接说明-2026-05-09.md`

但最终只保留了经过当前源码核对后仍然成立的入口与边界。  
凡是“当时怀疑什么”“当时修到哪”“当时用户反馈还没过”的阶段信息，都留在原始交接文，不直接抬升成现行规格。
