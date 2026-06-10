---
title: GAS Replication And Effect Context
tags:
  - knowledge
  - gameplay-framework
  - gas
  - replication
  - effect-context
aliases:
  - GAS Replication And Effect Context
  - GAS 复制边界与 EffectContext 主链
---

# GAS 复制边界与 EffectContext 主链

更新时间：2026-05-19  
适用范围：当前项目中 Ability 实例复制、属性复制后的事件补齐、自定义 `GameplayEffectContext` 在战斗链里的实际作用。  
不适用范围：所有 `StateTree` / `SmartObject` / 非 GAS 框架主题。

## 1. `ReplicateYes` 在这个项目里到底代表什么

当前 `GA_Sprint` 明确写了：

- `ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes`

这在当前项目里至少能说明两件事：

1. 该 Ability 实例被当作可复制实例处理
2. Ability 的正式激活/结束相关网络行为不会被当成纯本地实例处理

但当前不能顺手多推一步，写成“服务器里的 Ability 成员变量都会自动同步给客户端”。

## 2. `GA_Sprint` 是当前最直观的反例

优先看：

- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Sprint.h`
- `Source/AegisOdyssey/AbilitySystem/Abilities/Attack/Locomotion/GA_Sprint.cpp`

当前类里至少有这些普通成员：

1. `SprintSpeedBonusAmount`
2. `SprintCost`
3. `bVigorExhaustedBroadcasted`

本轮核对后确认：

1. 这两个文件里没有 `GetLifetimeReplicatedProps(...)`
2. 也没有 `DOREPLIFETIME...` 之类的显式复制声明

所以当前稳定结论是：

**`ReplicationPolicy = ReplicateYes` 不能单独推出这些 C++ 字段已自动复制。**

## 3. 当前项目真正把“单次战斗结算真相”放在哪里

答案不是 Ability 普通成员，而是 `FAOGameplayEffectContext`。

优先看：

- `Config/DefaultGame.ini`
- `Source/AegisOdyssey/AOAbilitySystemGlobals.*`
- `Source/AegisOdyssey/AOAbilityTypes.*`

当前项目已经做了三件正式接线：

1. 在 `DefaultGame.ini` 里注册 `AbilitySystemGlobalsClassName="/Script/AegisOdyssey.AOAbilitySystemGlobals"`
2. `UAOAbilitySystemGlobals::AllocGameplayEffectContext()` 返回 `new FAOGameplayEffectContext()`
3. `FAOGameplayEffectContext` 已实现 `NetSerialize(...)`、`Duplicate()` 和 `TStructOpsTypeTraits`

这说明当前项目不是“偶尔 new 一个自定义 Context 试试”，而是已经把它接成 GAS 全局工厂。

## 4. 当前 `FAOGameplayEffectContext` 里实际承载什么

优先看：

- `Source/AegisOdyssey/AOAbilityTypes.h`

当前这份上下文除了父类标准字段，还额外承载：

1. `bIsCritical`
2. `bWasBlocked`
3. `bWasParried`
4. `bHitInvulnerability`
5. `DamageMultiplier`
6. `AttackTag`
7. `SkillTag`
8. `WeaponTag`
9. `DamageTypeTags`

这批字段的共同点是：

- 都属于“一次攻击/一次伤害结算”的上下文真相  
- 不是角色长期属性面板上的常驻状态

## 5. 当前是谁在写这些字段

优先看：

- `Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.cpp`
- `Source/AegisOdyssey/ExecCal/ExecCal_Damage.cpp`

当前正式写入链是：

1. `AOCharacterCombatManagerComponent` 在应用元效果前，把 `AttackTag / SkillTag / WeaponTag / DamageTypeTags / WasBlocked / WasParried / HitInvulnerability / DamageMultiplier / HitResult / SourceObject` 写进 `ContextHandle`
2. `ExecCal_Damage` 在执行期读取这份上下文，并回填 `bIsCritical`

因此它不是纯消费结构，而是当前战斗结算链的共享数据面。

## 6. 当前是谁在消费这些字段

优先看：

- `Source/AegisOdyssey/ExecCal/ExecCal_Damage.cpp`
- `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.cpp`

当前消费链已确认：

1. `ExecCal_Damage` 读取 `DamageMultiplier / AttackTag / SkillTag / WeaponTag`
2. `AOHealthAttributeSet::PostGameplayEffectExecute(...)` 直接把 `FAOGameplayEffectContext` 当正式上下文使用
3. `AOHealthAttributeSet` 基于它生成统一战斗消息和浮字语义

这里最重要的项目事实是：

`AOHealthAttributeSet` 当前不是温和地“尝试读取”；它直接 `static_cast` 后 `check(SourceEffectContext)`。  
这代表当前伤害链默认要求这份自定义上下文必然存在。

## 7. 为什么“属性已经复制”不等于“UI 想要的变化回调已经到位”

优先看：

- `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.*`
- `Source/AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.*`

本轮核对后，当前稳定模式很明确：

1. `OnRep_*` 先做 `GAMEPLAYATTRIBUTE_REPNOTIFY(...)`
2. 然后手动广播项目自己的变化事件

当前例子：

1. `OnRep_Health -> OnHealthChange.Broadcast(...)`
2. `OnRep_MaxHealth -> OnMaxHealthChange.Broadcast(...)`
3. `OnRep_Vigor -> OnVigorChanged.Broadcast(...)`
4. `OnRep_MaxVigor -> OnMaxVigorChanged.Broadcast(...)`

这条结论比历史排查稿更重要：

**项目 UI 真正依赖的不是“引擎某个神秘自动回调”，而是项目自己在 `OnRep_*` 后补齐的业务广播。**

## 8. 这轮最该记住的三条边界

1. `ReplicationPolicy` 不是任意 C++ 字段自动同步开关。
2. `EffectContext` 是当前战斗结算共享真相层，不是随便塞业务临时变量的口袋。
3. 属性复制到客户端后，UI 侧需要的变化通知，当前项目仍然靠 `OnRep_*` 手动补齐。
