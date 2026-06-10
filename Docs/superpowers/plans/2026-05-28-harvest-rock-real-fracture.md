# Harvest Rock Real Fracture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把当前岩石节点“受击后弹一下再隐藏”的伪破碎表现，升级为真正的岩石破碎表现，同时保持现有采集主链、生命周期与可重生配置不变。

**Architecture:** 保持 `StateTree -> GA_Harvest -> Resolver -> HarvestableComponent -> HarvestableActor -> HarvestableRock` 这条正式链路不动。岩石节点改为“完整态表现”和“破碎态表现”双表示结构：采空前显示完整岩石，采空后切到 `Geometry Collection` 破碎表示，并使用已同步的耗尽事件上下文驱动命中点局部范围破碎；重生时回到完整态并重建破碎组件。

**Tech Stack:** Unreal Engine 5.6 C++, Chaos Geometry Collection, Harvest lifecycle replication, Blueprint 仅做资源挂接与少量参数调优

---

## 1. 先把问题说清楚

当前 `AAOHarvestableRock` 的实现，只是：

- 采空时关闭采集命中
- 打开模拟物理
- 给主体 `PrimitiveComponent` 一个冲量
- 按配置隐藏 / 保留 / 销毁

这不是真正的“破碎”，只是“被打飞/震开”。

你现在要的“岩石破碎”，语义上应该是：

- 采空后，完整岩石不再继续存在为原来的完整体
- 屏幕上出现明确的碎裂结果
- 破碎起点应与采集命中点有关
- 破碎应围绕命中点展开，而不是整体像树那样往某个方向倒
- 同一套采集生命周期、重生配置、蓝图资源挂接方式继续沿用

结论：

- 现有 `Rock` 方案不符合目标
- 岩石节点族必须从“单主体受力”切换到“完整态 -> 破碎态”双表示方案

---

## 2. 方案对比与结论

### 方案 A：继续用单个 StaticMesh / Primitive 做假破碎

做法：

- 保持当前主体组件
- 加更强的 impulse、Niagara、音效
- 用视觉特效伪装成破碎

优点：

- 改动最小

缺点：

- 本质上还是假的
- 无法满足“真正破碎”的目标
- 后续无论怎么调，本体依然是一整块石头

结论：

- 不采用

### 方案 B：采空时生成一个单独的“破碎岩石 Actor”

做法：

- 完整岩石 Actor 负责采集
- 采空时隐藏完整体
- 额外生成一个只负责 Chaos 破碎的 Actor
- 重生时销毁破碎 Actor，恢复完整体

优点：

- 完整态与破碎态隔离清晰
- 对重生重置最稳
- 如果后面要做更严格的网络同步，也更容易扩展成单独复制对象

缺点：

- Actor 数量更多
- 生命周期管理更重
- 对当前“节点族逻辑尽量都收在子类里”来说略偏重

结论：

- 可作为兜底方案
- 不是第一推荐

### 方案 C：同一个 Rock Actor 内部维护“完整态 + 破碎态”双表示

做法：

- `AAOHarvestableRock` 内保留完整态可见组件
- 同时持有一个默认隐藏的 `GeometryCollectionComponent`
- 采空后隐藏完整态、激活破碎态、触发碎裂
- 重生时销毁并重建破碎组件，再恢复完整态

优点：

- 最符合你现在的 C++ 节点族思路
- 蓝图只要挂完整岩石资源和破碎资源
- 不需要把破碎表现拆成另一套蓝图逻辑
- 文件分类和职责边界清晰

缺点：

- 需要处理好破碎组件的重置
- 需要补 Chaos 相关模块依赖

结论：

- 采用方案 C

---

## 3. 修正后的岩石节点族目标

修正后，`AAOHarvestableRock` 不再被定义为“受击崩一下的节点族”，而应被定义为：

> 一个具有完整态资源、破碎态资源、采空后真实碎裂表现、并可按统一采集生命周期重生的岩石节点族。

它必须满足下面这些边界：

### 系统边界

- 不改采集目标来源
- 不改 `Resolver` 主职责
- 不改奖励结算
- 不改统一 respawn 配置来源
- 不把运行时真相挪出 `UAOHarvestableComponent`

### 对象层职责

