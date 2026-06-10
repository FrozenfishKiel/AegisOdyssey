# Harvest Cpp-First Node Families Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在现有采集系统主链不变的前提下，把采集节点对象层进一步做成 `C++ 优先` 的节点族体系，优先落地树木、岩石、灌木三类节点，并尽量减少逐个蓝图手工拼表现的成本。

**Architecture:** 继续保持 `StateTree -> GA_Harvest -> Resolver -> HarvestableComponent -> Inventory` 这条正式主链不动，把“采空后节点怎么表现、重生后怎么恢复”继续收口在对象层。`UAOHarvestableComponent` 负责运行时真相，`AAOHarvestableActor` 负责公共对象层桥接，`Tree / Rock / Bush` 子类在 C++ 中直接实现默认表现，蓝图只负责挂资源和做极少量调整。

**Tech Stack:** Unreal Engine 5 C++, Gameplay Ability System, StateTree, Blueprint 子类资源配置

---

## 1. 现有基础，不要重复建设

这轮计划默认承认并复用下面这些现有能力，不再重复发明：

- 正式采集主链已经完成：
  - `STT_PlayHarvest` 发起采集动作
  - `GA_Harvest` 在命中窗内做工具 Socket Trace
  - `AOHarvestResolver` 做统一重判定与统一结算
  - `UAOHarvestableComponent` 持有节点运行时状态
  - `UAOInventoryStatics` 负责正式入包
- depleted / respawn 生命周期已经存在：
  - `HandleHarvestNodeDepleted(...)`
  - `HandleHarvestNodeRespawned()`
- `AAOHarvestableActor` 已经是公共对象基类
- `AAOHarvestableTree` 已经是第一种正式对象子类

这轮不做的事：

- 不改采集目标来源
- 不改 Resolver 主职责
- 不改奖励入包链
- 不把节点真相挪出 `HarvestableComponent`

这轮真正要做的是：

- 把对象层做得更像系统
- 把对象表现尽量前推到 C++
- 把三类节点变成正式 C++ 节点族，而不是蓝图手工散拼

---

## 2. 这轮工作的真实目标

你当前最想做的，其实不是“设计一个理论框架”，而是：

1. 让树木、岩石、灌木都变成正式 C++ 子类
2. 让它们的默认 depleted 表现直接在 C++ 就能跑
3. 尽量不要再为每个节点蓝图单独手工搭完整逻辑

所以这轮方案要同时满足两个条件：

- 系统上是对的
- 工程上是省事的

这里的“省事”不是偷边界，而是：

- 统一节点族抽象
- 统一公共桥接
- 每个节点族给出一套默认 C++ 表现
- 蓝图只剩资源挂接和少量参数调整

---

## 3. 推荐方向

### 方案 A：蓝图主导，每种对象只给薄 C++ 壳

优点：

- 灵活

缺点：

- 你已经明确不想继续一个个蓝图手工做
- 很容易导致表现逻辑重新散到蓝图里
- 后面继续扩节点族时成本会继续偏高

结论：

- 不采用

### 方案 B：推荐方案，C++ 公共基类 + C++ 节点族默认表现 + 蓝图只挂资源

做法：

- `AAOHarvestableActor` 做好公共桥接
- `AAOHarvestableTree / Rock / Bush` 各自在 C++ 内直接给出默认 depleted / respawn 表现
- 蓝图只负责：
  - 挂 `StaticMesh / SkeletalMesh`
  - 挂 `Niagara / Sound`
  - 调少量物理参数或显示参数

优点：

- 符合你当前最想做的工作方式
- 以后继续扩对象时非常顺手
- 程序员阅读成本低

缺点：

- 需要先把对象层基类扩展点收紧

结论：

- 采用

---

## 4. 当前代码结构对这轮工作的启示

### 4.1 现有公共基类还偏薄

当前 `AAOHarvestableActor` 已经有：

- `ApplyDefaultHarvestDepletedState()`
- `ApplyDefaultHarvestRespawnedState()`
- Primitive 碰撞快照保存与恢复

但还没有真正把“公共桥接 -> 节点族默认表现 -> 可选蓝图补充”这条顺序固定死。

这意味着如果现在直接继续加 `Rock / Bush`，很容易出现：

- Tree 一套写法
- Rock 一套写法
- Bush 一套写法

结果又开始散。

### 4.2 Tree 已经是现成模板，但不能当万能父类

当前 Tree 已经做了这些事情：

