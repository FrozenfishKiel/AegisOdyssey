# UE5智能对象 Smart Object 从介绍到基础案例实现

## 1. 这篇文档是写给谁的

这篇文档面向两类场景：

- 你想真正理解 UE5 的 Smart Object 系统，而不是只记住几个 API 名字。
- 你想把 Smart Object 接到当前项目里，并且和现在已经存在的 `StateTree + AI MoveTo` 体系结合起来。

本文基于当前项目使用的 **UE 5.6** 来讲，且尽量用你项目现有结构做映射，而不是写一个脱离项目上下文的通用教程。

---

## 2. 一句话先说清楚：什么是 Smart Object

Smart Object 可以理解成：

**“场景中可被 AI 或玩家查询、占用、使用、释放的一类标准化交互资源。”**

它关注的不是“这个 Actor 长什么样”，而是“这个对象提供了什么可用行为槽位，以及谁现在能不能用它”。

比如下面这些都适合做成 Smart Object：

- 椅子上的坐下点
- 掩体后的蹲伏点
- 炮台/机枪位
- 工作台的操作位
- 营火边的休息位
- 医疗台的治疗位

这些对象有一个共同点：

- 不是单纯“点击就触发”的 UI 交互
- 而是有明确的空间位置
- 可能会被多个 AI 竞争
- 可能有资格限制、状态限制、数量限制

Smart Object 正是为这种“**世界资源式交互**”设计的。

---

## 3. 它解决了什么问题

如果不用 Smart Object，我们通常会这么做：

- AI 自己搜索一个椅子 Actor
- AI 自己判断这个椅子能不能坐
- AI 自己处理多人抢同一个座位
- AI 自己计算应该站到哪里
- AI 自己管理使用中/使用完状态

这样写到后面，交互逻辑会很散：

- 一部分在 Actor
- 一部分在 AIController
- 一部分在 BehaviorTree / StateTree
- 一部分在动画或 Ability 里

Smart Object 的价值就在于把这些问题标准化：

- 查找什么能用
- 过滤谁能用
- 抢占一个槽位
- 获取这个槽位的世界位置
- 执行使用行为
- 使用完释放槽位

所以它本质上是：

**资源管理系统 + 空间化交互入口 + AI 可复用协议。**

---

## 4. 它和普通“可交互 Actor”的区别

普通可交互 Actor 更像：

- `AChairActor`
- `Interact()`
- 里面自己写移动、动画、占用逻辑

Smart Object 更像：

- `Chair` 只是视觉或拥有者
- `USmartObjectComponent` 声明“我提供一个可坐的 slot”
- AI 去系统里查询“附近有没有可坐的 slot”
- 找到后 claim
- 拿到 slot transform
- 执行自己的行为
- 完成后 release

也就是说，Smart Object 把“对象外观”和“对象可提供的交互能力”拆开了。

---

## 5. Smart Object 的核心组成

理解 Smart Object，抓住下面 6 个词就够了。

### 5.1 Definition

`USmartObjectDefinition`

它是一个资产，定义这个 Smart Object 有哪些槽位、哪些标签、哪些条件、哪些行为定义。

引擎源码里，`USmartObjectDefinition` 是 Smart Object 的核心资产类型：

- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectDefinition.h`

### 5.2 Slot

`FSmartObjectSlotDefinition`

一个 Smart Object 可以有多个 slot。

例如：

- 一把单人椅子有 1 个 slot
- 一张长椅有 3 个 slot
- 一个掩体可能有左侧探头位、右侧探头位两个 slot

slot 上通常会有：

- 局部偏移位置
- 局部旋转
- Activity Tags
- User Tag Filter
- 选择条件
- Behavior Definitions

### 5.3 Component

`USmartObjectComponent`

它通常挂在世界中的 Actor 上，把这个 Actor 注册进 Smart Object 子系统。

源码位置：

- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectComponent.h`

你可以把它理解成“**把场景对象暴露成 Smart Object 资源**”的桥梁。

