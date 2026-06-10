---
title: Combat UI World Health Bar And Floating Text
tags:
  - knowledge
  - combat-system
  - combat-ui
  - mvvm
  - world-health-bar
aliases:
  - Combat UI World Health Bar And Floating Text
  - 战斗UI 世界血条与目标侧跳字
---

# 战斗UI 世界血条与目标侧跳字

更新时间：2026-05-19  
适用范围：当前项目里 `FAOCombatResultMessage -> HUD/MVVM -> 目标血条观察 -> 目标侧跳字` 这条战斗结果观察与表现下游链。  
不适用范围：攻击命中统一结算主链、所有具体蓝图动画样式、所有敌人资产是否已经挂齐组件。

## 1. 这份文档解决什么问题

这份文档不重讲战斗伤害怎么结算。

它只回答：

1. 当前统一战斗结果怎样进入 HUD / MVVM。
2. 目标血条、观察者、目标侧跳字三者当前怎样分工。
3. 哪些旧方案已经过时，后续不要再写回去。
4. 如果战斗 UI 表现不对，先查哪些入口。

## 2. 当前稳定主链

当前已经核实的主链是：

1. 战斗结算层产出 `FAOCombatResultMessage`
2. `UAOCombatMessageSubsystem` 在当前世界内本地广播
3. `UAOHUDViewModelComponent` 订阅消息并构建 `FAOCombatFeedbackViewData`
4. `UMVVM_HUD` 把反馈写入 `UMVVM_CombatFeedbackFeed`
5. `UMVVM_LocalCombatState` 同步本地状态镜像
6. `UAOLocalTargetHealthBarObserverComponent` 根据本地相关反馈登记目标血条显示资格
7. 目标身上的 `UAOCombatFloatingTextComponent` 消费“应进世界跳字通道”的反馈

一句话说，当前已经不是“UI 自己订阅底层结果然后再猜一遍真相”，而是：

**战斗系统先给统一真相，HUD 桥接层把它翻译成本地表现数据，表现层只消费翻译后的结果。**

## 3. 当前消息总线边界

优先看：

- `Source/AegisOdyssey/AOCombatMessageSubsystem.*`
- `Source/AegisOdyssey/Player/AOPlayerController.*`

当前已经确认：

1. `OnCombatResultMessage` 现在是 native multicast，不再走 dynamic multicast。
2. `BroadcastCombatResult(...)` 是源入口，负责本地广播和服务端定向转发。
3. `BroadcastCombatResultLocal(...)` 只负责当前世界内广播一次，不做网络转发。
4. `AAOPlayerController::ClientBroadcastCombatResultMessage(...)` 收到 RPC 后只调用 `BroadcastCombatResultLocal(...)`。

这意味着两个历史误区已经过时：

1. 不要再把 `FAOCombatResultMessage` 塞回蓝图 dynamic multicast 参数复制链。
2. 不要再把客户端回放写回源广播函数形成递归回环。

## 4. 当前 HUD / MVVM 边界

优先看：

- `Source/AegisOdyssey/UI/AOHUDViewModelComponent.*`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_HUD.*`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_CombatFeedbackFeed.*`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_LocalCombatState.*`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_TargetHealthBarCollection.*`

当前已经确认：

1. `UAOHUDViewModelComponent` 是“统一战斗消息 -> 本地 HUD 数据”的桥接点。
2. `BuildLocalCombatFeedbackViewData(...)` 把统一结果翻译成本地玩家视角下的 `FAOCombatFeedbackViewData`。
3. `UMVVM_HUD::ApplyCombatFeedbackViewData(...)` 会把反馈继续写入 `UMVVM_CombatFeedbackFeed`。
4. `UMVVM_LocalCombatState` 只保存本地状态镜像，不反过来充当战斗结算真相源。
5. `UMVVM_TargetHealthBarCollection` 只是把观察者入口交给 UI，不自己持有目标真相。

所以当前应固定理解为：

1. 底层 combat message 不等于 UI 数据。
2. UI 不直接重解释统一结算真相。
3. HUD 聚合根负责子 ViewModel 组织，不再镜像一套旧事件反馈体系。

## 5. 当前 `FAOCombatFeedbackViewData` 的路由语义

优先看：

- `Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackViewData.h`
- `Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.*`

当前 `FAOCombatFeedbackViewData` 已经不只是“战斗结果抄一份”，还带有本地路由语义：

- `bIsLocalRelevant`
- `bIsLocalInstigator`
- `bIsLocalTarget`
- `bShouldEnqueueForHUD`
- `bShouldEnqueueForWorldFloatingText`
- `bIsImportantCombatFeedback`