- depleted 时阻断采集命中链
- 按 `HideTree / KeepFallenTree / DestroyActor` 处理
- 可选启用物理并施加倒地方向 impulse
- respawn 时恢复可见、碰撞、物理关闭

这说明 Tree 已经足够作为“复杂节点族模板”。

但不能把 Rock / Bush 继续堆进 Tree 分支里，否则 Tree 会变成伪通用类。

---

## 5. 建议落地的对象层结构

### 5.1 公共层

保留并增强：

- `AAOHarvestableActor`

建议让它成为真正的对象层总桥接器，统一负责：

- 吃到 depleted / respawn 生命周期
- 执行公共默认状态切换
- 调用节点族 C++ 默认表现
- 如有必要，再给蓝图一个轻量补充入口

### 5.2 第一批节点族

新增或完善：

- `AAOHarvestableTree`
- `AAOHarvestableRock`
- `AAOHarvestableBush`

它们的系统定位分别是：

- Tree：复杂物理型节点族模板
- Rock：破坏碎裂型节点族模板
- Bush：轻量受力/隐藏型节点族模板

### 5.3 蓝图角色

蓝图应该退到这些工作：

- 挂模型
- 挂材质
- 挂 `Niagara / Sound`
- 调少量默认参数

蓝图不再承担：

- depleted 主逻辑
- respawn 主逻辑
- 对象状态切换

---

## 6. 三类节点的 C++ 默认表现建议

### 6.1 Tree

目标：

- 继续保留当前 Tree 的成熟逻辑

默认表现：

- 采空时先移出采集命中链
- 依据配置：
  - 隐藏
  - 保留倒地树
  - 直接销毁
- 如启用物理则施加倒地方向 impulse
- 重生时恢复可见性、碰撞、物理状态

Tree 是“复杂动作型对象”的默认模板。

### 6.2 Rock

目标：

- C++ 里直接给一套“破碎后消失/隐藏”的默认行为

默认表现建议：

- 采空时先阻断采集命中链
- 关闭继续采集的关键碰撞
- 打开模拟物理，给一个受击方向 impulse，让它有明显“崩开/震开”的感觉
- 同时可选：
  - 立即隐藏
  - 延迟隐藏
  - 直接销毁

重生时：

- 关闭模拟物理
- 恢复可见
- 恢复碰撞
- 回到完整静态状态

这里的“破碎”在第一版不一定非要做真实 `Geometry Collection`，完全可以先是“受力 + 隐藏/销毁”的 C++ 默认行为，后面再让蓝图资源升级表现。

### 6.3 Bush

目标：

- 比 Tree 更轻，比纯隐藏更有一点“被拨倒/弹开”的感觉

默认表现建议：

- 采空时先阻断采集命中链
- 打开模拟物理
- 给一个较轻的 impulse，让它产生明显受击倾倒感
- 很短延迟后隐藏，或者直接隐藏但保留一个轻微受击过程

重生时：

- 关闭模拟物理
- 恢复可见
- 恢复碰撞

Bush 的重点不是复杂物理，而是“廉价但统一的受击感”。

### 6.4 三类节点族的具体对象逻辑边界

#### Tree 节点族逻辑

Tree 节点族负责的是“有明确主干、有倒地方向、有倒地后残留状态”的采集对象。

它应当内建的对象逻辑包括：

- 采空后先退出 `ECC_Visibility` 的采集命中链，防止继续被工具 Trace 打到。
- 依据生命周期上下文算倒地方向：
  - 优先用 `HarvesterActor` 相对方向
  - 其次用 `HitDirection`
  - 最后退回默认方向
- 根据配置选择最终 disposition：
  - `HideTree`
  - `KeepFallenTree`
  - `DestroyActor`
- 如果保留倒地，则启用物理并施加 impulse。
- respawn 时恢复：
  - 采集命中链
  - 可见性
  - 碰撞
  - 物理关闭

Tree 节点族不负责：

- 掉什么物品
- 扣多少进度
- 何时进入 depleted
- 何时开始 respawn timer

这些都仍然属于采集主链和 `HarvestableComponent`。

#### Rock 节点族逻辑

Rock 节点族负责的是“没有树干倒地方向语义，但有明显破坏/崩裂感”的采集对象。

它应当内建的对象逻辑包括：

- 采空后先退出采集命中链。
- 关闭继续采集的关键碰撞。
- 启用模拟物理或局部受力，让对象有“崩开/震开”的感觉。
- 根据配置决定最终结果：
  - 立即隐藏
  - 延迟隐藏
  - 直接销毁
