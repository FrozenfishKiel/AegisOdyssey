---
title: Gameplay Framework Decisions
tags:
  - knowledge
  - gameplay-framework
  - gas
  - decisions
aliases:
  - Gameplay Framework Decisions
  - GameplayFramework 已锁定设计
---

# GameplayFramework 已锁定设计

更新时间：2026-05-19  
适用范围：当前项目在 GAS 复制边界、属性复制回调、统一战斗上下文、`StateTree` 事件注入边界这些已经锁定的框架事实。  
不适用范围：尚未接入工程的 `SmartObject` 具体落地方案、敌人 AI 运行时状态真相。

## 1. Ability 的 `ReplicationPolicy` 只决定实例复制策略

已经锁定：

1. `ReplicateYes` 说明该 Ability 实例进入复制链
2. 不等于 Ability 内所有 C++ 成员自动复制
3. 业务字段是否复制，仍要看具体属性和显式复制声明

## 2. 不把 `ReplicateYes` 误写成“服务端算出的局部状态都会自动同步”

已经锁定：

1. Ability 激活状态和相关 GAS 正式复制链可以同步
2. 但普通 C++ 成员变量不是因此自动变成网络真相层
3. 需要同步的业务状态，要么走正式属性复制，要么走显式 RPC / GE / Message / Context

## 3. 自定义 `GameplayEffectContext` 已经是正式基础设施

已经锁定：

1. 项目已注册 `UAOAbilitySystemGlobals`
2. `AllocGameplayEffectContext()` 正式返回 `FAOGameplayEffectContext`
3. 当前战斗链已经依赖这套上下文传递统一结算真相

因此后续不能再把它写成“只是教程级做法”。

## 4. 战斗真相先写进统一上下文，再让后续各层读取

已经锁定：

1. `AOCharacterCombatManagerComponent` 负责把攻击标签、武器标签、伤害类型、格挡/招架、命中结果、伤害倍率等信息写进 `FAOGameplayEffectContext`
2. `ExecCal_Damage` 继续基于同一份上下文做伤害计算，并回填暴击结果
3. `AOHealthAttributeSet` 再读取同一份上下文生成统一战斗结果消息

这意味着当前项目采用的是“统一上下文串联结算链”，不是让每一层重新自行推断。

## 5. Attribute 复制后的 UI 业务通知由项目自己补齐

已经锁定：

1. `GAMEPLAYATTRIBUTE_REPNOTIFY(...)` 是必要入口
2. 但它本身不等于 UI 层所需的所有自定义业务事件都会自动补齐
3. 需要客户端 UI 感知的属性，当前项目模式是 `OnRep_* + GAMEPLAYATTRIBUTE_REPNOTIFY + 自定义 Broadcast`

## 6. `Health` 和 `Vigor` 当前已经统一回到同一种客户端回调模式

已经锁定：

1. `AOHealthAttributeSet::OnRep_Health / OnRep_MaxHealth` 会手动广播
2. `AOCombatAttributeSet::OnRep_Vigor / OnRep_MaxVigor` 现在也会手动广播
3. 因此“Health 正常但 Vigor 客户端不回调”不能再写成当前事实

## 7. AttributeSet 里的自定义事件是当前项目 UI 感知边界的一部分

已经锁定：

1. `OnHealthChange / OnMaxHealthChange`
2. `OnVigorChanged / OnMaxVigorChanged`

这些不是可有可无的装饰层，而是当前项目把属性复制结果转成 UI/业务通知的正式桥。

## 8. `EffectContext` 当前承载的是“单次战斗结算真相”

已经锁定：

1. 它适合承载命中、格挡、招架、暴击、攻击标签、武器标签、伤害类型等“本次结算上下文”
2. 不应把与单次结算无关的长期运行时状态乱塞进去
3. 当前项目里它已经和战斗结算主链强绑定

## 9. `AOHealthAttributeSet` 当前可以直接断言拿到自定义上下文

已经锁定：

1. `AOHealthAttributeSet::PostGameplayEffectExecute(...)` 里直接 `static_cast<FAOGameplayEffectContext*>`
2. 代码里还显式 `check(SourceEffectContext)`

这说明当前项目已经把“伤害链必须来自自定义 Context”当成真实前提，而不是温和兼容分支。

## 10. `GameplayFramework` 这一包后续应继续按子主题拆批

已经锁定：

1. GAS 复制边界
2. GAS 属性复制回调
3. 自定义 EffectContext
4. `StateTree`
5. `SmartObject`

这些主题虽然都归到 `GameplayFramework`，但不应该因为系统标签相同就混成一篇总文档。

## 11. `StateTree` 输入事件当前由消费侧组件注入，不回改 Hero 原始广播职责

已经锁定：

1. `HeroComponent` 和 `InputBufferComponent` 继续负责原始输入广播
2. `UAOCombatStateTree` / `UAOCombatLocomotionStateTree` 在消费侧把输入转成 `FStateTreeEvent`
3. 当前项目没有把“StateTree 输入事件系统”做成全局新输入源替代物

## 12. `StateTree` 事件标签和负载当前是分层承载

已经锁定：

1. `FStateTreeEvent.Tag` 承载输入标签
2. `FCombatStateTreeInputEvent` 当前只稳定承载 `InputType`
3. 不把这套当前结构写成“Payload 内自带完整 Tag + InputType”

## 13. `StateTreeAI` 和 `GameplayFramework` 的知识边界要分开

已经锁定：

1. `GameplayFramework` 这里记录框架层事件注入和模块边界
2. `StateTreeAI` 那个独立知识包继续记录敌人 AI 的运行时状态真相、消费链和排查顺序
3. 不因为都带 `StateTree` 就把两类内容重新混回一个包

## 14. `SmartObject` 当前不能写成已接入框架

已经锁定：

1. `.uproject` 没有启用 `SmartObjects` / `GameplayBehaviorSmartObjects`
2. `Build.cs` 没有 `SmartObjectsModule`
3. 当前源码和内容资产里也没有正式接线

因此这轮只能把 `SmartObject` 写成：

- 已被认真研究过的候选框架

不能写成：

- 当前项目已落地运行的一部分

## 15. `WeaponInstance` 是武器运行时表现路由的统一出口

当前项目里，武器攻击表现不再优先从静态武器蓝图对象直接硬找，而是先从 `WeaponInstance` 往下找。

已经锁定：

1. `WeaponInstance` 负责承接当前运行时武器实例的表现出口。
2. 后续要找 socket、附着组件、当前武器默认攻击表现时，优先沿 `WeaponInstance -> 运行时生成的武器 Actor / Component -> 对应 socket` 这条线解析。
3. 这条线也适用于静态网格体和骨骼网格体，不要把武器表现接线理解成只搜一种组件类型。
