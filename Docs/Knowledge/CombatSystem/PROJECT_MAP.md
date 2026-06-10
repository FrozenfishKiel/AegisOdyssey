---
title: Combat System Project Map
tags:
  - knowledge
  - combat-system
  - project-map
aliases:
  - Combat System Project Map
  - 战斗系统项目地图
---

# 战斗系统项目地图

更新时间：2026-05-19  
适用范围：当前项目中“攻击命中统一结算、防御语义、统一消息与反馈”这条战斗主链。  
不适用范围：目标血条 MVVM 细节、目标侧跳字实现细节、所有具体 UI 表现规则、所有技能案例执行细节。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前战斗系统的统一主链已经落到哪一层。
2. 攻击来源、战斗上下文、防御语义、结果消息分别在哪里。
3. 技能/普攻/投射体/AOE 该如何接回同一条主链。
4. 后续继续做战斗系统时，先看哪些代码入口。

## 2. 当前正式主线

当前项目战斗系统的正式方向已经不是“每种攻击各算各的结果”，而是：

**命中采集可以分散，但命中结算必须统一回战斗主链。**

当前主线可以概括为：

1. 各攻击实现完成命中采集
2. 构建统一攻击来源与战斗上下文
3. 进入统一结算入口
4. 产出统一战斗结果
5. 通过统一消息 / ViewModel / Cue 驱动表现层

## 3. 当前关键结构落点

### 3.1 攻击来源与战斗上下文

优先看：

- `Source/AegisOdyssey/AOAbilityTypes.h`
- `Source/AegisOdyssey/AOAbilityTypes.cpp`

当前 `FAOGameplayEffectContext` 已真实承载：

- `bIsCritical`
- `bWasBlocked`
- `bWasParried`
- `bHitInvulnerability`
- `DamageMultiplier`
- `AttackTag`
- `SkillTag`
- `WeaponTag`
- `DamageTypeTags`

这说明战斗真相上下文已经不是纯设计想法，而是正式的网络可序列化结构。

### 3.2 命中结果消息

优先看：

- `Source/AegisOdyssey/AOCombatResultMessage.h`
- `Source/AegisOdyssey/AOCombatResultMessage.cpp`

当前统一结果消息已经有正式结构：

- `EAOCombatResultType`
- `EAOCombatFloatingTextType`
- `bShouldDisplayFloatingText`
- `bIsCritical`
- `bWasBlocked`
- `bWasParried`
- `bHitInvulnerability`
- `bTargetBroken`
- `AttackTag / SkillTag / WeaponTag / DamageTypeTags`
- `HealthDamage / StaminaDamage / VigorCost`
- `CueTag`
- `HitResult`

这说明“战斗结果消息化”已经进入正式结构层。

### 3.3 属性与结算入口

优先看：

- `Source/AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.*`
- `Source/AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.*`
- `Source/AegisOdyssey/ExecCal/ExecCal_Damage.cpp`

当前属性侧至少已经明确存在：

- `Health / MaxHealth / Damage / Healing`
- `Stamina`
- `Vigor`
- `OnHealthChange`
- `OnDamageChanged`
- `OnOutOfHealth`
- `TAG_Gameplay_DamageImmunity`
- `TAG_AO_Damage_Message`

### 3.4 现有攻击实现接入点

当前各类攻击已经开始往统一结构靠拢。

优先看：

- `GA_LightAttack.cpp`
- `GA_HeavyAttack.cpp`
- `AOSkillGameplayAbility.cpp`
- `AOSkillProjectile_Fireball.cpp`
- `AOSkillAreaSequenceRuntime_VolcanoBurst.cpp`

这里要理解的重点不是单个技能怎么打，而是：

- 命中采集层怎样收集目标
- 命中结果怎样带上 `AttackTag / SkillTag / WeaponTag / DamageTypeTags`
- 再怎样送回战斗结算尾链

### 3.5 防御与机动入口

优先看：

- `GA_Roll.cpp`
- `GA_Block.cpp`
- `AOAnimNotifyState_SendGameplayEventWindow.cpp`
- `STT_PlayRollAnimation.cpp`
- `STT_PlayBlockAnimation.cpp`

这里是当前翻滚、格挡、弹反窗口语义最自然的接入口。

## 4. 当前已经落地的战斗资源语义

### 4.1 `Stamina`

当前正式语义是：

- `Stamina` = 韧性条

### 4.2 `Vigor`

当前正式语义是：

- `Vigor` = 体力条

这条语义不仅出现在历史文档里，当前代码里也已经有：

