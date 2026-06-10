# GAS属性回调问题：Health回调正常但Vigor回调失败

## 问题描述

在客户端上，Health和MaxHealth的属性变化回调能够正常触发，但Vigor和MaxVigor的属性变化回调无法触发。单机和服务器环境下所有属性回调都正常。

## 问题现象

### 正常情况（Health和MaxHealth）
- 单机环境：回调正常 ✅
- 服务器环境：回调正常 ✅
- 客户端环境：回调正常 ✅

### 异常情况（Vigor和MaxVigor）
- 单机环境：回调正常 ✅
- 服务器环境：回调正常 ✅
- 客户端环境：回调失败 ❌

## 代码分析

### 1. MVVM_HUD的回调注册

**文件位置**：`d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\UI\ViewModel\MVVM_HUD.cpp:58-78`

```cpp
// Health和MaxHealth的回调注册
SourceASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetMaxHealthAttribute()).AddLambda([this]
    (const FOnAttributeChangeData& Data)
{
    SetMaxHealth(Data.NewValue);
});

SourceASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute()).AddLambda([this]
    (const FOnAttributeChangeData& Data)
{
    SetHealth(Data.NewValue);
});

// Vigor和MaxVigor的回调注册
SourceASC->GetGameplayAttributeValueChangeDelegate(CombatAttributeSet->GetVigorAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
{
    SetVigor(Data.NewValue);
});

SourceASC->GetGameplayAttributeValueChangeDelegate(CombatAttributeSet->GetMaxVigorAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
{
    SetMaxVigor(Data.NewValue);
});
```

**关键点**：
- Health和MaxHealth通过`GetGameplayAttributeValueChangeDelegate`注册回调
- Vigor和MaxVigor也通过`GetGameplayAttributeValueChangeDelegate`注册回调
- 两种属性的注册方式完全相同

### 2. Health和MaxHealth的OnRep函数

**文件位置**：`d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOHealthAttributeSet.cpp:192-205`

```cpp
void UAOHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, Health, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes

    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
    
    OnHealthChange.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);  // ← 关键：广播自定义委托

    if (!bOutOfHealth && CurrentHealth <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
    }

    bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UAOHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, MaxHealth, OldValue);
    
    OnMaxHealthChange.Broadcast(nullptr,nullptr,nullptr,GetMaxHealth() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue(),GetMaxHealth());  // ← 关键：广播自定义委托
}
```

**关键点**：
- `OnRep_Health`中调用了`OnHealthChange.Broadcast`
- `OnRep_MaxHealth`中调用了`OnMaxHealthChange.Broadcast`
- `OnHealthChange`和`OnMaxHealthChange`是**自定义委托**

### 3. Vigor和MaxVigor的OnRep函数

**文件位置**：`d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOCombatAttributeSet.cpp:101-108`

```cpp
void UAOCombatAttributeSet::OnRep_Vigor()
{
    UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_Vigor: Vigor replicated to %.2f"), GetVigor());
    // ← 没有调用任何委托！
}

void UAOCombatAttributeSet::OnRep_MaxVigor()
{
    // ← 空的！
}
```

**关键点**：
- `OnRep_Vigor`中**没有调用**任何委托
- `OnRep_MaxVigor`是**空的**

## 引擎源码分析

### 1. GAMEPLAYATTRIBUTE_REPNOTIFY宏的定义

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\AttributeSet.h:404-408`

```cpp
#define GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, PropertyName, OldValue) \
{ \
    static FProperty* ThisProperty = FindFieldChecked<FProperty>(ClassName::StaticClass(), GET_MEMBER_NAME_CHECKED(ClassName, PropertyName)); \
    GetOwningAbilitySystemComponentChecked()->SetBaseAttributeValueFromReplication(FGameplayAttribute(ThisProperty), PropertyName, OldValue); \
}
```

**关键点**：
- `GAMEPLAYATTRIBUTE_REPNOTIFY`宏会调用`SetBaseAttributeValueFromReplication`
- `SetBaseAttributeValueFromReplication`会更新聚合器，但**不会广播**`AttributeValueChangeDelegates`

### 2. SetBaseAttributeValueFromReplication的实现

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\GameplayEffect.cpp:3568-3616`

