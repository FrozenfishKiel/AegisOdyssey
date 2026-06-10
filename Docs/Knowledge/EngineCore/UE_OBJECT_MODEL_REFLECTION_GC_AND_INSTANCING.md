---
title: UE Object Model Reflection GC And Instancing
tags:
  - knowledge
  - engine-core
  - unreal-engine
  - reflection
  - garbage-collection
  - instancing
aliases:
  - UE Object Model Reflection GC And Instancing
  - UE 对象模型 反射 GC 与实例化
---

# UE 对象模型 反射 GC 与实例化

更新时间：2026-05-19  
适用范围：理解当前项目如何建立在 UE5.6 的 `UObject` 类型系统、反射系统、GC、CDO/实例化链路之上。  
不适用范围：普通 C++ 值类型、STL 智能指针、非 `UObject` 内存模型。

## 1. `UObject` 运行时类型判断不是在讲标准 C++ RTTI

对当前工程而言，更实用也更权威的说法是：

1. `UObject` 有自己的运行时类型系统。
2. 它的中心对象是 `UClass`。
3. 日常判断与转换入口是 `StaticClass()`、`IsA()`、`Cast<>`、`CastChecked<>`、`ExactCast<>`。

从 UE5.6 源码能直接确认：

- `ExactCast` 走的是 `Src->GetClass() == To::StaticClass()`
- `CastChecked` 是 UE 自己的检查式转换
- `dynamic_cast` 在 `Casts.h` 里被映射到了 UE 私有实现

所以在项目知识库里，`C++的RTTI.md` 最终只能保留成边界知识：

- 它能帮助理解“运行时识别”这个概念。
- 但不能拿它替代 `UObject` 体系的真实判断方式。

## 2. UE 反射不是“宏自己会反射”

更准确的理解顺序是：

1. 头文件里写 `UCLASS / USTRUCT / UPROPERTY / UFUNCTION`
2. UHT 解析这些标记
3. 生成对应注册代码与元信息代码
4. 模块加载后，把这些类、结构、属性、函数注册进运行时
5. 运行时再通过 `UClass / UStruct / FProperty / UFunction` 消费这些信息

这也是为什么：

- 宏本身在 `ObjectMacros.h` 里可以看上去非常“空”
- 但运行时依然能拿到完整反射信息

如果后续需要追源码，最关键的不是宏截图，而是：

- `ObjectMacros.h` 里的声明入口
- `DECLARE_CLASS / IMPLEMENT_CLASS`
- `StaticRegisterNatives...`
- 运行时的 `UClass / FProperty` 结构

## 3. GC 的关键不是“有没有 `UPROPERTY`”，而是“能不能被遍历到”

UE GC 在这里最容易被讲歪。

当前应固定采用下面这套说法：

1. GC 的核心是可达性分析。
2. 引擎会从 root set 和 keep flags 出发，向下追踪对象引用图。
3. 脚本属性、对象容器、结构体里的对象字段、`AddReferencedObjects(...)` 都会参与这张图。
4. 遍历不到的对象才会走向垃圾回收。

因此：

- `UPROPERTY` 不是“把对象变成 root”
- 它更接近“让这个引用进入 GC 可见图”

这也是为什么历史文章里那种：

- “标了 `UPROPERTY` 所以对象是 root”
- “对象 Destroy 以后仍然有强引用所以 GC 不会回收”

这种说法都不够严。

更准确的表达应该是：

- `Destroy`、`PendingKill`、`Garbage`、`RootSet`、`可达性` 是几套不同但会相互影响的状态
- 必须回到 UE 的遍历和标记语义上理解

## 4. `TObjectPtr`、`TWeakObjectPtr`、`TSoftObjectPtr` 在当前项目里的意义

当前工程里三类指针都已经大量出现，但语义不同：

### 4.1 `TObjectPtr`

适合：

- 作为反射属性中的对象字段
- 作为需要被 UE 对象系统识别和遍历的稳定对象引用

项目里典型例子：

- `UAOInventoryItemDefinition::Fragments`
- `UAOInventoryItemInstance::ItemCDO`
- `UAOEquipmentInstance::Instigator`

