---
title: Skill Input Event And Duplicate Consumption Risks
tags:
  - knowledge
  - state-tree-ai
  - skill-system
  - debugging
aliases:
  - 技能输入事件与重复消费风险
  - StateTree 技能输入重复消费边界
---

# 技能输入事件与重复消费风险

更新时间：2026-05-19  
适用范围：`SkillSystem` 与 `StateTreeAI` 交界处的技能输入事件语义、输入转发链、重复消费风险与排查起点。  
不适用范围：具体某个技能 Ability 的资源配置；把单次交接文里的阶段性修法直接当成当前统一机制。

## 1. 这篇文档解决什么问题

`技能系统当前进度与遗留BUG交接说明-2026-05-09.md` 里提到过一个关键遗留问题：  
技能输入进入 `StateTree` 后，按住周期内会不会被重复消费。

这篇文档只负责把这件事当前已经核实的边界收紧清楚：

1. 当前技能输入相关 `StateTree` 事件到底从哪来。
2. 当前源码里哪些“曾经尝试过的去重方案”其实并不存在。
3. 后续排查时应先看哪条链，而不是先改哪个技能。

## 2. 当前已经核实的源码事实

### 2.1 `AOCombatStateTree` 当前同时订阅两路输入来源

当前 `AOCombatStateTree::InitializeComponent()` 会同时订阅：

- `UAOHeroComponent`
  - `OnPressInputLoad`
  - `OnReleaseInputLoad`
  - `OnStartInputLoad`
- `UAOInputBufferComponent`
  - `OnPressInputBuffer`
  - `OnStartInputBuffer`
  - `OnReleaseInputBuffer`

随后都统一走：

- `CallStateTreeToSentEvent(...)`
- `SendStateTreeEvent(...)`

### 2.2 `AOCombatLocomotionStateTree` 当前也在做同样的事

当前 `AOCombatLocomotionStateTree::InitializeComponent()` 同样同时订阅：

- `Hero` 原始输入广播
- `InputBuffer` 重放广播

并且也会直接调用：

- `CallStateTreeToSentEvent(...)`
- `SendStateTreeEvent(...)`

因此当前不能把“技能输入只会进一个 `StateTree`”当成默认事实。

### 2.3 当前通用基类里没有那套历史交接文提到的去重门闩

回到当前源码核对后，可以确认：

- `UAOStateTreeComponentBase.cpp` 当前非常薄
- 没有 `ObservedPressedInputTags`
- 没有 `ShouldForwardStateTreeInputEvent(...)`
- 也没有那种“按住周期只放行一次 Trigger、等 Release 解锁”的通用实现

这意味着历史交接文里记录过的那轮修法，当前不能写成“项目已经稳定采用的统一机制”。

## 3. 当前更准确的问题定义

当前这类问题更适合定义成：

1. `Hero` 原始输入广播和 `InputBuffer` 重放广播是否会共同进入 `StateTree`。
2. `CombatStateTree` 与 `CombatLocomotionStateTree` 是否会同时消费同一份技能输入语义。
3. `Trigger / Start / Release` 在 `Hero / InputBuffer / StateTree / ASC` 四层之间是否仍然没有完全统一为一套“按住周期”语义。

它不应被粗暴简化成：

- “某个技能 Ability 自己重复激活”
- “只要在 Ability 里再加个布尔锁就能解决”

## 4. 这类问题当前的正确排查顺序

建议固定按下面顺序查：

1. `AOHeroComponent`
2. `AOInputBufferComponent`
3. `AOCombatStateTree`
4. `AOCombatLocomotionStateTree`
5. `UAOAbilitySystem::AbilityInputTagPressed/Started/Released(...)`
6. `UAOAbilitySystem::ProcessAbilityInput(...)`
7. 最后才看具体技能 Ability

这样做的原因是：

- 前四层回答“同一份输入语义进来了几次、被谁消费了”
- 后两层回答“GAS 层最后按哪套输入状态机解释”
- 技能 Ability 往往只是最后一个消费点，不是第一现场

## 5. 当前不该再走的误修方向

### 5.1 不要先在单个技能 Ability 里打补丁

如果根因是输入链重复转发，那么在单个技能里加布尔锁，只会把问题局部遮住。  
它不能解决：

- 其他技能同类问题
- 状态树层面仍然重复消费事件
- 输入语义在不同链路上不一致

### 5.2 不要先把某一层广播粗暴砍掉

当前也不能在没核清楚职责前，就简单删掉：

- `Hero` 那一路
- `InputBuffer` 那一路
- 其中一个 `StateTree` 组件的订阅

因为这几个层的职责边界仍然需要结合具体玩法链确认。  
先砍一条链，容易变成“表面不重复了，但某些时机根本收不到输入”。

### 5.3 不要把历史交接文里的修法当成已实装事实

历史交接文里那轮关于：

- `ObservedPressedInputTags`
- `ShouldForwardStateTreeInputEvent(...)`
- 只靠 `Release` 解锁

的讨论，当前更适合当“曾经尝试过的修法线索”。  
它不能直接作为今天的代码事实被抄回知识库正文。

## 6. 和 `SkillSystem` 的边界关系

`SkillSystem` 当前已经稳定提供了两条命令入口：

- `InjectSkillSlotInputCommand*`
- `ExecuteSkillSlotCommand*`

其中 `StateTree / AI` 正式应该走的是：

- `ExecuteSkillSlotCommand*`

所以如果这里再出现重复消费问题，优先关注的不是“技能系统有没有正式入口”，而是：

- 进入 `StateTree` 之前的输入语义有没有重复
- `StateTree` 到 `ASC` 之间有没有多处消费同一命令

相关知识请联动看：

- [[技能系统交接与遗留问题边界]]
- [[PROJECT_MAP]]
- [[KNOWN_ISSUES]]

## 7. 本轮来源

本轮主要来源：

- `Notice/HistoryNotice/技能系统当前进度与遗留BUG交接说明-2026-05-09.md`

但最终结论只保留了经当前源码核实后仍成立的风险边界。  
凡是历史交接里“当时试过但现在代码里看不到”的内容，都没有被当成现行机制写入。
