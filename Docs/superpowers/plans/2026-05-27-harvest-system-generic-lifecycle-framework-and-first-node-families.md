# Harvest System Generic Lifecycle Framework And First Node Families Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为采集系统建立通用的节点耗尽/重生生命周期框架，并以树、岩石、灌木作为第一批正式落地对象验证这套系统抽象。

**Architecture:** 保持现有正式链路 `StateTree -> GA_Harvest -> Resolver -> HarvestableComponent -> Inventory` 不变，把“节点采空后对象如何表现、对象如何在重生后恢复”统一收口到对象层生命周期框架。`UAOHarvestableComponent` 继续负责运行时真相与生命周期切换，`AAOHarvestableActor` 负责系统级公共桥接，具体节点族只是在这套框架上实现自己的对象表现，蓝图只负责挂资源和少量演出。

**Tech Stack:** Unreal Engine 5 C++, Gameplay Ability System, StateTree, Blueprint 子类资源配置

---

## 1. 先明确系统边界

这轮不是“给树补个事件，再顺手做石头和灌木”。

这轮真正要做的是采集系统通用能力：

1. 任意采集节点在合法采空后，都能走统一的对象层生命周期桥。
2. 任意采集节点在 respawn 后，都能走统一的恢复入口。
3. 树、岩石、灌木只是第一批对象族，用来验证这套系统，而不是系统本体。

也就是说，这轮交付必须先回答：

- 采集系统怎样统一表达“节点已经被采空”这件事？
- 采集系统怎样统一表达“节点已经恢复可采”这件事？
- 对象层怎样在不污染主链的前提下承接这些生命周期？
- 后续新增 `Ore`、`Fiber`、`Herb`、`Crystal`、`Coral` 等节点时，如何沿用同一套抽象？

如果这个问题没有先立住，哪怕把树、岩石、灌木都做出来，后面系统还是会散。

---

## 2. 当前已锁定、不能退回去的系统事实

根据当前知识库和源码，下面这些边界已经成立：

- 正式采集目标不是 StateTree 预选出来的，而是在命中窗中通过工具自身 Socket Trace 命中后解析出来的。
- `UAOHarvestableComponent` 持有节点运行时真相：
  - `CurrentProgress`
  - `bDepleted`
  - `bRespawnPending`
- `UAOHarvestResolver` 只负责统一校验和统一结算：
  - 不直接改背包
  - 不直接改节点运行时状态
- 奖励统一走 `InventoryStatics`，不能绕开库存正式入口。
- `AAOHarvestableActor` 是当前对象层公共基类。
- `AAOHarvestableTree` 是当前唯一正式落地的节点族子类。
- `IAOHarvestableTarget` 已经定义了对象层生命周期入口：
  - `HandleHarvestNodeDepleted(const FAOHarvestLifecycleContext&)`
  - `HandleHarvestNodeRespawned()`

这意味着这轮不能做的事包括：

- 不能把节点真相挪出 `HarvestableComponent`
- 不能把采空表现塞回 `Resolver` 或 `GA_Harvest`
- 不能把系统重新做成“树专属方案”
- 不能让蓝图接管核心状态切换

---

## 3. 需求重述

你提的两个表面需求：

1. 每个采集点采集完毕后给一个事件，用于树倒塌、石头破坏、灌木消失等对象反应
2. 除了树之外，还要继续做岩石采集和灌木采集

真正对应到系统设计，应该被重述为：

### 3.1 系统级需求

采集系统需要一个通用的对象层生命周期框架，统一承接：

- depleted 生命周期
- respawn 生命周期

这个框架必须满足：

- C++ 公共基类主导
- 节点族用 C++ 子类覆写自己的表现
- 蓝图只做资源和轻量演出
- 同一套 respawn 语义被所有节点共用

### 3.2 第一批落地对象

在这套系统框架之上，当前第一批正式落地对象是：

- Tree
- Rock
- Bush

它们只是第一批验证对象，不是系统边界本身。

---

## 4. 方案比较

### 方案 A：继续按对象一个个补接口实现

做法：

- 每个对象子类直接自己实现 `HandleHarvestNodeDepleted/Respawned`
- 树、岩石、灌木分别写自己的逻辑

优点：

- 上手快
- 改动少

缺点：

- 没有系统级统一时序
- 以后新增第四种节点时很容易继续复制粘贴
- 很容易退化成“某几个节点能跑，但系统抽象没有真正立住”