```cpp
void FActiveGameplayEffectsContainer::SetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue, const FGameplayAttributeData& OldValue)
{
    FAggregatorRef* RefPtr = AttributeAggregatorMap.Find(Attribute);
    if (RefPtr && RefPtr->Get())  // ← 关键：如果有聚合器
    {
        FAggregator* Aggregator = RefPtr->Get();
        if (FGameplayAttribute::IsGameplayAttributeDataProperty(Attribute.GetUProperty()))
        {
            const float ServerBaseValue = NewValue.GetBaseValue();
            const float OldBaseValue = OldValue.GetBaseValue();
            
            // Reset to the server's old value
            constexpr bool bDoNotExecuteCallbacksValue = false;
            Aggregator->SetBaseValue(OldBaseValue, bDoNotExecuteCallbacksValue);

            // Evaluate what the old value would have resulted in...  We do this to ensure the correct "old value" for the callbacks.
            FAggregatorEvaluateParameters EvaluationParameters;
            EvaluationParameters.IncludePredictiveMods = true;
            float OldEvaluatedValue = Aggregator->Evaluate(EvaluationParameters);
            Owner->SetNumericAttribute_Internal(Attribute, OldEvaluatedValue);

            // Now set the new value and go through all of the aggregations...
            Aggregator->SetBaseValue(ServerBaseValue, bDoNotExecuteCallbacksValue);
            UE_LOG(LogGameplayEffects, Log, TEXT("SetBaseAttributeValueFromReplication [%s]: %s rewound to state NewBaseValue: %.2f  OldCurrentValue: %.2f"), OwnerIsNetAuthority ? TEXT("Authority") : TEXT("Client"), *Attribute.AttributeName, ServerBaseValue, OldEvaluatedValue);
        }

        FScopedAggregatorOnDirtyBatch::GlobalFromNetworkUpdate = true;
        OnAttributeAggregatorDirty(Aggregator, Attribute);  // ← 调用这个，但不会广播回调
        FScopedAggregatorOnDirtyBatch::GlobalFromNetworkUpdate = false;
    }
    else  // ← 关键：如果没有聚合器
    {
        // No aggregators on the client but still broadcast the dirty delegate
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        if (FOnGameplayAttributeChange* LegacyDelegate = AttributeChangeDelegates.Find(Attribute))
        {
            LegacyDelegate->Broadcast(NewValue.GetCurrentValue(), nullptr);
        }
        PRAGMA_ENABLE_DEPRECATION_WARNINGS

        if (FOnGameplayAttributeValueChange* Delegate = AttributeValueChangeDelegates.Find(Attribute))
        {
            FOnAttributeChangeData CallbackData;
            CallbackData.Attribute = Attribute;
            CallbackData.NewValue = NewValue.GetCurrentValue();
            CallbackData.OldValue = OldValue.GetCurrentValue();
            CallbackData.GEModData = nullptr;

            Delegate->Broadcast(CallbackData);  // ← 这里会广播回调
        }
    }
}
```

**关键点**：
- 如果**有聚合器**，会调用`OnAttributeAggregatorDirty`，但**不会广播**`AttributeValueChangeDelegates`
- 如果**没有聚合器**，会直接广播`AttributeValueChangeDelegates`

