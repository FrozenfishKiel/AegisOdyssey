---
title: ReferenceStudies Decisions
tags:
  - knowledge
  - reference-studies
  - decisions
  - unreal-engine
  - external-study
aliases:
  - ReferenceStudies Decisions
  - ReferenceStudies 已锁定认识
---

# ReferenceStudies 已锁定认识

更新时间：2026-05-19  
适用范围：记录 `ReferenceStudies` 这类外部研究资料已经可以稳定保留的整理原则与阶段性结论。  
不适用范围：把这些结论直接当成 `AegisOdyssey` 的运行时事实；记录具体调参步骤和某一次蓝图接线过程。

## 1. `ReferenceStudies` 只保留可借鉴机制，不保留“样例现状即我方现状”

已经锁定：

1. `ReferenceStudies` 的职责是沉淀外部研究材料中的长期参考价值。
2. 它不负责宣布当前项目已经正式接入了哪些外部框架。
3. 任何主题笔记都必须把“UE 正式机制”“外部样例组织方式”“当前项目映射”三层拆开。

## 2. 解释 GASP 时，必须把主选择层和支撑修正层拆开

已经锁定：

1. Motion Matching、Trajectory、Pose History、Traversal 结果选择属于主选择层。
2. `MotionWarping`、`OffsetRootBone`、`FootPlacement`、Leg IK、Grounding 属于支撑修正层。
3. 以后写文档时，不能再把所有动画节点都混成一条“locomotion 主链”。

## 3. Motion Matching 的稳定理解应回到 `Schema / Database / Query / History`

已经锁定：

1. `UPoseSearchSchema` 定义查询特征格式与搜索规则。
2. Database 承载被索引的动作库。
3. `Pose History / Trajectory` 提供的是查询输入，不是最终动作结果。
4. `FAnimNode_MotionMatching` 做的是基于查询的动作选择，不是普通状态机切换。

## 4. Traversal 的稳定理解应回到“检测 -> 结果 -> 选择 -> 执行”

已经锁定：

1. Traversal 不是单个跳跃函数。
2. 它至少包含环境检测、中间结果结构、动作类型选择和执行对齐这几层。
3. `MotionWarping` 更适合作为 traversal 执行阶段的重要对齐工具，而不是 traversal 本体。

## 5. 插件启用、资产导入和 Tag 存在，不等于主链已经成立

已经锁定：

1. 插件、配置、Tag 和资产只能证明“前置条件已存在”。
2. 只有当源码或运行时链路也明确出现时，才能更高置信度地写成“当前项目已正式接入”。
3. 对 GASP / 高级动画这两组主题，当前项目还不能只凭前置条件就写成完整运行时事实。

## 6. 如果将来要吸收 GASP，应优先按机制分批迁移

已经锁定：

1. 优先迁移 `Trajectory` 输入约定。
2. 再迁移 `PoseSearch Schema / Database` 组织方式。
3. 再迁移 traversal 结果结构与动作选择边界。
4. 最后再评估 `MotionWarping`、`OffsetRootBone`、`FootPlacement` 等支撑层是否值得接入。

## 7. 解释高级动画系统时，必须把组织层、时间控制层和表现修正层拆开

已经锁定：

1. `UAnimLayerInterface + LinkAnimClassLayers(...)` 属于动画系统组织层。
2. `Sequence Evaluator + Distance Matching` 属于动画播放时间控制层。
3. `Stride Warping / Orientation Warping / Rotate Root Bone / Additive` 属于表现修正层。
4. 以后写文档时，不能把这三层混写成一个模糊的“高级动画主链”。

## 8. 动画层是 UE 正式支持的可替换边界，不是样例蓝图技巧

已经锁定：

1. `UAnimLayerInterface` 是引擎正式能力。
2. `LinkAnimClassLayers(...)` 的语义是替换对应 layer 的实现，而不是整体替换主 AnimBP 全部逻辑。
3. 共享状态更新、惯性化、全局状态机等逻辑仍更适合留在主 AnimBP 或共享基类中。

## 9. Distance Matching 的稳定定义是“距离到时间的映射机制”

已经锁定：