- respawn 时恢复：
  - 静态姿态
  - 可见性
  - 碰撞
  - 关闭模拟物理

Rock 节点族的第一版重点是统一破坏型默认行为，不是追求最复杂的碎裂资源方案。

后续如果真要升级：

- `Geometry Collection`
- 分裂碎块
- 更复杂的 `Niagara / Sound`

也应该挂在 Rock 节点族扩展上，而不是把系统重新改一遍。

#### Bush 节点族逻辑

Bush 节点族负责的是“轻量、有受击感、但不需要复杂倒塌或碎裂”的采集对象。

它应当内建的对象逻辑包括：

- 采空后退出采集命中链。
- 启用轻量模拟物理或直接施加轻 impulse。
- 给玩家一个“被拨倒/被弹开”的即时反馈。
- 在很短时间内隐藏，或者直接隐藏但保留瞬时受击表现。
- respawn 时恢复：
  - 可见性
  - 碰撞
  - 关闭模拟物理

Bush 节点族的重点是：

- 足够轻
- 足够统一
- 不让同类灌木对象每次都重写一遍逻辑

#### 三类节点族共同遵守的系统约束

无论是 Tree、Rock 还是 Bush，都必须遵守：

- depleted 表现只在一次合法采集完整成立后触发。
- 背包满导致整次采集失败时，不进入对象耗尽表现。
- respawn 计时不由对象子类自己管理。
- 运行时真相不保存在对象子类，而保存在 `UAOHarvestableComponent`。
- 子类只处理“对象怎么表现”，不处理“系统怎么结算”。

---

## 7. 为什么这条路对你现在最合适

你现在最怕的是：

- 采集系统主链没问题
- 但每加一个对象都要手工配蓝图逻辑
- 最后时间都花在重复劳动上

这条 `C++ 节点族默认表现` 路线的价值就在这里：

- 程序里直接提供一套对象族默认行为
- 同类对象只需要继承对应节点族蓝图，挂不同资源
- 不用每个对象重新接 `depleted / respawn` 行为

也就是说，后面新增 10 棵树、8 块石头、12 丛灌木时，你做的是：

- 选对继承基类
- 换资源
- 调少量参数

而不是：

- 重新接生命周期
- 重新写隐藏逻辑
- 重新写受力逻辑

---

## 8. 文件分类必须规整的硬规则

这一条不是建议，是这轮必须遵守的结构纪律。

### 8.1 总原则

按“系统职责”分层，不按“临时方便”堆文件。

也就是说：

- 公共系统层放公共系统层
- 节点族逻辑放节点族目录
- 具体对象资源放蓝图资产

不要出现：

- 所有节点族都塞进一个 `HarvestNodes.cpp`
- Tree / Rock / Bush 共用一个大而杂的 `AOHarvestableObject`
- 为了图快把 Rock / Bush 写进 Tree 文件里开分支

### 8.2 推荐目录结构

```text
Source/AegisOdyssey/Harvest/
  Abilities/
  Core/
  Cue/
  Definition/
  Fragments/
  Items/
  Nodes/
    Tree/
      AOHarvestableTree.h
      AOHarvestableTree.cpp
    Rock/
      AOHarvestableRock.h
      AOHarvestableRock.cpp
    Bush/
      AOHarvestableBush.h
      AOHarvestableBush.cpp
  StateTree/
  System/
```

### 8.3 每层职责

`Core/`

- 只放系统共用的节点对象层基础设施
- 例如：
  - `AOHarvestableActor`
  - `AOHarvestableComponent`
  - `AOHarvestableTarget`
  - `AOHarvestTypes`

这里不能写某个具体节点族的专属逻辑。

`Nodes/Tree/`

- 只放树节点族逻辑
- 不能混入 Rock / Bush 行为

`Nodes/Rock/`

- 只放岩石节点族逻辑
- 以后矿脉如果逻辑接近，也要先判断是不是属于 Rock 子族扩展，而不是随手塞进 Tree

`Nodes/Bush/`

- 只放灌木节点族逻辑
- 不要混树的倒地逻辑，不要混岩石的破坏逻辑

### 8.4 后续扩展规则

后面如果新增对象，先问自己一句：

- 它是现有节点族的资源变体？
- 还是一个新的节点族？

如果是资源变体：

- 不新增 C++ 类
- 直接继承对应蓝图资源模板

如果是新的节点族：

- 在 `Nodes/<Family>/` 下新开独立目录
- 新建独立 C++ 类

不要因为“只有一个类”就把它临时塞回 `Core/`。