结论：

- 适合临时补丁，不适合这轮系统建设

### 方案 B：推荐方案，系统级桥接框架 + 节点族覆写

做法：

- `HarvestableComponent` 继续维护运行时真相与状态切换
- `AAOHarvestableActor` 作为系统级桥接层，统一承接 depleted / respawn 生命周期
- `AAOHarvestableActor` 固化公共处理顺序：
  - 公共默认状态切换
  - native hook
  - Blueprint 轻补充
- `Tree / Rock / Bush` 作为第一批节点族，只实现自己的对象表现

优点：

- 系统边界清楚
- 对象层职责清楚
- 第一批对象能做，后续更多对象也能沿用
- 符合你要求的 C++ 主导模式

缺点：

- 需要先做一次公共桥接设计，不是立刻堆子类

结论：

- 这是当前正确方向

### 方案 C：把生命周期和表现全部数据化

做法：

- 在 `HarvestableDefinition` 里配耗尽表现、隐藏策略、延迟、物理、碎裂等
- 通用 Actor 基类按配置解释执行

优点：

- 看起来可配置强

缺点：

- 会把对象语义重新挤回数据表
- 很快把 `Definition` 做成杂糅配置容器
- 不符合当前“子类自己决定表现”的设计方向

结论：

- 当前阶段过重，不采用

---

## 5. 推荐系统设计

采用方案 B。

### 5.1 系统分层

这套框架应长期保持这四层：

#### 第 1 层：采集主链层

包括：

- `StateTree`
- `GA_Harvest`
- `HarvestWindow`
- `Resolver`
- `Inventory`

职责：

- 发起动作
- 命中对象
- 做统一校验
- 做统一奖励结算

这一层不关心树会不会倒、石头会不会碎、灌木会不会隐藏。

#### 第 2 层：节点运行时真相层

核心是：

- `UAOHarvestableComponent`

职责：

- 维护节点当前进度
- 维护 depleted / respawn pending 状态
- 切换生命周期状态
- 广播生命周期事件

这一层回答的是“节点现在处于什么状态”，不是“节点怎么演”。

#### 第 3 层：系统级对象生命周期桥

核心是：

- `AAOHarvestableActor`

职责：

- 把组件发出来的 depleted / respawn 生命周期桥接到对象层
- 执行公共默认处理
- 给节点族提供稳定的 C++ 扩展点
- 给蓝图提供轻量可选演出入口

这一层是这轮最关键的新系统抽象。

#### 第 4 层：节点族对象表现层

第一批包括：

- `AAOHarvestableTree`
- `AAOHarvestableRock`
- `AAOHarvestableBush`

未来还可以包括：

- `AAOHarvestableOre`
- `AAOHarvestableHerb`
- `AAOHarvestableCrystal`

职责：

- 只定义自己的 depleted / respawn 表现
- 不接管主链
- 不接管运行时真相
- 不发明自己的 respawn 机制

---

## 6. 系统级生命周期桥应该长什么样

### 6.1 统一顺序

推荐系统统一采用这个顺序：

1. `HarvestableComponent` 发现节点进入 depleted 或 respawned
2. `AAOHarvestableActor` 接收该生命周期通知
3. 基类执行公共默认状态切换
4. 基类调用 C++ native hook
5. 基类再触发 Blueprint 轻量事件

这条顺序必须被系统锁死，原因是：

- 公共状态切换应该先于对象表现
- 节点族逻辑应该先于蓝图演出
- 蓝图只能补表现，不能篡改主状态

### 6.2 为什么这不是“随便加个事件”

这里的“事件”本质上不是裸广播，而是系统生命周期桥。

如果只是加一个 delegate：

- 监听方会变多
- 调用顺序会变散
- 谁负责公共处理会不清楚

所以这里应该做的是：

- 生命周期主入口函数
- 固定时序
- 稳定 native hook
- 可选 Blueprint 观察口

而不是“到处都能监听一下”的松散事件模型。

### 6.3 推荐的基类能力

`AAOHarvestableActor` 应成为系统级基类桥接器，至少承担：

- 保存 Primitive 碰撞快照
- 默认 depleted 状态切换
- 默认 respawn 状态恢复
- `OnHarvestNodeDepletedNative(...)`
- `OnHarvestNodeRespawnedNative()`
- `ReceiveHarvestNodeDepleted(...)`
- `ReceiveHarvestNodeRespawned()`

