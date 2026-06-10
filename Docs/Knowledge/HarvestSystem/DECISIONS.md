---
title: Harvest System Decisions
tags:
  - knowledge
  - harvest-system
  - decisions
aliases:
  - Harvest System Decisions
  - 采集系统已锁定设计
---

# 采集系统已锁定设计

更新时间：2026-05-28  
适用范围：当前采集系统对象边界、动作边界、命中边界、结算边界里已经锁定为稳定结论的设计。  
不适用范围：具体资源蓝图调参、最终美术/音效方案。

## 1. 采集的本质语义已经锁定

已经锁定：

1. 采集要按“工具挥击 -> 命中对象 -> 结算资源”理解
2. 不是预先选好目标再请求掉落
3. 不是摄像机对准某物就算正式采集

## 2. 正式采集目标只能在命中窗内解析

已经锁定：

1. `StateTree` 只负责发起这次采集动作
2. 不提前写死正式采集目标
3. `TargetActor / TargetComponent` 只表示本次真实命中后解析出的运行时上下文

## 3. 采集 Trace 必须来自工具自身

已经锁定：

1. 采集轨迹必须来自工具自身 `TraceStartSocketName / TraceEndSocketName`
2. 不允许从玩家摄像机直接发正式采集判定
3. 工具如何判定命中，由 `HarvestToolFragment.HitCheckConfig` 决定

## 4. 采集与战斗共享思路，不强行共享类

已经锁定：

1. 动画驱动命中窗口
2. 工具自身轨迹判定
3. 服务端统一重判定和结算
4. 实现仍保持采集自己的入口文件与结构

## 5. 采集工具继续留在现有物品/装备体系里

已经锁定：

1. `HarvestToolDefinition` 继承 `EquipmentDefinition`
2. `HarvestToolInstance` 是正式运行时实例层
3. `HarvestToolFragment` 是工具定义里的采集配置块
4. `HarvestToolProfile` 承担机械语义身份

这意味着后续不要再起一套脱离现有装备链的新“采集工具系统”。

## 6. 采集对象自己决定是否接受一次采集请求

已经锁定：

1. 工具提供自己的能力和参数
2. 对象通过自身定义与当前运行时状态决定是否接受
3. 对象不只是被动资源袋

当前正确理解是：

- 工具负责“我怎么采”
- 对象负责“我接不接受这次采”

## 7. 采集节点运行时真相已经锁定在组件层

已经锁定：

1. `HarvestableDefinition` 只放静态定义
2. `HarvestableComponent` 维护 `CurrentProgress / bDepleted / bRespawnPending`
3. `HarvestableActor` 是公共对象层桥接器，不再只是薄壳
4. 公共桥接顺序固定为：
   `ApplyDefault... -> OnHarvestNode...Native -> ReceiveHarvestNode...`
5. 子类对象自己决定 depleted / respawn 表现

## 8. 节点耗尽后的处理必须统一入口 + 子类多态

已经锁定：

1. 公共层只做状态切换与生命周期分发
2. 树、石头、草、矿脉等对象以后各自处理自己的耗尽反应
3. 不把“树倒下”误写成采集系统唯一公共表现
4. Tree 第一阶段已改成走 native 扩展点，不再直接覆写接口最终入口
5. Tree 当前 `HideTree` 语义不是立刻消失，而是先倒地反馈，再由树子类自己的延时隐藏收尾

## 9. 采集奖励必须先能完整入包，才能正式结算

已经锁定：

1. 采集奖励先翻译成正式库存批次
2. 先检查能否完整接收
3. 不能完整接收就整次失败
4. 不允许节点进度已经扣掉但玩家没拿到东西

## 10. 奖励入包统一走正式库存入口

已经锁定：

1. `Harvest` 不直接改背包
2. 不自己写旁路 UI 结果
3. 统一走 `InventoryStatics`

## 11. 服务端权威范围已经锁定

已经锁定：

1. 正式命中是否合法
2. 节点进度如何变化
3. 奖励是什么
4. 奖励是否进包
5. 节点是否 depleted / respawn

客户端当前只负责：

1. 动作表现
2. 轻量预览
3. 状态展示

## 12. 多人同时采集的第一版规则已经锁定

已经锁定：

1. 默认共享采集
2. 不做独占锁定
3. 服务器按顺序结算
4. 后到请求如果节点已空，直接失败

## 13. 掉落时机按掉落条目自己配置

已经锁定：

1. `PerHit`
2. `OnDepleted`
3. `Both`

这意味着奖励发放时机当前不是系统全局写死的一套。

## 14. 目录分层已经锁定

已经锁定：

1. `Harvest/` 下按业务层分目录
2. 不再回到 `Actors/Components/Interfaces/Types` 这类机械平铺
3. 具体节点继续按 `Nodes/Tree/`、后续 `Nodes/Rock/`、`Nodes/Bush/`、`Nodes/Ore/` 扩展

## 15. Harvest GameplayCue 当前仍保留兜底链

已经锁定的当前事实是：

1. 节点组件有 `MulticastPlayHarvestCue(...)`
2. 当前表现可以不完全依赖标准 GC 资产注册链

这说明“表现能播出来”当前已成立，但“Harvest Cue 已完全回到项目统一 GC 资产规范”不能写成既成事实。