### 8.5 蓝图资源分类建议

如果后面也要整理资产层，建议跟着 C++ 节点族走：

```text
Content/.../Harvest/Nodes/Tree/
Content/.../Harvest/Nodes/Rock/
Content/.../Harvest/Nodes/Bush/
```

这样代码和资源是一一对齐的，接手的人不会迷路。

---

## 9. 阶段推进方式

这轮实现不建议一次性同时改 `Core + Tree + Rock + Bush`。

按下面三个阶段推进：

### 第一阶段：`Core + Tree`

目标：

- 先把公共对象层桥接收口
- 先让 Tree 对齐新的 `C++ 优先节点族` 模式

这一阶段重点验证：

- 公共桥接顺序是否合理
- Tree 的 depleted / respawn 表现是否自然
- 只靠少量蓝图配置时，Tree 是否已经能高效复用
- 程序员 review 时是否能沿着 `AAOHarvestableActor -> AAOHarvestableTree -> HarvestableComponent` 快速确认入口
- 手工验证时是否只需确认 Tree 的默认 depleted / respawn，不需要额外补蓝图逻辑

只有第一阶段稳定后，才进入第二阶段。

### 第二阶段：`Rock`

目标：

- 在第一阶段稳定后，新增 Rock 节点族

这一阶段重点验证：

- 这套对象层抽象是否能承接“破坏型节点”
- Rock 是否能在 C++ 中直接跑出统一默认表现
- 是否真的减少了岩石类对象逐个蓝图手工搭逻辑的成本
- Review 时是否能快速定位 `AAOHarvestableRock.cpp` 中的 depleted / respawn 默认实现
- 手工验证时是否能在最少蓝图配置下直接确认“破坏后隐藏/销毁、重生后恢复”的闭环

Rock 和 Tree 差异最大，所以它是验证“这套系统是不是通用”的关键阶段。

### 第三阶段：`Bush`

目标：

- 在 `Core + Tree + Rock` 已经稳定后，新增 Bush 节点族

这一阶段重点验证：

- 这套对象层抽象是否也适用于轻量节点
- Bush 是否能用最轻的逻辑挂上同一套系统
- 轻量对象是否也能维持“少蓝图劳动”的目标
- Review 时是否能快速定位 `AAOHarvestableBush.cpp` 中的默认行为
- 手工验证时是否能用最少资源配置确认“受击后隐藏、重生后恢复”的闭环

Bush 是收尾阶段，不应该反过来影响前两个阶段的系统边界。

---

## 10. 文件结构规划

### 10.1 需要修改的现有文件

- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h:31-61`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp:22-99`
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h:24-64`
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp:13-140`

### 10.2 需要新增的文件

- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h`
- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`

### 10.3 建议更新的知识库文件

- `Docs/Knowledge/HarvestSystem/PROJECT_MAP.md`
- `Docs/Knowledge/HarvestSystem/DECISIONS.md`
- `Docs/Knowledge/HarvestSystem/KNOWN_ISSUES.md`

---

## 11. 实施任务

### Task 1: 把公共 Harvest Actor 收口成真正的 C++ 对象层桥接器

**Files:**
- Modify: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h:31-61`
- Modify: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp:22-99`

- [ ] 明确 `AAOHarvestableActor` 的职责不是“薄默认类”，而是“对象层公共桥接器”。
- [ ] 增加稳定的 protected native 扩展口，供节点族覆写默认表现。
- [ ] 固化调用顺序：先公共默认处理，再节点族默认表现，最后再考虑蓝图轻补充。
- [ ] 保留现有碰撞快照保存/恢复逻辑，不要破坏已经存在的默认公共行为。
- [ ] 确认公共层不接管任何采集主链、奖励链、状态真相链职责。

### Task 2: 让 Tree 成为稳定的复杂节点族模板

**Files:**
- Modify: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h:24-64`
- Modify: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp:13-140`

- [ ] 把 Tree 当前逻辑整理到新的公共桥接模式上。
- [ ] 保持 `HideTree / KeepFallenTree / DestroyActor` 三种语义不变。
- [ ] 保持倒地方向、Impulse、可见性恢复、物理恢复逻辑不变。
- [ ] 明确 Tree 是“复杂物理型节点族模板”，不是通用父类替代品。
- [ ] 确认 Tree 蓝图后续只需要挂资源和调参数，不再重写主逻辑。

### Task 3: 新增 Rock C++ 节点族，直接给默认破坏表现

**Files:**
- Create: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h`
- Create: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`