其中：

- `Native` 是给 C++ 子类覆写的正式扩展口
- `Receive...` 是给蓝图挂轻量表现的补充口

蓝图不应该直接承担：

- `HandleHarvestNodeDepleted` 的主逻辑
- respawn 时机控制
- 运行时状态切换

---

## 7. 第一批对象族如何挂在这套系统上

### 7.1 Tree

Tree 是第一批里的复杂模板。

它在系统中的定位不是“采集系统本体”，而是：

- 一个具备方向性和物理倒塌表现的节点族样板

它的对象层表现包括：

- depleted 时退出采集命中链
- 按配置决定隐藏、保留倒地或销毁
- 如有需要启用物理倒地
- respawn 时恢复可见、碰撞、物理关闭、重新可采

### 7.2 Rock

Rock 是第一批里的破坏型模板。

它在系统中的定位是：

- 验证“非树类破坏对象”是否能复用同一套生命周期框架

它的对象层表现包括：

- depleted 时退出采集命中链
- 播放碎裂或破坏表现
- 最终隐藏或销毁
- respawn 时恢复到完整静态可采形态

Rock 不需要树的倒地方向逻辑。

### 7.3 Bush

Bush 是第一批里的轻量模板。

它在系统中的定位是：

- 验证“没有复杂物理表现的轻节点”也能走同一套框架

它的对象层表现包括：

- depleted 时退出采集命中链
- 直接隐藏或缩回
- respawn 时恢复可见和碰撞

Bush 不应被做成树的弱化版，也不应强行套碎裂逻辑。

---

## 8. respawn 必须是系统统一语义

这一点必须写死，因为它决定系统是不是系统。

统一约束如下：

- 所有节点族共用 `FAOHarvestRespawnConfig`
- respawn 计时与状态切换统一由 `UAOHarvestableComponent` 控制
- 节点族只负责“恢复后长什么样”

明确禁止：

- 树自己再起一套 timer
- 岩石自己再写一套 respawn 机制
- 灌木通过蓝图 Delay 自己恢复
- 后续任何新节点族发明自己的“半系统化 respawn”

只要这些约束被破坏，系统就会回到对象各写各的状态机。

---

## 9. 目录与文件规划

### 9.1 现有文件需要承担的系统职责

- `Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.h/.cpp`
  - 保持运行时真相层职责
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableTarget.h`
  - 保持生命周期桥接口定义
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h/.cpp`
  - 升格为系统级对象生命周期桥
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h/.cpp`
  - 作为第一批复杂节点族模板

### 9.2 第一批新增文件

- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h`
- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`

### 9.3 蓝图侧落点

不是这轮方案的主逻辑，但要预留使用方式：

- `BP_*Tree` 继承 `AAOHarvestableTree`
- `BP_*Rock` 继承 `AAOHarvestableRock`
- `BP_*Bush` 继承 `AAOHarvestableBush`

蓝图只负责：

- Mesh / Niagara / Sound / Material
- 少量轻演出
- 可选 Blueprint 事件补表现

蓝图不负责：

- 生命周期真相
- depleted 判定
- respawn 时机
- 奖励逻辑

---

## 10. 文档上必须写清楚的系统结论

这轮除了代码设计，还必须把下面这些话写进知识库：

1. 采集系统已经有通用的节点生命周期框架。
2. 节点采空后的对象表现不属于主链，不属于 Resolver，不属于库存，而属于对象层。
3. `Tree / Rock / Bush` 只是第一批节点族，不是系统边界本身。
4. 后续新增节点族必须优先沿用 `Core + Nodes/<Family>` 模式。
5. respawn 是系统统一语义，不是各节点族自定义机制。

---

## 11. 手工验收口径

### 11.1 系统级验收

所有节点族都必须满足：

- depleted 只在一次合法采集完整成立后触发
- 背包满导致整次采集失败时，不进入 depleted 表现
- respawn 前不可继续采集
- respawn 后恢复可采
- 多人同时采集时，后到失败请求不能错误触发对象表现

### 11.2 第一批对象验收

Tree：

- 采空后按配置倒塌/隐藏/销毁
- respawn 后恢复

Rock：

- 采空后破坏/碎裂并隐藏或销毁
- respawn 后恢复完整形态

Bush：

- 采空后轻量隐藏
- respawn 后恢复

### 11.3 架构验收

程序员在阅读代码时，应能很快回答：

- 系统生命周期桥在哪里？
- 节点运行时真相在哪里？
- 某个节点族自己的 depleted 表现在哪里？
- 蓝图可以改什么，不能改什么？

如果回答不出来，说明系统抽象还没立住。

---

## 12. 实施顺序建议

### Task 1: 建立系统级对象生命周期桥

**Files:**
- Modify: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`
- Modify: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.h`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableTarget.h`

