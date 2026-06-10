---
title: Combat Resolution And Defense
tags:
  - knowledge
  - combat-system
  - resolution
  - defense
aliases:
  - Combat Resolution And Defense
  - 战斗系统结算与防御语义说明
---

# 战斗系统结算与防御语义说明

更新时间：2026-05-19  
适用范围：当前战斗系统如何解释命中、如何组织防御语义、如何承接结果消息。  
不适用范围：所有具体表现资源配置、所有世界血条 MVVM 展开细节。

## 1. 当前命中不等于最终伤害

当前战斗系统的正式口径是：

- 命中采集层只负责“打到了谁、在哪里打到、这次属于什么攻击”
- 最终是否掉血、掉多少、是否被格挡/弹反/无敌帧拦截，要由统一结算层决定

## 2. 当前统一真相字段

当前已经确认存在并可网络序列化的关键真相字段包括：

- `bIsCritical`
- `bWasBlocked`
- `bWasParried`
- `bHitInvulnerability`
- `AttackTag`
- `SkillTag`
- `WeaponTag`
- `DamageTypeTags`

这些字段当前在：

- `FAOGameplayEffectContext`

里持有，是统一结算解释权的核心。

## 3. 当前统一结果消息字段

当前统一结果消息已经能承接：

- 结果类型
- 是否建议显示提示字
- 是否暴击/格挡/弹反/无敌
- 生命伤害
- 韧性伤害
- 体力消耗
- 攻击来源标签
- 推荐 `CueTag`
- `HitResult`

这说明“结算结果 -> 下游订阅”的桥已经比早期方案更完整。

## 4. 当前结算解释框架

### 4.1 生命伤害

当前结算层需要解释：

- 是否真正扣血
- 最终扣多少血
- 是否暴击

### 4.2 韧性伤害

当前结算层需要解释：

- 本次命中扣除多少 `Stamina`
- 是否进入破韧

### 4.3 体力消耗

当前结算层需要解释：

- 本次防御或机动行为扣除多少 `Vigor`

## 5. 当前防御语义理解方式

### 5.1 无敌帧

当前正式理解：

- 无敌帧成立时，本次命中被改写为 `Invulnerable`

### 5.2 格挡

当前正式理解：

- 格挡不是单纯动作播放，而是正式改写战斗结果的一层语义

它至少要影响：

- 是否掉血
- `Vigor` 消耗
- `Stamina` 扣减
- 结果消息类型

### 5.3 弹反

当前正式理解：

- 弹反不是普通格挡的一个视觉特效
- 它是更高优先级的结果改写

它至少要影响：

- 攻击中断
- 攻击方韧性扣减
- 被弹反馈
- 结果消息类型

## 6. 当前消息与 UI 的桥

战斗系统当前已经形成的正确方向是：

- 先有统一结果消息
- 再有 `CombatFeedbackViewData`
- 再有 `MVVM_CombatResources`
- UI / HUD 消费这些结果

而不是：

- UI 反过来猜测当前是不是暴击、是不是格挡、是不是无敌

## 7. 当前最常见的误判点

### 7.1 把 `DamageTypeTags` 当成完整抗性系统

当前不是。

它只是已经正式进入上下文的伤害类型入口，后续抗性系统还可以继续扩。

### 7.2 把 `FAOCombatResultMessage` 当成纯 UI 结构

当前不是。

它承接的是统一战斗结果，不只是 UI 显示字段。

### 7.3 把翻滚/格挡只当动作 Ability

当前也不够。

它们已经被明确要求继续推进为正式战斗结算语义。

## 8. 当前仍待继续整理的内容

本轮没有继续展开这些内容：

- 目标血条 MVVM 子主题
- `CombatFeedbackFeed` 的完整拆分过程
- `LocalCombatState` 的完整沉淀
- 具体格挡/弹反实现代码是否已经完整闭环
- 多段/持续命中策略的具体代码落点

## 9. 当前筛选语义的实现状态

当前这套命中筛选里，先记住两件实现事实：

1. `RequiredSourceTags` / `RequiredTargetTags` / `RequiredAttackTags` / `RequiredSkillTags` / `RequiredWeaponTags` / `RequiredDamageTypeTags` 目前都是组内 `AND` 语义。
2. `Critical / Blocked / Parried / HitInvulnerability` 目前是三态要求：`Ignore` / `MustBeTrue` / `MustBeFalse`。

这记录的是当前实现状态，不是最终设计宣言。

这些内容后续应再按清单开新轮深提炼，不应挤进本轮主文档。

## 10. 当前关于破韧和无伤窗口的正式解释边界

这部分必须和历史误判切开。

### 10.1 破韧不是正式伤害关闭条件

当前正式解释应当是：

- 破韧代表目标处于失衡受控态
- 不代表后续命中失效
- 不代表正式伤害入口关闭

也就是说，后续追击命中如果已经成立，结算层不应该因为目标正处于 `bIsBroken` 就直接吞掉伤害。

### 10.2 `DamageImmunity` 才是正式“命中成立但不掉血”的一类入口

当前正式解释应当是：

- `Gameplay.DamageImmunity` 可以让目标进入无伤窗口
- 这条语义和破韧不是一回事
- 后续排查“命中了但没掉血”时，必须把这两类情况区分开

因此当前更准确的理解是：

1. `bIsBroken` 解决的是受控态与反应问题
2. `DamageImmunity` 解决的是正式无伤窗口问题
3. 两者不能再混成同一层前置判断
