---
title: GASP Locomotion Motion Matching And Traversal
tags:
  - knowledge
  - reference-studies
  - locomotion
  - motion-matching
  - traversal
  - unreal-engine
aliases:
  - GASP 运动框架 Motion Matching 与 Traversal
  - GASP locomotion 参考结论
---

# GASP 运动框架 Motion Matching 与 Traversal

更新时间：2026-05-19  
适用范围：理解 `GASP笔记01-03` 中长期可复用的 locomotion / motion matching / traversal 结论，并把它们映射回 UE5.6 的真实机制入口。  
不适用范围：把本文直接当成 `AegisOdyssey` 当前角色移动系统实现说明；具体 AnimGraph 资产搭建步骤；单个参数调优手册。

## 1. 先把 GASP 放回正确位置

这三篇历史笔记的真正价值，不是“证明项目现在就是这么跑的”，而是提供一套外部 locomotion 样例的可拆机制图谱。

更稳妥的理解是：

1. GASP 是外部参考样例。
2. 里面混合了 UE 通用机制、样例项目具体组织方式、以及历史观察笔记。
3. 最终能沉淀进知识库的，应是其中稳定、可迁移、可复用的结构认识。

## 2. GASP 这套东西的主链到底是什么

把三篇笔记合起来看，主链可以稳定拆成四段：

1. 输入与角色运动状态整理
2. Trajectory / Pose History 查询输入生成
3. Motion Matching / Chooser / Database 动作选择
4. Traversal 检测、动作选择与 Motion Warping 执行

这条主链之外，还有一组支撑层：

1. 倾斜
2. 瞄准视角跟随
3. Offset Root Bone
4. Foot Placement
5. Leg IK
6. 地面对齐与脚部 planting

这里最重要的结构性认识是：

- 主链负责“选什么动作、什么时候切”。
- 支撑层负责“把选出来的动作修得更自然”。

## 3. Motion Matching 的稳定机制边界

从 UE5.6 插件源码看，Motion Matching 相关入口主要在：

1. `Engine/Plugins/Animation/PoseSearch/Source/Runtime/Public/PoseSearch/PoseSearchSchema.h`
2. `.../PoseSearch/AnimNode_MotionMatching.h`
3. `.../PoseSearch/AnimNode_MotionMatching.cpp`
4. `.../PoseSearch/PoseSearchHistoryCollectorAnimNodeLibrary.h`

当前可以稳定写下来的机制认识是：

1. `UPoseSearchSchema` 是查询特征格式与搜索成本规则的定义点。
2. `FAnimNode_MotionMatching` 是运行时执行 motion matching 选择的核心 AnimNode。
3. `Pose History / TransformTrajectory` 提供的是查询输入，不是最终动画结果。
4. 数据库、Schema、Trajectory、Pose History 需要成套理解，不能把 motion matching 简化成“一个节点自动选动作”。

所以 GASP 笔记里关于 motion matching 最值得保留的，不是某一张蓝图截图，而是这个问题意识：

- 查询输入怎么组织。
- 数据库怎么分组。
- 什么属于选择层，什么属于后处理层。

## 4. Traversal 的稳定机制边界

这三篇笔记对 traversal 的可保留结论，不应写成“某个函数就是翻越系统”，而应写成一条结果驱动链：

1. 输入角色当前移动模式、速度、朝向、胶囊体参数。
2. 通过 trace / ledge / clearance 检测环境。
3. 把结果整理成 traversal check result 之类的中间结构。
4. 再基于结果去选动作类型、动画区间或数据库。
5. 最后进入 motion warping / root motion 对齐执行。

这比“看懂一张 traversal 蓝图图”更稳定，因为它描述的是机制边界，而不是样例工程的图节点摆法。

## 5. Motion Warping、Offset Root Bone、Foot Placement 各自负责什么

这部分历史笔记最容易混层，我这里把它们收敛成三个稳定定位：

### 5.1 Motion Warping

从 `MotionWarpingComponent.h` 看，`UMotionWarpingComponent` 的核心职责是维护 warp target 与 root motion modifier。

因此更稳妥的理解是：

1. Motion Warping 负责让动画执行更好地对齐目标点、目标朝向、目标骨骼或目标组件。
2. 它适合作为 traversal、处决、近战吸附这类动作执行阶段的对齐工具。
3. 它不是 motion matching 的同义词，也不是 traversal 检测逻辑本身。

### 5.2 Offset Root Bone

