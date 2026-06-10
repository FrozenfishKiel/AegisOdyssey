# 自定义 GameplayEffectContext 完整实现指南

## 概述

在 GAS (Gameplay Ability System) 中，`FGameplayEffectContext` 是 GameplayEffect 执行时的上下文信息容器，包含了效果来源、目标、HitResult 等信息。然而，默认的 `FGameplayEffectContext` 往往无法满足游戏特定的需求，比如传递交易ID、背包索引等自定义数据。

本文档详细说明如何自定义 `GameplayEffectContext` 并实现网络序列化。

## 为什么需要自定义 EffectContext

### 默认 EffectContext 的限制

默认的 `FGameplayEffectContext` 只包含以下标准字段：

```cpp
struct FGameplayEffectContext
{
    TWeakObjectPtr<AActor> Instigator;           // 发起者
    TWeakObjectPtr<AActor> EffectCauser;          // 效果造成者
    TWeakObjectPtr<UObject> SourceObject;         // 源对象
    TWeakObjectPtr<UObject> AbilityCDO;           // Ability CDO
    TArray<TWeakObjectPtr<AActor>> Actors;        // 相关Actor
    TSharedPtr<FHitResult> HitResult;            // 击中结果
    FVector WorldOrigin;                          // 世界坐标原点
    bool bHasWorldOrigin;                         // 是否有世界原点
    // ...
};
```

### 实际应用场景

在交易系统中，我们需要传递以下额外信息：

- **TargetSaleID**: 交易的物品ID（用于查询价格表）
- **TargetBackPackIndex**: 背包中的索引位置（用于操作物品）

这些信息需要在网络间传输，并且在整个 GameplayEffect 流程中保持一致。

---

## 完整实现步骤

### 步骤 1: 创建自定义 EffectContext 结构体

**文件**: `ISAbilityTypes.h`

```cpp
#pragma once

#include "GameplayEffectTypes.h"
#include "ISAbilityTypes.generated.h"

USTRUCT(BlueprintType)
struct FISGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    // Getter 和 Setter 方法
    FName GetTargetSaleID() const { return TargetSaleID; }
    void SetTargetSaleID(const FName InNewID) { TargetSaleID = InNewID; }

    int32 GetTargetBackPackIndex() const { return TargetBackPackIndex; }
    void SetTargetBackPackIndex(const int32 InNewIndex) { TargetBackPackIndex = InNewIndex; }

    // 返回脚本结构体
    virtual UScriptStruct* GetScriptStruct() const override
    {
        return StaticStruct();
    }

    // 网络序列化方法
    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

    // 创建副本方法
    virtual FISGameplayEffectContext* Duplicate() const override;

protected:
    // 自定义属性
    UPROPERTY()
    FName TargetSaleID = FName();

    UPROPERTY()
    int32 TargetBackPackIndex = -1;
};
```

**关键点说明：**

1. **继承自 `FGameplayEffectContext`**: 必须继承才能获得所有父类功能
2. **`USTRUCT(BlueprintType)`**: 使结构体可以在蓝图中使用
3. **`GENERATED_BODY()`**: 生成必要的反射代码
4. **`UPROPERTY()`**: 标记需要序列化的成员变量
5. **虚函数重写**: `GetScriptStruct`、`NetSerialize`、`Duplicate` 必须重写

---

### 步骤 2: 实现 NetSerialize 方法

**文件**: `ISAbilityTypes.cpp`

