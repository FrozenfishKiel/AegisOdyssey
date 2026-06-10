---
title: EngineCore Decisions
tags:
  - knowledge
  - engine-core
  - decisions
  - unreal-engine
aliases:
  - EngineCore Decisions
  - EngineCore 已锁定认识
---

# EngineCore 已锁定认识

更新时间：2026-05-19  
适用范围：当前工程在理解 UE 底层对象模型时，已经可以视为稳定结论的认识边界。  
不适用范围：纯 C++ 非 `UObject` 类型系统、未来可能接入的新引擎机制、Blueprint 编辑器具体交互细节。

## 1. `UObject` 运行时识别优先使用 UE 类型系统

已经锁定：

1. 当前工程解释 `UObject` 运行时类型时，优先使用 `Cast<> / CastChecked<> / ExactCast<> / IsA() / StaticClass()`。
2. 不把标准 C++ RTTI 当作项目里 `UObject` 识别的主叙事。
3. `ExactCast<>` 只能用于“精确类相等”，不能写成“等价于 IsA”。

## 2. 反射应该理解为 UHT + 生成代码 + 运行时元对象

已经锁定：

1. `UCLASS / USTRUCT / UPROPERTY / UFUNCTION` 是反射标记入口，不是运行时反射对象本身。
2. 运行时真正起作用的是 `UClass / UStruct / UFunction / FProperty`。
3. 以后写知识库时，不再把“宏展开截图”直接写成反射本体。

## 3. GC 应按“可达性”理解，不按“谁是 root”二分法理解

已经锁定：

1. `UPROPERTY` 的正确作用是让引用进入可达性遍历体系，而不是把目标对象直接变成 root。
2. root set、脚本属性引用、容器引用、结构体引用、`AddReferencedObjects(...)` 都共同决定对象是否可达。
3. 讨论“为什么对象没有被回收”时，必须先分清它是：
   - 被 root 保活
   - 被反射属性链保活
   - 被 `AddReferencedObjects(...)` 保活
   - 只是被弱引用观察

## 4. CDO 是真实对象，不再接受“不是运行时对象”这种写法

已经锁定：

1. CDO 是真实存在的 `UObject`，并带 `RF_ClassDefaultObject`。
2. 新实例的默认属性初始化本质上依赖 CDO 或 archetype。
3. 以后文档里不再把 CDO 写成“只是抽象模板、不是对象”。

## 5. `DefaultToInstanced` 不等于“没有默认对象”

已经锁定：

1. `DefaultToInstanced` 讨论的是子对象实例化策略，不否定类级 CDO 的存在。
2. `EditInlineNew` 讨论的是是否能在拥有者内部内联创建/编辑该类实例。
3. 以后文档里不再把 `DefaultToInstanced` 解释成“对象默认是实例，不是默认对象”。

## 6. 项目内“CDO”一词必须区分引擎术语和业务术语

已经锁定：

1. 引擎术语里的 CDO 指 `ClassDefaultObject`。
2. 项目代码里的 `GetItemCDO()`、`ItemCDO` 目前并不总是直接返回 `UClass::GetDefaultObject()`。
3. 因此后续文档必须显式区分：
   - 引擎原生 CDO
   - 项目里“定义层对象 / 模板对象 / 业务上叫 CDO 的对象”

## 7. 项目当前已经大量真实依赖 instanced 子对象模型

已经锁定：

1. `UAOInventoryItemFragment`
2. `UAOEquipmentFeatureAction`
3. `UAOHarvestToolFragment`
4. `UAOSkillExecutionDefinition` 这一类对象

都已经在项目里按 `DefaultToInstanced + EditInlineNew` 模式真实使用。

这意味着：

- 这不是理论设计。
- 是当前工程定义层建模的稳定基础设施。
