---
title: Gameplay Framework Project Map
tags:
  - knowledge
  - gameplay-framework
  - gas
  - project-map
aliases:
  - Gameplay Framework Project Map
  - GameplayFramework 项目地图
---

# GameplayFramework 项目地图

更新时间：2026-05-19  
适用范围：当前项目里 GAS 运行时上下文、Ability 实例复制边界、属性复制回调边界、`StateTree` 事件注入边界、`SmartObject` 接线边界这几块已经核实的事实。  
不适用范围：敌人 AI 决策值与巡逻/目标状态真相、具体 StateTree 资产排布、所有还未进入首轮深提炼的别的 GameplayFramework 主题。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前项目里 `GameplayEffectContext` 的正式基础设施放在哪里。
2. `GameplayAbility ReplicationPolicy` 在这个项目里到底意味着什么，不意味着什么。
3. 属性复制到客户端后，哪些回调是引擎自动保证的，哪些需要项目自己补。
4. 当前 `StateTree` 事件到底接到了哪一层。
5. `SmartObject` 现在是不是已经进入工程。
6. 后续继续接 `GameplayFramework` 这一包时，先看哪些代码入口。

## 2. 本轮主主题

当前这一轮 `GameplayFramework` 首次深提炼已覆盖两组子主题：

1. `GAS ReplicationPolicy分析.md`
2. `GAS属性回调问题分析.md`
3. `自定义GameplayEffectContext完整实现指南.md`
4. `StateTree事件系统笔记.md`
5. `UE5智能对象SmartObject从介绍到基础案例实现.md`

但这里仍然只沉淀框架层事实。  
涉及敌人 AI 运行时状态真相、巡逻/追击消费链的内容，继续归到 [[StateTree AI 项目地图]] 那个独立知识包。

## 3. 当前 EffectContext 主链

优先看：

- `Config/DefaultGame.ini`
- `Source/AegisOdyssey/AOAbilitySystemGlobals.*`
- `Source/AegisOdyssey/AOAbilityTypes.*`
- `Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.*`
- `Source/AegisOdyssey/ExecCal/ExecCal_Damage.cpp`
- `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.cpp`

当前已经确认的正式链路是：

1. `DefaultGame.ini` 把 `AbilitySystemGlobals` 注册成 `UAOAbilitySystemGlobals`
2. `UAOAbilitySystemGlobals::AllocGameplayEffectContext()` 统一分配 `FAOGameplayEffectContext`
3. 战斗侧在 `AOCharacterCombatManagerComponent` 里把攻击、武器、伤害类型、格挡/招架、命中结果等信息写进这份上下文
4. `ExecCal_Damage` 读取这份上下文，继续补充暴击结果和最终伤害倍率语义
5. `AOHealthAttributeSet::PostGameplayEffectExecute(...)` 再读取同一份上下文，把最终结果整理成统一战斗消息

这说明自定义 `GameplayEffectContext` 在当前项目里已经不是“可选扩展点”，而是正式战斗结算链的一部分。

## 4. 当前 Ability 实例复制边界

优先看：

- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Sprint.*`

当前已确认：

1. `UGA_Sprint` 显式设置了 `ReplicationPolicy = ReplicateYes`
2. 这能说明该 Ability 实例会进入可复制实例链
3. 但这不等于它的所有 C++ 成员都会自动复制

在当前项目里，这条边界最典型的观察点就是 `GA_Sprint`：

1. `SprintSpeedBonusAmount`
2. `SprintCost`
3. `bVigorExhaustedBroadcasted`

这些成员当前没有对应的 `GetLifetimeReplicatedProps(...)` 和 `DOREPLIFETIME...` 自定义复制逻辑，不能写成“因为 `ReplicateYes` 所以它们自动同步到客户端”。

## 5. 当前属性复制回调边界

优先看：

- `Source/AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.*`
- `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.*`

当前已确认的项目模式是：

1. 属性本身复制仍然走 `ReplicatedUsing = OnRep_*`
2. `OnRep_*` 里先调用 `GAMEPLAYATTRIBUTE_REPNOTIFY(...)`
3. 再手动广播项目自己的属性事件

当前已成立的例子：

1. `AOHealthAttributeSet::OnRep_Health / OnRep_MaxHealth`
2. `AOCombatAttributeSet::OnRep_Vigor / OnRep_MaxVigor`

这意味着：

- “客户端已经收到属性复制”  
  不等于  
- “项目 UI / ViewModel 想要的自定义变化通知已经自动发出”

## 6. 当前 `StateTree` 事件注入边界

优先看：

- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.*`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.*`
- `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.*`
- `Source/AegisOdyssey/Character/AOHeroComponent.*`
- `Source/AegisOdyssey/Character/AOInputBufferComponent.*`

当前已确认：

1. `UAOStateTreeComponentBase` 是项目通用基类，默认不自动启动并启用复制
2. `UAOCombatStateTree` 和 `UAOCombatLocomotionStateTree` 会把 `HeroComponent` / `InputBufferComponent` 的输入委托转成 `FStateTreeEvent`
3. 当前事件语义拆成两层：
   - `Event.Tag` 承载输入标签
   - `Payload.InputType` 承载输入类型
4. `FCombatStateTreeInputEvent` 当前并不保存输入标签本身

这说明项目已经正式存在“StateTree 事件注入层”，但这份框架笔记不把 AI 资产层消费细节也混进来。

## 7. 当前 `SmartObject` 接线边界

优先看：

- `AegisOdyssey.uproject`
- `Source/AegisOdyssey/AegisOdyssey.Build.cs`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.*`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.*`

当前已确认：

1. 工程已经启用 `GameplayStateTreeModule / StateTreeModule / AIModule / NavigationSystem`
2. 工程已经有 `DefaultStateTree` 自动挂载和动态添加组件后的 `RestartLogic()` 补偿链
3. 工程已经有通用 `FSTT_MoveToLocation`
4. 但工程还没有启用 `SmartObjects` / `GameplayBehaviorSmartObjects` 插件
5. `Build.cs` 没有 `SmartObjectsModule`
6. 源码、配置和内容资产里都没有当前项目对 `SmartObject` 的正式接线

因此当前正确理解是：

- `SmartObject` 的项目前置条件已经具备一部分
- 但它本身还没有进入当前运行时主链

## 8. 当前真相层和消费层

### 8.1 真相层

当前应视为真相层的有：

- `UAOAbilitySystemGlobals`
- `FAOGameplayEffectContext`
- `AOCharacterCombatManagerComponent` 写入的战斗上下文字段
- `ExecCal_Damage` 对上下文的补写
- `AttributeSet` 的 `OnRep_*` 手动广播模式
- `UAOStateTreeComponentBase`
- `UAOCombatStateTree` / `UAOCombatLocomotionStateTree` 的事件注入入口

### 8.2 消费层

当前应视为消费层的有：

- `GA_Sprint` 这类具体 Ability
- `ExecCal_Damage`
- `AOHealthAttributeSet`
- HUD / ViewModel / Combat Message
- StateTree 资产运行时消费层
- `[[StateTree AI 项目地图]]` 里记录的 AI 侧 Evaluator / Task / Condition 消费链

这条边界很重要：

1. `ReplicationPolicy` 决定的是 Ability 实例复制策略，不是任意业务字段复制策略。
2. `EffectContext` 决定的是一次 GE/伤害结算共享什么上下文，不是任何系统都应往里塞临时状态。
3. `OnRep_*` 决定的是属性到达客户端后的入口，不是完整 UI 业务的自动完成器。
4. `StateTree` 事件注入层负责“把输入送进树”，不等于“这里已经完整记录了树内 AI 运行时真相”。
5. `SmartObject` 接线研究和 `SmartObject` 已正式进入工程，是两回事。

## 9. 当前继续扩展时的优先阅读顺序

如果后续继续接 `GameplayFramework`，当前推荐顺序是：

1. `Config/DefaultGame.ini`
2. `Source/AegisOdyssey/AOAbilitySystemGlobals.*`
3. `Source/AegisOdyssey/AOAbilityTypes.*`
4. `Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.*`
5. `Source/AegisOdyssey/ExecCal/ExecCal_Damage.cpp`
6. `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.*`
7. `Source/AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.*`
8. `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Sprint.*`
9. `Source/AegisOdyssey/AbilitySystem/AOAbilitySystem.*`
10. `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.*`
11. `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.*`
12. `Source/AegisOdyssey/StateTree/CombatStateTree/AOCombatLocomotionStateTree.*`
13. `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.*`
14. `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.*`
15. `AegisOdyssey.uproject`
16. `Source/AegisOdyssey/AegisOdyssey.Build.cs`

## 10. 本轮提炼来源

本轮主要从下面五篇历史文档提炼，并结合当前代码核对：

- `Notice/HistoryNotice/GAS ReplicationPolicy分析.md`
- `Notice/HistoryNotice/GAS属性回调问题分析.md`
- `Notice/HistoryNotice/自定义GameplayEffectContext完整实现指南.md`
- `Notice/HistoryNotice/StateTree事件系统笔记.md`
- `Notice/HistoryNotice/UE5智能对象SmartObject从介绍到基础案例实现.md`

沉淀后的稳定文档分别是：

- [[GameplayFramework 已锁定设计]]
- [[GAS 复制边界与 EffectContext 主链]]
- [[StateTree 事件与 SmartObject 接线边界]]
- [[GameplayFramework 已知边界与历史偏差]]

## 11. 当前武器表现链入口

如果后续继续碰武器攻击表现、运行时 socket 解析或命中特效分流，优先看：

- `Source/AegisOdyssey/Animation/NotifyState/AOAttackEffectWindow.*`
- `Source/AegisOdyssey/Combat/Effects/AOAttackEffectProfile.*`
- `Source/AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_WeaponTrail.*`
- `Source/AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_Burst.*`
- `Source/AegisOdyssey/Equipment/Weapons/AOWeaponInstance.*`
- `Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.*`
- `Source/AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.cpp`

这条主链记成：

`WeaponInstance -> AttackEffectProfile -> GameplayCue -> HealthAttributeSet`