```cpp
#include "ISAbilityTypes.h"

bool FISGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    uint32 RepBits = 0;

    // === 保存阶段：标记哪些字段需要序列化 ===
    if (Ar.IsSaving())
    {
        // 序列化父类字段（第0-6位）
        if (bReplicateInstigator && Instigator.IsValid())
        {
            RepBits |= 1 << 0;
        }
        if (bReplicateEffectCauser && EffectCauser.IsValid())
        {
            RepBits |= 1 << 1;
        }
        if (AbilityCDO.IsValid())
        {
            RepBits |= 1 << 2;
        }
        if (bReplicateSourceObject && SourceObject.IsValid())
        {
            RepBits |= 1 << 3;
        }
        if (Actors.Num() > 0)
        {
            RepBits |= 1 << 4;
        }
        if (HitResult.IsValid())
        {
            RepBits |= 1 << 5;
        }
        if (bHasWorldOrigin)
        {
            RepBits |= 1 << 6;
        }

        // 序列化自定义字段（第7-8位）
        if (TargetSaleID.IsValid())
        {
            RepBits |= 1 << 7;
        }
        if (TargetBackPackIndex != -1)
        {
            RepBits |= 1 << 8;
        }
    }

    // 序列化标记位（9位，因为用到了第8位）
    Ar.SerializeBits(&RepBits, 9);

    // === 序列化父类字段 ===
    if (RepBits & (1 << 0))
    {
        Ar << Instigator;
    }
    if (RepBits & (1 << 1))
    {
        Ar << EffectCauser;
    }
    if (RepBits & (1 << 2))
    {
        Ar << AbilityCDO;
    }
    if (RepBits & (1 << 3))
    {
        Ar << SourceObject;
    }
    if (RepBits & (1 << 4))
    {
        SafeNetSerializeTArray_Default<31>(Ar, Actors);
    }
    if (RepBits & (1 << 5))
    {
        if (Ar.IsLoading())
        {
            if (!HitResult.IsValid())
            {
                HitResult = TSharedPtr<FHitResult>(new FHitResult());
            }
        }
        HitResult->NetSerialize(Ar, Map, bOutSuccess);
    }
    if (RepBits & (1 << 6))
    {
        Ar << WorldOrigin;
        bHasWorldOrigin = true;
    }
    else
    {
        bHasWorldOrigin = false;
    }

    // === 序列化自定义字段 ===
    if (RepBits & (1 << 7))
    {
        Ar << TargetSaleID;
    }
    if (RepBits & (1 << 8))
    {
        Ar << TargetBackPackIndex;
    }

    // === 初始化 ASC 的逻辑 ===
    if (Ar.IsLoading())
    {
        AddInstigator(Instigator.Get(), EffectCauser.Get());
    }

    bOutSuccess = true;
    return true;
}
```

**关键技术点：**

#### 1. 位标记优化

使用位标记（Bit Flags）来指示哪些字段需要序列化，这样可以减少网络传输的数据量：

```cpp
uint32 RepBits = 0;

// 设置第7位
if (TargetSaleID.IsValid())
{
    RepBits |= 1 << 7;  // 等价于 RepBits = RepBits | 128
}

// 检查第7位
if (RepBits & (1 << 7))  // 等价于 RepBits & 128
{
    // 序列化 TargetSaleID
}
```

**位运算说明：**
- `1 << n`: 将1左移n位，得到第n位的掩码
- `|=`: 按位或运算，设置对应位为1
- `&`: 按位与运算，检查对应位是否为1

#### 2. SerializeBits 参数

```cpp
Ar.SerializeBits(&RepBits, 9);
```

**参数说明：**
- 第一个参数：位标记变量的指针
- 第二个参数：需要序列化的位数（不是最大索引+1）

**常见错误：**
```cpp
// 错误：只用了第8位，但传了8
Ar.SerializeBits(&RepBits, 8);  // 只能序列化第0-7位

// 正确：用了第8位，传9
Ar.SerializeBits(&RepBits, 9);  // 可以序列化第0-8位
```

#### 3. 加载时初始化 HitResult

```cpp
if (RepBits & (1 << 5))
{
    if (Ar.IsLoading())
    {
        if (!HitResult.IsValid())
        {
            HitResult = TSharedPtr<FHitResult>(new FHitResult());
        }
    }
    HitResult->NetSerialize(Ar, Map, bOutSuccess);
}
```

**原因：** `HitResult` 是 `TSharedPtr`，在加载时可能为空，需要先分配内存。

#### 4. AddInstigator 调用

```cpp
if (Ar.IsLoading())
{
    AddInstigator(Instigator.Get(), EffectCauser.Get());
}
```

**作用：** 在加载完成后，重新初始化 Instigator 和 EffectCauser 的关系。

---

### 步骤 3: 实现 Duplicate 方法

**文件**: `ISAbilityTypes.cpp`

```cpp
FISGameplayEffectContext* FISGameplayEffectContext::Duplicate() const
{
    FISGameplayEffectContext* NewContext = new FISGameplayEffectContext();
    *NewContext = *this;  // 拷贝所有字段

    // 深拷贝 HitResult
    if (GetHitResult())
    {
        NewContext->AddHitResult(*GetHitResult(), true);
    }
    return NewContext;
}
```