- [ ] 先把 `AAOHarvestableActor` 的职责从“普通公共节点基类”收口为“系统级生命周期桥”。
- [ ] 固化统一顺序：公共默认处理 -> native hook -> Blueprint 轻补充。
- [ ] 设计稳定的 C++ 扩展口，避免每个节点族重复拷贝接口实现。
- [ ] 设计 Blueprint 可选扩展口，但明确限制它只承担资源和演出补充。
- [ ] 复核这一步没有破坏 `HarvestableComponent` 作为运行时真相持有者的角色。

### Task 2: 对齐 Tree 到系统桥接模型

**Files:**
- Modify: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h`
- Modify: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp`

- [ ] 把 Tree 的专属 depleted / respawn 表现迁移到新的 native hook 模式。
- [ ] 保持现有 `HideTree / KeepFallenTree / DestroyActor` 语义不变。
- [ ] 确认 Tree 仍然先退出采集命中链，再做专属表现。
- [ ] 确认 Tree respawn 时恢复顺序清晰，不把对象恢复逻辑散落到别处。
- [ ] 把 Tree 定义成“第一批复杂节点族模板”，而不是系统默认模板。

### Task 3: 新增第一批 Rock 节点族

**Files:**
- Create: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h`
- Create: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`

- [ ] 为 Rock 建立正式节点族子类，而不是继续从 Tree 开分支。
- [ ] 设计 Rock depleted 后的破坏/隐藏/销毁表现边界。
- [ ] 设计 Rock respawn 后的静态恢复路径。
- [ ] 明确 Rock 不自己管理 respawn 机制。
- [ ] 记录 Rock 作为“第一批破坏型节点族”的系统定位。

### Task 4: 新增第一批 Bush 节点族

**Files:**
- Create: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h`
- Create: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp`

- [ ] 为 Bush 建立正式节点族子类，而不是做 Tree 的弱化分支。
- [ ] 设计 Bush depleted 后的轻量隐藏语义。
- [ ] 设计 Bush respawn 后的直接恢复路径。
- [ ] 明确 Bush 也完全受统一 respawn 语义约束。
- [ ] 记录 Bush 作为“第一批轻量节点族模板”的系统定位。

### Task 5: 补系统知识库

**Files:**
- Modify: `Docs/Knowledge/HarvestSystem/PROJECT_MAP.md`
- Modify: `Docs/Knowledge/HarvestSystem/DECISIONS.md`
- Optional Modify: `Docs/Knowledge/HarvestSystem/KNOWN_ISSUES.md`

- [ ] 补上系统级生命周期桥说明。
- [ ] 写清楚 `Tree / Rock / Bush` 是第一批节点族，不是系统边界本身。
- [ ] 写清楚未来节点族应沿同一模型扩展。
- [ ] 写清楚 Blueprint 的权限边界。
- [ ] 写清楚统一 respawn 语义的系统约束。

### Task 6: 做系统级验收而不是只测三个对象

**Files:**
- Read: `Source/AegisOdyssey/Harvest/Abilities/GA_Harvest.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`

- [ ] 验证生命周期桥只在合法结算成功后触发。
- [ ] 验证失败采集不会错误驱动对象表现。
- [ ] 验证统一 respawn 语义对第一批节点族全部成立。
- [ ] 验证程序员能通过 `Core -> Nodes/<Family>` 快速读懂系统结构。
- [ ] 验证这套框架已经足以承接下一批节点族，而不是只够服务当前三种对象。

---

## 13. 最终结论

这轮方案必须被理解成“先做采集系统通用生命周期框架，再让第一批对象挂上去”。

不是：

- 先做三种对象，之后再看要不要抽系统

而是：

- 先把系统级生命周期桥立住
- 再用 Tree / Rock / Bush 验证这套桥接方式

只有这样，后面继续做 `Ore`、`Fiber`、`Herb`、`Crystal` 之类对象时，采集系统才真的还是一个系统，而不是一堆对象专案的集合。
