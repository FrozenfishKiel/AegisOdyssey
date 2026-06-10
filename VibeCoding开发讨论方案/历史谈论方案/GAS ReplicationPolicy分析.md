# GameplayAbility ReplicationPolicy 分析

## 问题背景

在`GA_Sprint`中设置了`ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes`，需要了解：
1. ReplicateNo和ReplicateYes的区别是什么？
2. 如果是ReplicateYes，服务器的运算数据会自动复制到客户端吗？

## 源码分析

### 1. EGameplayAbilityReplicationPolicy的定义

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\Abilities\GameplayAbilityTypes.h:96`

```cpp
namespace EGameplayAbilityReplicationPolicy
{
    enum Type : int
    {
        // We don't replicate the instance of the ability to anyone.
        ReplicateNo         UMETA(DisplayName = "Do Not Replicate"),

        // We replicate the instance of the ability to the owner.
        ReplicateYes        UMETA(DisplayName = "Replicate"),
    };
}
```

**关键区别**：
- **ReplicateNo**：不复制Ability实例到任何人
- **ReplicateYes**：复制Ability实例到拥有者（owner）

### 2. GetLifetimeReplicatedProps的实现

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\Abilities\GameplayAbility.cpp:2293`

```cpp
void UGameplayAbility::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    if (GetReplicationPolicy() != EGameplayAbilityReplicationPolicy::ReplicateNo)
    {
        if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
        {
            BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
        }
    }
}
```

**关键点**：
- 如果`ReplicationPolicy != ReplicateNo`（即`ReplicateYes`），则会复制Blueprint中标记为Replicated的属性
- **注意**：这里只复制Blueprint中的属性，不复制C++中的属性

### 3. Ability实例的复制

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\AbilitySystemComponent_Abilities.cpp:1188`

```cpp
UGameplayAbility * AbilityInstance = NewObject<UGameplayAbility>(Owner, Ability->GetClass());
check(AbilityInstance);

// Add it to one of our instance lists so that it doesn't GC.
if (AbilityInstance->GetReplicationPolicy() != EGameplayAbilityReplicationPolicy::ReplicateNo)
{
    Spec.ReplicatedInstances.Add(AbilityInstance);
    AddReplicatedInstancedAbility(AbilityInstance);
}
else
{
    Spec.NonReplicatedInstances.Add(AbilityInstance);
}

return AbilityInstance;
```

**关键点**：
- 如果`ReplicationPolicy != ReplicateNo`（即`ReplicateYes`），则将Ability实例添加到`ReplicatedInstances`列表中，并调用`AddReplicatedInstancedAbility`进行复制
- 如果`ReplicationPolicy == ReplicateNo`，则将Ability实例添加到`NonReplicatedInstances`列表中，不进行复制

## ReplicateNo vs ReplicateYes 对比

| ReplicationPolicy | 含义 | Ability实例位置 | 复制行为 |
|-------------------|------|----------------|---------|
| **ReplicateNo** | 不复制Ability实例到任何人 | 只在服务器上存在（`NonReplicatedInstances`） | 不复制到客户端 |
| **ReplicateYes** | 复制Ability实例到拥有者 | 服务器和客户端都存在（`ReplicatedInstances`） | 复制到客户端 |

## ReplicateYes的复制行为

### 1. 复制的内容

| 内容 | 是否复制 | 说明 |
|------|---------|------|
| **Ability实例** | ✅ 是 | Ability实例会被复制到客户端 |
| **Blueprint中标记为Replicated的属性** | ✅ 是 | Blueprint中标记为Replicated的属性会被复制 |
| **C++中标记为Replicated的属性** | ❌ 否 | C++中的属性不会被自动复制 |
| **Ability的执行状态** | ✅ 是 | Ability的激活、结束等状态会被复制 |
| **GameplayEffect** | ✅ 是 | Ability应用的GameplayEffect会被复制 |

### 2. 不复制的内容

| 内容 | 是否复制 | 说明 |
|------|---------|------|
| **C++中的成员变量** | ❌ 否 | C++中的成员变量不会被自动复制 |
| **局部变量** | ❌ 否 | 函数中的局部变量不会被复制 |
| **AbilityTask** | ❌ 否 | AbilityTask不会被复制，只在客户端上运行 |

### 3. 如何让C++属性复制

如果你想让C++中的属性复制，需要在`GetLifetimeReplicatedProps`中手动添加：

```cpp
void UGA_Sprint::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 手动添加需要复制的属性
    DOREPLIFETIME_CONDITION_NOTIFY(UGA_Sprint, SprintSpeedBonusAmount, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGA_Sprint, bVigorExhaustedBroadcasted, COND_None, REPNOTIFY_Always);
}
```

## 对GA_Sprint的影响

### 当前代码

```cpp
UGA_Sprint::UGA_Sprint(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;  // ← 当前设置
    SprintSpeedBonusAmount = 0.f;
    bVigorExhaustedBroadcasted = false;
}
```

### 当前设置（ReplicateYes）的影响

**复制的内容**：
- ✅ Ability实例会被复制到客户端
- ✅ Ability的激活、结束等状态会被复制
- ✅ Ability应用的GameplayEffect会被复制

**不复制的内容**：
- ❌ C++中的成员变量（如`SprintSpeedBonusAmount`、`bVigorExhaustedBroadcasted`）不会被自动复制

### 如果改为ReplicateNo

**影响**：
- ❌ 只有服务器有Ability实例
- ❌ 客户端无法访问Ability的实例数据
- ❌ 客户端无法预测Ability的执行
- ✅ 所有的逻辑都必须在服务器上执行
- ✅ 节省网络带宽

## 实际影响分析

### 适用场景

**ReplicateNo**：
- 不需要客户端感知的Ability
- 服务器端计算、后台逻辑等
- 节省网络带宽

**ReplicateYes**：
- 需要客户端感知的Ability
- 需要客户端预测
- 需要客户端显示UI

### 对GA_Sprint的建议

**当前问题**：
- GA_Sprint需要客户端预测速度加成
- GA_Sprint需要客户端感知体力消耗

**建议**：
- 保持`ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes`
- 如果需要复制C++属性，手动添加到`GetLifetimeReplicatedProps`中

## 待解决的问题

1. 是否需要复制`SprintSpeedBonusAmount`？
2. 是否需要复制`bVigorExhaustedBroadcasted`？
3. 是否需要复制其他C++属性？

## 参考资料

- 源码位置：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\Abilities\GameplayAbilityTypes.h:96`
- 源码位置：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\Abilities\GameplayAbility.cpp:2293`
- 源码位置：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\AbilitySystemComponent_Abilities.cpp:1188`
