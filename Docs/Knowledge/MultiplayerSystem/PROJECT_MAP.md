---
title: Multiplayer System Project Map
tags:
  - knowledge
  - multiplayer-system
  - project-map
aliases:
  - Multiplayer System Project Map
  - 多人联机系统项目地图
---

# 多人联机系统项目地图
更新时间：2026-06-04

这份文档只做一件事：  
**把当前多人联机方向最值得先看的文档、工程入口和主干判断集中起来。**

相关文档：
- [[Multiplayer System Decisions]]
- [[Multiplayer System Known Issues]]
- [[Multiplayer System Current State 2026-06-04]]
- [[多人联机系统临时资料/多人联机系统预研笔记-2026-05-29]]
- [[多人联机系统临时资料/多人联机系统设计方案-2026-05-29]]
- [[多人联机系统临时资料/多人联机系统第一阶段推进记录-2026-05-29]]

## 1. 当前这套多人联机系统的主问题到底是什么

当前项目讨论的不是：

1. 编辑器里怎么双开两个玩家
2. 局域网试玩怎么随便跑通
3. 先做几个房间按钮

当前真正要解决的是：

**AegisOdyssey 如果以后要做成 Steam PC 场景下的正式多人游戏，应该怎么设计联机系统，并且怎样先立住 Dedicated Server 工程基线。**

所以当前最应该先记住的主线不是 UI，而是：

`联机产品目标 -> 会话与连接底座 -> GameMode/Experience 世界主链 -> Dedicated Server 工程基线`

## 2. 当前最值得先看的文档是什么

如果你刚接手这条线，建议优先按这个顺序看。

### 2.1 先看需求和认知澄清

- [[多人联机系统临时资料/多人联机系统预研笔记-2026-05-29]]

先看这份的原因不是它最正式，而是它把最容易说混的事情先拆开了：

1. PIE 多人模拟不等于正式联机
2. LAN 和 Dedicated Server 不是同一个维度
3. 当前真正关心的是“谁来承载服务器”
4. 项目最终希望同时容纳“玩家自托管”和“独立服务器托管”

### 2.2 再看项目级设计方案

- [[多人联机系统临时资料/多人联机系统设计方案-2026-05-29]]

这份文档是从“需求”走向“项目设计”的关键过渡。  
它重点回答：

1. 这套联机系统应该怎么分层
2. 为什么多人业务不能散在 Widget 里
3. 为什么要在项目层补联机协调层
4. 为什么首版应优先做在线玩家自托管闭环，但仍然保留 Dedicated Server 升级路径

### 2.3 最后看第一阶段推进记录

- [[多人联机系统临时资料/多人联机系统第一阶段推进记录-2026-05-29]]

这份文档最有价值的地方，是它把“现在真要开工，第一阶段到底先做什么”收住了。

当前最重要的推进顺序，已经被压成：

1. 先补 `Server Target`
2. 先清理阻碍 `Server` 构建的 Editor-only 依赖
3. 先选定最小多人测试地图和最小玩法链
4. 先验证 `DS 启动 + 客户端连接 + 两人进图`

## 3. 当前最值得先看的工程入口是什么

### 3.1 工程与构建入口

- [AegisOdyssey.uproject](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/AegisOdyssey.uproject)
- [AegisOdyssey.Build.cs](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AegisOdyssey.Build.cs)
- [AegisOdyssey.Target.cs](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey.Target.cs)
- [AegisOdysseyEditor.Target.cs](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdysseyEditor.Target.cs)

这里最关键的现状是：

1. 当前只有 `Game` 和 `Editor` 目标
2. 当前还没有独立的 `Server` 目标
3. `Build.cs` 里项目级 `OnlineSubsystem` 仍未正式接入
4. `Build.cs` 里当前直接包含了 `UnrealEd` 依赖，后续 Dedicated Server 基线阶段要重点排查

### 3.2 会话与用户底座入口

- [CommonSessionSubsystem.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Plugins/CommonUser/Source/CommonUser/Public/CommonSessionSubsystem.h)
- [CommonSessionSubsystem.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Plugins/CommonUser/Source/CommonUser/Private/CommonSessionSubsystem.cpp)
- [CommonUserSubsystem.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Plugins/CommonUser/Source/CommonUser/Public/CommonUserSubsystem.h)
- [CommonUserSubsystem.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Plugins/CommonUser/Source/CommonUser/Private/CommonUserSubsystem.cpp)

这一层是当前最值得复用的底层能力。  
后面项目自己的多人系统不应该绕开它们另造一套“自制会话基础设施”。

### 3.3 项目级全局承载入口

- [AOGameInstance.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameInstance.h)
- [AOGameInstance.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameInstance.cpp)
- [AOGameInstanceSubsystem.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/SubSystem/AOGameInstanceSubsystem.h)
- [AOGameInstanceSubsystem.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/SubSystem/AOGameInstanceSubsystem.cpp)

当前这层很轻，这反而是好事。  
它意味着后面可以比较干净地补一层专门的多人联机协调子系统。

### 3.4 世界与玩法承载入口

- [AOGameMode.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOGameMode.h)
- [AOGameMode.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOGameMode.cpp)
- [AOExperienceManagerComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.cpp)
- [AOWorldSettings.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOWorldSettings.h)
- [AOWorldSettings.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOWorldSettings.cpp)

当前多人系统不应该重造一套新的世界初始化框架。  
它更合理的方向是：会话成功后，把玩家送入现有 `GameMode / Experience` 主链。

## 4. 当前最该记住的主判断

### 4.1 当前项目不是零联机基础

项目已经有：

1. `CommonUser / CommonSession`
2. `CommonGame / CommonUI`
3. 现成的服务端世界主链
4. 知识库里已经明确的服务端权威玩法思路

### 4.2 当前项目还没进入 Dedicated Server 工程阶段

当前最明显的现实问题仍然是：

1. 没有 `Server Target`
2. 没有正式独立服务器构建路径
3. 还没有正式开始排查 Server 构建依赖边界

### 4.3 当前最先该做的不是多人大厅，而是 DS 工程基线验证

这条线当前的第一阶段，不应该先追求：

1. 房间列表
2. 匹配
3. 大厅 UI
4. 云服务器部署

而应当先验证：

1. DS 能不能构建
2. DS 能不能独立启动
3. 客户端能不能连接
4. 两名玩家能不能进入同一张测试地图

## 5. 当前多人联机知识库内部应该怎么跳转

建议从这里继续往下读：

1. [[Multiplayer System Current State 2026-06-04]]
2. [[Multiplayer System Decisions]]
3. [[Multiplayer System Known Issues]]

其中分工是：

1. `Current State` 记录当前真实处于哪一步
2. `Decisions` 只记录已经锁定的判断
3. `Known Issues` 记录当前明确还没解决、或者容易误判的点