`AAOHarvestableRock` 只负责：

- 完整态与破碎态的切换
- 破碎资源的激活
- 破碎起点与破碎范围参数的解析
- 破碎后隐藏 / 保留 / 销毁的对象层表现
- respawn 时的对象层恢复

它不负责：

- 采空判定
- 扣多少进度
- 掉什么奖励
- 什么时候启动 respawn timer

---

## 4. 岩石真实破碎的推荐技术路线

### 4.1 表现结构

每个岩石节点由两套表现组成：

- `IntactRockComponent`
  - 采空前使用
  - 一般是 `UStaticMeshComponent` 或当前作为主体的完整岩石可见组件
- `FracturedRockComponent`
  - 采空后使用
  - 类型为 `UGeometryCollectionComponent`
  - 默认隐藏、默认无碰撞、默认不参与模拟

蓝图子类只负责：

- 挂完整岩石网格
- 挂破碎岩石 `Geometry Collection` 资源
- 调少量参数

蓝图不负责：

- 手工切显示
- 手工开物理
- 手工决定破碎时机

### 4.2 depleted 时的对象层流程

当 `OnHarvestNodeDepletedNative(...)` 触发时，岩石应执行：

1. 退出采集命中链
2. 隐藏完整态组件并关闭其碰撞
3. 显示 `GeometryCollectionComponent`
4. 开启 Chaos 模拟
5. 以命中点作为主要破碎起点
6. 以可调半径定义局部破碎范围
7. 对 `Geometry Collection` 施加破碎应变/破坏场
8. 可选叠加很轻的局部冲量扰动，但它服务的是碎裂自然感，不是整体推出语义
9. 按 disposition 决定：
   - `HideRock`：延迟隐藏破碎结果
   - `KeepBrokenRock`：保留破碎残骸
   - `DestroyActor`：破碎表现启动后短延时销毁，避免刚触发就没了

### 4.3 respawn 时的对象层流程

重生不是“把已经碎掉的 GC 组件硬复原”，而是走更稳的重建流程：

1. 停用并隐藏当前破碎组件
2. 销毁旧的运行时破碎组件实例
3. 用同一个 `Geometry Collection` 资源重建一个全新组件
4. 新组件恢复为默认隐藏、默认不模拟、默认不参与碰撞
5. 恢复完整态组件可见与碰撞
6. 恢复采集命中链

这样做的原因很直接：

- Chaos 破碎后的运行时状态不适合依赖“手动回卷”
- 对采集节点这种会频繁重生的对象，重建组件比试图重置碎裂状态更稳

---

## 5. 破碎起点与破碎范围怎么定义

这个部分必须完全围绕岩石破碎本身来定义，不要拿树去类比。

### 5.1 树关心的是“倒向哪边”

树是整体倒下，重点是方向一致。

### 5.2 岩石关心的是“从哪里碎、碎多大范围”

岩石不应该套用树那套“朝哪个方向倒”的设计。
它更需要下面这些信息：

- `Break Origin`
  - 优先取 `LifecycleContext.HitLocation`
  - 用来决定破碎从哪里爆开
- `Break Radius`
  - 可调参数
  - 用来定义命中点周围多大范围内发生局部破碎
- `Break Strain`
  - 可调参数
  - 用来定义这次局部破碎有多强
- `Secondary Noise Impulse`
  - 可调参数
  - 只用于让碎裂看起来没那么死板，不承担“朝前打碎”的语义

所以岩石建议采用：

- 破碎起点看命中点
- 破碎范围看半径参数
- 破碎强度看局部破坏参数

而不是直接复用树那套方向逻辑。

这是“借用同步链路，但岩石的表现模型独立定义”。

---

## 6. 网络与同步怎么处理

这里不建议把“每一块碎片的每一帧物理状态”都当成严格同步目标。

当前阶段的正确目标应当是：

- 破碎触发时机同步
- 破碎起点同步
- 破碎范围与强度参数一致
- 节点 depleted / respawn 真相同步

依赖现有链路即可：

- 服务端完成真实采集结算
- `UAOHarvestableComponent` 复制 depleted 事件上下文
- 客户端收到 `OnRep_LastDepletedEvent()` 后进入同样的 Rock 破碎逻辑

这意味着：

