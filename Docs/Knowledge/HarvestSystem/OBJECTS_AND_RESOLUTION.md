---
title: Harvest Objects And Resolution
tags:
  - knowledge
  - harvest-system
  - objects
  - resolution
aliases:
  - Harvest Objects And Resolution
  - 采集系统对象定义与结算链说明
---

# 采集系统对象定义与结算链说明

更新时间：2026-05-19  
适用范围：当前采集系统对象定义层、工具层、节点运行时层、命中结算层、入包链。  
不适用范围：所有具体资源 BP、美术参数、未落地的更多节点子类。

## 1. 当前采集不是“树自己掉木头”

当前主链里，节点对象不直接改背包，也不直接做完整系统结算。

当前分层已经很明确：

1. 状态树负责发起采集动作
2. Ability 负责消费动作快照并在命中窗内提交一次采集命中
3. Resolver 负责统一结算
4. 节点组件负责当前运行时状态
5. 正式库存入口负责入包

## 2. 采集工具当前怎么组织

优先看：

- `AOHarvestToolDefinition`
- `AOHarvestToolFragment`
- `AOHarvestToolProfile`
- `AOHarvestToolInstance`

当前层次语义是：

1. `HarvestToolDefinition`
   - 回答“这件可装备物作为采集工具时是什么”
   - 当前默认实例类型已指向 `HarvestToolInstance`

2. `HarvestToolFragment`
   - 回答“这把工具怎么采”
   - 当前至少承载 `BaseHarvestPower` 和 `HitCheckConfig`

3. `HarvestToolProfile`
   - 回答“这类工具在采集规则里的机械身份是什么”
   - 当前是对象响应规则的 key

4. `HarvestToolInstance`
   - 回答“这把具体工具是谁”
   - 当前主链已经允许优先带实例快照，不再只回头读 Definition

## 3. 采集对象当前怎么组织

优先看：

- `AOHarvestableDefinition`
- `AOHarvestableComponent`
- `AOHarvestableTarget`
- `AOHarvestableActor`

当前语义是：

1. `HarvestableDefinition`
   - 回答“这个节点本身是什么”
   - 当前放静态定义：总进度、重生配置、默认工具响应、差异化工具响应、掉落条目、命中/耗尽表现配置

2. `HarvestableComponent`
   - 回答“这个节点现在是什么状态”
   - 当前维护：`CurrentProgress`、`bDepleted`、`bRespawnPending`

3. `HarvestableTarget`
   - 回答“外部怎么拿到这个对象的采集组件，以及 depleted/respawn 后怎么分发对象侧反应”

4. `HarvestableActor`
   - 是默认公共基类
   - 默认公共 depleted/respawn 反应是开关碰撞

## 4. 树节点当前怎么特殊化

优先看：

- `Harvest/Nodes/Tree/AOHarvestableTree.*`

当前树子类已经落地的方向是：

1. 树耗尽后先退出采集 Trace 命中链
2. 再按配置决定是隐藏、保留倒地树、还是直接销毁
3. 可选开启物理并施加倒地方向冲量
4. respawn 时恢复可见、碰撞和可采状态

这说明“树节点原生基类”当前已经不是纯设计稿，而是正式结构的一部分。

## 5. 当前命中上下文怎么传

优先看：

- `AOHarvestTypes.h`

当前正式结构至少包括：

1. `FAOHarvestRuntimeContext`
2. `FAOHarvestHitContext`
3. `FAOHarvestTargetData`
4. `FAOHarvestResult`
5. `FAOHarvestRewardEntry`
6. `FAOHarvestLifecycleContext`

其中最关键的边界是：

1. `ToolDefinition / ToolInstance` 是这次动作开始时的稳定工具快照
2. `ToolFragment` 只是进程内便捷缓存，不是稳定网络字段
3. `TargetActor / TargetComponent` 是命中后才出现的上下文，不参与激活数据稳定快照

## 6. 当前动作发起到命中提交流程

当前正式链路是：

1. `STT_PlayHarvest` 读取当前已装备工具
2. 找到匹配输入标签的已授予采集 Ability
3. 组装 `FAOHarvestTargetData`
4. 下发蒙太奇、播放速率、工具快照
5. `GA_Harvest` 激活并播放蒙太奇
6. `AOHarvestWindow` 打开 `State_Harvest_HitWindow`
7. `UAT_WaitHarvestHit` 在每个命中窗只提交一次采集命中
8. 权威端进入 `ExecuteHarvestHit()`

## 7. 当前 Resolver 结算做什么，不做什么

优先看：

- `AOHarvestResolver::ResolveHarvestRequest(...)`
- `AOHarvestResolver::FinalizeHarvestRewards(...)`

当前 Resolver 做的事：

1. 验证上下文是否合法
2. 验证目标当前还能否接受采集
3. 根据对象 `ToolProfileResponses` 或 `DefaultToolResponse` 解析工具调参
4. 计算 `RequestedProgress`
5. 构建奖励条目

当前 Resolver 不做的事：

1. 不直接扣节点当前进度
2. 不直接切 depleted
3. 不直接改背包

## 8. 当前节点进度谁来扣

当前这一步已经明确收口到：

- `HarvestableComponent::ResolveHarvestProgressRequest(...)`

也就是说：

1. Resolver 给出这次理论请求进度
2. 节点组件结合当前 `CurrentProgress` 算出真正 `AppliedProgress`
3. 节点组件决定 `RemainingProgress` 和 `bDepletedAfterHit`

这保证了“当前进度真相”仍然在节点组件，不在 Resolver 副本里。

## 9. 当前奖励什么时候真正提交

当前顺序已经明确：

1. Resolver 生成奖励条目
2. `GA_Harvest` 把奖励翻译成 `FAOInventoryReceiveBatch`
3. 先检查能否完整接收
4. 完整接收成功后，才调用 `ApplyHarvestResultWithContext(...)`

这条顺序很关键，因为它保证：

**背包装不下时，这次采集整体不成立，不会出现节点已扣进度但奖励没进包。**

## 10. 当前节点生命周期怎么分发

当前正式链路是：

1. `ApplyHarvestResultWithContext(...)`
2. 组件切 `CurrentProgress / bDepleted`
3. depleted 时分发 `HandleHarvestNodeDepleted(...)`
4. 如可重生则启动 respawn timer
5. timer 到时 `ResetHarvestNodeState()`
6. 再分发 `HandleHarvestNodeRespawned()`

这说明 depleted / respawn 当前已经是一套正式生命周期，不是仅靠蓝图临时写法维持。

## 11. 当前目录治理结果

这一轮核对后确认，Harvest 当前已经明确分成：

1. `Core`
2. `Definition`
3. `Fragments`
4. `Items`
5. `Abilities`
6. `StateTree`
7. `System`
8. `Cue`
9. `Nodes`

因此后续继续加 `Rock / Bush / Ore` 时，正确方向是继续扩 `Nodes/`，而不是回到技术类型平铺。