### 3. OnAttributeAggregatorDirty的实现

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\GameplayEffect.cpp:3307-3360`

```cpp
void FActiveGameplayEffectsContainer::OnAttributeAggregatorDirty(FAggregator* Aggregator, FGameplayAttribute Attribute, bool bFromRecursiveCall)
{
    check(AttributeAggregatorMap.FindChecked(Attribute).Get() == Aggregator);

    // Our Aggregator has changed, we need to reevaluate this aggregator and update the current value of the attribute.
    // Note that this is not an execution, so there are no 'source' and 'target' tags to fill out in the FAggregatorEvaluateParameters.
    // ActiveGameplayEffects that have required owned tags will be turned on/off via delegates, and will add/remove themselves from attribute
    // aggregators when that happens.
    
    FAggregatorEvaluateParameters EvaluationParameters;

    if (Owner->IsNetSimulating())
    {
        if (FScopedAggregatorOnDirtyBatch::GlobalFromNetworkUpdate && Aggregator->NetUpdateID != FScopedAggregatorOnDirtyBatch::NetUpdateID)
        {
            // We are a client. The current value of this attribute is the replicated server's "final" value. We dont actually know what the 
            // server's base value is. But we can calculate it with ReverseEvaluate(). Then, we can call Evaluate with IncludePredictiveMods=true
            // to apply our mods and get an accurate predicted value.
            // 
            // It is very important that we only do this exactly one time when we get a new value from the server. Once we set the new local value for this
            // attribute below, recalculating the base would give us the wrong server value. We should only do this when we are coming directly from a network update.
            // 
            // Unfortunately there are two ways we could get here from a network update: from the ActiveGameplayEffect container being updated or from a traditional
            // OnRep on the actual attribute uproperty. Both of these could happen in a single network update, or potentially only one could happen
            // (and in fact it could be either one! the AGE container could change in a way that doesnt change the final attribute value, or we could have the base value
            // of the attribute actually be modified (e.g,. losing health or mana which only results in an OnRep and not in a AGE being applied).
            // 
            // So both paths need to lead to this function, but we should only do it one time per update. Once we update the base value, we need to make sure we dont do it again
            // until we get a new network update. GlobalFromNetworkUpdate and NetUpdateID are what do this.
            // 
            // GlobalFromNetworkUpdate - only set to true when we are coming from an OnRep or when we are coming from an ActiveGameplayEffect container net update.
            // NetUpdateID - updated once whenever an AttributeSet is received over the network. It will be incremented one time per actor that gets an update.
            //
            // See UAttributeSet::PostNetReceive().

            if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Attribute.GetUProperty()))
            {
                // Legacy float attribute case requires the base value to be deduced from the final value, as it is not replicated
                const float FinalValue = Owner->GetNumericAttribute(Attribute);
                const float BaseValue = Aggregator->ReverseEvaluate(FinalValue, EvaluationParameters);
                UE_LOG(LogGameplayEffects, Log, TEXT("Reverse Evaluated %s. FinalValue: %.2f  BaseValue: %2.f.  Setting BaseValue.  (Role: %s)"), *Attribute.GetName(), FinalValue, BaseValue, *UEnum::GetValueAsString(Owner->GetOwnerRole()));

                Aggregator->SetBaseValue(BaseValue, false);
            }

            Aggregator->NetUpdateID = FScopedAggregatorOnDirtyBatch::NetUpdateID;
        }

        EvaluationParameters.IncludePredictiveMods = true;
    }

    const float NewValue = Aggregator->Evaluate(EvaluationParameters);
    if (EvaluationParameters.IncludePredictiveMods)
    {
        const float OldValue = Owner->GetNumericAttribute(Attribute);
        UE_LOG(LogGameplayEffects, Log, TEXT("[%s] Aggregator Evaluated %s. OldValue: %.2f  NewValue: %.2f"), *UEnum::GetValueAsString(Owner->GetOwnerRole()), *Attribute.GetName(), OldValue, NewValue);
    }

    InternalUpdateNumericalAttribute(Attribute, NewValue, nullptr, bFromRecursiveCall);  // ← 关键：调用这个
}
```

**关键点**：
- `OnAttributeAggregatorDirty`会调用`InternalUpdateNumericalAttribute`
- `InternalUpdateNumericalAttribute`会更新属性值

### 4. InternalUpdateNumericalAttribute的实现

**文件位置**：`d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\GameplayEffect.cpp:3250-3270`

```cpp
void FActiveGameplayEffectsContainer::InternalUpdateNumericalAttribute(const FGameplayAttribute& Attribute, float NewValue, const FGameplayEffectModCallbackData* GEModData, bool bFromReplication)
{
    // ...
    if (bFromReplication)
    {
        // We are coming from a replication event. Do not call PostAttributeChange.
        // PostAttributeChange will be called by the OnRep on the attribute itself.
        // ← 关键：不调用PostAttributeChange
    }
    else
    {
        Owner->PostAttributeChange(Attribute, OldValue, NewValue);  // ← 调用PostAttributeChange
    }
}
```

**关键点**：
- 如果`bFromReplication`为`true`，**不会调用**`PostAttributeChange`
- `PostAttributeChange`会触发回调

## 根本原因

### Health和MaxHealth的执行流程

```
服务器修改属性 
  ↓
