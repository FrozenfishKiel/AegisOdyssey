---
title: EngineCore Known Issues
tags:
  - knowledge
  - engine-core
  - known-issues
  - unreal-engine
aliases:
  - EngineCore Known Issues
  - EngineCore 已知边界与历史偏差
---

# EngineCore 已知边界与历史偏差

更新时间：2026-05-19  
适用范围：本轮 `EngineCore` 深提炼中识别出的高风险误读点、历史文章偏差、以及项目术语与引擎术语的混层。  
不适用范围：完整运行时 bug 列表。

## 1. `C++的RTTI.md` 不能直接当成 UE `UObject` 类型系统说明

当前不应继续这样写：

1. “UE 的运行时识别主要就是 `typeid` 和 `dynamic_cast`”
2. “理解了标准 C++ RTTI，就等于理解了 UE 对象识别”

当前已经核实的项目真相是：

- `UObject` 体系主要消费 `UClass`、`IsA()`、`Cast<>`
- UE 在 `Casts.h` 里对 `dynamic_cast` 有自己的处理

所以这篇历史文章最多只能作为“运行时类型识别概念导入”，不能作为项目事实正文。

## 2. `UE反射.md` 里很多截图是在展示生成结果，不是机制边界

当前不应继续这样写：

1. “看到某段 `gen.cpp` 就等于完整理解反射”
2. “宏展开截图本身就是反射系统”

更稳的写法应该是：

- 宏只是入口
- UHT 和生成代码负责桥接
- `UClass / UStruct / UFunction / FProperty` 才是运行时反射对象

## 3. `UE垃圾回收.md` 对 `UPROPERTY` 与 root 的说法过松

当前最容易误导人的点是：

1. 把 `UPROPERTY` 直接写成“root 延伸”
2. 把“还没被 GC 回收”写成“对象还活着、逻辑上就没问题”
3. 把弱引用、软引用、反射引用、root 引用混成一类

当前应该固定的纠偏是：

- `UPROPERTY` 让引用进入可达性分析，不等于把目标对象变 root
- root、可达、`PendingKill`、`Garbage` 不是同一个词
- 讨论生命周期时必须说明是在讲哪一层状态

## 4. `UE默认对象和实例化.md` 最大的问题是把 CDO 讲成了“不是对象”

当前不应继续这样写：

1. “默认对象不是运行时对象”
2. “`DefaultToInstanced` 的意思是默认是实例，不是默认对象”

源码已经能确认：

- `ClassDefaultObject` 是真实 `UObject`
- 它带 `RF_ClassDefaultObject`
- 对象初始化链会直接消费 CDO 或 archetype 默认数据

所以这篇历史文章只能保留“引出问题”的价值，不能直接作为结论。

## 5. `EditDefaultsOnly` vs `EditAnywhere` 在历史文章里的归纳过度简化

当前不应继续这样写：

1. “`EditDefaultsOnly` 只能改默认对象，`EditAnywhere` 才能改实例，所以实例化子对象一律必须 `EditAnywhere`”

原因是：

- 这组 specifier 讨论的是编辑暴露边界
- 还会和对象所在上下文、是否为 instanced 子对象、资源编辑位置等一起影响最终表现

所以如果后续真要写这块，必须基于项目里具体属性与编辑场景再单独整理，不能直接从历史文章抽一句结论。

## 6. 项目里的 `ItemCDO` 命名和引擎原生 CDO 不是一回事

这是当前最值得警惕的项目级误判点。

从当前实现看：

1. `UAOInventoryItemInstance` 里有 `ItemCDO`
2. `SetItemDef(...)` 用的是 `NewObject<UAOInventoryItemDefinition>(Outer, InDef)`
3. 这不等于直接调用 `InDef->GetDefaultObject()`

因此：

- 业务层叫它 `CDO`，不代表它就是引擎层 `ClassDefaultObject`
- 后续如果继续整理库存、装备、采集等定义层对象，一定要先区分这两个概念

## 7. 当前整理规则

后续继续往 `Docs/Knowledge/EngineCore` 补内容时，默认遵守：

1. 只要涉及 `UObject` 类型识别，优先回 `Casts.h` 和 `Class.h`。
2. 只要涉及 GC，优先回 `GarbageCollection.cpp` 和 `AddReferencedObjects(...)` 路径。
3. 只要涉及默认对象、实例化、子对象复制，优先回 `UObjectGlobals.cpp` 的构造与 `InitProperties(...)` 链。
4. 只要项目代码里写了 “CDO”，都先检查它到底是不是引擎原生 CDO。