### 5.4 Request / Filter

- `FSmartObjectRequestFilter`
- `FSmartObjectRequest`
- `FSmartObjectRequestResult`

它们描述：

- 我要找什么类型的 Smart Object
- 搜索范围多大
- 用户是谁
- 要求哪些 ActivityTags
- 是否要求特定行为定义

源码位置：

- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectRequestTypes.h`

### 5.5 Claim Handle

`FSmartObjectClaimHandle`

这是“我已经成功占用某个 slot”的凭证。

只要 claim 成功，后续很多操作都会围绕这个 handle 展开：

- 获取 slot transform
- 标记 occupied
- 获取 behavior definition
- 注册失效回调
- 释放 slot

### 5.6 Subsystem

`USmartObjectSubsystem`

这是运行时的大脑，负责：

- 注册 Smart Object
- 空间查询
- 评估条件
- claim / occupy / release
- 获取 slot 的位置和朝向

源码位置：

- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectSubsystem.h`

---

## 6. 运行时到底发生了什么

Smart Object 的典型运行链路是：

1. 场景中的 Actor 挂了 `USmartObjectComponent`
2. 组件引用一个 `USmartObjectDefinition`
3. 组件注册到 `USmartObjectSubsystem`
4. AI 发起查询，请求附近符合条件的 slot
5. 子系统返回一组 `FSmartObjectRequestResult`
6. AI 从结果里挑一个 slot 并 claim
7. 成功后拿到 `FSmartObjectClaimHandle`
8. AI 读取 slot 的 transform，移动过去
9. AI 执行“使用行为”
10. 使用结束，release 该 claim handle

你可以把它记成：

**Find -> Claim -> Move -> Use -> Release**

这五步是 Smart Object 的核心节奏。

---

## 7. Smart Object 和几个常见系统的关系

### 7.1 和 StateTree 的关系

StateTree 负责“决策和流程”。

Smart Object 负责“资源查询和占用”。

一个很自然的组合是：

- StateTree 判断当前需要休息
- 任务节点发起 Smart Object 查询
- 成功后输出 slot location 和 claim handle
- 下一个任务移动到 slot
- 再下一个任务执行坐下或等待
- 退出状态时释放 claim handle

### 7.2 和 BehaviorTree 的关系

BehaviorTree 同理。

引擎其实已经给了一个现成例子：

- `UBTTask_FindAndUseGameplayBehaviorSmartObject`
- 文件位置：
  `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Private/AI/BTTask_FindAndUseGameplayBehaviorSmartObject.cpp`

这个任务做的事情，本质上就是：

- 查找
- claim
- 调用使用任务

所以如果你理解了这个 BTTask 的思路，就理解了 Smart Object 的基本玩法。

### 7.3 和 EQS 的关系

EQS 可以负责“搜索候选点”。

Smart Object 可以负责“把候选点变成可占用资源”。

有时你会这样组合：

- 先用 EQS 找一批 Smart Object
- 再对这些结果进行 claim

### 7.4 和 Gameplay Behavior 的关系

Smart Object 本身不强制你必须用 Gameplay Behavior。

它只负责把“可用资源”和“行为定义入口”组织起来。

你后续可以选择：

- 用 Gameplay Behavior
- 用 StateTree task
- 用 Gameplay Ability
- 用 Montage
- 用你自己的业务逻辑

这点很重要。

**Smart Object 不等于 Gameplay Behavior。**

Gameplay Behavior 只是 Smart Object 的一种常见使用方式。

---

## 8. 从引擎源码看几个最关键的 API

下面这些 API 是你真正应该记住的。

### 8.1 查询

`USmartObjectSubsystem::FindSmartObjects`

定义位置：

- `SmartObjectSubsystem.h`

用途：

- 根据范围和 filter 找到候选 slot

### 8.2 抢占

`USmartObjectSubsystem::MarkSlotAsClaimed`

用途：

- 把某个 slot 标记为已被当前用户 claim