客户端接收复制 
  ↓
OnRep_Health 
  ↓
GAMEPLAYATTRIBUTE_REPNOTIFY 
  ↓
SetBaseAttributeValueFromReplication 
  ↓
有聚合器 
  ↓
OnAttributeAggregatorDirty 
  ↓
不广播回调 ❌
  ↓
但是！OnRep_Health中调用了OnHealthChange.Broadcast 
  ↓
自定义委托被触发 ✅
```

**关键点**：
- `GetGameplayAttributeValueChangeDelegate`的回调**不会被触发**
- 但是`OnHealthChange`自定义委托**会被触发**

### Vigor和MaxVigor的执行流程

```
服务器修改属性 
  ↓
客户端接收复制 
  ↓
OnRep_Vigor 
  ↓
GAMEPLAYATTRIBUTE_REPNOTIFY 
  ↓
SetBaseAttributeValueFromReplication 
  ↓
有聚合器 
  ↓
OnAttributeAggregatorDirty 
  ↓
不广播回调 ❌
  ↓
而且！OnRep_Vigor中没有调用任何委托 
  ↓
没有委托被触发 ❌
```

**关键点**：
- `GetGameplayAttributeValueChangeDelegate`的回调**不会被触发**
- `OnRep_Vigor`中**没有调用**任何委托
- **没有委托被触发**

## 解决方案

### 方案1：在AOCombatAttributeSet的OnRep函数中手动触发回调（推荐）

**文件位置**：`d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOCombatAttributeSet.cpp`

```cpp
void UAOCombatAttributeSet::OnRep_Vigor()
{
    UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_Vigor: Vigor replicated to %.2f"), GetVigor());
    
    // 手动触发回调
    if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(GetVigorAttribute()).Broadcast(FOnAttributeChangeData(GetVigorAttribute(), GetVigor(), GetVigor(), nullptr));
    }
}

void UAOCombatAttributeSet::OnRep_MaxVigor()
{
    UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_MaxVigor: MaxVigor replicated to %.2f"), GetMaxVigor());
    
    // 手动触发回调
    if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(GetMaxVigorAttribute()).Broadcast(FOnAttributeChangeData(GetMaxVigorAttribute(), GetMaxVigor(), GetMaxVigor(), nullptr));
    }
}
```

**说明**：
- 在`OnRep`函数中手动广播`GetGameplayAttributeValueChangeDelegate`
- 这会触发MVVM_HUD中的回调
- 和Health、MaxHealth的实现方式一致

### 方案2：在MVVM_HUD的OnRep函数中手动触发UI更新

**文件位置**：`d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\UI\ViewModel\MVVM_HUD.cpp`

```cpp
void UMVVM_HUD::OnRep_Vigor()
{
    SetVigor(Vigor);  // ← 手动触发UI更新
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
    UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获取Vigor"));
}