**关键点说明：**

1. **创建新实例**: 使用 `new` 创建新的 EffectContext
2. **赋值运算符**: 使用 `*NewContext = *this` 拷贝所有字段
3. **深拷贝 HitResult**: `HitResult` 是 `TSharedPtr`，需要深拷贝

**为什么需要深拷贝？**

```cpp
// 浅拷贝（错误）
TSharedPtr<FHitResult> HitResult1 = MakeShared<FHitResult>();
TSharedPtr<FHitResult> HitResult2 = HitResult1;  // 两个指针指向同一个对象

// 深拷贝（正确）
TSharedPtr<FHitResult> HitResult1 = MakeShared<FHitResult>();
TSharedPtr<FHitResult> HitResult2 = MakeShared<FHitResult>(*HitResult1);  // 创建新对象
```

---

### 步骤 4: 添加 TStructOpsTypeTraits 特性

**文件**: `ISAbilityTypes.h`（结构体定义之后）

```cpp
template<>
struct TStructOpsTypeTraits<FISGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FISGameplayEffectContext>
{
    enum
    {
        WithNetSerializer = true,  // 启用自定义网络序列化
        WithCopy = true            // 启用拷贝操作
    };
};
```

**关键点说明：**

#### 1. 模板特化

```cpp
template<>
struct TStructOpsTypeTraits<FISGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FISGameplayEffectContext>
```

**作用：** 为 `FISGameplayEffectContext` 类型特化 `TStructOpsTypeTraits` 模板。

#### 2. 特性枚举

```cpp
enum
{
    WithNetSerializer = true,  // 启用自定义网络序列化
    WithCopy = true            // 启用拷贝操作
};
```

**常用特性：**

| 特性 | 说明 | 用途 |
|------|------|------|
| `WithNetSerializer` | 启用自定义网络序列化 | 使用 `NetSerialize` 方法 |
| `WithCopy` | 启用拷贝操作 | 支持赋值运算符 |
| `WithIdentical` | 启用相等比较 | 支持 `==` 运算符 |
| `WithExportText` | 启用文本导出 | 支持序列化为文本 |

**为什么需要 `WithCopy = true`？**

因为 `HitResult` 是 `TSharedPtr`，需要启用拷贝操作才能正确复制。

---

### 步骤 5: 创建自定义 AbilitySystemGlobals 类

**文件**: `ISAbilitySystemGlobals.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ISAbilitySystemGlobals.generated.h"

UCLASS()
class ISLANDSURVIVAL_API UISAbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_BODY()

public:
    // 重写工厂方法，返回自定义的 EffectContext
    virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
```

**关键点说明：**

1. **继承自 `UAbilitySystemGlobals`**: 必须继承才能重写工厂方法
2. **`UCLASS()`**: 使类可以被反射系统识别
3. **`GENERATED_BODY()`**: 生成必要的反射代码
4. **虚函数重写**: `AllocGameplayEffectContext` 是工厂方法

---

### 步骤 6: 实现 AllocGameplayEffectContext 方法

**文件**: `ISAbilitySystemGlobals.cpp`

```cpp
#include "ISAbilitySystemGlobals.h"
#include "ISAbilityTypes.h"

FGameplayEffectContext* UISAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FISGameplayEffectContext();
}
```

**关键点说明：**

1. **包含头文件**: 必须包含 `ISAbilityTypes.h` 才能创建 `FISGameplayEffectContext`
2. **返回自定义类型**: 返回 `FISGameplayEffectContext*` 而不是 `FGameplayEffectContext*`
3. **工厂模式**: 这是工厂模式的典型应用

---

### 步骤 7: 在配置文件中注册

**文件**: `Config/DefaultGame.ini`

```ini
[/Script/GameplayAbilities.AbilitySystemGlobals]
+AbilitySystemGlobalsClassName="/Script/IslandSurvival.ISAbilitySystemGlobals"
```

**配置格式说明：**

```
/Script/{模块名}.{类名}
```

- **模块名**: 你的项目模块名（如 `IslandSurvival`）
- **类名**: 自定义的 `AbilitySystemGlobals` 类名

**如何找到模块名？**

查看项目中的 `.Build.cs` 文件，例如 `IslandSurvival.Build.cs`：

```cpp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "GameplayAbilities"  // GAS 模块
});
```

