---
title: Lyra Camera Mode Stack And Third Person Penetration Avoidance
tags:
  - knowledge
  - reference-studies
  - lyra
  - camera
  - third-person
  - unreal-engine
aliases:
  - Lyra 相机模式栈与第三人称防穿模
  - Lyra Camera Mode Stack
---

# Lyra 相机模式栈与第三人称防穿模

更新时间：2026-05-19  
适用范围：提炼 `Lyra的相机系统.md` 中长期可复用的相机架构认识，重点是模式栈、视图混合、控制旋转同步和第三人称防穿模。  
不适用范围：把本文直接写成 `AegisOdyssey` 当前相机设计说明；把 Lyra 某个 UI Camera 细节误写成 UE 通用标准；把单次调试截图当成稳定结论。

## 1. 先把这篇文章放回正确位置

这篇历史文章真正有价值的，不是某一张镜头截图，而是它把 Lyra 相机系统拆成了几层可复用边界：

1. `PlayerCameraManager`
2. `CameraComponent`
3. `CameraMode`
4. `CameraModeStack`
5. 第三人称防穿模

因此它在知识库里的正确位置，是 `ReferenceStudies` 下的相机架构参考，而不是当前项目现状文档。

## 2. Lyra 相机的重点不是“一个跟随镜头”，而是模式栈

更稳的理解是：

1. `PlayerCameraManager` 负责总体视图更新入口。
2. `CameraComponent` 负责每帧评估当前相机模式并产出最终 `FMinimalViewInfo`。
3. `CameraMode` 负责单个相机模式的视图逻辑。
4. `CameraModeStack` 负责多个模式的叠加、混合与替换。

这意味着 Lyra 相机不是“状态机硬切一个摄像机”，而是“把多个视图模式按权重叠加成最终视图”。

## 3. `CameraModeStack` 的本质是“按栈混合视图”

这套结构最重要的地方不在类名，而在混合语义：

1. 栈里每一层都是一个 `CameraMode`。
2. 每个模式维护自己的 blend 时间、blend 函数、blend 权重。
3. 评估时先更新栈内模式，再从底层往上层混合最终视图。
4. 当上层模式权重到 100% 时，下层失去意义的条目可以被移除。

这比“切第一人称 / 切第三人称 / 切瞄准镜头”的硬切方式更稳，因为它天然支持：

1. 瞄准过渡
2. 受击 FOV 偏移
3. 临时能力镜头
4. 特殊模式叠加基础模式

## 4. 视图混合的关键不是位置，而是整组视图参数

Lyra 风格里，混合对象不是单个坐标，而是一整组相机视图：

1. `Location`
2. `Rotation`
3. `ControlRotation`
4. `FieldOfView`

这很重要，因为它说明“镜头模式切换”本质上不是只改机位，而是一起改：

1. 画面位置
2. 相机朝向
3. 玩家控制朝向同步
4. FOV 表现

所以相机模式栈真正提供的是“完整视图状态混合边界”。

## 5. `CameraComponent` 的稳定职责是“每帧把模式栈翻译成最终相机”

Lyra 风格 `CameraComponent` 的主线可收敛成：

1. 更新当前应使用的模式
2. 评估模式栈
3. 同步 `PlayerController` 的控制旋转
4. 应用临时 FOV 偏移
5. 把最终视图回写到 CameraComponent 与 `DesiredView`

因此 `CameraComponent` 更适合被理解为：

- 模式栈到真实相机视图之间的执行层

而不是一个只存位置的普通组件。

## 6. 默认相机模式不应写死在镜头系统内部

历史文章里另一个值得保留的点，是默认相机模式来源不必硬编码在 CameraComponent 里。

更稳的写法是：

1. 相机组件只提供“向外请求当前模式”的入口。
2. 当前角色或 PawnData 决定默认模式是什么。
3. 特殊能力或临时状态可以覆盖默认模式。

这让相机系统更接近 Lyra 其它系统的组织方式：  
模式决定权在玩法层，模式执行权在相机层。

## 7. 第三人称镜头稳定性的关键不只是碰撞，而是预测式防穿模

这篇资料里最值得保留的一块，是第三人称防穿模的理解方式。

更稳的理解不是：

- 摄像机有碰撞，所以会自动防穿模

而是：