当前默认路由含义是：

1. HUD 主提示优先按 `bShouldEnqueueForHUD` 走。
2. 世界跳字优先按 `bShouldEnqueueForWorldFloatingText` 走。
3. `bShouldEnqueueForWorldFloatingText` 当前主要服务于“本地玩家打出去且建议显示跳字”的结果。

## 6. 当前目标侧三类组件的职责拆分

### 6.1 `UAOLocalTargetHealthBarObserverComponent`

它当前只负责：

1. 站在本地玩家视角维护“哪些目标值得显示血条”。
2. 根据距离、战斗态、最近交战痕迹刷新目标显示资格。
3. 对目标身上的 `UAOTargetHealthBarComponent` 下发 `SetRequestedVisible(...)`。

它不负责：

1. 目标血量真相。
2. 世界跳字。
3. 原始战斗消息解释。

### 6.2 `UAOTargetHealthBarComponent`

它当前只负责：

1. 目标自身血量真相绑定。
2. 自己的 `UMVVMTargetHealthBar`。
3. 自己的头顶血条 `WidgetComponent`。
4. 血条显隐执行。

它不负责：

1. 跳字池。
2. 命中点锚点。
3. 多个瞬时跳字实例管理。

### 6.3 `UAOCombatFloatingTextComponent`

它当前只负责：

1. 接收已经完成本地路由的 `FAOCombatFeedbackViewData`。
2. 解析世界锚点位置。
3. 生成或复用跳字 `WidgetComponent`。
4. 把数据交给跳字 Widget 做表现。

它不负责：

1. 血条显隐。
2. 目标生命真相。
3. 本地观察资格判断。

## 7. 当前蓝图 / Widget 接入面

优先看：

- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.*`
- `Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.*`
- `Source/AegisOdyssey/UI/WorldHealthBar/AOCombatFloatingTextWidget.*`

当前主 HUD 侧可直接拿的入口包括：

1. `GetCombatResourcesViewModel()`
2. `GetLocalCombatStateViewModel()`
3. `GetCombatFeedbackFeedViewModel()`
4. `GetTargetHealthBarCollectionViewModel()`
5. `ConsumePendingCombatFeedback()`

蓝图便捷入口包括：

1. `GetMainHUDViewModel`
2. `GetCombatFeedbackFeedViewModel`
3. `GetTargetHealthBarCollectionViewModel`
4. `ConsumePendingCombatFeedbackFromFeed`
5. `ShouldRouteToHUD`
6. `ShouldRouteToWorldFloatingText`
7. `BuildRecommendedCombatText`

## 8. 当前排查顺序

如果战斗 UI / 目标血条 / 跳字表现不对，当前推荐顺序是：

1. 先查 `FAOCombatResultMessage` 是否真的被发出。
2. 再查 `UAOCombatMessageSubsystem` 是否只做了一次本地广播，没有回环。
3. 再查 `UAOHUDViewModelComponent::BuildLocalCombatFeedbackViewData(...)` 是否把该结果判成了本地相关。
4. 再查 `UMVVM_CombatFeedbackFeed` 是否收到了反馈。
5. 目标血条问题再查 `UAOLocalTargetHealthBarObserverComponent -> UAOTargetHealthBarComponent`。
6. 世界跳字问题再查 `bShouldEnqueueForWorldFloatingText -> UAOCombatFloatingTextComponent`。

## 9. 当前仍未完全收口的部分

这一轮确认的主要是 C++ 结构和职责边界，不等于所有表现已经收口。

后续仍需继续核的有：

1. 跳字 Widget 蓝图样式、动画、字号、颜色、轨迹。
2. 敌人 Blueprint 是否普遍已经挂齐 `UAOCombatFloatingTextComponent`。
3. 世界跳字池化是否要从轻量版继续升级。
4. 目标血条进入、延迟隐藏、距离隐藏是否所有资产上都已统一调好。

## 10. 本轮提炼来源

本轮主要从下面两篇历史文档提炼，并结合当前代码核对：

1. `Notice/HistoryNotice/战斗系统UI与目标血条-MVVM改造当前进度与新AI交接说明-2026-05-13.md`
2. `Notice/HistoryNotice/战斗系统UI与目标血条和目标侧跳字当前进度与新AI交接说明-2026-05-14.md`

当前更偏战斗主链的内容继续留在：

- [[战斗系统项目地图]]
- [[战斗系统已锁定设计]]
- [[战斗系统结算与防御语义说明]]

当前更偏战斗结果观察与表现下游的内容，则收束到本页。
