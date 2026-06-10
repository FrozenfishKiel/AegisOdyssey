---
title: ReferenceStudies Project Map
tags:
  - knowledge
  - reference-studies
  - project-map
  - unreal-engine
  - external-study
aliases:
  - ReferenceStudies Project Map
  - ReferenceStudies 项目地图
---

# ReferenceStudies 项目地图

更新时间：2026-05-19  
适用范围：整理 `Notice/HistoryNotice` 中外部项目、样例工程、引擎示例和学习笔记里可以长期复用的稳定认识，并明确它们与当前项目事实之间的边界。  
不适用范围：把外部样例直接写成 `AegisOdyssey` 当前实现说明；记录单次调参过程；保存历史截图式接线过程。

## 1. 这个包解决什么问题

`ReferenceStudies` 只回答三件事：

1. 外部资料里哪些内容已经沉淀成了可复用机制认识。
2. 这些认识里哪些属于 UE 正式机制，哪些只是某个样例工程的组织方式。
3. 当前项目已经明确采用了哪些对应思路，哪些仍然只能停留在参考层。

它不是“项目现状包”，而是“外部参考研究包”。

## 2. 当前已收口的三组主题

### 2.1 GASP：locomotion / motion matching / traversal 参考

来源：

1. `GASP笔记01.md`
2. `GASP笔记02.md`
3. `GASP笔记03.md`

对应主题笔记：

- [[GASP Locomotion Motion Matching And Traversal]]

这组资料沉淀下来的稳定结论主要是：

1. Motion Matching 应回到 `Schema / Database / Query Trajectory / Pose History / AnimNode_MotionMatching` 这一组正式机制理解。
2. Traversal 应回到“环境检测 -> 中间结果结构 -> 动画选择 -> Motion Warping 执行”的结果驱动链理解。
3. `MotionWarping`、`OffsetRootBone`、`FootPlacement` 更适合作为支撑修正层，而不是动作主选择层。
4. 当前项目虽已具备插件、配置、Tag 和资产前置条件，但还不能仅凭这些前置条件就写成“整套 GASP locomotion 已正式接线”。

### 2.2 高级动画层 / Distance Matching / Warping 参考

来源：

1. `高级运动系统笔记01.md`
2. `高级运动系统笔记02.md`
3. `高级运动系统笔记03.md`

对应主题笔记：

- [[Advanced Animation Layers Distance Matching And Warping]]

这组资料沉淀下来的稳定结论主要是：

1. `UAnimLayerInterface` 与 `LinkAnimClassLayers(...)` 属于 UE 正式支持的动画层边界，不只是某个样例蓝图技巧。
2. `Sequence Evaluator` 的本质是显式时间取样器，而不是普通自动播放节点。
3. Distance Matching 的本质是“距离到动画时间的映射机制”，不是某个单独的刹车技巧。
4. `Stride Warping`、`Orientation Warping`、`Rotate Root Bone`、Additive 倾斜属于表现修正层。
5. 当前项目源码层已明确存在 crouch 与 camera crouch offset，但尚未确认高级动画主链正式接入。

### 2.3 Lyra：模块化角色与 GAS 接线参考

来源：

1. `Lyra的GAS系统.md`
2. `Lyra的GAS系统02.md`
3. `Lyra的角色系统.md`

对应主题笔记：

- [[Lyra Modular Character And GAS Wiring]]

这组资料沉淀下来的稳定结论主要是：

1. Lyra 的 GAS 架构应理解为“模块化角色框架 + 运行时能力注入模式”，而不只是一套 GAS 教程碎片。
2. 稳定核心是 `PlayerState` 持久 ASC、当前 `Pawn/Character` 作为 `AvatarActor`、`PawnExtension/Hero` 初始化链负责接线与重绑。
3. `UGameFrameworkComponentManager`、扩展事件和 GameFeatureAction 构成运行时注入机制。
4. `AbilitySet` 是一组可回收的授予包，而不是简单能力表。
5. 输入链应理解为 `InputAction -> InputTag -> AbilitySpec Handle 队列 -> ProcessAbilityInput`。
6. 当前项目已明确采用了很多 Lyra 风格对应点，但仍应放在 `ReferenceStudies`，因为这一轮来源仍是 Lyra 学习材料。

### 2.4 Lyra：相机模式栈与第三人称防穿模参考

来源：

1. `Lyra的相机系统.md`

对应主题笔记：

- [[Lyra Camera Mode Stack And Third Person Penetration Avoidance]]

这组资料沉淀下来的稳定结论主要是：

