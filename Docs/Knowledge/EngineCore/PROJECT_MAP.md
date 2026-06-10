---
title: EngineCore Project Map
tags:
  - knowledge
  - engine-core
  - project-map
  - unreal-engine
aliases:
  - EngineCore Project Map
  - EngineCore 项目地图
---

# EngineCore 项目地图

更新时间：2026-05-19  
适用范围：当前项目里 `UObject` 类型识别、UE 反射、GC 可达性、CDO/实例化链路这几块最基础、最容易被误写的引擎机制。  
不适用范围：标准 C++ RTTI 教程、所有非 `UObject` 普通 C++ 对象的生命周期、Blueprint 资源级编辑流程细节。

## 1. 这份文档解决什么问题

这份文档只回答：

1. 当前项目里判断 `UObject` 运行时类型，应该优先依赖什么机制。
2. `UCLASS / USTRUCT / UPROPERTY / UFUNCTION` 到底在 UE 里扮演什么角色。
3. UE GC 到底是“谁引用谁就活着”，还是“只有 root 才活着”。
4. CDO、默认子对象、实例化、`DefaultToInstanced` 这些词在当前工程里分别落在哪。
5. 后续继续整理底层机制文档时，先看哪些引擎源码和哪些项目代码入口。

## 2. 当前四个核心主题

本轮 `EngineCore` 深提炼只覆盖四篇历史文档对应的四个主题：

1. `C++的RTTI.md`
2. `UE反射.md`
3. `UE垃圾回收.md`
4. `UE默认对象和实例化.md`

但最终沉淀不是“把四篇历史文章搬过来”，而是只保留已经被 UE5.6 源码和当前工程共同验证的稳定结论。

## 3. 当前 `UObject` 类型系统真相

优先看：

- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\Templates\Casts.h`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`
- 项目内所有 `Cast<> / CastChecked<> / ExactCast<> / StaticClass()` 的真实用法

当前已经确认：

1. UE 的 `UObject` 体系主要依赖 `UClass` 反射类型系统，而不是把标准 C++ RTTI 当主通路。
2. 项目里日常类型判断和安全转换主入口是 `Cast<>`、`CastChecked<>`、`ExactCast<>`、`IsA()`、`StaticClass()`。
3. `ExactCast<>` 明确按 `GetClass() == To::StaticClass()` 做“精确类相等”判断，不等于“可向上兼容的 IsA”。
4. `Casts.h` 里存在 `#define dynamic_cast UE::CoreUObject::Private::DynamicCast`，说明在这套上下文里 UE 已经显式接管了这条语义入口。

所以当前项目里正确理解是：

- 普通 C++ RTTI 是语言知识。
- 但 `UObject` 运行时识别，应该优先按 UE 自己的类型系统来写、来读、来解释。

## 4. 当前反射注册真相

优先看：

- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\ObjectMacros.h`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\CoreNative.cpp`

当前已经确认：

1. `UPROPERTY / UFUNCTION / USTRUCT` 这些宏在头文件里本身只是供 UHT 和生成代码流程识别的标记入口。
2. `GENERATED_BODY()`、`DECLARE_CLASS(...)`、`IMPLEMENT_CLASS(...)` 这些宏才进一步把静态注册、`StaticClass()`、类元信息接进运行时。
3. 运行时真正承载反射信息的不是“宏本身”，而是 `UClass / UStruct / UFunction / FProperty` 这些对象。
4. 因此反射的真实链路应该理解成：
   `源码标记 -> UHT 生成代码 -> 编译进模块 -> 类/属性/函数元信息注册到运行时`

## 5. 当前 GC 真相

优先看：

- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\GarbageCollection.cpp`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\UObjectGlobals.cpp`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\ObjectPtr.h`

当前已经确认：

1. UE 对 `UObject` 做的是基于可达性分析的 mark-and-sweep。
2. 源码注释明确写了：先把对象视为不可达，再从 root set 和 keep flags 出发做可达性遍历。
3. 反射属性、容器里的对象引用、结构体里的对象引用，以及 `AddReferencedObjects(...)` 都会进入这套遍历。
4. `UPROPERTY` 的正确语义不是“把对象变成 root”，而是“让这个引用进入反射/遍历体系，从而可能让目标对象保持可达”。
5. `CallAddReferencedObjects(...)` 是 GC 继续向下追踪非纯属性引用的重要补口。
6. UE5.6 已存在增量可达性分析，`GIsIncrementalReachabilityPending` 不是历史概念，而是当前实现的一部分。

## 6. 当前 CDO 与实例化真相

优先看：

- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectGlobals.h`
- `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\UObjectGlobals.cpp`

