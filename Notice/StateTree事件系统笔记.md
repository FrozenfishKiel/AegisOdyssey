# StateTree 事件系统笔记

## FInstancedStruct 和 FConstStructView 详解

### FInstancedStruct（可变结构体实例）

#### 定义
```cpp
USTRUCT()
struct FInstancedStruct
{
    GENERATED_BODY()
    
    // 可以存储任意类型的结构体
    // 可以修改内部数据
    // 拥有内存所有权
};
```

#### 特点
- **可变**：可以修改内部数据
- **拥有内存**：管理结构体的内存
- **类型安全**：通过 `InitializeAs<T>()` 指定类型
- **序列化支持**：支持网络序列化
- **动态类型**：可以在运行时改变存储的结构体类型

#### 用法示例
```cpp
// 创建实例
FInstancedStruct Payload;

// 初始化为特定类型
Payload.InitializeAs<FCombatStateTreeInputEvent>(
    FCombatStateTreeInputEvent(InTargetTag, InInputType)
);

// 访问和修改数据
FCombatStateTreeInputEvent* Data = Payload.GetMutablePtr<FCombatStateTreeInputEvent>();
if (Data)
{
    Data->InputTag = NewTag;  // 可以修改
    Data->InputType = EInputType::Start;
}

// 检查类型
if (Payload.GetScriptStruct() == FCombatStateTreeInputEvent::StaticStruct())
{
    // 类型匹配
}

// 重置
Payload.Reset();
```

---

### FConstStructView（不可变结构体视图）

#### 定义
```cpp
struct FConstStructView
{
    // 指向结构体的只读视图
    // 不拥有内存，只是引用
    // 不能修改内部数据
};
```

#### 特点
- **不可变**：只能读取，不能修改
- **轻量级**：不拥有内存，只是指针引用
- **高效**：拷贝成本低（只拷贝指针）
- **类型安全**：通过 `GetScriptStruct()` 获取类型信息
- **只读访问**：提供 const 访问接口

#### 用法示例
```cpp
FInstancedStruct Payload;
Payload.InitializeAs<FCombatStateTreeInputEvent>(...);

// 创建只读视图
FConstStructView View(Payload);

// 只能读取数据
const FCombatStateTreeInputEvent* Data = View.GetPtr<FCombatStateTreeInputEvent>();
if (Data)
{
    FGameplayTag Tag = Data->InputTag;  // 可以读取
    EInputType Type = Data->InputType;
    
    // Data->InputTag = NewTag;  // 错误！不能修改
}

// 检查有效性
if (View.IsValid())
{
    // 视图有效
}

// 检查类型
if (View.GetScriptStruct() == FCombatStateTreeInputEvent::StaticStruct())
{
    // 类型匹配
}
```

---

## 两者对比

| 特性 | FInstancedStruct | FConstStructView |
|------|----------------|-----------------|
| 可变性 | 可变 | 不可变 |
| 内存所有权 | 拥有 | 不拥有（引用） |
| 拷贝成本 | 高（深拷贝） | 低（只拷贝指针） |
| 访问权限 | 可读可写 | 只读 |
| 用途 | 存储和修改数据 | 传递只读数据 |
| 网络序列化 | 支持 | 不直接支持 |

---

## 在 StateTree 事件系统中的应用

### FStateTreeEvent 结构体

```cpp
USTRUCT(BlueprintType)
struct FStateTreeEvent
{
    GENERATED_BODY()
    
    /** Tag 描述事件 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Categories="StateTreeEvent"))
    FGameplayTag Tag;

    /** 可选的事件负载 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
    FInstancedStruct Payload;

    /** 可选的发送者信息 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
    FName Origin;
};
```

### 发送事件的完整流程

#### 方式1：使用临时变量（推荐）

```cpp
void UAOCombatStateTree::CallStateTreeToSentEvent(
    const FGameplayTag InTargetTag, 
    const EInputType InInputType)
{
    if (!InTargetTag.IsValid())
    {
        return;
    }
    
    FStateTreeEvent Event;
    
    // 直接初始化 Payload
    Event.Payload.InitializeAs<FCombatStateTreeInputEvent>(
        FCombatStateTreeInputEvent(InTargetTag, InInputType)
    );
    Event.Tag = InTargetTag;
    
    SendStateTreeEvent(Event);
}
```

#### 方式2：使用中间变量

```cpp
void UAOCombatStateTree::CallStateTreeToSentEvent(
    const FGameplayTag InTargetTag, 
    const EInputType InInputType)
{
    if (!InTargetTag.IsValid())
    {
        return;
    }
    
    FStateTreeEvent Event;
    
    // 创建中间变量
    FInstancedStruct Payload;
    Payload.InitializeAs<FCombatStateTreeInputEvent>(
        FCombatStateTreeInputEvent(InTargetTag, InInputType)
    );
    
    // 赋值（可能触发隐式转换）
    Event.Payload = Payload;
    Event.Tag = InTargetTag;
    
    SendStateTreeEvent(Event);
}
```