### 8.3 使用中

`USmartObjectSubsystem::MarkSlotAsOccupied`

用途：

- 在 claim 之后，进一步标记“正在被使用”

不是所有项目都必须手动调用它，但如果你的逻辑严格区分：

- 已预定
- 正在使用

那它会很有用。

### 8.4 读取位置

- `USmartObjectSubsystem::GetSlotLocation`
- `USmartObjectSubsystem::GetSlotTransform`

用途：

- 获取 AI 应该移动到的位置和朝向

### 8.5 释放

- `USmartObjectSubsystem::MarkSlotAsFree`
- `USmartObjectSubsystem::Release`

用途：

- 使用结束后把资源还回去

---

## 9. 你项目当前和 Smart Object 的关系

我先把当前项目状态说清楚，这样后面的案例就不会脱节。

### 9.1 当前项目已经有的基础

当前项目已经有：

- `StateTree`
- `GameplayStateTree`
- `AIModule`
- `GameplayTasks`
- `NavigationSystem`
- 自定义 `StateTree` 组件
- 自定义 `MoveToLocation` 任务

关键文件：

- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.cpp`

其中 `FSTT_MoveToLocation` 已经封装了 `UAITask_MoveTo`，非常适合拿来接 Smart Object 输出的位置。

### 9.2 当前项目还没有接入的部分

从当前工程文件看：

- `AegisOdyssey.uproject` 里已经启用了 `StateTree` 和 `GameplayStateTree`
- 但还没有显式启用 `SmartObjects` 或 `GameplayBehaviorSmartObjects`
- `Source/AegisOdyssey/AegisOdyssey.Build.cs` 里也还没有 `SmartObjectsModule` 或 `GameplayBehaviorSmartObjectsModule`

所以如果要正式接入 Smart Object，第一步不是写 AI 逻辑，而是先补模块依赖。

---

## 10. 一个最小案例：AI 找到椅子并坐下

我们先定义目标：

**让敌人或 NPC 在空闲时，搜索附近一把可坐的椅子，走到椅子对应 slot，停留几秒，然后释放。**

这是一个非常标准的 Smart Object 入门案例。

---

## 11. 这个案例为什么适合当入门

因为它几乎把 Smart Object 的核心全覆盖了：

- 世界中有资源
- 资源有明确站位点
- 多个 AI 可能竞争
- 需要 claim
- 需要移动到目标点
- 使用结束需要 release

而且它不会一上来就把你拖进复杂战斗逻辑。

---

## 12. 两种实现路线

我建议把实现路线分成两种理解。

### 12.1 路线 A：引擎原生演示路线

特点：

- 更接近 Epic 官方范式
- 常配合 `GameplayBehaviorSmartObjects`
- 适合快速理解系统

流程：

- Smart Object 定义 slot
- slot 携带 `GameplayBehaviorSmartObjectBehaviorDefinition`
- AI 找到并 claim
- 调用 `UAITask_UseGameplayBehaviorSmartObject`

### 12.2 路线 B：更适合你当前项目的路线

特点：

- 保留你现在的 `StateTree + MoveToLocation`
- Smart Object 只负责“找资源 + 占资源 + 给位置”
- 真正执行坐下、等待、播放动画仍由你自己的 StateTree 任务负责

流程：

- Smart Object 负责输出 `ClaimHandle + SlotTransform`
- 现有 `FSTT_MoveToLocation` 负责移动
- 你自己的任务负责坐下/等待/播放动画
- 退出状态时释放 claim

**对当前项目来说，我更推荐路线 B。**

原因很简单：

- 侵入性更小
- 你现有系统复用率更高
- 调试路径更短
- 不会一上来把 Gameplay Behavior 体系一起引进来

---

## 13. 正式接入前的工程准备

### 13.1 启用插件

至少需要：

- `SmartObjects`

如果你要走“原生演示路线”，再加上：

- `GameplayBehaviorSmartObjects`

如果引擎要求连带启用依赖，也一并打开。

### 13.2 添加模块依赖

如果你要在 C++ 里直接访问 Smart Object API，`AegisOdyssey.Build.cs` 至少要增加：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "SmartObjectsModule"
});
```