- [ ] 新建 `AAOHarvestableRock`，不要挂在 Tree 名下做分支。
- [ ] 在 C++ 中实现 Rock depleted 默认表现：阻断命中链、受击感、破坏后隐藏/销毁。
- [ ] 在 C++ 中实现 Rock respawn 默认表现：关闭物理、恢复显示、恢复碰撞。
- [ ] 第一版先优先满足“统一、省蓝图劳动、能稳定跑”，不要一上来追求最复杂碎裂资产方案。
- [ ] 把 Rock 定义成“破坏型节点族模板”，供后续不同岩石/矿脉蓝图直接继承。

### Task 4: 新增 Bush C++ 节点族，直接给默认轻量受击表现

**Files:**
- Create: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h`
- Create: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`

- [ ] 新建 `AAOHarvestableBush`。
- [ ] 在 C++ 中实现 Bush depleted 默认表现：阻断命中链、启用轻量模拟物理、施加较轻 impulse、再隐藏。
- [ ] 在 C++ 中实现 Bush respawn 默认表现：关闭模拟物理、恢复显示与碰撞。
- [ ] 控制 Bush 逻辑足够轻，避免变成 Tree 的简化抄写版。
- [ ] 把 Bush 定义成“轻量受力型节点族模板”。

### Task 5: 补一版“C++ 优先节点族”知识库说明

**Files:**
- Modify: `Docs/Knowledge/HarvestSystem/PROJECT_MAP.md`
- Modify: `Docs/Knowledge/HarvestSystem/DECISIONS.md`
- Modify: `Docs/Knowledge/HarvestSystem/KNOWN_ISSUES.md`

- [ ] 写清楚对象层的推荐方向已经转为“C++ 默认行为优先”。
- [ ] 写清楚 Tree / Rock / Bush 的系统定位和默认表现边界。
- [ ] 写清楚蓝图只挂资源和调参数，不再承担主表现逻辑。
- [ ] 写清楚后续新增对象应直接继承对应节点族模板，而不是重复做一套蓝图逻辑。
- [ ] 记录这条路线的目的就是降低新增采集对象时的重复劳动。

### Task 6: 做面向“少蓝图劳动”的验收

**Files:**
- Read: `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- Read: `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`

**程序员 review 入口：**
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.h`
- `Source/AegisOdyssey/Harvest/Core/AOHarvestableActor.cpp`
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h`
- `Source/AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.cpp`
- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h`
- `Source/AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.cpp`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h`
- `Source/AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.cpp`

**手工测试步骤：**
- 分别放一个 Rock 和一个 Bush 到测试关卡中，蓝图侧只保留最少资源挂接和基础参数，不额外拼装节点逻辑。
- 进入游戏后，对 Rock 进行一次合法采集，确认 depleted 后的默认表现能直接跑通：命中链被阻断，节点进入破坏/隐藏/销毁之一的预期结果。
- 对 Bush 进行一次合法采集，确认 depleted 后的默认表现能直接跑通：命中链被阻断，轻量受击感出现，随后按配置进入隐藏或消失状态。
- 等待 respawn，到时间后确认 Rock 和 Bush 都恢复可见、恢复碰撞，并关闭模拟物理。
- 再次采集，确认重复流程仍然稳定，没有回到“每个对象都要单独补蓝图逻辑”的旧模式。

**验收步骤：**
- [ ] 验证三类节点在只有最少蓝图配置的情况下，默认 depleted / respawn 表现就能跑通。
- [ ] 重点复核 Rock / Bush 的默认行为是否无需额外蓝图分支即可成立。
- [ ] 验证新增同类对象时主要工作只剩资源挂接，而不是重写对象逻辑。
- [ ] 验证失败采集不会错误触发对象表现。
- [ ] 验证 respawn 仍统一由 `HarvestableComponent` 控制。
- [ ] 验证这条方案确实减少了逐个蓝图手工搭逻辑的成本。

---

## 12. 最终结论

这轮最值得做的工作，不是再谈一轮抽象，而是把对象层真正做成你能高效复用的 `C++ 节点族模板`。

系统层面上，它仍然是采集系统对象层的正式演进。

工程层面上，它解决的是你现在最实际的问题：

- 不想每个采集对象都手工做一套蓝图逻辑
- 想让同类对象直接继承 C++ 默认行为
- 想把主要工作收敛成“换资源、调少量参数”

按这条路往下做，后续你继续加树、岩石、灌木，甚至将来加矿脉、草药、晶体，工作方式都会明显更顺手。