模块名就是文件名（去掉 `.Build.cs`），这里是 `IslandSurvival`。

---

## 工作原理

### 完整调用链

```
游戏启动
    ↓
读取 DefaultGame.ini
    ↓
发现 AbilitySystemGlobalsClassName 配置
    ↓
创建 UISAbilitySystemGlobals 实例
    ↓
UAbilitySystemGlobals::Get().AllocGameplayEffectContext() 被调用
    ↓
返回 FISGameplayEffectContext* (而不是默认的 FGameplayEffectContext*)
    ↓
所有 ASC 创建的 EffectContext 都是自定义类型
    ↓
可以安全地 static_cast 为 FISGameplayEffectContext* 使用
```

### MakeEffectContext 流程

```cpp
// 在 AbilitySystemComponent 中
FGameplayEffectContextHandle UAbilitySystemComponent::MakeEffectContext()
{
    // 调用全局的 AllocGameplayEffectContext
    FGameplayEffectContext* Context = UAbilitySystemGlobals::Get().AllocGameplayEffectContext();

    // 初始化上下文
    Context->AddInstigator(GetOwner(), GetOwner());

    return FGameplayEffectContextHandle(Context);
}
```

**关键点：**
- `UAbilitySystemGlobals::Get()` 返回的是我们配置的 `UISAbilitySystemGlobals`
- `AllocGameplayEffectContext()` 返回的是 `FISGameplayEffectContext*`

---

## 文件结构总结

```
项目根目录/
├── Source/
│   └── IslandSurvival/
│       ├── Public/
│       │   ├── ISAbilityTypes.h          (步骤1 + 步骤4)
│       │   └── ISAbilitySystemGlobals.h  (步骤5)
│       └── Private/
│           ├── ISAbilityTypes.cpp        (步骤2 + 步骤3)
│           └── ISAbilitySystemGlobals.cpp (步骤6)
└── Config/
    └── DefaultGame.ini                   (步骤7)
```

---

## 验证清单

完成以上步骤后，检查以下内容：

- [ ] 自定义 EffectContext 结构体继承自 `FGameplayEffectContext`
- [ ] 实现了 `NetSerialize` 方法
- [ ] 实现了 `Duplicate` 方法
- [ ] 添加了 `TStructOpsTypeTraits` 特性
- [ ] 创建了自定义 `AbilitySystemGlobals` 类
- [ ] 实现了 `AllocGameplayEffectContext` 方法
- [ ] 在 `DefaultGame.ini` 中配置了类路径
- [ ] 重启了编辑器（修改配置后必须重启）

---

## 常见问题

### Q1: 为什么需要配置 AbilitySystemGlobals？

**A:** 如果不配置，`MakeEffectContext()` 会创建默认的 `FGameplayEffectContext`，而不是你的自定义类型。这样在 `static_cast` 时会导致崩溃或未定义行为。

### Q2: SerializeBits 的参数为什么是 9 而不是 8？

**A:** 参数表示需要序列化的位数，不是最大索引。如果你用到了第8位（索引8），那么需要序列化9位（索引0-8）。

### Q3: 为什么 HitResult 需要深拷贝？

**A:** `HitResult` 是 `TSharedPtr`，浅拷贝会导致多个 EffectContext 共享同一个 HitResult 对象，修改一个会影响其他。

### Q4: UPROPERTY() 宏是必须的吗？

**A:** 对于网络序列化，`UPROPERTY()` 是必须的，否则变量不会被序列化。但 `FName` 和 `int32` 等基本类型可以不加。

### Q5: 为什么需要重写 GetScriptStruct？

**A:** `GetScriptStruct()` 返回结构体的 `UScriptStruct` 指针，用于反射系统。如果不重写，会返回父类的 `UScriptStruct`，导致反射错误。

---

## 注意事项

1. **重启编辑器**: 修改 `DefaultGame.ini` 后必须重启编辑器才能生效
2. **位标记管理**: 添加新字段时，注意分配新的位，不要与现有字段冲突
3. **SerializeBits 参数**: 确保参数大于等于使用的最大位数+1
4. **深拷贝**: 对于指针类型（如 `TSharedPtr`），必须实现深拷贝
5. **类型转换**: 使用 `static_cast` 而不是 `dynamic_cast`，因为类型是确定的
6. **网络同步**: 自定义属性会自动在网络间同步，无需额外处理

---

