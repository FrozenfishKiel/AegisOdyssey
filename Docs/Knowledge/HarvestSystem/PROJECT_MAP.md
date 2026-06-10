---
title: Harvest System Project Map
tags:
  - knowledge
  - harvest-system
  - project-map
aliases:
  - Harvest System Project Map
  - 采集系统项目地图
---

# 采集系统项目地图

更新时间：2026-05-28  
适用范围：当前项目里“采集动作状态、工具轨迹命中、服务端重判定、统一结算、正式入包、节点生命周期”这条采集主链。  
不适用范围：具体某棵树或某种矿脉的最终资源蓝图配置、所有美术表现细节、未来尚未落地的 Rock/Bush/Ore 子类。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前采集系统的正式主链已经落到哪里。
2. 采集对象、采集工具、采集动作、统一结算、节点状态分别放在哪一层。
3. 哪些层是当前真相层，哪些层只是动作发起或表现层。
4. 后续继续扩采集系统时，先看哪些代码入口。

## 2. 当前正式主链

当前采集系统已经不是“预先选一个目标再请求掉资源”的旧理解。

当前已经落地的主链是：

**`StateTree -> STT_PlayHarvest -> GA_Harvest -> HarvestWindow -> Tool Socket Trace -> HarvestResolver -> HarvestableComponent -> InventoryStatics -> BackPack`**

这条链的正式语义是：

1. 角色进入一次采集动作状态
2. 状态树下发这次动作的蒙太奇和工具快照
3. `GA_Harvest` 在命中窗内按工具自身 Socket 做真实挥击判定
4. 服务端统一做最终重判定与统一结算
5. 奖励通过正式库存入口进入背包
6. 节点自身再推进进度、depleted、respawn 生命周期

## 3. 当前关键结构落点

### 3.1 动作发起层

优先看：

- `Source/AegisOdyssey/Harvest/StateTree/STT_PlayHarvest.*`
- `Source/AegisOdyssey/Harvest/Abilities/GA_Harvest.*`
- `Source/AegisOdyssey/Animation/NotifyState/AOHarvestWindow.*`

这一层负责：

1. 当前角色是否进入采集动作状态
2. 本次采集播哪段蒙太奇
3. 本次挥击用的是什么工具快照
4. 什么时候正式进入采集命中窗

当前已经确认：

- `StateTree` 不提前指定正式采集目标
- `AOHarvestWindow` 是正式采集结算入口
- 没有命中窗，挥击只会播动画，不会正式采集成功

### 3.2 采集工具层

优先看：

- `Source/AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.*`
- `Source/AegisOdyssey/Harvest/Definition/AOHarvestToolProfile.*`
- `Source/AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.*`
- `Source/AegisOdyssey/Harvest/Items/AOHarvestToolInstance.*`

当前已经确认的层次是：

1. `HarvestToolDefinition` 继续留在现有 `EquipmentDefinition` 体系里
2. `HarvestToolFragment` 负责采集配置块
3. `HarvestToolProfile` 负责机械语义身份
4. `HarvestToolInstance` 是正式运行时实例层

这说明采集工具当前已经不是“临时斧头特例”，而是完整接入了现有物品/装备体系。

### 3.3 采集对象层

优先看：

- `Source/AegisOdyssey/Harvest/Definition/AOHarvestableDefinition.*`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.*`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableTarget.h`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.*`
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.*`

当前已经确认：

1. `HarvestableDefinition` 只描述节点静态定义
2. `HarvestableComponent` 持有运行时真相
3. `IAOHarvestableTarget` 只暴露组件入口和 depleted/respawn 生命周期回调
4. `AOHarvestableActor` 是默认公共基类
5. `AOHarvestableTree` 已是树节点专用子类，不再只是方案概念
6. `AOHarvestableActor` 当前已经固定对象层桥接顺序：
   `公共默认状态 -> C++ 节点族默认表现 -> 蓝图轻量补充`

### 3.4 统一结算层

优先看：

