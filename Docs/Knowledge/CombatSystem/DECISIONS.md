---
title: Combat System Decisions
tags:
  - knowledge
  - combat-system
  - decisions
aliases:
  - Combat System Decisions
  - 战斗系统已锁定设计
---

# 战斗系统已锁定设计

更新时间：2026-05-19  
适用范围：当前战斗系统已经锁定的统一主链、资源语义、防御优先级、UI 边界。  
不适用范围：所有数值平衡细节、所有目标血条 MVVM 展开细节。

## 1. 当前已锁定总方向

当前战斗系统已锁定的核心不是“每个攻击分别补能打中的逻辑”，而是：

**让所有攻击命中都通过统一上下文、统一结算、统一消息和统一反馈主链收口。**

## 2. 已锁定的统一主链边界

### 2.1 命中采集可以分散

已经锁定：

- 普攻、重击、技能、投射体、AOE 可以在各自实现里做命中采集

### 2.2 命中结算必须统一

已经锁定：

- 命中采集层不自己决定完整战斗真相
- 统一结算层才解释伤害、防御、暴击、破韧、提示字

### 2.3 UI 只能订阅结果

已经锁定：

- UI 不允许自己猜暴击、格挡、无敌、提示字
- UI 只消费统一消息 / ViewModel

## 3. 已锁定的资源语义

### 3.1 `Stamina`

已经锁定：

- `Stamina` = 韧性条

### 3.2 `Vigor`

已经锁定：

- `Vigor` = 体力条

### 3.3 破韧规则方向

已经锁定：

- 韧性归零后进入破韧
- 破韧期间角色不能正常行动
- 破韧恢复后韧性回满

## 4. 已锁定的防御优先级

当前正式顺序已经锁定为：

1. 敌我关系过滤
2. 方向/角度过滤
3. 无敌帧
4. 弹反
5. 格挡
6. 正常受击

后续任何实现都不应随意打乱这条解释优先级。

## 5. 已锁定的防御结果口径

### 5.1 无敌帧

已经锁定：

- 无敌帧成立时直接视为没打到
- 不掉血
- 不跳字

### 5.2 格挡

已经锁定：

- 必须支持完全格挡和部分格挡
- 完全格挡不跳字
- 格挡会影响 `Vigor` 和 `Stamina`

### 5.3 弹反

已经锁定：

- 弹反成功会打断攻击方
- 会大幅扣除攻击方韧性
- 在未破韧前也必须给出明显被弹反馈

## 6. 已锁定的攻击来源最小集

当前已经锁定并且代码已落地的最小集包括：

- `AttackTag`
- `SkillTag`
- `WeaponTag`
- `DamageTypeTags`

同时，统一上下文中已经有：

- `bIsCritical`
- `bWasBlocked`
- `bWasParried`
- `bHitInvulnerability`

这说明攻击来源与战斗真相字段已经不只是方案，而是现行结构。

## 7. 已锁定的多段/持续命中方向

当前历史方案已经明确，后续也必须继续保持：

- 不再允许“同一次攻击永远只命中一次”的窄逻辑统治所有攻击

统一方向至少支持：

- 单次命中
- 分段命中
- 间隔重复命中
- 周期持续命中

## 8. 已锁定的消息与反馈边界

### 8.1 消息层

已经锁定：

- 战斗结果要通过统一消息广播

### 8.2 GameplayCue / 表现层

已经锁定：

- Cue 和反馈层只消费结算结果
- 不自己改写伤害真相

### 8.3 提示字例外规则

已经锁定：

- 完全格挡不跳字
- 无敌帧命中失败不跳字

## 9. 当前第一轮核对后确认的现实进度

这轮核对后确认，战斗系统当前真实进度比部分历史文档更往前：

1. `FAOGameplayEffectContext` 中的关键战斗真相字段已经落地
2. `FAOCombatResultMessage` 已经作为统一结果消息结构存在
3. `MVVM_CombatResources` 已经作为本地资源观察层存在

这意味着后续知识整理不该再把战斗系统写成“只有方案、还没形成结构”。

## 10. 后续整理时必须遵守的规则

1. 不把目标血条 MVVM 细节混入战斗主链地图。  
2. 不把具体技能执行细节写成战斗系统主链本体。  
3. 不把表现层推测重新写回成结算真相来源。  
4. 凡是涉及“当前已支持的上下文字段”，优先以 `AOAbilityTypes.*` 和 `AOCombatResultMessage.*` 为准。

## 11. 战斗 UI 下游继续按“消息 -> ViewData -> HUD/观察者/目标侧组件”分层

已经锁定：