1. `DistanceMatchToTarget(...)` 解决的是目标距离对应动画时间点的问题。
2. `AdvanceTimeByDistanceMatching(...)` 解决的是实际位移对应的动画时间推进量问题。
3. 它依赖可用的距离曲线。
4. stop、start、land 只是常见使用场景，不是它的定义本体。

## 10. Warping 节点属于修动作，不属于选动作

已经锁定：

1. `Stride Warping` 修正步幅与实际速度不匹配的问题。
2. `Orientation Warping` 修正移动方向与原始动作朝向不匹配的问题。
3. `Rotate Root Bone` 更适合作为原地转身、朝向补偿或恢复阶段的姿态修正工具。
4. 这些机制都不负责决定“选哪段动作”。

## 11. Lyra 的 GAS 架构应理解为模块化角色接线模式，而不是零散 GAS 教程

已经锁定：

1. Lyra 的稳定核心不是某个单独能力蓝图，而是 `PlayerState ASC + Pawn Avatar + InitState 链 + 扩展事件 + AbilitySet` 这组组合。
2. 它解决的是多人、重生、换 Pawn、GameFeature 注入下的能力接线与重绑问题。
3. 以后整理 Lyra 相关资料时，应优先按“模块化角色与 GAS 接线”来写，而不是拆成过细的技能碎片。

## 12. `PlayerState` 持久 ASC 是 Lyra 风格选择，不是所有 GAS 项目的唯一标准

已经锁定：

1. 在 Lyra 里，把 ASC 放在 `PlayerState` 上有明确目标：跨死亡、跨 Pawn、跨重生保持持久玩家状态。
2. 这是一种稳定架构选择，但不是所有 GAS 项目的强制要求。
3. 以后写项目知识库时，必须把“Lyra 风格推荐”与“UE 必须如此”分开。

## 13. `AbilitySet` 的稳定定位是“可授予、可回收、可组合”的授权包

已经锁定：

1. `AbilitySet` 不只是能力列表，它可以同时封装 Ability、GameplayEffect、AttributeSet。
2. 稳定实现必须配套授予句柄或回收句柄。
3. 输入 Tag 挂在 `AbilitySpec` 的动态来源 Tag 上，是它与输入链连接的重要边界。

## 14. Lyra 风格输入链应理解为 `InputAction -> InputTag -> Handle 队列 -> ProcessAbilityInput`

已经锁定：

1. 输入不是直接把 `InputAction` 硬绑到某个 Ability 类。
2. 更稳定的理解是：输入先映射到 Tag，再由 ASC 根据 Tag 找到匹配 Spec，最后统一由输入泵处理。
3. 这样做的价值在于：输入映射、能力授予与能力激活被拆成了更松耦合的三层。

## 15. AttributeSet 的稳定关注点是复制回调、GE 生命周期与边界约束

已经锁定：

1. `GetLifetimeReplicatedProps`、`OnRep_*`、`GAMEPLAYATTRIBUTE_REPNOTIFY` 处理的是复制同步边界。
2. `PreGameplayEffectExecute / PostGameplayEffectExecute` 处理的是 GE 生效前后的拦截与结算逻辑。
3. `PreAttributeChange / PostAttributeChange` 处理的是属性值边界、联动修正和广播时机。
4. 以后解释 AttributeSet 时，优先写生命周期边界，而不是零散列函数名。

## 16. 当前项目已经显式采用了多处 Lyra 风格对应点，但仍应写成“对应实现”

已经锁定：

1. 当前项目明确存在 `UAOExtPawnComponent + UAOHeroComponent` 初始化链。
2. 当前项目明确存在 `GF_AddAbilities`、`AAOPlayerState::NAME_AOAbilityReady`、`AAOCharacter::NAME_AOAbilityReady` 扩展事件接线。
3. 当前项目明确存在 `UAOAbilitySet`、输入 Tag 驱动输入泵以及 AttributeSet 生命周期处理。
4. 这些结论足以说明项目受 Lyra 架构影响很深，但还不能偷换成“项目完全等同于 Lyra”。

## 17. Lyra 相机系统应理解为“模式栈 + 视图混合 + 第三人称防穿模”三层组合

已经锁定：

