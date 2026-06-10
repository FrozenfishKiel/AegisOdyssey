# 动态装备来源属性集任务简报

## 当前结论

这份文档已经从“动态增删 AttributeSet”的错误方向收回。

当前锁定的正确实现是：

1. `UAOWeaponAttributeSet`
2. `UAOEquipmentAttributeSet`

继续保留为独立的来源属性集，不并回角色核心属性集。

但它们**不再随装备/卸下动态创建和移除**，而是改成：

1. 继续通过 `PawnData -> AbilitySets -> AbilitySet.AttributeSets` 这条原有数据链描述
2. `AbilitySet` 在授予时按类确保依赖的 AttributeSet 已挂到 ASC 上
3. 装备/卸下时只改这些属性集里的数值与授予出去的 GE/Ability
4. 不再改“属性集存不存在”
5. `AbilitySet.AttributeSets` 只负责“确保属性集在 ASC 上可用”，不再把共享 AttributeSet 实例绑定进某次授予句柄的回收语义

---

## 为什么回退

之前那版做了：

1. `UAOAbilitySystem::AcquireDynamicAttributeSet(...)`
2. `UAOAbilitySystem::ReleaseDynamicAttributeSet(...)`
3. `AODerivedAttributeTags`
4. 基于“拓扑变化后协调派生 GE”的一整套动态结构方案

这条路现在确认不继续使用，原因很明确：

MMC 虽然可以声明 `bSnapshot = false`，但如果它依赖的来源 AttributeSet 在派生 GE 初次建立捕获关系时并不稳定存在，那么后面继续围绕“动态注册后再补协调”打补丁，工程收益很差，风险很高，也容易继续把问题做复杂。

所以这次已经及时止损，改回稳定结构来源。

---

## 本轮代码结果

### 1. 已删除的错误方向

已经清理掉：

1. `Source/AegisOdyssey/AODerivedAttributeTags.h`
2. `Source/AegisOdyssey/AODerivedAttributeTags.cpp`
3. `UAOAbilitySystem` 里动态 AttributeSet 拓扑追踪与派生 GE 协调代码
4. `UAOAbilitySet` 里动态 AttributeSet 占用记录与回收逻辑

### 2. 已落地的正确方向

已经改成：

1. `AAOCharacter` 仍然保留核心健康属性集默认子对象
2. `AbilitySet.AttributeSets` 恢复参与实际授予链
3. `UAOAbilitySystem::EnsureSpawnedAttributeSet(...)` 负责按类确保只注册一次
4. `PawnData` 继续只通过 `AbilitySets` 驱动这条链
5. `FAOAbilitySet_GrantedHandles` 只回收 Ability / GameplayEffect 句柄，不再对共享 AttributeSet 实例做对称移除

其中：

1. `UAOWeaponAttributeSet` 负责武器来源属性
2. `UAOEquipmentAttributeSet` 负责正式装备来源属性
3. `UAOCombatAttributeSet` 继续负责角色最终战斗属性汇总

### 3. 当前语义

现在的语义应该这样理解：

1. Weapon / Equipment 来源属性集始终存在
2. MMC 抓取的是稳定存在的来源 AttributeSet
3. 装备与卸下只负责改变来源数值和授予句柄
4. 不再依赖“运行时动态增删 AttributeSet”去驱动 MMC

---

## 现阶段边界

这份回退只解决一件事：

把错误的动态 AttributeSet 存在性方案撤掉，并把结构改回稳定来源。

这不等于：

1. 所有装备数值资产已经全部重新配置完
2. 所有来源属性写入路径已经最终收束完
3. 所有派生 GE / MMC 资产侧都已经验证完毕

后续如果还有“数值变了但最终属性没动”的问题，下一步应该继续查：

1. 来源数值到底写进了哪个 AttributeSet
2. 派生 GE 是否真的挂在角色身上
3. MMC 捕获项是否对到了当前来源属性
4. 资产层是否还有旧属性引用

而不是再回到“继续动态创建/删除 AttributeSet”的方向。

---

## 本轮验证

本轮已完成：

1. 工程全量编译通过
2. `AegisOdysseyEditor Win64 Development` 构建成功

编译结论：

当前代码层已经不存在前一版动态 AttributeSet 拓扑方案的残留编译依赖。

---

## 当前文档口径

关于这轮“动态 AttributeSet -> 稳定 AttributeSet 来源”的最终落地，以本文档为准。

`Docs/GAS_MMC_动态AttributeSet联动问题复盘.md` 保留的是当时排查过程与阶段性思路，
其中涉及 `AcquireDynamicAttributeSet(...)`、`ReleaseDynamicAttributeSet(...)`、拓扑协调等内容，
现阶段都不再作为执行方案继续推进。