1. 先确定对角色和瞄准更安全的参考位置。
2. 再从该位置向目标镜头方向发多条带偏转的 feeler。
3. 根据命中结果，按权重回推或修正最终镜头位置。
4. 必要时向被观察对象发送“镜头已压近”的辅助信号。

这是一套预测式镜头修正逻辑，而不是单条射线或单次碰撞响应。

## 8. `PenetrationAvoidanceFeeler` 的稳定定位

`Feeler` 这类结构真正表达的是：

1. 检测方向偏移
2. 世界命中权重
3. Pawn 命中权重
4. 检测体积
5. 检测间隔与节流

这说明它不是“多打几条线”这么简单，而是一种可调度的镜头感知阵列。  
因此后续写相机文档时，更适合把它写成“镜头避障探针配置”，而不是零散魔法参数。

## 9. UI Camera 应视为 Lyra 中的上层扩展，而不是相机骨架本体

这篇文章里提到了 UI Camera 和 `SetViewTarget(...)` 优先级覆盖。  
但这部分更适合作为 Lyra 的上层能力，而不是整个相机架构的必须部分。

更稳的表述是：

1. 相机骨架的稳定核心是模式栈与视图混合。
2. UI Camera 是可能存在的上层优先级覆盖层。
3. 不是所有继承 Lyra 风格相机系统的项目都必须照搬这一层。

## 10. 当前项目里已明确采用的相机对应点

这轮不能只信历史文章，必须回到当前项目源码核对。

已确认：

1. 当前项目存在 `AAOPlayerCameraManager`、`UAOCameraComponent`、`UAOCameraMode`、`UAOCameraModeStack`。
2. `UAOCameraComponent::GetCameraView(...)` 明确执行了模式更新、栈评估、控制旋转同步、FOV offset 叠加和最终 `DesiredView` 填充。
3. `UAOCameraComponent::UpdateCameraModes()` 明确通过 `DetermineCameraModeDelegate` 向玩法层请求当前模式。
4. `UAOHeroComponent::DetermineCameraMode()` 明确从 `PawnData->DefaultCameraMode` 获取默认模式，也支持 `AbilityCameraMode` 覆盖。
5. `UAOCameraModeStack` 明确存在 `PushCameraMode(...)`、`EvaluateStack(...)`、`UpdateStack(...)`、`BlendStack(...)`。
6. `UAOCameraMode_ThirdPerson` 明确存在蹲伏镜头 offset、预测式防穿模、多 feeler 探针与 `IAOCameraAssistInterface` 支持。

这足以说明当前项目在相机骨架层面明显承袭了 Lyra。

## 11. 当前项目里没有明显确认的 Lyra UI Camera 对应主链

同样需要明确边界。

当前源码层我没有看到与 Lyra 文章中 UI Camera 组件同等清晰的主链，包括：

1. 单独 UI Camera 管理组件
2. UI Camera 优先级覆盖主镜头
3. UI 驱动的 `SetViewTarget(...)` 主用法

因此这部分不能写成“项目已采用”，最多只能保留为 Lyra 参考层。

## 12. 对当前项目真正有价值的借鉴方向

如果后续继续吸收这套设计，更值得保留的是：

1. 相机模式由玩法层决定、由相机层执行。
2. 用模式栈做平滑过渡，而不是硬切镜头。
3. 把控制旋转同步视为相机输出的一部分。
4. 把第三人称防穿模设计成可调 feeler 阵列，而不是纯碰撞开关。
5. 把“能力相机”“默认相机”“受击 FOV 偏移”都当成可叠加模式来组织。

## 13. 适用范围与不适用范围再收束一次

### 13.1 适用范围

1. 理解 Lyra 风格第三人称相机为什么更稳定。
2. 评估相机系统是否应从“单镜头逻辑”升级成“模式栈逻辑”。
3. 给当前项目的相机扩展、能力镜头和防穿模调优提供参考。

### 13.2 不适用范围

1. 不能据此断言当前项目已完整照搬 Lyra UI Camera 层。
2. 不能把 Lyra 某个相机类名或混合细节写成 UE 强制标准。
3. 不能把本文直接当成当前项目相机现状说明。

## 14. 关联文档

- [[ReferenceStudies Project Map]]
- [[ReferenceStudies Decisions]]
- [[ReferenceStudies Known Issues]]
- [[Lyra Modular Character And GAS Wiring]]