- 同一时刻碎
- 从同一命中点起碎
- 按同一套半径与强度参数碎

至于 Chaos 碎块的细节运动，如果客户端和服务端不是逐碎块严格复制，局部轨迹可能不完全逐帧一致，但对采集资源节点而言这是合理取舍。

如果以后明确要求“碎块轨迹也必须严格一致”，再升级为：

- 单独的破碎 Actor
- 或者更强的服务端主导碎块复制方案

这不应混入当前第一阶段方案。

---

## 7. 文件分类必须怎么落

这次不能把岩石真实破碎逻辑继续堆在现有 `AOHarvestableRock.h/.cpp` 两个文件里无限变厚。

推荐结构：

```text
Source/AegisOdyssey/Harvest/
  Core/
    AOHarvestableActor.h
    AOHarvestableActor.cpp
    AOHarvestableComponent.h
    AOHarvestableComponent.cpp
    AOHarvestTypes.h
  Nodes/
    Rock/
      AOHarvestRockTypes.h
      AOHarvestableRock.h
      AOHarvestableRock.cpp
```

### 各文件职责

`AOHarvestRockTypes.h`

- 只放岩石节点族专属枚举和配置结构
- 例如：
  - 破碎 disposition
  - 破碎调参配置
  - 破碎延时配置
  - 破碎半径/强度配置

`AOHarvestableRock.h`

- 只放类声明、组件引用、对外可调参数、函数声明

`AOHarvestableRock.cpp`

- 只放岩石完整态/破碎态切换
- 破碎触发
- 破碎组件重建
- respawn 恢复

不要做的事：

- 不把 Rock 的真实破碎逻辑混进 `Tree`
- 不把 Rock 的专属配置混进 `AOHarvestTypes.h`
- 不为了图快把 Geometry Collection 细节塞回公共基类

---

## 8. 需要补的工程依赖

当前工程已有 `Niagara`，但还没有为岩石真实破碎明确声明 Chaos Geometry Collection 运行时依赖。

这轮要检查并补齐：

- `GeometryCollectionEngine`
- `FieldSystemEngine`
- 如有需要再补：
  - `Chaos`
  - `ChaosSolverEngine`

原则：

- 只补运行时真实需要的模块
- 不引编辑器侧 Fracture 工具模块进 Runtime

---

## 9. 分阶段实施建议

### 第一阶段：把“真破碎骨架”立起来

目标：

- 岩石采空后，不再是完整网格受力
- 而是切换到 `Geometry Collection` 并真实破碎

完成标准：

- 采空后完整体消失
- 破碎体出现并碎裂
- respawn 能恢复完整态

### 第二阶段：把破碎方向和手感调顺

目标：

- 破碎起点稳定落在命中点附近
- 局部破碎范围和破坏强度符合手感预期
- 不会乱飞到很夸张

主要可调项：

- 破碎半径
- 破碎应变强度
- 局部扰动力度
- 局部扰动衰减
- 隐藏延时
- 残骸保留时长

### 第三阶段：补资源与表现层

目标：

- Blueprint 子类只挂：
  - 完整岩石资源
  - 对应破碎 `Geometry Collection`
  - 可选 Niagara / 音效
- 不再需要蓝图手工写生命周期

---

## 10. 这轮方案对原有大方案的修正

原先“大节点族方案”里，Rock 一节写的是：

- 受击
- impulse
- 隐藏 / 保留 / 销毁

这部分现在应视为已被本方案取代。

更新后的理解应当是：

- Tree：整体倒下型
- Rock：真实破碎型
- Bush：轻量受击隐藏型

三者共用统一采集主链和重生配置，但对象层表现模型不同，不应强行写成同一种物理套路。

---

## 11. 最终建议

如果现在开工，顺序不应是“继续在旧 Rock 上加大 impulse”，而应是：

1. 先把 Rock 定义成双表示节点
2. 再把 `Geometry Collection` 接成 depleted 后的正式表现
3. 再接命中点局部范围破碎
4. 最后做 respawn 的破碎组件重建

这条路是和你当前系统目标一致的：

- C++ 公共基类 + 子类覆写
- 蓝图只配资源和少量表现
- 同一套可重生配置
- 文件分类规整
- 不脱离当前采集系统主链
