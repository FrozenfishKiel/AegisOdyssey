---
title: Multiplayer System Current State 2026-06-04
tags:
  - knowledge
  - multiplayer-system
  - current-state
aliases:
  - Multiplayer System Current State 2026-06-04
  - 多人联机系统当前状态 2026-06-04
---

# 多人联机系统当前状态
更新时间：2026-06-04

这份文档只记录当前多人联机线真实推进到了哪，不写未来愿景，不写泛泛教程。

相关文档：
- [[Multiplayer System Project Map]]
- [[Multiplayer System Decisions]]
- [[Multiplayer System Known Issues]]
- [[多人联机系统临时资料/多人联机系统设计方案-2026-05-29]]
- [[多人联机系统临时资料/多人联机系统第一阶段推进记录-2026-05-29]]

> [!important]
> 当前多人联机线已经从“纯预研聊天”进入“可写入知识库的设计阶段”，但还没有进入正式实施计划阶段。

## 1. 当前阶段已经完成了什么

### 1.1 需求侧主干已经收出来了

当前已经明确：

1. 目标不是编辑器内双开模拟
2. 目标不是只做局域网试玩
3. 目标是 Steam PC 场景下、以互联网联机为主的多人系统
4. 长期形态希望兼容“玩家自托管”和“独立服务器托管”

### 1.2 产品入口骨架已经有了第一版定义

当前已经收成：

`主界面`

1. 单人游戏
2. 多人游戏
3. 设置
4. 退出

`多人游戏`

1. 创建游戏
2. 加入游戏
3. 返回

这里的含义已经基本定住：

1. `创建游戏` 首版默认优先服务于玩家自托管
2. `加入游戏` 作为未来可扩展的总入口保留

### 1.3 项目级设计思路已经明确

当前已经明确的设计方向是：

1. 不要把多人业务直接散进 Widget
2. 项目层需要联机协调层
3. 底层优先复用 `CommonUser / CommonSession`
4. 会话成功后继续进入现有 `GameMode / Experience` 世界主链

### 1.4 Dedicated Server 的关键认知边界已经补清楚

当前已经明确：

1. 项目现在缺的是单独的 `Server Target`
2. `Server Target` 写的是构建身份，不是多人玩法逻辑
3. 编辑器里的 DS 调试形态不等于正式独立 DS 程序
4. Dedicated Server 基线不应该拖到项目做完才第一次验证

## 2. 当前工程现状已经确认到什么程度

### 2.1 当前工程已有的基础

当前项目已经有：

1. `CommonGame`
2. `CommonUser`
3. `OnlineFramework`
4. `CommonUI`
5. `Networking`
6. `NetCore`

同时还存在：

1. `UAOGameInstance`
2. `UAOGameInstanceSubsystem`
3. `AAOGameMode`
4. `UAOExperienceManagerComponent`

这说明项目并不是纯单机空白工程。

### 2.2 当前工程的明确缺口

当前最直接的缺口是：

1. 还没有独立 `Server Target`
2. 还没有正式 Dedicated Server 构建路径
3. 还没有项目级多人联机协调子系统
4. 还没有正式锁定在线服务路线

### 2.3 当前已识别出的工程风险

目前已明确的工程风险之一是：

1. `AegisOdyssey.Build.cs` 当前直接带着 `UnrealEd`

这意味着只要项目开始补 `Server Target`，构建边界问题大概率会第一时间暴露出来。

## 3. 当前最合理的下一阶段目标是什么

当前最合理的下一阶段目标不是“完整多人系统开工”，而是：

**进入最小联机验证闭环阶段。**

这条线当前已经被压成：

1. 补 `Server Target`
2. 清理阻碍 `Server` 构建的 Editor-only 依赖
3. 选定最小多人测试地图和最小玩法链
4. 验证 `DS 启动 + 客户端连接 + 两人进图`

## 4. 当前阶段还没有进入什么

为了防止后续误判，这里单独列出来。

当前多人联机线还没有进入：

1. 正式实施计划
2. 正式多人菜单实现
3. 正式服务器列表实现
4. 云服务器部署阶段
5. 全玩法联机化阶段

## 5. 当前阶段最应该怎么使用这套知识库

如果后面继续推进多人联机，当前推荐的阅读顺序是：

1. [[Multiplayer System Project Map]]
2. [[Multiplayer System Decisions]]
3. [[Multiplayer System Known Issues]]
4. [[多人联机系统临时资料/多人联机系统设计方案-2026-05-29]]
5. [[多人联机系统临时资料/多人联机系统第一阶段推进记录-2026-05-29]]

这里的作用分工已经比较清楚：

1. `Project Map` 管入口
2. `Decisions` 管已锁定判断
3. `Known Issues` 管当前缺口和风险
4. `Current State` 管当前推进到了哪