void UMVVM_HUD::OnRep_MaxVigor()
{
    SetMaxVigor(MaxVigor);  // ← 手动触发UI更新
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
    UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获取MaxVigor"));
}
```

**说明**：
- 在`OnRep`函数中手动调用`SetVigor`和`SetMaxVigor`
- 这会触发UI更新

## 技术概念

### 聚合器（Aggregator）

在Unreal Engine的Gameplay Ability System（GAS）中，**聚合器（FAggregator）**是一个用于计算属性最终值的系统。

#### 聚合器的工作原理

```
基础值（Base Value） + 修饰符（Modifiers） = 最终值（Final Value）
```

#### 示例

```
Vigor的基础值：100
修饰符：+10 (Buff), -5 (Debuff), -12 (Sprint Cost)
最终值：100 + 10 - 5 - 12 = 93
```

#### 聚合器的作用

| 作用 | 说明 |
|------|------|
| **管理修饰符** | 管理所有影响该属性的GameplayEffect修饰符 |
| **计算最终值** | 将基础值和所有修饰符聚合起来，计算出最终值 |
| **触发更新** | 当修饰符变化时，重新计算最终值并更新属性 |

### 为什么有聚合器就不广播回调？

#### 引擎设计原因

1. **性能优化**：避免重复广播回调
   - 聚合器会自动更新属性值
   - 如果每次聚合器更新都广播回调，会导致性能问题

2. **避免重复更新**：防止UI重复更新
   - 聚合器更新时，属性值可能不会改变
   - 如果每次都广播回调，会导致UI重复更新

3. **客户端预测**：支持客户端预测机制
   - 客户端预测时，聚合器会频繁更新
   - 如果每次都广播回调，会导致UI频繁闪烁

## 总结

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| **为什么Health和MaxHealth的回调会被触发？** | Health和MaxHealth的OnRep函数中调用了自定义委托OnHealthChange和OnMaxHealthChange | 在Vigor和MaxVigor的OnRep函数中手动触发回调 |
| **为什么Vigor和MaxVigor的回调不会被触发？** | Vigor和MaxVigor的OnRep函数中没有调用任何委托，且GetGameplayAttributeValueChangeDelegate的回调在客户端复制时不会被触发 | 在Vigor和MaxVigor的OnRep函数中手动触发回调 |
| **为什么有聚合器就不广播回调？** | 引擎设计：有聚合器的属性通过聚合器管理，不会直接广播回调，避免性能问题和重复更新 | 在OnRep函数中手动触发回调 |

## 推荐方案

**推荐使用方案1**，因为：
1. 和Health、MaxHealth的实现方式一致
2. 代码更加统一和规范
3. 便于维护和理解

## 相关文件

### 项目文件
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOHealthAttributeSet.h`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOHealthAttributeSet.cpp`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOCombatAttributeSet.h`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\AbilitySystem\Attributes\AOCombatAttributeSet.cpp`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\UI\ViewModel\MVVM_HUD.h`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\UI\ViewModel\MVVM_HUD.cpp`

### 引擎源码文件
- `d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\AttributeSet.h`
- `d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\GameplayEffect.cpp`
- `d:\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Private\AttributeSet.cpp`

## OnRep函数的参数规则详解

### 1. 参数类型必须与属性类型匹配

**规则**：OnRep函数的参数类型必须与被复制的属性类型匹配。

#### 示例1：FGameplayAttributeData类型的属性

```cpp
// 属性定义
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AO|Health", Meta = (AllowPrivateAccess = true))
FGameplayAttributeData Health;

// OnRep函数
void OnRep_Health(const FGameplayAttributeData& OldValue);  // ← 参数类型匹配 ✅
```

**关键点**：
- 属性类型是`FGameplayAttributeData`
- OnRep函数的参数类型是`const FGameplayAttributeData&`
- 类型匹配 ✅

#### 示例2：float类型的属性

```cpp
// 属性定义
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AO|Health", Meta = (AllowPrivateAccess = true))
float Health;

