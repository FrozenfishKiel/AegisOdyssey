# UE 默认对象和实例化

## 默认对象

### 定义
默认对象是类的模板，存储了类的默认属性值。

### 特点
- 每个类都有一个默认对象
- 存储在内存中，用于初始化新对象
- 可以在编辑器中编辑默认属性
- **不是运行时对象**

### 例子
```cpp
// 在编辑器中编辑默认属性
UAOFragment_EquipAnimation* DefaultObject = UAOFragment_EquipAnimation::StaticClass()->GetDefaultObject();
DefaultObject->EquipMontage = SomeMontage;  // 编辑默认属性

// 创建新对象时，会从默认对象复制属性
UAOFragment_EquipAnimation* NewObject = NewObject<UAOFragment_EquipAnimation>(Parent);
// NewObject->EquipMontage 会从 DefaultObject->EquipMontage 复制
```

## 实例化

### 定义
实例化是创建一个对象的具体实例，用于运行时。

### 特点
- 每个实例都有自己的属性副本
- 可以在运行时修改属性
- 可以在编辑器中编辑实例属性
- **是运行时对象**

### 例子
```cpp
// 创建一个实例
UAOFragment_EquipAnimation* Instance = NewObject<UAOFragment_EquipAnimation>(Parent);
Instance->EquipMontage = SomeMontage;  // 修改实例属性
```

## DefaultToInstanced

### 定义
`DefaultToInstanced` 是一个 UCLASS 修饰符，表示这个类的对象默认是实例化的，而不是默认对象。

### 特点
- 这个类的对象默认是实例化的
- 不是默认对象
- 可以在实例中编辑属性
- 适用于需要在父对象中内联编辑的类

### 例子
```cpp
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class AEGISODYSSEY_API UAOInventoryItemFragment : public UObject
{
    GENERATED_BODY()
};
```

## EditInlineNew

### 定义
`EditInlineNew` 是一个 UCLASS 修饰符，表示这个类可以在父对象中内联编辑，创建新实例。

### 特点
- 可以在父对象中内联编辑
- 可以创建新实例
- 适用于需要在父对象中编辑的子对象

### 例子
```cpp
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class AEGISODYSSEY_API UAOInventoryItemFragment : public UObject
{
    GENERATED_BODY()
};

UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOInventoryItemDefinition : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TArray<TObjectPtr<UAOInventoryItemFragment>> Fragments;
};
```

## 具体场景

### 场景1：在蓝图编辑器中编辑 UAOInventoryItemDefinition
```cpp
// 1. 在蓝图编辑器中编辑 UAOInventoryItemDefinition
UAOInventoryItemDefinition* ItemDef = ...;

// 2. Fragments 数组中的每个 Fragment 都是作为子对象实例化的
UAOFragment_EquipAnimation* Fragment = NewObject<UAOFragment_EquipAnimation>(ItemDef);
ItemDef->Fragments.Add(Fragment);

// 3. 在蓝图编辑器中编辑 Fragment 的属性
Fragment->EquipMontage = SomeMontage;  // 编辑实例属性
```

### 场景2：EditDefaultsOnly vs EditAnywhere
```cpp
// EditDefaultsOnly
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
UAnimMontage* EquipMontage;
// ❌ 只能在默认对象中编辑，不能在实例中编辑

// EditAnywhere
UPROPERTY(EditAnywhere, BlueprintReadOnly)
UAnimMontage* EquipMontage;
// ✅ 可以在默认对象和实例中编辑
```

## 总结

### 默认对象
- 类的模板
- 存储默认属性值
- 不是运行时对象
- 用于初始化新对象

### 实例化
- 创建对象的具体实例
- 有自己的属性副本
- 是运行时对象
- 可以在运行时修改属性

### DefaultToInstanced
- 对象默认是实例化的
- 不是默认对象
- 可以在实例中编辑属性

### EditInlineNew
- 可以在父对象中内联编辑
- 可以创建新实例

## 实际应用

在装备系统中，`UAOFragment_EquipAnimation` 是作为子对象实例化的，所以需要使用 `EditAnywhere` 而不是 `EditDefaultsOnly`：

```cpp
UCLASS()
class AEGISODYSSEY_API UAOFragment_EquipAnimation : public UAOInventoryItemFragment
{
    GENERATED_BODY()
public:
    // ✅ 使用 EditAnywhere
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EquipAnimation")
    UAnimMontage* EquipMontage;  // 装备动画

    // ✅ 使用 EditAnywhere
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EquipAnimation")
    UAnimMontage* UnEquipMontage;  // 不装备动画
};
```

如果使用 `EditDefaultsOnly`，在蓝图编辑器中编辑后会消失，因为 `EditDefaultsOnly` 不允许在实例中编辑属性。