#### 方式3：使用 FConstStructView（不推荐，除非有特殊需求）

```cpp
void UAOCombatStateTree::CallStateTreeToSentEvent(
    const FGameplayTag InTargetTag, 
    const EInputType InInputType)
{
    if (!InTargetTag.IsValid())
    {
        return;
    }
    
    FStateTreeEvent Event;
    
    // 创建可变实例
    FInstancedStruct Payload;
    Payload.InitializeAs<FCombatStateTreeInputEvent>(
        FCombatStateTreeInputEvent(InTargetTag, InInputType)
    );
    
    // 创建只读视图
    FConstStructView View(Payload);
    
    // 赋值（FInstancedStruct 可能接受 FConstStructView）
    Event.Payload = View;
    Event.Tag = InTargetTag;
    
    SendStateTreeEvent(Event);
}
```

---

## 在状态树中接收事件

### 检查 Payload 类型

```cpp
// 在自定义 Condition 中
bool FSTC_BlockInputType::TestCondition(FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 获取当前 Event
    const FStateTreeEvent* CurrentEvent = Context.GetCurrentEvent();
    if (!CurrentEvent || !CurrentEvent->Payload.IsValid())
    {
        return false;
    }
    
    // 检查 Payload 是否是 FCombatStateTreeInputEvent
    if (CurrentEvent->Payload.GetScriptStruct() != 
        FCombatStateTreeInputEvent::StaticStruct())
    {
        return false;
    }
    
    // 检查 InputType 是否匹配
    const FCombatStateTreeInputEvent* InputEvent = 
        CurrentEvent->Payload.GetPtr<FCombatStateTreeInputEvent>();
    return InputEvent && InputEvent->InputType == InstanceData.ExpectedInputType;
}
```

---

## 最佳实践

### 1. 创建和初始化
```cpp
// 推荐：直接初始化
Event.Payload.InitializeAs<FCombatStateTreeInputEvent>(...);

// 不推荐：不必要的中间转换
FInstancedStruct Payload;
Payload.InitializeAs<FCombatStateTreeInputEvent>(...);
Event.Payload = FConstStructView(Payload);
```

### 2. 访问数据
```cpp
// 读取数据（使用 GetPtr）
const FCombatStateTreeInputEvent* Data = Payload.GetPtr<FCombatStateTreeInputEvent>();
if (Data)
{
    FGameplayTag Tag = Data->InputTag;
}

// 修改数据（使用 GetMutablePtr）
FCombatStateTreeInputEvent* Data = Payload.GetMutablePtr<FCombatStateTreeInputEvent>();
if (Data)
{
    Data->InputTag = NewTag;
}
```

### 3. 类型检查
```cpp
// 检查类型
if (Payload.GetScriptStruct() == FCombatStateTreeInputEvent::StaticStruct())
{
    // 类型匹配
}

// 检查有效性
if (Payload.IsValid())
{
    // Payload 有效
}
```

### 4. 重置
```cpp
// 重置 Payload
Payload.Reset();

// 重新初始化
Payload.InitializeAs<AnotherStruct>(AnotherStruct(...));
```

---

## 常见问题

### Q1: 什么时候使用 FInstancedStruct？
**A**: 当你需要：
- 存储和修改结构体数据
- 支持网络序列化
- 在运行时改变存储的类型

### Q2: 什么时候使用 FConstStructView？
**A**: 当你需要：
- 只读访问结构体数据
- 高效传递数据（避免拷贝）
- 提供只读接口

### Q3: 为什么 FStateTreeEvent.Payload 使用 FInstancedStruct？
**A**: 因为：
- 需要支持网络序列化
- 需要存储任意类型的结构体
- 需要在运行时改变类型

### Q4: 如何在蓝图中使用？
**A**: 
- `FInstancedStruct` 在蓝图中可以直接使用
- `FConstStructView` 主要用于 C++ 代码
- 蓝图中通常不需要关心底层实现

---

## 相关源码位置

- `d:\UE_5.6\Engine\Plugins\Runtime\StateTree\Source\StateTreeModule\Public\StateTreeEvents.h`
- `d:\UE_5.6\Engine\Plugins\Runtime\StateTree\Source\StateTreeModule\Private\StateTreeEvents.cpp`
- `d:\UE_Project\AO\AegisOdyssey\Source\AegisOdyssey\StateTree\CombatStateTree\AOCombatStateTree.cpp`

---

## 总结

- **FInstancedStruct**：可变、拥有内存、支持序列化，用于存储和修改数据
- **FConstStructView**：不可变、轻量级、高效，用于只读访问
- 在 StateTree 事件系统中，使用 `FInstancedStruct` 作为 Payload
- 直接初始化 `Event.Payload.InitializeAs<T>(...)` 是最简洁的方式
- 访问数据时使用 `GetPtr<T>()`（只读）或 `GetMutablePtr<T>()`（可写）