// OnRep函数
void OnRep_Health(float OldValue);  // ← 参数类型匹配 ✅
```

**关键点**：
- 属性类型是`float`
- OnRep函数的参数类型是`float`
- 类型匹配 ✅

#### 示例3：int32类型的属性

```cpp
// 属性定义
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AO|Health", Meta = (AllowPrivateAccess = true))
int32 Health;

// OnRep函数
void OnRep_Health(int32 OldValue);  // ← 参数类型匹配 ✅
```

**关键点**：
- 属性类型是`int32`
- OnRep函数的参数类型是`int32`
- 类型匹配 ✅

### 2. 参数名可以是任意的

**规则**：参数名可以是任意的，不一定是`OldValue`。

#### 示例1：使用OldValue

```cpp
void OnRep_Health(const FGameplayAttributeData& OldValue)
{
    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
}
```

**关键点**：
- 参数名是`OldValue`
- 语义清晰，表示旧值

#### 示例2：使用PreviousValue

```cpp
void OnRep_Health(const FGameplayAttributeData& PreviousValue)
{
    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - PreviousValue.GetCurrentValue();
}
```

**关键点**：
- 参数名是`PreviousValue`
- 语义清晰，表示之前的值

#### 示例3：使用Old

```cpp
void OnRep_Health(const FGameplayAttributeData& Old)
{
    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - Old.GetCurrentValue();
}
```

**关键点**：
- 参数名是`Old`
- 简洁明了

### 3. 为什么使用const和&？

#### const的作用

```cpp
void OnRep_Health(const FGameplayAttributeData& OldValue);
```

**关键点**：
- `const`表示参数是**只读**的
- 防止在OnRep函数中意外修改旧值
- 提高代码的安全性和可读性

#### &（引用）的作用

```cpp
void OnRep_Health(const FGameplayAttributeData& OldValue);
```

**关键点**：
- `&`表示参数是**引用传递**
- 避免不必要的拷贝，提高性能
- 对于大型结构体（如`FGameplayAttributeData`），引用传递更高效

#### 示例：值传递 vs 引用传递

```cpp
// 值传递（不推荐）
void OnRep_Health(FGameplayAttributeData OldValue)  // ← 会拷贝整个结构体
{
    // ...
}

// 引用传递（推荐）
void OnRep_Health(const FGameplayAttributeData& OldValue)  // ← 不会拷贝，只是引用
{
    // ...
}
```

**关键点**：
- 值传递会拷贝整个结构体，性能较差
- 引用传递不会拷贝，性能更好

### 4. 可以没有参数

**规则**：OnRep函数可以没有参数。

#### 示例：无参数的OnRep函数

```cpp
// 属性定义
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "AO|Vigor", Meta = (AllowPrivateAccess = true))
FGameplayAttributeData Vigor;