从 `AnimNode_OffsetRootBone.h` 看，`FAnimNode_OffsetRootBone` 的核心是管理根骨偏移的累积、插值、锁定与释放。

因此更稳妥的理解是：

1. 它解决的是角色组件运动与动画根骨表现之间的偏移协调问题。
2. 它属于支撑修正层，不属于动作选择层。
3. 它的各种 mode 讨论的是偏移如何积累、保持、释放，而不是 traversal 或 motion matching 本体。

### 5.3 Foot Placement

从 `AnimNode_FootPlacement.h` 看，`FAnimNode_FootPlacement` 关心的是 planted / unplanted、地面法线、脚部锁定、骨骼补偿、插值恢复。

因此更稳妥的理解是：

1. 它解决的是脚部与地面接触的自然性问题。
2. 它是 grounding / IK 修正层的一部分。
3. 它不决定选哪段动作，只负责把当前动作在地面接触上修得更可信。

## 6. 当前项目的真实落点

这部分必须明确区分“有前置条件”和“已经正式接线”。

### 6.1 已确认存在的前置条件

当前工程已确认存在：

1. `AegisOdyssey.uproject` 已启用 `PoseSearch`、`MotionWarping`、`AnimationWarping`、`MotionTrajectory`、`Chooser`、`AnimationLocomotionLibrary`。
2. `AegisOdyssey.Build.cs` 已加入 `MotionWarping` 模块依赖。
3. `DefaultGameplayTags.ini` 已存在 `MotionMatching.*` 相关 tag。
4. `DefaultEngine.ini` 已存在 `OffsetRootBone` 相关 CVar。
5. `Content/Characters/UEFN_Mannequin/Animations/MotionMatchingData/*` 下已有 Schema、Database、Normalization Set、Channel 等资产。
6. `Content/Characters/UEFN_Mannequin/Animations/Traversal/*` 下已有大量 traversal 相关动画资产。
7. `Content/Blueprints/Data/*Traversal*` 下已有 traversal 输入/输出结果结构资产。

### 6.2 当前还不能直接写成正式事实的部分

当前 `Source` 层没有检出明确的：

1. `PoseSearch`
2. `MotionMatching`
3. `PoseSearchHistory`
4. `FootPlacement`
5. `OffsetRootBone`
6. `UMotionWarpingComponent`
7. `AddOrUpdateWarpTarget(...)`

这意味着当前更稳妥的项目结论是：

- 项目已经导入并启用了这套外部 locomotion / traversal 所需的大量插件、配置与资产。
- 但仅凭当前可见 C++ 证据，还不能直接宣告整套 GASP locomotion 已成为 `AegisOdyssey` 当前稳定运行时主链。

## 7. 这批研究对当前项目最有价值的迁移建议

如果后续真的要借用这套体系，当前更推荐按下面顺序迁移：

1. 先明确当前项目自己的角色运动问题到底是“动作选择问题”还是“动作表现问题”。
2. 如果是动作选择问题，优先研究 `Trajectory -> Pose History -> Schema / Database -> Motion Matching` 这一段。
3. 如果是 traversal 问题，优先研究“检测结果结构”和“动画选择边界”，不要先抄完整蓝图。
4. 如果是落地表现问题，再分别评估 `Motion Warping`、`Offset Root Bone`、`Foot Placement` 是否值得单独接入。

这里最重要的原则是：

- 以机制拆分迁移。
- 不以整套样例蓝图照搬迁移。

## 8. 调试与验证入口

这三篇笔记里提到的 `Rewind Debugger` 方向是有价值的，但它属于“调试已接入系统”的工具，不属于“证明当前项目已经接线”的证据。

更稳妥的用途是：

1. 未来若正式接入 motion matching，再用它看搜索结果与 cost 变化。
2. 现在不能把“笔记里展示过 Rewind Debugger”直接写成项目当前工作流事实。

## 9. 本轮来源与校对基线

本轮主要从下面三篇历史文档提炼，并与 UE5.6 插件源码、当前工程配置和资产路径交叉校对：

- `Notice/HistoryNotice/GASP笔记01.md`
- `Notice/HistoryNotice/GASP笔记02.md`
- `Notice/HistoryNotice/GASP笔记03.md`

相关联文档：

- [[ReferenceStudies 项目地图]]
- [[ReferenceStudies 已锁定认识]]
- [[ReferenceStudies 已知边界与历史偏差]]