- `MMC_CalculateStamina`
- `MMC_CalculateVigor`
- `MVVM_CombatResources`

## 5. 当前 UI / ViewModel 侧战斗结果承接

虽然本轮不展开 UI 子主题，但战斗系统主链已经有明确下游承接层：

- `AOCombatFeedbackViewData`
- `MVVM_CombatResources`
- `MVVM_HUD`

这说明战斗结果与资源观察已经开始从统一战斗主链往 ViewModel 收束，而不是让 UI 纯猜。

## 6. 当前继续扩展时的优先排查顺序

### 6.1 第一层：命中来源对不对

先查：

- `AttackTag`
- `SkillTag`
- `WeaponTag`
- `DamageTypeTags`

### 6.2 第二层：上下文解释对不对

再查：

- `bIsCritical`
- `bWasBlocked`
- `bWasParried`
- `bHitInvulnerability`

### 6.3 第三层：属性结算和消息结果对不对

最后查：

- 最终生命伤害
- `StaminaDamage`
- `VigorCost`
- `EAOCombatResultType`
- `bShouldDisplayFloatingText`

## 7. 本轮与目标血条 MVVM 的边界

当前 `战斗系统当前实现与验收说明书` 后半部分已经开始讨论目标血条 MVVM。

这部分很重要，但它更适合后续单独整理为：

- 战斗 UI / World Health Bar 子主题

因此本轮 `CombatSystem` 主地图只保留一句边界：

- 目标血条是战斗结果观察层的相关下游，不属于本轮“攻击命中统一结算主链”的主骨架正文。

当前这一块已经单独收束到：

- [[战斗UI 世界血条与目标侧跳字]]

## 8. 本轮提炼来源

本轮主要从下面三篇历史文档提炼，并结合当前代码核对：

- `Notice/HistoryNotice/战斗系统设计方案-攻击命中统一结算与防御反馈框架.md`
- `Notice/HistoryNotice/战斗系统当前实现与验收说明书-2026-05-11.md`
- `Notice/HistoryNotice/技能系统与战斗系统衔接交接总览-2026-05-10.md`
- `Notice/HistoryNotice/战斗系统UI与目标血条-MVVM改造当前进度与新AI交接说明-2026-05-13.md`
- `Notice/HistoryNotice/战斗系统UI与目标血条和目标侧跳字当前进度与新AI交接说明-2026-05-14.md`

沉淀后的稳定文档分别是：

- [[战斗系统已锁定设计]]
- [[战斗系统结算与防御语义说明]]
- [[战斗系统已知边界与历史偏差]]
- [[战斗UI 世界血条与目标侧跳字]]

## 9. 当前武器攻击表现链入口

优先看：

- `Source/AegisOdyssey/Animation/NotifyState/AOAttackEffectWindow.*`
- `Source/AegisOdyssey/Animation/NotifyState/AOAttackTrailWindow.*`
- `Source/AegisOdyssey/Combat/Effects/AOAttackEffectProfile.*`
- `Source/AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_WeaponTrail.*`
- `Source/AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_Burst.*`
- `Source/AegisOdyssey/Equipment/Weapons/AOWeaponInstance.*`
- `Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.*`
- `Source/AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.cpp`

这条链的主顺序记成：

`AOAttackEffectWindow / AOAttackTrailWindow -> AOAttackEffectProfile -> GameplayCue -> AOHealthAttributeSet`

## 10. 当前武器攻击表现链要特别注意的两个现实边界

### 10.1 判定窗口和刀光窗口现在是两层语义

当前不要再把“武器判定窗口”和“刀光显示窗口”看成同一个入口。

当前正式分工是：

1. `AOAttackEffectWindow`
   - 负责武器判定窗口 / 标签窗口语义
2. `AOAttackTrailWindow`
   - 负责刀光显示窗口
   - 负责 `CombatWindowBegin / CombatWindowEnd`

这意味着当前回主链时，至少要分清两条触发：

- 判定相关：从 `AOAttackEffectWindow` 看
- 刀光持续表现相关：从 `AOAttackTrailWindow` 看

### 10.2 当前 `WeaponTrail` 不等于完整 Ribbon 正式版

当前地图里还要显式记住一点：  
`AOCombatGameplayCueNotify_WeaponTrail` 现在更接近一条围绕 `SpawnSystemAttached(...)` 的朴素试验实现。

也就是说，当前理解这条链时不要默认：

1. 已经稳定每帧持续推历史样本
2. 已经完整走 `History Ribbon`
3. 已经要求资源侧必须配 Scratch / 数组读取

这些更高级的内容目前仍属于后续可研究方向，不是当前地图默认事实。