1. Lyra 相机的核心不是单个 CameraComponent，而是 `PlayerCameraManager + CameraComponent + CameraMode + CameraModeStack` 这条分层链。
2. `CameraModeStack` 的重点是“按栈混合模式”，而不是状态机硬切镜头。
3. 第三人称镜头稳定性的关键不只是 SpringArm 碰撞，而是基于多条 feeler 的预测式防穿模。
4. 当前项目已经明确采用了相机模式栈、默认相机模式委托和第三人称防穿模骨架。
5. 但当前项目没有明显照搬 Lyra 的 UI Camera 优先级层，因此这部分仍应写成“局部承袭、局部裁剪”。

### 2.5 Lyra：库存 Definition / Fragment / Instance / Equipment 参考

来源：

1. `Lyra库存系统.md`

对应主题笔记：

- [[Lyra Inventory Definition Fragment Instance And Equipment]]

这组资料沉淀下来的稳定结论主要是：

1. Lyra 库存的稳定核心是 `Definition + Fragment + Instance + FastArray` 四层分工。
2. `InventoryItemDefinition` 负责声明模板与 Fragment，`InventoryItemInstance` 负责运行时实例状态。
3. `FFastArraySerializer` 负责库存容器复制边界，而不是“物品系统本体”。
4. Equipment 更适合理解为“复用库存定义层，再叠加穿戴、授予与表现逻辑”的上层系统。
5. 当前项目仍保留了这套骨架，但装备、正式装备槽、技能来源物和 FeatureAction 已经形成明显项目化分层，不应再写成 Lyra 原样。

## 3. 推荐阅读顺序

### 3.1 先看 UE 正式机制

优先问：

1. 这件事在 UE5.6 里的正式入口是什么。
2. 它属于引擎能力、插件能力还是样例工程组织方式。
3. 当前结论能否脱离某一篇历史文章单独成立。

### 3.2 再看外部样例如何组装

再问：

1. 这个样例是如何把正式机制拼成完整系统的。
2. 哪些是稳定架构边界，哪些只是实现细节。
3. 哪些调参、蓝图接线或 workaround 不应进入知识库正文。

### 3.3 最后映射当前项目

最后才问：

1. 当前项目有无明确代码入口。
2. 当前项目是否存在对应插件、配置、Tag、资产与生命周期事件。
3. 能否把“参考研究”升级成“项目现状事实”。

## 4. 当前项目里已明确出现的参考落点

### 4.1 locomotion / animation 方向

已明确：

1. 工程已启用 `PoseSearch`、`MotionWarping`、`AnimationWarping`、`MotionTrajectory`、`Chooser` 等插件。
2. 工程已存在 `MotionMatching.*` 相关 Tag、`OffsetRootBone` 相关配置以及一批 motion matching / traversal 资产。
3. 工程源码层已存在 crouch 与 camera crouch offset 的运行时处理。

尚未确认：

1. motion matching 主链
2. traversal 执行主链
3. `LinkAnimClassLayers(...)`
4. Distance Matching / Warping / Turn In Place 的源码入口

### 4.2 Lyra 风格 GAS / 模块化角色方向

已明确：

1. `UAOExtPawnComponent` 与 `UAOHeroComponent` 负责初始化状态链与 ASC 接线。
2. 当前项目存在 `AAOPlayerState::NAME_AOAbilityReady`、`AAOCharacter::NAME_AOAbilityReady` 扩展事件。
3. `GF_AddAbilities` 使用 `UGameFrameworkComponentManager::AddExtensionHandler(...)` 做运行时授予。
4. `UAOAbilitySet` 提供能力、效果、属性集的打包授予与回收。
5. `UAOAbilitySystem` 采用基于输入 Tag 的能力输入泵。
6. `UAOHealthAttributeSet` 等属性集已明确采用复制回调与 GE 前后处理模式。

需要继续保持谨慎的地方：

1. 当前项目是“Lyra 风格对应实现”，不等于与 Lyra 一字不差。
2. 不应把 Lyra 中某个具体命名、类图或蓝图组织误写成 UE 必选标准。

## 5. 当前主题笔记

- [[GASP Locomotion Motion Matching And Traversal]]
- [[Advanced Animation Layers Distance Matching And Warping]]
- [[Lyra Modular Character And GAS Wiring]]
- [[Lyra Camera Mode Stack And Third Person Penetration Avoidance]]
- [[Lyra Inventory Definition Fragment Instance And Equipment]]

## 6. 关联文档

- [[ReferenceStudies Decisions]]
- [[ReferenceStudies Known Issues]]