## 实际应用示例

完成上述实现步骤后，就可以在实际项目中使用自定义的 EffectContext 了。以下是一个完整的交易系统应用示例，展示了从设置到应用的完整流程。

### 应用场景：交易系统

在交易系统中，我们需要：
- 传递交易的物品ID（用于查询价格表）
- 传递背包索引（用于操作物品）
- 在网络间同步这些信息

### 阶段 1: 设置自定义属性

在交易开始时，创建 EffectContext 并设置自定义属性：

```cpp
// 文件: ISTradingSystemComponent.cpp
#include "ISAbilityTypes.h"

void UISTradingSystemComponent::TradBegin_Implementation(
    AActor* TargetActor, 
    const FName InTargetID, 
    const int32 TargetIndex)
{
    // 获取相关组件
    AISCharacter* TargetCharacter = IISPlayerInterface::Execute_GetSourceCharacter(GetOwner());
    if (!TargetCharacter) return;

    AISPlayerState* PlayerState = IISPlayerInterface::Execute_GetSourcePlayerState(GetOwner());
    if (!PlayerState) return;

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
    if (!SourceASC) return;

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC) return;

    // === 关键步骤：创建 EffectContext ===
    FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();

    // === 转换为自定义类型并设置属性 ===
    FISGameplayEffectContext* ISEffectContext = static_cast<FISGameplayEffectContext*>(ContextHandle.Get());
    if (!ISEffectContext) return;

    // 设置自定义属性
    ISEffectContext->SetTargetSaleID(InTargetID);          // 交易的物品ID
    ISEffectContext->SetTargetBackPackIndex(TargetIndex);   // 背包索引

    // 设置效果来源
    ContextHandle.AddSourceObject(TargetActor);

    // 创建并应用 GameplayEffect
    FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(
        NPCTradInComingCoinEffect,  // GameplayEffect 类
        1.f,                         // 等级
        ContextHandle                // 包含自定义属性的上下文
    );

    const FActiveGameplayEffectHandle ActivateEffectHandle = 
        SourceASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}
```

**关键点：**
1. 使用 `MakeEffectContext()` 创建上下文（会返回自定义类型）
2. 使用 `static_cast` 转换为自定义类型
3. 调用 Setter 方法设置自定义属性
4. 将 Context 传递给 `MakeOutgoingSpec`

---

### 阶段 2: 在 Execution Calculation 中使用

在自定义的 Execution Calculation 中读取自定义属性：

```cpp
// 文件: ExecCalc_InComingCoins.cpp
#include "ISAbilityTypes.h"

void UExecCalc_InComingCoins::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

    // 获取 ASC 和 Actor
    const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

    AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
    AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

    // === 获取 EffectSpec 和 Context ===
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

    // === 转换为自定义类型并读取属性 ===
    FISGameplayEffectContext* SourceEffectContext = 
        static_cast<FISGameplayEffectContext*>(EffectContextHandle.Get());

    // 获取标签和参数
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    FAggregatorEvaluateParameters EvaluateParams;
    EvaluateParams.SourceTags = SourceTags;
    EvaluateParams.TargetTags = TargetTags;

    // === 使用自定义属性查询价格表 ===
    const UISTraderSpecialData* TraderSpecialData = 
        UISAbilitysystemLibary::GetTraderSpecialData(SourceAvatar);
    if (!TraderSpecialData) return;

    const FTraderSalesData TraderSalesData = 
        TraderSpecialData->GetTraderSalesData(
            IISNPCInterface::Execute_GetCharacterName(SourceAvatar)
        );
    const UCurveTable* SalesCT = TraderSalesData.TradeSalesCurveTable;
    if (!SalesCT) return;

    // 使用自定义属性从价格表中查询价格
    const FRealCurve* SalseRealCT = SalesCT->FindCurve(
        SourceEffectContext->GetTargetSaleID(),  // 使用自定义属性
        FString()
    );
    if (!SalseRealCT) return;

    // 根据好感度计算价格
    const float ResultValue = SalseRealCT->Eval(
        IISNPCInterface::Execute_GetFavorability(SourceAvatar)
    );

    // 输出计算结果
    const FGameplayModifierEvaluatedData EvaluatedData(
        UISAttributeSet::GetInComingCoinsAttribute(),
        EGameplayModOp::Override,
        ResultValue
    );
    OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
```

