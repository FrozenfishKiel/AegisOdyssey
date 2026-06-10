---
title: Multiplayer System Known Issues
tags:
  - knowledge
  - multiplayer-system
  - known-issues
aliases:
  - Multiplayer System Known Issues
  - 多人联机系统已知问题
---

# 多人联机系统已知问题
更新时间：2026-06-04

这份文档只记录当前已经明确存在的问题、风险和容易误判的点。

相关文档：
- [[Multiplayer System Project Map]]
- [[Multiplayer System Decisions]]
- [[Multiplayer System Current State 2026-06-04]]

## 1. 当前工程还没有独立 `Server Target`

这是当前最直接的工程缺口。

现状是：

1. 有 `Game` 目标
2. 有 `Editor` 目标
3. 没有 `Server` 目标

这意味着当前项目还不能正式产出独立 Dedicated Server 程序。

## 2. 当前项目还没有正式决定在线服务路线

虽然已经存在 `CommonUser / CommonSession` 基础设施，但项目层还没有正式锁定：

1. 具体在线平台接线方案
2. 登录、会话、搜索、邀请到底优先走哪套服务

这件事和 Dedicated Server 构建不是同一个问题，但后续一定会影响多人产品层实现。

## 3. `Build.cs` 当前含有 Editor-only 风险依赖

当前 [AegisOdyssey.Build.cs](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AegisOdyssey.Build.cs) 里直接包含了 `UnrealEd`。

这在编辑器开发阶段未必立刻有问题，但一旦开始补 `Server Target`，这类依赖通常就是优先排查对象。

当前应把它视为：

1. Dedicated Server 构建阶段的高风险项
2. 很可能导致 Server 构建边界问题的入口

## 4. 当前还没有单独锁定“最小多人测试地图”和“最小玩法链”

当前设计层已经反复强调要先做最小验证闭环，但这一层还没有正式落成工程内的唯一测试目标。

如果这件事不先明确，后面很容易出现：

1. 一边补 DS，一边顺手修很多无关玩法
2. 地图选择不断漂移
3. 验证范围不断扩大

## 5. 当前多人菜单还没有正式产品层落地

虽然入口骨架已经讨论清楚：

1. `单人游戏`
2. `多人游戏`
3. `创建游戏`
4. `加入游戏`

但这些还主要停留在设计和笔记阶段，还没有进入正式项目实现。

这意味着当前不能误判成“产品入口已经完成”，只能算“产品入口已初步定型”。

## 6. 当前还没有项目级的多人联机协调层

现状是：

1. 有底层 `CommonUser / CommonSession`
2. 有现成 `GameMode / Experience` 世界主链
3. 但还没有 AegisOdyssey 自己的多人联机协调子系统

这意味着如果现在直接开始做多人菜单，极易把联机业务逻辑散落进 UI。

## 7. 当前最容易出现的误判：把编辑器里的 DS 当成正式 Dedicated Server

这是当前特别需要防止的认知错误。

当前必须明确：

1. 编辑器里的 DS 更接近调试服务器形态
2. 只有 `Server Target` 构建出来的独立服务器程序，才算正式 Dedicated Server 构建路径的一部分

如果这条边界不守住，后面很容易误以为“本地调试通了，就等于服务器阶段没问题”。

## 8. 当前多人线还没有进入正式实施计划阶段

虽然已经有：

1. 预研笔记
2. 设计方案
3. 第一阶段推进记录

但当前还没有正式写成逐文件、逐步骤、逐验证命令的实施方案。

所以现在不能误以为“已经可以无前置检查直接开工”。

## 9. 当前第一阶段最怕的风险是范围失控

根据当前讨论，第一阶段最容易失控的方式包括：

1. 先做大厅 UI
2. 先做服务器列表
3. 先做匹配
4. 先做云服务器部署
5. 一边补 DS，一边顺手修大量无关玩法

这些都属于偏离“最小联机验证闭环”的信号。