### 4.2 `TWeakObjectPtr`

适合：

- 运行时观察某个对象
- 不拥有对象生命周期
- 对象失效后允许引用自然失效

项目里典型例子：

- 各类 UI 绑定对象
- 技能运行时持有武器、角色、运行体的观察引用
- 一些目标缓存、观察者、会话模型

### 4.3 `TSoftObjectPtr`

适合：

- 资源路径型引用
- 允许对象不常驻内存
- 需要时再解析或加载

项目里典型例子：

- 各类 `UAnimMontage`
- `PrimaryDataAsset`
- 数据注册表和资产入口

## 5. CDO 是真实对象，实例初始化会依赖它

当前最稳的说法是：

1. 每个反射类都有类默认对象。
2. 这个对象就是 CDO。
3. 它是一个真实分配出来的 `UObject`，带 `RF_ClassDefaultObject`。
4. 后续新实例会从它或对应 archetype 拷贝默认属性。

从引擎主链路来看：

1. `StaticConstructObject_Internal(...)`
2. `StaticAllocateObject(...)`
3. 调类构造函数并传入 `FObjectInitializer`
4. `PostConstructInit()`
5. `InitProperties(...)`

其中 `InitProperties(...)` 这一段非常关键，因为它直接说明：

- 新对象的默认值来源是“已有默认数据”
- 不是“只靠 C++ 构造函数现场写出来”

## 6. `DefaultToInstanced` / `EditInlineNew` 的正确理解

这两个词最容易被误用。

当前应固定理解成：

1. `DefaultToInstanced`
   说明这种子对象在被别的对象作为属性持有时，倾向于以实例方式存在。
2. `EditInlineNew`
   说明这类对象可以在拥有者内部以内联方式创建和编辑。

它们解决的问题是：

- 定义层对象如何挂载自己的可编辑子对象
- 这些子对象在资源、定义、实例链上如何组织

它们不意味着：

- 这个类没有 CDO
- 这个类不参与默认值传播
- 这个类天然就是运行时瞬态对象

## 7. 这些机制在当前项目里怎么落地

当前项目最典型的落地是“定义对象 + 内联片段/动作 + 运行时实例”这条建模链。

例如：

1. `UAOInventoryItemDefinition`
   用 `Fragments` 持有一组 `UAOInventoryItemFragment`
2. `UAOInventoryItemFragment`
   使用 `DefaultToInstanced, EditInlineNew`
3. `UAOEquipmentFeatureAction`
   也是 `DefaultToInstanced, EditInlineNew`
4. `UAOHarvestToolFragment`
   同样走这条模式

这说明当前工程大量业务定义并不是靠：

- 一个大而平的 DataTable 字段集合

而是靠：

- 定义对象下挂一组 instanced 子对象块

这已经是项目级稳定结构，不是偶然写法。

## 8. 当前项目里有一个需要特别警惕的术语混用

项目代码里存在：

- `ItemCDO`
- `GetItemCDO()`

这种命名。

但从当前实现看，`SetItemDef(...)` 里实际做的是：

- `NewObject<UAOInventoryItemDefinition>(Outer, InDef)`

这更接近：

- 基于某个定义类创建了一个对象实例

而不一定等于：

- 直接取 `InDef->GetDefaultObject()`

所以后续文档必须明确区分：

1. 引擎语义里的 CDO
2. 项目业务层把“定义对象模板”口语上叫成 CDO

否则会在分析默认值传播、复制、GC 和定义读取路径时反复混淆。

## 9. 当前最稳的阅读心智模型

如果只保留一套最有用的心智模型，应该是：

1. `UClass` 定义了“这是什么对象”
2. 反射元信息决定“引擎能不能认识这个字段/函数/结构”
3. GC 看的是“引用图里能不能走到”
4. CDO/Archetype 决定“新对象初始化时默认值从哪来”
5. `DefaultToInstanced` 决定“某类子对象是怎么嵌进拥有者建模里的”

这五条足够覆盖当前工程大部分底层对象问题。
