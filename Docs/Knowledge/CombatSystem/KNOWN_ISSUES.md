---
title: Combat System Known Issues
tags:
  - knowledge
  - combat-system
  - known-issues
aliases:
  - Combat System Known Issues
  - 战斗系统已知边界与历史偏差
---

# 战斗系统已知边界与历史偏差

更新时间：2026-05-19  
适用范围：当前战斗系统第一轮深提炼中已识别出的文档混层、边界风险和知识污染点。  
不适用范围：完整运行时 bug 列表。

## 1. 当前这轮记录什么

这份文档当前主要记录：

1. 历史战斗文档里哪些内容不适合直接并入战斗主骨架正文
2. 当前哪些边界最容易在后续整理时被重新写混

## 2. 已确认的文档混层问题

### 2.1 战斗系统文档已经混入目标血条 MVVM 子主题

`战斗系统当前实现与验收说明书-2026-05-11.md` 后半部分已经明显切入：

- 世界目标血条 MVVM
- 目标侧 / 观察者侧 / Widget 边界
- `TargetHealthBar` 相关 ViewModel 设计

这些内容重要，但更适合后续独立整理成：

- Combat UI / World Health Bar 子主题

因此本轮主骨架没有把它直接并进 `CombatSystem` 地图正文。

### 2.2 技能系统与战斗系统交接文档混入大量技能细节

`技能系统与战斗系统衔接交接总览-2026-05-10.md` 一半是高价值战斗衔接边界，一半仍然是：

- 火球术
- 火山喷发
- 技能父类
- GameplayCue 接入

这些内容有助于理解战斗系统如何被技能接入，但不应反过来把 `CombatSystem` 主骨架写成技能执行详解。

## 3. 当前最容易误判的边界

### 3.1 把目标血条 MVVM 当成战斗主链本体

当前不是。

它是战斗结果观察层的相关下游，不是“攻击命中统一结算主链”的核心正文。

### 3.2 把 `FAOGameplayEffectContext` 里已有字段当成“还只是设计”

当前不是。

这些字段已经存在于代码和网络序列化逻辑中。

### 3.3 把 `FAOCombatResultMessage` 当成“未来才会用”的结构

当前也不是。

它已经是现行的统一战斗结果消息结构。

## 4. 当前仍待后续继续整理的高价值内容

以下内容都已经露出明确结构，但本轮没有继续展开：

1. 目标血条 MVVM 子主题
2. `CombatFeedbackFeed` 和 `LocalCombatState` 的进一步拆分
3. 具体格挡/弹反/无敌帧实现代码闭环
4. 多段/持续命中策略的具体实现落点
5. 伤害消息广播是否已在所有链路中完全启用

这些内容后续都适合再开新的深提炼轮次。

## 4.1 当前已确认的 UI / 血条 / 跳字误判点

### 4.1.1 不要把 `UAOCombatMessageSubsystem` 再写回 dynamic multicast

当前不能这样写。

当前源码现状是：

1. `OnCombatResultMessage` 已是 native multicast
2. 订阅走 `AddUObject`
3. 客户端回放只走 `BroadcastCombatResultLocal(...)`

因此后续如果再把这条链写回蓝图 dynamic multicast，就会把历史已修过的 VM 参数复制风险重新引回来。

### 4.1.2 不要把目标血条和目标侧跳字继续视为同一组件职责

当前不是。

当前已经拆成：

1. `UAOTargetHealthBarComponent`
2. `UAOLocalTargetHealthBarObserverComponent`
3. `UAOCombatFloatingTextComponent`

如果后续整理时再写成“目标侧表现统一由 TargetHealthBarComponent 管”，就会把当前职责边界写回旧方案。

### 4.1.3 不要把 `LocalCombatState` 当成战斗结算真相层

当前不是。

它是 HUD 侧本地状态镜像，只适合做本地状态显示，不适合反向推出统一战斗真相。

## 5. 当前整理规则

后续往 `Docs/Knowledge/CombatSystem` 继续提炼时，默认遵守：

1. 先区分“战斗主链”与“战斗 UI 下游”。
2. 不把技能案例执行细节写成战斗系统正文主体。
3. 任何涉及“当前结果消息是否存在”的说法，优先核 `AOCombatResultMessage.*`。
4. 任何涉及“当前上下文字段是否已落地”的说法，优先核 `AOAbilityTypes.*`。

## 6. 当前仍待确认的表现链问题

这轮调试里已经确认过一个高频现象：

- 命中有伤害，但命中特效偶发不出现

这件事当前不能再写成伤害结算链失效，因为伤害链已经能正常走到 `ExecCal_Damage` 和 `AOHealthAttributeSet`。  
当前更像表现层分流或分发问题，待继续确认的分支包括：

1. `SourceObject -> AttackEffectProfile` 解析有没有偶发拿不到东西。
2. 基础 `GameplayCue.Combat.Hit` 和 `HitConfirmed` 分发出去的额外 Combat GC 是否有一条没走通。
3. Combat GC 内部分组是否匹配失败。
4. `HitResult` 或落点解析不完整，导致 Cue 落到不可见位置。

## 7. 已关闭的历史误判：不要再把“命中了但不出伤”默认归因到破韧后的合理设计

这条现在应该明确记成“历史误判”，不是继续悬而未决。

已经确认过的事实是：

1. 近战普攻“扫描命中但不出伤”曾和破韧提示时机强相关。
2. 根因是旧逻辑把 `bIsBroken` 错当成了正式伤害入口的早退条件。
3. 这不是正确设计，而是已经定位并修正过的病根。

因此后续遇到类似现象时，不要再先入为主地认为：

- 破韧后本来就不该掉血
- 这是合理战斗设计
- 不出伤只是表现层没跟上

当前正确做法是：

1. 先查正式伤害入口有没有又被新的前置条件错误挡掉。
2. 再查目标是不是确实处于 `Gameplay.DamageImmunity`。
3. 最后才查表现层。

## 8. 当前刀光轨迹资料最容易被旧方案误导

这块当前最大的知识污染点，不是代码 bug，而是旧文档和当前分支状态不完全一致。

后续最容易犯的误判有三类：

1. 把 `AOAttackEffectWindow` 继续当成刀光 Begin / End 派发入口。
2. 把当前 `WeaponTrail` 继续理解成完整 `History Ribbon` 主线。
3. 把旧方案里要求 Scratch / 数组读取的内容，直接当成当前资源制作必经步骤。

当前这些说法都不该再直接当现状。

更稳妥的处理规则是：

1. 当前代码事实优先看 `AOAttackEffectWindow.h`、`AOAttackTrailWindow.cpp`、`AOCombatGameplayCueNotify_WeaponTrail.cpp`。
2. 旧方案里的高级 Niagara 路线继续保留，但只当研究草案。
3. 任何关于“当前刀光就是这样工作的”结论，都必须先回源码核对。