**关键点：**
1. 从 `ExecutionParams` 获取 `GameplayEffectSpec`
2. 从 `Spec` 获取 `EffectContext`
3. 使用 `static_cast` 转换为自定义类型
4. 调用 Getter 方法读取自定义属性
5. 使用自定义属性查询数据表或执行逻辑

---

### 阶段 3: 在 AttributeSet 中处理属性变化

在 AttributeSet 的 `PostGameplayEffectExecute` 中使用自定义属性：

```cpp
// 文件: ISAttributeSet.cpp
#include "ISAbilityTypes.h"

void UISAttributeSet::PostGameplayEffectExecute(
    const struct FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // 设置上下文属性
    FEffectProperties Properties;
    SetEffectContext(Data, Properties);

    // === 处理金币扣除（购买） ===
    if (Data.EvaluatedData.Attribute == GetInComingCoinsAttribute())
    {
        const float LocalValue = GetInComingCoins();
        const float NewCoin = GetCoins() - LocalValue;

        if (NewCoin > 0)  // 金币足够
        {
            SetCoins(FMath::Clamp(NewCoin, 0.f, 99999));

            // === 获取自定义 EffectContext ===
            FISGameplayEffectContext* SourceEffectContext = 
                static_cast<FISGameplayEffectContext*>(Properties.EffectContextHandle.Get());

            // 获取商人的交易背包
            UISTradingSystemComponent* SourceTradBackPack = 
                Properties.SourceCharacter->GetComponentByClass<UISTradingSystemComponent>();
            if (!SourceTradBackPack) return;

            // === 使用背包索引操作物品 ===
            if (FItemInformation* TargetItem = 
                &SourceTradBackPack->InventoryContainer[SourceEffectContext->GetTargetBackPackIndex()])
            {
                // 玩家背包添加物品
                UISItemsContainer* PlayerBackPack = 
                    IISPlayerInterface::Execute_GetSourceCharacter(Properties.TargetAvatarActor)
                        ->GetComponentByClass<UISItemsContainer>();
                if (!PlayerBackPack) return;

                PlayerBackPack->ToPickUpItemsInBackPack(*TargetItem, 1);

                // 商人背包移除物品
                SourceTradBackPack->DiscardItem(
                    SourceEffectContext->GetTargetBackPackIndex(),  // 使用自定义属性
                    1
                );
            }
        }
    }

    // === 处理金币增加（出售） ===
    if (Data.EvaluatedData.Attribute == GetInRecoverCoinsAttribute())
    {
        const float LocalValue = GetInRecoverCoins();
        const float NewCoin = GetCoins() + LocalValue;

        // === 获取自定义 EffectContext ===
        FISGameplayEffectContext* SourceEffectContext = 
            static_cast<FISGameplayEffectContext*>(Properties.EffectContextHandle.Get());

        // 获取相关组件
        UISTradingSystemComponent* SourceTradBackPack = 
            Properties.SourceCharacter->GetComponentByClass<UISTradingSystemComponent>();
        if (!SourceTradBackPack) return;

        UISCharacterInventory* TargetTradBackPack = 
            Properties.TargetCharacter->GetComponentByClass<UISCharacterInventory>();
        if (!TargetTradBackPack) return;

        UISTradingSystemComponent* TargetTradBack = 
            Properties.TargetCharacter->GetComponentByClass<UISTradingSystemComponent>();
        if (!TargetTradBack) return;

        // === 使用背包索引操作物品 ===
        if (FItemInformation* TargetItem = 
            &TargetTradBackPack->InventoryContainer[SourceEffectContext->GetTargetBackPackIndex()])
        {
            UISItemsContainer* PlayerBackPack = 
                IISPlayerInterface::Execute_GetSourceCharacter(Properties.TargetAvatarActor)
                    ->GetComponentByClass<UISItemsContainer>();
            if (!PlayerBackPack) return;

            // 更新金币
            SetCoins(FMath::Clamp(NewCoin, 0.f, 99999));

            // 触发交易成功事件
            TargetTradBack->SetTradTarget(*TargetItem, LocalValue);

            // 玩家背包移除物品
            PlayerBackPack->DiscardItem(
                SourceEffectContext->GetTargetBackPackIndex(),  // 使用自定义属性
                1
            );
        }
    }
}
```

