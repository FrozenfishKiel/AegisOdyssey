---
title: Harvest System Known Issues
tags:
  - knowledge
  - harvest-system
  - known-issues
aliases:
  - Harvest System Known Issues
  - 采集系统已知边界与历史偏差
---

# 采集系统已知边界与历史偏差

更新时间：2026-05-28  
适用范围：当前 `HarvestSystem` 这一轮深提炼里已经识别出的历史混层、实现边界与后续整理风险。  
不适用范围：完整运行时 bug 列表。

## 1. 当前这两篇历史文档混了设计、交接、目录治理和排坑

这一轮来源文档里同时混有：

1. 采集对象/工具/结算的长期设计边界
2. 当前已经落地的主链交接
3. 目录重构规范
4. 编码/注释/GC 资产链等排坑记录

因此本轮知识包没有把它们塞成一篇“采集总文档”，而是拆成：

- `PROJECT_MAP`
- `DECISIONS`
- `OBJECTS_AND_RESOLUTION`
- `KNOWN_ISSUES`

## 2. 当前最容易误判的边界

### 2.1 不要把状态树预览目标写成正式采集目标

当前不是。

已确认当前事实：

1. `STT_PlayHarvest` 只负责发起动作
2. 正式目标由命中窗内真实挥击结果解析

### 2.2 不要把 Resolver 写成节点真相层

当前不是。

已确认当前事实：

1. Resolver 只做统一重判定和统一结算
2. 当前进度真相仍在 `HarvestableComponent`

### 2.3 不要把“树会倒”写成公共采集系统行为

当前不是。

树倒下当前只是 `AOHarvestableTree` 子类自己的 depleted 反应，包含 `HideTree` 分支下的延时隐藏收尾。

### 2.4 不要把 Harvest GameplayCue 写成“已经彻底回归标准资产注册链”

当前不能这样写。

已确认当前事实：

1. 采集表现当前能播
2. 但组件仍保留了 `MulticastPlayHarvestCue(...)` 兜底链
3. 因此标准 GC 资产路径治理不能写成已完全收口

### 2.5 不要把历史建议目录结构原样当成当前目录事实

历史设计文里曾建议过 `Components/Actors/Types` 等分法。  
当前真实代码已经重构成 `Core/Definition/Fragments/Items/Nodes/...` 这套目录。

## 3. 当前仍未完全收口、但价值很高的后续主题

以下内容都已显露价值，但不在本轮正文沉淀范围内：

1. `Rock / Bush / Ore` 等更多节点子类
2. 树节点完整资源蓝图、物理主干体、表现资源接线模板
3. Harvest Cue 标准资产注册链彻底回归项目统一规范
4. 历史乱码注释和可读性清理
5. 采集资源实例层更丰富的品质/纯度/批次语义
6. `AegisOdyssey.Harvest` 自动化测试当前需要带 `-DDC-ForceMemoryCache` 启动，
   否则命令行环境可能先死在本机 DDC 可写节点配置上

## 4. 当前整理规则

后续继续往 `Docs/Knowledge/HarvestSystem` 提炼时，默认遵守：

1. 先区分“动作发起”“对象运行时真相”“统一结算”“正式入包”“节点表现子类”五层，不要混写。
2. 任何涉及“正式采集目标从哪里来”的说法，优先核 `STT_PlayHarvest`、`GA_Harvest` 和 `AOHarvestWindow`。
3. 任何涉及“节点当前进度谁说了算”的说法，优先核 `AOHarvestableComponent`。
4. 任何涉及“奖励怎么进背包”的说法，优先核 `BuildRewardReceiveBatch(...)` 与 `InventoryStatics`。
5. 任何涉及“当前目录规范”的说法，都按实际代码目录写，不按早期建议目录写。