如果你还要用引擎现成的 “MoveToAndUseSmartObjectWithGameplayBehavior” 那一套，再加：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "GameplayBehaviorSmartObjectsModule"
});
```

---

## 14. 编辑器里先把资源搭起来

先不写 C++，先把资源搭出来是最稳的。

### 14.1 创建 Smart Object Definition

创建一个资产，例如：

- `SO_Chair_Definition`

在这个 Definition 里做几件事：

- 添加 1 个 slot
- 给这个 slot 起名，例如 `Seat`
- 设置 `Offset`
  让 slot 位于椅子前方或座位中心附近
- 设置 `Rotation`
  让 AI 到位后朝向正确
- 给 slot 加上活动标签，例如：
  `SmartObject.Activity.Seat`

如果你需要更细分，也可以用：

- `SmartObject.Activity.Seat.Rest`
- `SmartObject.Activity.Seat.Eat`

### 14.2 如果你走路线 A，再加行为定义

在 slot 的 `BehaviorDefinitions` 里加：

- `UGameplayBehaviorSmartObjectBehaviorDefinition`

然后给它绑定一个 `GameplayBehaviorConfig`。

这个配置最终对应“到位之后做什么”。

### 14.3 创建椅子 Actor

椅子可以是：

- 一个普通蓝图 Actor
- 一个 StaticMeshActor 的子类
- 一个你自定义的世界交互 Actor

给它挂上：

- `USmartObjectComponent`

然后把 Definition 指向：

- `SO_Chair_Definition`

这样它就变成了场景里的一个 Smart Object 资源。

---

## 15. 路线 A：最贴近官方的基础用法

这一套更像“看懂官方怎么做”。

### 15.1 运行逻辑

AI 流程大致是：

1. 在附近搜索带 `Seat` 活动标签的 Smart Object
2. 过滤出带 `UGameplayBehaviorSmartObjectBehaviorDefinition` 的 slot
3. claim 一个可用 slot
4. 调用 `UAITask_UseGameplayBehaviorSmartObject::MoveToAndUseSmartObjectWithGameplayBehavior`
5. 到位后执行 behavior
6. 任务结束后释放

### 15.2 关键现成类

引擎已经提供了：

- `UGameplayBehaviorSmartObjectBehaviorDefinition`
- `UAITask_UseGameplayBehaviorSmartObject`
- `UBTTask_FindAndUseGameplayBehaviorSmartObject`

源码位置：

- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Public/GameplayBehaviorSmartObjectBehaviorDefinition.h`
- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Public/AI/AITask_UseGameplayBehaviorSmartObject.h`
- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Private/AI/BTTask_FindAndUseGameplayBehaviorSmartObject.cpp`

### 15.3 这条路线的优点

- 官方链路完整
- 查询、移动、使用的配套比较现成
- 适合做一个独立 Demo

### 15.4 这条路线的缺点

- 对你当前项目来说有点“额外引入一套行为层”
- 如果你后续实际执行动作还是想放进 StateTree，就会出现两套流程混用

所以我不建议你把它作为当前项目的第一落点。

---

## 16. 路线 B：更适合当前项目的落地方案

这一节是重点。

### 16.1 目标

让 Smart Object 只负责：

- 找到椅子
- 占住椅子
- 提供椅子 slot 的 transform

剩下的都交给你现在的 StateTree：

- `MoveToLocation`
- 播动画
- 等待
- 释放

### 16.2 最自然的状态流

一个很自然的 StateTree 状态流是：

1. `FindSeatSmartObject`
2. `MoveToLocation`
3. `PlaySitAnimationOrWait`
4. `ReleaseSeatSmartObject`

你现在已经有第 2 步：

- `FSTT_MoveToLocation`