1. `FAOCombatResultMessage` 是统一结算真相。
2. `FAOCombatFeedbackViewData` 是本地玩家视角下的表现路由数据。
3. `UMVVM_CombatFeedbackFeed`、`UMVVM_LocalCombatState`、`UMVVM_TargetHealthBarCollection` 是 HUD / MVVM 消费层。
4. `UAOLocalTargetHealthBarObserverComponent`、`UAOTargetHealthBarComponent`、`UAOCombatFloatingTextComponent` 分别承担观察资格、目标血条、目标侧跳字三种不同职责。

不再建议：

1. 让 Widget 自己直接订阅底层结算真相后再解释一次。
2. 把目标血条和目标侧跳字重新揉回同一种组件类型。
3. 把 `Observer` 重新写成跳字转发入口或目标真相层。

## 12. 武器攻击表现已经收口成单独编排层

已经锁定：

1. `AOAttackEffectWindow` 负责把攻击窗口时机转换成表现触发点。
2. `AOAttackEffectProfile` 只负责编排“什么时候触发哪些 `GameplayCue`”。
3. `AOCombatGameplayCueNotify_WeaponTrail` 负责过程型表现。
4. `AOCombatGameplayCueNotify_Burst` 负责瞬时命中表现。
5. `WeaponInstance` 是武器运行时表现的统一出口，不再直接硬绑具体武器蓝图对象。
6. 同一个 `AttackEffectProfile` 可以挂多个触发条目，同一个触发语义也可以分发多个 `CueTag`。
7. Combat 表现层只编排 `GameplayCue`，不再混用手动 `Spawn Niagara` 和 `ExecuteGameplayCue` 两套入口。

## 13. 破韧不再作为正式伤害入口的总开关

已经锁定：

1. `bIsBroken` 不能再被当成“后续命中不再结算正式伤害”的总开关。
2. 破韧只代表目标进入失衡受控态，不代表后续命中无效。
3. 破韧后的追击伤害应继续按正式伤害入口结算，不允许再出现“扫到了但不掉血”的旧语义。

这条锁定来自已经确认过的历史病根修正：  
近战普攻“扫描命中但不出伤”的问题，根因不是 Niagara，也不是 GameplayCue，而是旧逻辑错误把 `bIsBroken` 用成了伤害早退条件。

需要同时记住的边界：

1. `bIsBroken` 状态本身没有被整体废掉。
2. 它仍然可以用于部分受控态和反应类逻辑。
3. 但这类使用不再等于正式伤害结算入口的关闭条件。

## 14. `DamageImmunity` 保留为正式无伤窗口，不和破韧混用

已经锁定：

1. `Gameplay.DamageImmunity` 继续作为正式无伤窗口语义存在。
2. 它和 `bIsBroken` 不是同类条件，不能因为修了破韧误判就顺手一起删掉。
3. 后续如果再出现“命中了但没掉血”，要先区分到底是错误前置判断，还是目标确实处于 `DamageImmunity`。

也就是说，当前正式口径是：

- `bIsBroken` 不再控制正式伤害入口
- `DamageImmunity` 仍然可以控制“命中成立但不掉血”

## 15. 刀光显示窗口和武器判定窗口已经正式分离

已经锁定：

1. `AOAttackEffectWindow` 现在只保留武器判定窗口 / 标签窗口语义。
2. `AOAttackTrailWindow` 单独负责刀光显示时长，以及 `CombatWindowBegin / CombatWindowEnd` 的派发。
3. 后续不再允许默认把“武器判定窗口”和“刀光显示窗口”绑回同一个 `AnimNotifyState`。

这条边界的原因不是风格偏好，而是已经确认：

1. 武器判定窗口通常很短。
2. 刀光显示窗口为了观感往往应该更长。
3. 两者绑死会天然导致表现生硬。

## 16. 当前 `WeaponTrail` 的现实状态是朴素试验路径，不是完整 Ribbon 主线

已经锁定：

1. 当前主分支里的 `AOCombatGameplayCueNotify_WeaponTrail` 更接近围绕 `SpawnSystemAttached(...)` 的试验实现。
2. 当前代码事实不是“已经稳定每帧推连续历史样本 + Ribbon 消费”的正式版本。
3. `History Ribbon`、Scratch、数组读取这类更重路线，目前更适合作为研究方向保留，不能再直接写成当前工程事实。

当前应按下面这条现实路径理解：

1. 从运行时 `WeaponInstance` 解出武器组件和 socket。
2. 根据开始/结束 socket 计算当前线段、中点和朝向。
3. 用 `SpawnSystemAttached(...)` 挂出 Niagara。
4. 暂时不再把“持续 Tick 推 `TrailStart / TrailEnd / Width / Direction`”当成当前已稳定落地结论。

这条锁定的意义是：  
后续可以继续升级视觉路线，但知识库里必须先把当前真实边界写对。