// OnRep函数
void OnRep_Vigor()
{
    UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_Vigor: Vigor replicated to %.2f"), GetVigor());
}
```

**关键点**：
- 没有参数
- 只能获取当前值（通过`GetVigor()`）
- 无法获取旧值

### 5. 引擎如何调用OnRep函数

Unreal Engine的复制系统会根据OnRep函数的签名决定是否传递参数：

#### 伪代码示例

```cpp
// 引擎内部实现（伪代码）
void Engine::ProcessReplicatedProperty(UObject* Obj, FProperty* Property, void* NewValuePtr)
{
    // 1. 保存旧值
    void* OldValuePtr = Property->ContainerPtrToValuePtr<void>(Obj);
    FGameplayAttributeData OldValue = *(FGameplayAttributeData*)OldValuePtr;
    
    // 2. 更新属性为新值
    Property->CopyCompleteValue(OldValuePtr, NewValuePtr);
    
    // 3. 调用OnRep函数
    if (Property->RepNotifyFunc != NAME_None)
    {
        // 检查OnRep函数是否有参数
        UFunction* OnRepFunc = Obj->FindFunction(Property->RepNotifyFunc);
        
        if (OnRepFunc->NumParms == 0)
        {
            // 没有参数：调用OnRep函数
            Obj->ProcessEvent(OnRepFunc, nullptr);
        }
        else if (OnRepFunc->NumParms == 1)
        {
            // 有参数：传递旧值
            FMemory Mem;
            void* Params = Mem.GetFormattedMemory(OnRepFunc->ParmsSize);
            Property->CopyCompleteValue(Params, &OldValue);  // ← 传递旧值
            Obj->ProcessEvent(OnRepFunc, Params);
        }
    }
}
```

**关键点**：
- 引擎会检查OnRep函数的参数数量
- 如果没有参数，直接调用OnRep函数
- 如果有参数，传递旧值给OnRep函数

### 6. 为什么需要参数？

#### 无参数的OnRep函数

**优点**：
- 简单，不需要处理旧值
- 适用于只需要知道当前值的场景

**缺点**：
- 无法获取旧值
- 无法计算变化量
- 无法判断属性是增加还是减少

#### 有参数的OnRep函数

**优点**：
- 可以获取旧值
- 可以计算变化量
- 可以判断属性是增加还是减少
- 可以根据变化量执行不同的逻辑

**缺点**：
- 稍微复杂一些
- 需要处理旧值

### 7. 实际应用示例

#### 示例1：Health属性（有参数）

```cpp
void UAOHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, Health, OldValue);

    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
    
    // 根据变化量执行不同的逻辑
    if (EstimatedMagnitude > 0)
    {
        // Health增加
        UE_LOG(LogTemp, Warning, TEXT("Health increased by %.2f"), EstimatedMagnitude);
    }
    else if (EstimatedMagnitude < 0)
    {
        // Health减少
        UE_LOG(LogTemp, Warning, TEXT("Health decreased by %.2f"), -EstimatedMagnitude);
    }
    
    OnHealthChange.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
}
```

**关键点**：
- 使用旧值计算变化量
- 根据变化量执行不同的逻辑

#### 示例2：Vigor属性（无参数）

```cpp
void UAOCombatAttributeSet::OnRep_Vigor()
{
    UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_Vigor: Vigor replicated to %.2f"), GetVigor());
    
    // 只能获取当前值
    const float CurrentVigor = GetVigor();
    
    // 无法获取旧值，无法计算变化量
    // 无法判断Vigor是增加还是减少
}
```

**关键点**：
- 只能获取当前值
- 无法获取旧值
- 无法计算变化量

### 8. OnRep函数参数总结

| 问题 | 答案 |
|------|------|
| **为什么一定是const FGameplayAttributeData&？** | 因为属性类型是`FGameplayAttributeData`，参数类型必须匹配 |
| **还有别的参数吗？** | 参数类型取决于属性类型，可以是`float`、`int32`等 |
| **参数名一定是OldValue吗？** | 不一定，参数名可以是任意的 |
| **为什么使用const和&？** | `const`防止修改旧值，`&`避免不必要的拷贝 |
| **可以没有参数吗？** | 可以，OnRep函数可以没有参数 |

### 9. 推荐做法

**对于Health、MaxHealth等需要知道变化量的属性**：
- 使用有参数的OnRep函数
- 参数类型与属性类型匹配
- 使用`const`和`&`提高性能

**对于简单的属性**：
- 可以使用无参数的OnRep函数
- 只需要知道当前值即可

这就是为什么`OnRep_Health`有参数`const FGameplayAttributeData& OldValue`，而`OnRep_Vigor`没有参数的原因：Health需要知道变化量来触发不同的逻辑，而Vigor只需要知道当前值即可。