所以只需要再补：

- 一个查找并 claim 的任务
- 一个释放任务

---

## 17. 基础数据应该怎么存

你至少需要在 StateTree 节点实例数据里保存这些内容：

```cpp
USTRUCT()
struct FSeatSmartObjectContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Config")
    float SearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category="Config")
    float AcceptableRadius = 80.0f;

    UPROPERTY(VisibleAnywhere, Category="Output")
    FVector SeatLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category="Output")
    FRotator SeatRotation = FRotator::ZeroRotator;

    UPROPERTY(Transient)
    FSmartObjectClaimHandle ClaimHandle;
};
```

这里最重要的是两样：

- `SeatLocation`
- `ClaimHandle`

前者给 `MoveToLocation` 用，后者给“释放任务”用。

---

## 18. 查找并 claim 的核心逻辑

下面是一个适合接进你项目 StateTree 的最小思路。

### 18.1 任务职责

这个任务只做四件事：

1. 找到 AI 自己
2. 查询附近 seat 类型的 Smart Object
3. claim 一个可用 slot
4. 输出 slot transform

### 18.2 代码骨架

```cpp
USTRUCT()
struct FSTT_FindSeatSmartObjectInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Config")
    float SearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category="Config")
    FGameplayTagQuery ActivityRequirements;

    UPROPERTY(VisibleAnywhere, Category="Output")
    FVector GoalLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category="Output")
    FRotator GoalRotation = FRotator::ZeroRotator;

    UPROPERTY(Transient)
    FSmartObjectClaimHandle ClaimHandle;
};

USTRUCT(DisplayName="Find Seat Smart Object", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_FindSeatSmartObject : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTT_FindSeatSmartObjectInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;
};
```

### 18.3 参考实现

```cpp
EStateTreeRunStatus FSTT_FindSeatSmartObject::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    InstanceData.ClaimHandle = FSmartObjectClaimHandle();

    AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
    if (OwnerActor == nullptr)
    {
        return EStateTreeRunStatus::Failed;
    }

    APawn* UserPawn = Cast<APawn>(OwnerActor);
    if (UserPawn == nullptr)
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerActor))
        {
            UserPawn = AIController->GetPawn();
        }
    }

    if (UserPawn == nullptr)
    {
        return EStateTreeRunStatus::Failed;
    }

    USmartObjectSubsystem* SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(UserPawn->GetWorld());
    if (SmartObjectSubsystem == nullptr)
    {
        return EStateTreeRunStatus::Failed;
    }

    FSmartObjectRequestFilter Filter;
    Filter.ActivityRequirements = InstanceData.ActivityRequirements;

    const FVector UserLocation = UserPawn->GetActorLocation();
    const FBox QueryBox = FBox(UserLocation, UserLocation).ExpandBy(FVector(InstanceData.SearchRadius));
    const FSmartObjectRequest Request(QueryBox, Filter);

    TArray<FSmartObjectRequestResult> Results;
    if (!SmartObjectSubsystem->FindSmartObjects(Request, Results, UserPawn) || Results.IsEmpty())
    {
        return EStateTreeRunStatus::Failed;
    }

    const FSmartObjectActorUserData ActorUserData(UserPawn);
    const FConstStructView UserDataView = FConstStructView::Make(ActorUserData);

    for (const FSmartObjectRequestResult& Result : Results)
    {
        const FSmartObjectClaimHandle ClaimHandle =
            SmartObjectSubsystem->MarkSlotAsClaimed(
                Result.SlotHandle,
                ESmartObjectClaimPriority::Normal,
                UserDataView);

        if (!ClaimHandle.IsValid())
        {
            continue;
        }

        FTransform SlotTransform;
        if (!SmartObjectSubsystem->GetSlotTransform(ClaimHandle, SlotTransform))
        {
            SmartObjectSubsystem->Release(ClaimHandle);
            continue;
        }

        InstanceData.ClaimHandle = ClaimHandle;
        InstanceData.GoalLocation = SlotTransform.GetLocation();
        InstanceData.GoalRotation = SlotTransform.Rotator();
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Failed;
}
```