**关键点：**
1. 从 `Data.EffectSpec.GetContext()` 获取 Context
2. 使用 `static_cast` 转换为自定义类型
3. 调用 Getter 方法读取自定义属性
4. 使用自定义属性操作游戏逻辑（如背包系统）

---

### 完整数据流转图

```
玩家点击购买
    ↓
TradBegin_Implementation()
    ↓
创建 FGameplayEffectContextHandle
    ↓
static_cast<FISGameplayEffectContext*>
    ↓
SetTargetSaleID("物品ID")
SetTargetBackPackIndex(5)
    ↓
ApplyGameplayEffectSpecToSelf()
    ↓
[网络传输] → 自定义属性自动序列化
    ↓
UExecCalc_InComingCoins::Execute_Implementation()
    ↓
GetTargetSaleID() → 查询价格表
GetTargetBackPackIndex() → 5
    ↓
计算: 价格 = 曲线表[物品ID].Eval(好感度)
    ↓
输出: InComingCoins = 价格
    ↓
PostGameplayEffectExecute()
    ↓
GetTargetBackPackIndex() → 5
    ↓
操作: InventoryContainer[5] → 添加/移除物品
    ↓
交易完成
```

---

### 类型转换模式总结

在整个流程中，类型转换的模式是一致的：

```cpp
// 1. 创建 Context
FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

// 2. 转换为自定义类型
FISGameplayEffectContext* CustomContext = 
    static_cast<FISGameplayEffectContext*>(ContextHandle.Get());

// 3. 设置自定义属性
CustomContext->SetTargetSaleID(InTargetID);

// 4. 传递给 GameplayEffect
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, ContextHandle);

// 5. 在 Execution Calculation 中读取
const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
FISGameplayEffectContext* CustomContext = 
    static_cast<FISGameplayEffectContext*>(EffectContextHandle.Get());

// 6. 使用自定义属性
FName ID = CustomContext->GetTargetSaleID();
```

---

### 网络同步说明

自定义属性会自动在网络间同步，无需额外处理：

- **服务器端**: 设置自定义属性 → `NetSerialize` 序列化 → 发送到客户端
- **客户端端**: 接收数据 → `NetSerialize` 反序列化 → 恢复自定义属性

**验证网络同步：**

```cpp
// 在客户端的 Execution Calculation 中
FISGameplayEffectContext* Context = 
    static_cast<FISGameplayEffectContext*>(EffectContextHandle.Get());

// 这些值应该与服务器端设置的值一致
FName ID = Context->GetTargetSaleID();
int32 Index = Context->GetTargetBackPackIndex();
```

---

### 最佳实践

1. **始终使用 static_cast**: 因为类型是确定的（通过配置确保）
2. **添加空指针检查**: 转换后检查指针是否有效
3. **使用 Getter/Setter**: 不要直接访问成员变量
4. **文档化自定义属性**: 在注释中说明每个属性的用途
5. **单元测试**: 测试网络同步是否正常工作

---

## 扩展阅读

- [Unreal Engine 官方文档 - Gameplay Ability System](https://docs.unrealengine.com/5.6/en-US/gameplay-ability-system-in-unreal-engine/)
- [FGameplayEffectContext 源码](d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayEffectTypes.h)
- [UAbilitySystemGlobals 源码](d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\AbilitySystemGlobals.h)

---

## 总结

自定义 `GameplayEffectContext` 是扩展 GAS 功能的重要手段，通过以下步骤可以实现：

1. 创建自定义结构体，继承自 `FGameplayEffectContext`
2. 实现 `NetSerialize` 方法，处理网络序列化
3. 实现 `Duplicate` 方法，处理深拷贝
4. 添加 `TStructOpsTypeTraits` 特性
5. 创建自定义 `AbilitySystemGlobals` 类
6. 实现 `AllocGameplayEffectContext` 工厂方法
7. 在配置文件中注册自定义类

完成这些步骤后，就可以在 GAS 中传递自定义数据，并且这些数据会自动在网络间同步。实际应用中，通过三个阶段使用自定义 EffectContext：

1. **设置阶段**: 创建 Context 并设置自定义属性
2. **计算阶段**: 在 Execution Calculation 中读取自定义属性
3. **应用阶段**: 在 AttributeSet 中使用自定义属性处理游戏逻辑

整个过程通过 `static_cast` 进行类型转换，确保类型安全，并且网络同步自动完成，无需额外处理。