1. Lyra 相机的核心不只是 CameraComponent，而是 `PlayerCameraManager + CameraComponent + CameraMode + CameraModeStack` 这条分层链。
2. `CameraModeStack` 的重点是按权重混合多个相机模式，而不是简单二选一切换。
3. `FLyraCameraModeView` 这类视图结构的价值，在于把位置、旋转、控制旋转和 FOV 都纳入统一混合边界。
4. 第三人称镜头的稳定性不应只理解为“SpringArm 自动避障”，而应理解为预测式防穿模与模式视图混合的组合。

## 18. Lyra 的 UI Camera 不应被误写成当前项目已采用的通用相机层

已经锁定：

1. Lyra 资料里存在 UI Camera 优先级层和 `SetViewTarget(...)` 切换语义。
2. 但当前项目源码里没有明显对应的 UI Camera 管理组件主链。
3. 因此整理时应把 UI Camera 视为 Lyra 样例中的可选上层，而不是当前项目已确认采用的事实。

## 19. 当前项目明确采用了 Lyra 风格相机模式栈和第三人称防穿模骨架

已经锁定：

1. 当前项目存在 `UAOCameraComponent`、`UAOCameraMode`、`UAOCameraModeStack`、`AAOPlayerCameraManager`。
2. 当前项目 `UAOCameraComponent::GetCameraView(...)` 与 `UpdateCameraModes()` 明确采用了模式栈评估和 `DetermineCameraModeDelegate`。
3. `UAOHeroComponent::DetermineCameraMode()` 明确从 `PawnData->DefaultCameraMode` 解析默认相机模式。
4. 当前项目 `UAOCameraMode_ThirdPerson` 明确存在多 feeler 防穿模、蹲伏 offset 和 `IAOCameraAssistInterface` 介面。
5. 这些点足以说明当前项目相机系统在骨架层面明显承袭了 Lyra。

## 20. 库存系统应优先理解为 `Definition + Fragment + Instance + FastArray`

已经锁定：

1. `Definition` 负责模板级声明。
2. `Fragment` 负责可组合描述片段。
3. `Instance` 负责运行时物品实例状态。
4. `FastArray` 负责容器复制边界。
5. 这四层分工比“物品类 + 背包数组”更适合多人可复制场景。

## 21. Equipment 更适合理解为“在库存骨架之上的上层系统”

已经锁定：

1. Lyra 风格 Equipment 不是脱离库存独立存在的另一套物品系统。
2. 更稳的理解是：它复用库存定义层和实例层，再叠加穿戴、能力授予、表现 Actor 和动画逻辑。
3. 因此整理库存与装备时，应先讲共同骨架，再讲装备如何在其上扩展。

## 22. 当前项目仍保留 Lyra 风格库存骨架，但已经显著项目化

已经锁定：

1. 当前项目存在 `UAOInventoryItemDefinition`、`UAOInventoryItemFragment`、`UAOInventoryItemInstance`、`FAOInventoryList : FFastArraySerializer`。
2. 当前项目 `UAOInventoryComponent` 明确负责实例创建、入库、交换、复制子对象注册和批量接收校验。
3. 当前项目保留了 Fragment 思路，但运行时大量逻辑已经扩展到 `AOEquipmentDefinition`、`AOEquipmentInstance`、`AOFormalEquipmentManagerComponent`、`AOFormalEquipmentSlotInventoryComponent`、`AOEquipmentFeatureAction` 等项目特有分层。
4. 因此后续文档里不能再把项目库存/装备系统写成 Lyra 原样复刻。

## 23. 当前项目里的 `ItemCDO` 语义仍需谨慎书写

已经锁定：

1. 当前项目的 `UAOInventoryItemInstance::SetItemDef(...)` 并不总是直接持有引擎原生 `GetDefaultObject()` 返回值。
2. 它会为定义类生成并复制一个项目语义上的定义对象实例，再以 `ItemCDO` 命名持有。
3. 因此库存、装备、技能来源物相关文档里，必须继续显式区分“引擎原生 CDO”与“项目口语化称作 ItemCDO 的定义层对象”。

## 24. 当前包的输出优先服务于“将来如何借鉴”，而不是“现在已经怎么实现”

已经锁定：

1. 每轮整理都应优先沉淀稳定机制边界。
2. 只有当证据足够时，才把某个点升级为当前项目事实。
3. 这类外部研究资料的价值，首先在于减少未来架构判断的熵，而不是堆砌历史阅读过程。