### 18.4 这一段代码在做什么

- `FindSmartObjects` 负责找候选 slot
- `MarkSlotAsClaimed` 负责真正抢占
- `GetSlotTransform` 负责把 slot 变成世界空间目标点
- 抢到了但拿不到位置，就马上 `Release`

这是一个非常标准的 Smart Object 使用模式。

---

## 19. 然后怎么接你现有的 MoveToLocation

你项目现有的 `FSTT_MoveToLocation` 已经很适合承接这一步。

它当前做的事情是：

- 从 StateTree instance data 读取 `GoalLocation`
- 生成 `FAIMoveRequest`
- 创建 `UAITask_MoveTo`
- 在移动结束时回调 `FinishTask`

对应文件：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.cpp`

所以你可以直接把上一步输出的：

- `GoalLocation`

绑定到 `FSTT_MoveToLocation` 的输入上。

这就是为什么我说你项目非常适合走路线 B。

因为你已经把“移动到一个点”的这层抽象写好了。

---

## 20. 到位后做什么

到位后其实有很多选择。

最小实现可以先这么做：

- 播一个坐下 Montage
- 等待 2~5 秒
- 播起身 Montage

更简单一点甚至可以先只做：

- 到位
- `Wait 3s`
- 释放

因为对 Smart Object 来说，重点不是一开始就把动画做全，而是先把资源占用流跑通。

---

## 21. 为什么还需要 release 任务

因为 claim 不是“自动消失”的。

如果你不 release，后果通常就是：

- 这个座位会一直被占着
- 其他 AI 永远抢不到
- 长时间运行后逻辑越来越乱

所以 **release 是必须收尾的一步**。

最好做到：

- 正常完成时 release
- 中断时 release
- StateTree 状态切走时 release
- Actor 销毁时也考虑兜底 release

---

## 22. 释放任务的最小骨架

```cpp
USTRUCT(DisplayName="Release Smart Object", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_ReleaseSmartObject : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTT_FindSeatSmartObjectInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override
    {
        FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

        if (!InstanceData.ClaimHandle.IsValid())
        {
            return EStateTreeRunStatus::Succeeded;
        }

        AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
        if (OwnerActor == nullptr)
        {
            return EStateTreeRunStatus::Succeeded;
        }

        if (USmartObjectSubsystem* SmartObjectSubsystem =
            USmartObjectSubsystem::GetCurrent(OwnerActor->GetWorld()))
        {
            SmartObjectSubsystem->Release(InstanceData.ClaimHandle);
        }

        InstanceData.ClaimHandle = FSmartObjectClaimHandle();
        return EStateTreeRunStatus::Succeeded;
    }
};
```

这个任务不复杂，但非常关键。

---

## 23. 一个完整的 StateTree 逻辑长什么样

你可以把状态流搭成这样：

### 23.1 Idle / NeedRest

- 判断当前是否允许进入“找椅子休息”

### 23.2 FindSeatSmartObject

- 调用查询任务
- 输出 `GoalLocation`
- 输出 `ClaimHandle`

### 23.3 MoveToSeat

- 复用当前的 `FSTT_MoveToLocation`

### 23.4 UseSeat

- 播动画或等待

### 23.5 ReleaseSeat

- 释放 claim

### 23.6 ReturnToDefault / ContinuePatrol

- 返回巡逻、待机或后续行为

---

## 24. 如果以后要做得更完整，可以再加什么

当最小案例跑通后，你可以继续增强。

### 24.1 用 Tag 区分不同交互类型

例如：

- `SmartObject.Activity.Seat`
- `SmartObject.Activity.Cover`
- `SmartObject.Activity.Turret`

这样 AI 就可以按意图查找资源，而不是按具体 Actor 类型查找。

### 24.2 用 UserTagFilter 限制资格

例如：

- 只有 `AI.Role.Civilian` 才能用餐桌位
- 只有 `AI.Role.Soldier` 才能用机枪位

### 24.3 加 Selection Conditions

例如：

- 夜晚才能用睡觉点
- 受伤时才能用治疗点
- 战斗中不能坐椅子

### 24.4 区分 Claim 和 Occupied

如果你的行为更复杂，可以把资源状态分成：

- 已预定
- 正在使用

这样调试起来更清楚。

### 24.5 处理中断和失效

例如：

- AI 死亡
- 椅子被销毁
- slot 被禁用
- 状态树被强制切换

这些场景都要确保 claim 不会泄漏。

---

## 25. 常见误区

### 25.1 误区一：把 Smart Object 当成普通交互 Actor

Smart Object 不是“Actor 的另一个名字”。

它更像“交互资源协议”。

### 25.2 误区二：以为它必须配 BehaviorTree

不是。

它和 StateTree 配起来同样自然。

### 25.3 误区三：以为用了 Smart Object 就不用自己处理动画

不是。

Smart Object 负责资源层，动画和业务行为仍然要你自己的系统去执行。

### 25.4 误区四：只做 claim 不做 release

这基本一定会出问题。

### 25.5 误区五：一开始就把整套系统做太重

正确做法是：

先跑通最小链路：

- 查到
- 抢到
- 走到
- 等待
- 释放

然后再加动画、条件、资格和复杂规则。

---

## 26. 对当前项目最推荐的第一步

如果让我给你一个最稳的落地顺序，我会建议：

1. 启用 `SmartObjects` 插件和 `SmartObjectsModule`
2. 做一个 `SO_Chair_Definition`
3. 做一个带 `USmartObjectComponent` 的椅子 Actor
4. 写 `FSTT_FindSeatSmartObject`
5. 复用现有 `FSTT_MoveToLocation`
6. 做一个简单的 `Wait` 或播放坐下动画
7. 写 `FSTT_ReleaseSmartObject`

这样你只新增“资源层”，而不破坏现有 AI 流程。

---

## 27. 这套方案为什么适合你当前工程

因为你项目已经具备三个关键前提：

- 已经有 StateTree 体系
- 已经有 AI MoveTo 封装
- 已经在做 AI 决策和状态拆分

所以 Smart Object 接进来以后，不需要推翻重做。

它更像是在你现有系统上补了一层：

**“世界资源查询与占用能力”。**

这比重新发明一套“可交互座位系统”划算得多。

---

## 28. 用一句话收尾

如果只记一句话，请记这个：

**Smart Object 不是让 AI ‘知道椅子是什么’，而是让 AI ‘知道这里有一个现在可以被我占用和使用的座位资源’。**

这就是它和普通 Actor 交互最大的区别。

---

## 29. 本文中提到的关键源码参考

### 29.1 引擎源码

- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectDefinition.h`
- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectComponent.h`
- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectRequestTypes.h`
- `Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectSubsystem.h`
- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Public/GameplayBehaviorSmartObjectBehaviorDefinition.h`
- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Public/AI/AITask_UseGameplayBehaviorSmartObject.h`
- `Engine/Plugins/Runtime/GameplayBehaviorSmartObjects/Source/GameplayBehaviorSmartObjectsModule/Private/AI/BTTask_FindAndUseGameplayBehaviorSmartObject.cpp`

### 29.2 当前项目源码

- `AegisOdyssey.uproject`
- `Source/AegisOdyssey/AegisOdyssey.Build.cs`
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.h`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToLocation.cpp`

---

## 30. 下一步建议

如果你准备继续往下走，最值得做的下一份文档或实现有两个方向：

- 方向 A：直接给当前项目补一版 `FSTT_FindSeatSmartObject` / `FSTT_ReleaseSmartObject`
- 方向 B：先单独做一个最小 Smart Object Demo 关卡，把椅子坐下链路跑通

对当前工程阶段来说，我更推荐先做方向 A。