当前已经确认：

1. 每个反射类都有自己的 `ClassDefaultObject`。
2. `UClass::GetDefaultObject()` 会在需要时触发 `InternalCreateDefaultObjectWrapper()`。
3. CDO 是一个真实存在的 `UObject`，并带有 `RF_ClassDefaultObject` 标记。
4. 对象构造主链路是：
   `StaticConstructObject_Internal -> StaticAllocateObject -> ClassConstructor(FObjectInitializer) -> PostConstructInit -> InitProperties`
5. 新实例初始化时，属性默认值会从 archetype 或类默认对象复制，而不是凭空生成。
6. `DefaultToInstanced` 和 `EditInlineNew` 讨论的是“某类子对象是否以内联实例方式挂在拥有者上”，不等于“这个类没有 CDO”。

## 7. 当前项目里的直接落点

优先看：

- [AOInventoryItemDefinition.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryItemDefinition.h)
- [AOInventoryItemInstance.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryItemInstance.h)
- [AOInventoryItemInstance.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryItemInstance.cpp)
- [AOEquipmentInstance.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Equipment/AOEquipmentInstance.h)
- [AOEquipmentFeatureAction.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h)
- [AOHarvestToolFragment.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.h)
- [AOAbilityTypes.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AOAbilityTypes.h)
- [DefaultGame.ini](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Config/DefaultGame.ini)

当前项目里已经能直接看到：

1. `UAOInventoryItemFragment`、`UAOEquipmentFeatureAction`、`UAOHarvestToolFragment` 这类对象都在真实使用 `DefaultToInstanced + EditInlineNew`。
2. 项目大量使用 `Cast<>`、`CastChecked<>` 和 `GetItemCDO()` 风格读取定义层数据。
3. 项目大量使用 `TWeakObjectPtr` 持有运行时对象观察引用，也使用 `TSoftObjectPtr` 持有资源路径型引用。
4. `DefaultGame.ini` 已正式注册 `UAOAbilitySystemGlobals`，说明自定义 `GameplayEffectContext` 基础设施已经是工程真相，不是实验分支。

## 8. 当前真相层和消费层

### 8.1 真相层

当前应视为真相层的有：

- `UClass / UStruct / FProperty / UFunction`
- `Cast<> / ExactCast<> / IsA() / StaticClass()`
- `ClassDefaultObject`
- `StaticConstructObject_Internal / StaticAllocateObject / FObjectInitializer`
- `GC reachability + root set + AddReferencedObjects`

### 8.2 消费层

当前应视为消费层的有：

- 项目里各种 `GetItemCDO()` 读定义数据的业务逻辑
- 各种 `DefaultToInstanced` 片段、特性动作、执行定义
- 各种 `TWeakObjectPtr` 运行时观察引用
- 各种 `TSoftObjectPtr` 资源引用
- GAS / Inventory / Equipment / Harvest 这些系统对底层机制的具体消费

## 9. 当前继续扩展时的阅读顺序

如果后续继续整理 `EngineCore`，当前推荐顺序是：

1. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\Templates\Casts.h`
2. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\ObjectMacros.h`
3. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`
4. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectGlobals.h`
5. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\UObjectGlobals.cpp`
6. `D:\UE_5.6\Engine\Source\Runtime\CoreUObject\Private\UObject\GarbageCollection.cpp`
7. `Source/AegisOdyssey/Inventory/*`
8. `Source/AegisOdyssey/Equipment/*`
9. `Source/AegisOdyssey/Harvest/*`
10. `Source/AegisOdyssey/AOAbilityTypes.*`
11. `Config/DefaultGame.ini`

## 10. 本轮提炼来源

本轮主要从下面四篇历史文档提炼，并结合 UE5.6 源码与当前项目代码校对：

- `Notice/HistoryNotice/C++的RTTI.md`
- `Notice/HistoryNotice/UE反射.md`
- `Notice/HistoryNotice/UE垃圾回收.md`
- `Notice/HistoryNotice/UE默认对象和实例化.md`

沉淀后的稳定文档分别是：

- [[EngineCore 已锁定认识]]
- [[UE 对象模型 反射 GC 与实例化]]
- [[EngineCore 已知边界与历史偏差]]