- `Source/AegisOdyssey/Harvest/System/AOHarvestResolver.*`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestTypes.h`

当前 `HarvestResolver` 的职责已经稳定为：

1. 验证这次采集命中上下文是否合法
2. 根据 `ToolProfile` 与对象响应规则解析本次工具调参
3. 计算理论请求进度
4. 生成掉落奖励条目
5. 不直接改背包
6. 不直接改节点状态

当前它只做“统一重判定 + 统一结算”，不越权承担别的层职责。

### 3.5 正式入包层

优先看：

- `Source/AegisOdyssey/Harvest/Abilities/GA_Harvest.cpp`
- `Source/AegisOdyssey/Inventory/AOInventoryStatics.*`

当前奖励提交已经明确：

1. 采集结果先翻译成 `FAOInventoryReceiveBatch`
2. 先检查能否完整接收
3. 再走 `TryAddInventoryBatchToActor(...)`

这意味着当前采集系统没有偷偷拼旁路入包逻辑。

## 4. 当前真相层与观察/发起层

### 4.1 真相层

当前应视为真相层的有：

- `UAOHarvestableComponent`
- `FAOHarvestNodeRuntimeState`
- `UAOHarvestResolver`
- `GA_Harvest` 服务端奖励提交结果
- 背包最终入包结果

### 4.2 发起层 / 观察层

当前应视为发起层或观察层的有：

- `STT_PlayHarvest`
- `AOHarvestWindow`
- `FAOHarvestTargetData`
- 节点视觉/物理表现
- `HarvestGameplayCue` 及兜底表现链

这条边界很重要：

- 状态树负责发起动作
- 工具负责提供命中参数
- Resolver 负责结算
- 节点组件负责当前运行时状态

## 5. 当前采集命中语义

当前已经明确存在几条不能写反的语义：

1. 采集 Trace 必须来自工具自身的 `Start / End Socket`
2. 正式采集目标只能在命中窗内按本次挥击真实命中结果解析
3. 不能从摄像机直接发正式采集判定
4. `TargetActor / TargetComponent` 是运行时命中上下文，不是状态树预选目标

## 6. 当前 depleted / respawn 生命周期

优先看：

- `AOHarvestableComponent::ApplyHarvestResultWithContext(...)`
- `AOHarvestableComponent::StartRespawnTimerIfNeeded()`
- `AOHarvestableActor::HandleHarvestNodeDepleted_Implementation(...)`
- `AOHarvestableActor::OnHarvestNodeDepletedNative(...)`
- `AOHarvestableTree::OnHarvestNodeDepletedNative(...)`

当前已经落地的方向是：

1. 公共层先切运行时状态
2. 再通过接口分发 depleted / respawn 生命周期
3. `AOHarvestableActor` 先执行公共默认状态，再调用节点族 native 扩展点
4. 如有需要，最后才交给蓝图轻量补充
5. 当前 `Tree.HideTree` 已改为“先倒下，再延时隐藏”
6. 对象子类各自决定表现细节

这意味着“树会倒下”当前只是树子类行为，不是采集系统公共行为。

## 7. 当前目录分层

当前 Harvest 目录已经确认落地为：

- `Abilities/`
- `Core/`
- `Cue/`
- `Definition/`
- `Fragments/`
- `Items/`
- `Nodes/`
- `StateTree/`
- `System/`

其中 `Nodes/Tree/` 已经成立，说明目录治理不是历史建议，而是当前代码事实。

## 8. 当前继续扩展时的优先阅读顺序

如果后续继续接 `HarvestSystem`，当前推荐顺序是：

1. `Harvest/StateTree/STT_PlayHarvest.*`
2. `Harvest/Abilities/GA_Harvest.*`
3. `Animation/NotifyState/AOHarvestWindow.*`
4. `Harvest/Core/AOHarvestTypes.h`
5. `Harvest/System/AOHarvestResolver.*`
6. `Harvest/Core/AOHarvestableComponent.*`
7. `Harvest/Definition/AOHarvestableDefinition.*`
8. `Harvest/Definition/AOHarvestToolDefinition.*`
9. `Harvest/Fragments/AOHarvestToolFragment.*`
10. `Harvest/Items/AOHarvestToolInstance.*`
11. `Harvest/Core/AOHarvestableActor.*`
12. `Harvest/Nodes/Tree/AOHarvestableTree.*`
13. `TestProject/HarvestLifecycleTests.cpp`
14. `TestProject/TestHarvestLifecycleActors.*`

## 9. 本轮提炼来源

本轮主要从下面两篇历史文档提炼，并结合当前代码核对：

- `Notice/HistoryNotice/采集系统当前进度与目录规范交接说明-2026-05-12.md`
- `Notice/HistoryNotice/采集系统设计方案-对象定义-状态驱动-结算与同步框架.md`

沉淀后的稳定文档分别是：

- [[采集系统已锁定设计]]
- [[采集系统对象定义与结算链说明]]
- [[采集系统已知边界与历史偏差]]
