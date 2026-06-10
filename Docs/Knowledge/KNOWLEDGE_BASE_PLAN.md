---
title: AegisOdyssey Knowledge Base Plan
tags:
  - knowledge-base
  - documentation
  - project-rules
aliases:
  - Knowledge Base Plan
  - 知识库整理方案
---

# AegisOdyssey 知识库整理方案

更新时间：2026-05-19

## 1. 这份文档是干什么的

这份文档不是某一个系统的设计说明，也不是某一次开发交接记录。

它只解决一件事：

当前 `Notice/HistoryNotice` 里已经积累了大量历史笔记，但这些笔记的用途并不相同。后续需要把它们整理成一个**既能给人看，也能给后续 AI 看，而且能长期复用**的知识库。  
因此需要先固定一套整理原则、分类方法和执行顺序，避免后面一边整理一边改标准。

这份文档优先回答下面几个问题：

1. 哪些内容应该进入知识库，哪些不应该直接进入。
2. 历史笔记、知识库、案例库、交接记录之间的边界是什么。
3. `Docs/Knowledge` 后续应该按什么结构扩展。
4. 当前这批历史笔记，第一轮应该先整理哪些。

## 2. 当前判断：不要把历史笔记整包搬进知识库

当前 `Notice/HistoryNotice` 下有 50 篇历史笔记。

基于文件名和少量抽样开头判断，这批笔记至少混合了下面几类内容：

- 引擎机制学习笔记
- 外部项目或样例研究笔记
- 本项目具体系统设计方案
- 阶段性交接说明和当前进度记录
- Bug 排查、根因分析、避坑清单

这些东西不能原封不动地混在一起塞进 `Docs/Knowledge`。

原因很简单：

- 历史笔记强调“当时发生了什么”。
- 知识库强调“现在稳定成立的结论是什么”。
- 交接记录强调“下一个接手的人先看什么”。
- 排查文档强调“遇到问题时先查哪里，不要误判什么”。

所以知识库建设必须采用**提炼式沉淀**，而不是**原文搬运式归档**。

## 3. 文档层级怎么分

后续建议固定为三层：

### 3.1 历史档案层

路径：

- `Notice/HistoryNotice`

作用：

- 保留原始历史笔记
- 保留阶段性交接记录
- 保留当时的排查过程和上下文
- 作为后续知识提炼时的溯源材料

规则：

- 原文尽量不改
- 不要求格式完全统一
- 不要求每篇都做 Obsidian 化改造

### 3.2 知识沉淀层

路径：

- `Docs/Knowledge`

作用：

- 沉淀已经比较稳定、可复用、可导航的项目知识
- 让新会话、新同事、新 AI 能快速找到“当前真实结构”
- 把高频结论、定位入口、设计边界固定下来

规则：

- 只写提炼后的稳定结论
- 尽量不保留大段“当时怎么想”的过程文本
- 每篇文档都应该有明确适用范围和不适用范围
- 优先使用 Obsidian 适合的 frontmatter、tags、wikilinks

### 3.3 案例/排查层

路径建议：

- `Docs/Knowledge/DebugCases`
- 或者各系统包内的 `KNOWN_ISSUES.md`

作用：

- 记录高频误判点
- 记录典型故障的现象、根因、定位顺序
- 把“不要再踩第二次”的经验结构化

规则：

- 不写流水账
- 只保留现象、根因、入口、边界、修复结论

## 4. 当前历史笔记的第一轮分类建议

基于文件名和抽样内容，当前这批笔记适合先分成下面五组。

### 4.1 引擎/底层机制知识

这类内容适合整理成稳定知识卡片：

- `UE反射.md`
- `UE垃圾回收.md`
- `UE默认对象和实例化.md`
- `C++的RTTI.md`
- `GAS ReplicationPolicy分析.md`
- `GAS属性回调问题分析.md`
- `StateTree事件系统笔记.md`
- `自定义GameplayEffectContext完整实现指南.md`
- `UE5智能对象SmartObject从介绍到基础案例实现.md`

建议进入：

- `EngineCore`
- `GameplayFramework`

### 4.2 外部项目/样例研究

这类内容本质上是“参考资料消化”，不一定直接等于本项目事实，但有长期参考价值：

- `Lyra的GAS系统.md`
- `Lyra的GAS系统02.md`
- `Lyra的角色系统.md`
- `Lyra的相机系统.md`
- `Lyra库存系统.md`
- `GASP笔记01.md`
- `GASP笔记02.md`
- `GASP笔记03.md`
- `高级运动系统笔记01.md`
- `高级运动系统笔记02.md`
- `高级运动系统笔记03.md`

建议进入：

- `ReferenceStudies`

注意：

这类文档不应该和“本项目当前实现”混写在一篇里。

### 4.3 本项目系统设计与实现

这类内容是知识库的主体：

- `AI第四阶段战斗决策系统使用与实现说明.md`
- `AI第四阶段战斗决策与攻击欲望设计方案.md`
- `AI战斗决策参数调参与计算说明.md`
- `AI战斗决策系统现行文档说明.md`
- `AI状态树与战斗输入交接文档.md`
- `AI走位与巡逻设计方案.md`
- `技能系统设计方案-技能对象化与槽位装配框架.md`
- `技能系统设计方案-技能执行语义与隐式分类框架.md`
- `技能系统与战斗系统衔接交接总览-2026-05-10.md`
- `战斗系统设计方案-攻击命中统一结算与防御反馈框架.md`
- `交互对象系统工作方案与协作基线.md`
- `交互系统Mutation权限等待与统一调度设计说明.md`
- `联机交互箱子设计方案-玩家共享可见-AI可用-UI与同步策略.md`
- `消耗品与库存右键使用设计方案-现状-方案-实施阶段.md`
- `采集系统设计方案-对象定义-状态驱动-结算与同步框架.md`
- `正式装备栏系统方案锁定说明-2026-05-14.md`

建议拆到这些知识包：

- `AI`
- `SkillSystem`
- `CombatSystem`
- `InteractionSystem`
- `InventoryEquipment`
- `HarvestSystem`

### 4.4 交接/进度/阶段状态

这类内容不建议整篇直接进知识库正文，但非常适合作为提炼来源：

- `AI当前进度与新会话交接说明.md`
- `AI决策系统最新进度交接-属性区间与翻滚扩展.md`
- `AI开发实战案例评估与后续输入策略.md`
- `交互系统当前进度与新会话交接说明.md`
- `交互系统长期记忆与避坑清单.md`
- `技能系统当前进度与遗留BUG交接说明-2026-05-09.md`
- `当前工作进度与项目MAP交接说明-2026-05-12.md`
- `战斗系统UI与目标血条-MVVM改造当前进度与新AI交接说明-2026-05-13.md`
- `战斗系统UI与目标血条和目标侧跳字当前进度与新AI交接说明-2026-05-14.md`
- `战斗系统当前实现与验收说明书-2026-05-11.md`
- `采集系统当前进度与目录规范交接说明-2026-05-12.md`

建议处理方式：

- 不直接搬运
- 优先拆分进 `PROJECT_MAP.md`
- 设计决策进 `DECISIONS.md`
- 遗留问题进 `KNOWN_ISSUES.md`
- 仍保留原文作为历史档案

### 4.5 排查与故障案例

这类内容很适合沉淀为“可复用问题库”：

- `快捷栏数字键切换导致装备动画不播放-GAS Scope Lock问题排查说明.md`
- `技能系统输入映射隐性Bug排查笔记-IMC软引用与RegisterWithSettings误用.md`
- `遗留代码.md`

建议进入：

- 各系统包内的 `KNOWN_ISSUES.md`
- 或 `DebugCases`

## 5. 知识库目录怎么搭

当前 `Docs/Knowledge` 下已经有：

- `StateTreeAI`

而且它已经形成了一套比较对的模式：

- `PROJECT_MAP.md`
- `DECISIONS.md`
- `KNOWN_ISSUES.md`
- `AI_RULES.md`

后续不建议每个主题都重新发明格式，应该复用这套思路。

建议的第一版目录骨架：

- `Docs/Knowledge/EngineCore`
- `Docs/Knowledge/GameplayFramework`
- `Docs/Knowledge/AI`
- `Docs/Knowledge/SkillSystem`
- `Docs/Knowledge/CombatSystem`
- `Docs/Knowledge/InteractionSystem`
- `Docs/Knowledge/InventoryEquipment`
- `Docs/Knowledge/HarvestSystem`
- `Docs/Knowledge/ReferenceStudies`
- `Docs/Knowledge/DebugCases`

其中每个“项目系统包”优先统一这些文档：

- `PROJECT_MAP.md`
- `DECISIONS.md`
- `KNOWN_ISSUES.md`
- `RULES.md`（只有确实需要固定协作边界时再建）

补充原则：

- `PROJECT_MAP.md` 负责“先看哪里、结构边界、运行时真相在哪”
- `DECISIONS.md` 负责“当前已经锁定的设计决定”
- `KNOWN_ISSUES.md` 负责“已知故障、误判点、定位入口”
- `RULES.md` 负责“禁止什么、协作时必须遵守什么”

## 6. 第一轮整理优先级

当前不应该平均用力，而应该先整理最值钱、最常复用、最容易误判的系统。

建议顺序如下：

1. `AI`
2. `SkillSystem`
3. `CombatSystem`
4. `InteractionSystem`
5. `InventoryEquipment`
6. `HarvestSystem`
7. `EngineCore`
8. `ReferenceStudies`

原因：

- AI、技能、战斗相关文档最多，而且互相强耦合。
- 这几块同时也是后续最常继续改、最容易新开会话接手失败的区域。
- 现有 `StateTreeAI` 已经能作为模板，先扩最顺。
- 引擎知识和外部研究虽然重要，但它们不直接决定“项目当前真实结构”。
- `GameplayFramework` 仍然属于项目当前真实结构的一部分，但它内部主题跨度较大，进入这一阶段后应继续按 `GAS / StateTree / SmartObject` 子批次拆开推进，而不是一次性混写。

## 7. 每篇历史笔记进入知识库前怎么判断

不是每篇都值得单独变成一篇知识文档。

建议按下面三个去向判断：

### 7.1 直接提炼成知识文档

满足这些条件时，适合直接提炼：

- 讲的是稳定机制
- 后续大概率会反复查
- 不是只对某一天的阶段状态有效
- 能提炼出明确结论、边界和定位入口

### 7.2 只作为某篇知识文档的来源材料

满足这些条件时，不单独成篇：

- 主要是交接说明
- 含有大量“当时状态”
- 真正有价值的是其中几条结论，而不是整篇原文

### 7.3 只保留为历史档案

满足这些条件时，不建议纳入知识库正文：

- 时效性很强
- 与当前实现已经差异较大
- 过程噪音远多于稳定结论

## 8. Obsidian 化整理要求

后续所有新整理进 `Docs/Knowledge` 的文档，建议统一满足这些要求：

1. 有 frontmatter，至少包含 `title` 和 `tags`
2. 文档开头写清“适用范围”与“不适用范围”
3. 项目内文档之间优先用 `[[wikilinks]]`
4. 尽量避免只写结论不写入口，最好明确“先看哪些代码/文档”
5. 尽量避免把“设计方案”“当前实现”“排查记录”混成一篇

## 9. 当前阶段的实际执行策略

当前最合理的策略不是立即全文通读 50 篇笔记，而是分两轮。

### 第一轮：轻整理

目标：

- 建知识库骨架
- 建分类表
- 建统一模板
- 明确哪些该提炼、哪些只做档案

特点：

- 主要看文件名、开头、结构
- 不要求把每篇细节吃透

### 第二轮：重提炼

目标：

- 逐系统通读高价值文档
- 把交接文档拆成稳定知识
- 建立系统间链接
- 去掉重复表述和冲突结论

特点：

- 以系统为单位推进
- 优先整理 AI、技能、战斗

## 10. 当前已经固定下来的核心原则

后续整理时，默认遵守下面这些原则：

1. `HistoryNotice` 是历史档案，不是最终知识库。
2. `Docs/Knowledge` 只放提炼后的稳定知识，不做简单搬运。
3. 交接文档优先拆分，不整篇迁移。
4. Bug 排查文档优先沉淀“现象、根因、入口、误判点”。
5. 先整理项目真实结构，再整理外部参考学习。
6. 优先复用现有 `StateTreeAI` 的组织方式，不另起一套风格。

## 11. 下一步建议

如果继续推进，下一步最合理的是：

1. 先在 `Docs/Knowledge` 下建立第一版目录骨架。
2. 产出一份“50 篇历史笔记归属表”。
3. 从 `AI`、`SkillSystem`、`CombatSystem` 三组开始做第二轮提炼。

这一步之后，知识库才会真正从“历史文档堆积”变成“可导航、可复用、可交接”的项目知识系统。

## 12. 单轮处理容量约束

为了避免一次读太多原文后出现遗漏、混淆或错误提炼，后续整理默认按“安全批量”推进，而不是按理论上下文上限推进。

当前这批 `HistoryNotice` 文件的体量特征大致是：

- 总数 `50` 篇
- 平均约 `370` 行/篇
- 最长约 `1966` 行
- 超长文档（`801+` 行）共 `4` 篇
- 中等文档（`201-400` 行）最多

基于这批材料，后续默认采用下面的单轮处理规模：

1. 只按文件名做分类时：可以一次处理全部 `50` 篇。
2. 按开头和结构做轻量归类时：单轮控制在 `10-15` 篇普通文档，或 `4-6` 篇超长文档。
3. 深度阅读并提炼稳定结论时：单轮控制在 `2-4` 篇普通文档，或 `1-2` 篇超长文档。
4. 需要横向对比并合并成知识卡时：最好控制在“同一主题 `3-5` 篇”一组。

因此，后续实际工作节奏固定为：

- `轻整理`：约 `10` 篇 / 轮
- `深提炼`：约 `3` 篇 / 轮
- `超长方案文`：约 `1-2` 篇 / 轮

这个约束的目的不是保守，而是保证每一轮都能把结论稳定写回知识库，而不是只停留在上下文记忆里。

## 13. 当前已完成轮次

### 第一轮深提炼：`AI`

已完成文档来源：

- `AI第四阶段战斗决策系统使用与实现说明.md`
- `AI第四阶段战斗决策与攻击欲望设计方案.md`
- `AI战斗决策参数调参与计算说明.md`

已落地知识包：

- `Docs/Knowledge/AI`

本轮整理原则：

- 不是搬运原文
- 先读历史文档
- 再核对当前代码
- 只把仍然成立的内容写入知识库正文
- 历史文档与当前实现冲突的内容，单独写进已知问题/历史偏差

### 第二轮深提炼：`SkillSystem`

已完成文档来源：

- `技能系统设计方案-技能对象化与槽位装配框架.md`
- `技能系统设计方案-技能执行语义与隐式分类框架.md`

已落地知识包：

- `Docs/Knowledge/SkillSystem`

本轮整理原则：

- 仍然不是搬运原文
- 先沉淀对象模型、装配边界、执行结构这些主骨架
- 不把交接、BUG 修复、案例验收、战斗系统衔接混写进同一篇正文
- 后续再按清单继续拆下一轮

### 第三轮深提炼：`CombatSystem`

已完成文档来源：

- `战斗系统设计方案-攻击命中统一结算与防御反馈框架.md`
- `战斗系统当前实现与验收说明书-2026-05-11.md`
- `技能系统与战斗系统衔接交接总览-2026-05-10.md`

已落地知识包：

- `Docs/Knowledge/CombatSystem`

本轮整理原则：

- 只沉淀攻击命中统一结算、防御语义、统一结果消息主链
- 不把目标血条 MVVM 子主题混入战斗主骨架
- 不把火球/火山喷发等技能案例执行细节写成战斗系统正文主体

## 14. 第四轮深提炼补记

本轮已完成：

- `InteractionSystem`

本轮实际提炼来源：

- `交互对象系统工作方案与协作基线.md`
- `交互系统Mutation权限等待与统一调度设计说明.md`
- `联机交互箱子设计方案-玩家共享可见-AI可用-UI与同步策略.md`

本轮已落地知识文档：

- `Docs/Knowledge/InteractionSystem/PROJECT_MAP.md`
- `Docs/Knowledge/InteractionSystem/DECISIONS.md`
- `Docs/Knowledge/InteractionSystem/MUTATION_AND_CONTAINER_SYNC.md`
- `Docs/Knowledge/InteractionSystem/KNOWN_ISSUES.md`

本轮确认的当前真相：

- `InteractionSessionComponent` 已正式承担当前会话、`OwnerOnly` 会话复制、交互对象 owner 获取、Mutation 排队与放行
- `AAOChest` 已是“可交互容器对象 + 观察者会话入口 + IInventoryInterface 提供者”
- `UAOContainerInteractionSessionModel` 已是容器会话数据层，而不是纯计划中的抽象
- 容器详细内容当前走会话快照同步，不是默认广播给所有相关客户端

按既定顺序，下一轮继续：

1. `InventoryEquipment`
2. `HarvestSystem`
3. `GameplayFramework`
4. `EngineCore`
5. `ReferenceStudies`

## 15. 第五轮深提炼补记

本轮已完成：

- `InventoryEquipment`

本轮实际提炼来源：

- `正式装备栏系统方案锁定说明-2026-05-14.md`
- `消耗品与库存右键使用设计方案-现状-方案-实施阶段.md`
- `快捷栏数字键切换导致装备动画不播放-GAS Scope Lock问题排查说明.md`

本轮已落地知识文档：

- `Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md`
- `Docs/Knowledge/InventoryEquipment/DECISIONS.md`
- `Docs/Knowledge/InventoryEquipment/FORMAL_EQUIPMENT_AND_INVENTORY_USE.md`
- `Docs/Knowledge/InventoryEquipment/KNOWN_ISSUES.md`

本轮确认的当前真相：

- 统一入包当前继续以 `UAOInventoryStatics::TryAddInventoryBatchToActor(...)` 为主入口，玩家主接收容器是 `UAOBackPackComponent`
- 获取物品通知当前只对真正进入背包的结果广播，并已走通 `InventoryMessageSubsystem -> HUDViewModelComponent -> MVVM_HUD -> AOMainUI`
- 默认库存右键“使用”当前已经形成统一入口，但具体业务由实例类型分流：消耗品走 `AOFragment_Consumable`，正式装备走 `FormalEquipmentManager`
- `UAOFormalEquipmentManagerComponent` 已是正式装备长期穿戴真相层，`UAOFormalEquipmentSlotInventoryComponent` 只是五槽库存投影
- 正式装备属性授予当前已经统一走 `Definition.AbilitySetsToGrant`，`AOFragment_FormalEquipment` 只负责声明 `FormalSlotType`
- 历史 Scope Lock 排查对应的“现授现用能力扫描”风险，不应再直接写成当前装备动画主链事实；当前 `TryPlayEquipmentAnimation(...)` 已改成事件触发路径

按既定顺序，下一轮继续：

1. `HarvestSystem`
2. `GameplayFramework`
3. `EngineCore`
4. `ReferenceStudies`

## 16. 第六轮深提炼补记

本轮已完成：

- `HarvestSystem`

本轮实际提炼来源：

- `采集系统当前进度与目录规范交接说明-2026-05-12.md`
- `采集系统设计方案-对象定义-状态驱动-结算与同步框架.md`

本轮已落地知识文档：

- `Docs/Knowledge/HarvestSystem/PROJECT_MAP.md`
- `Docs/Knowledge/HarvestSystem/DECISIONS.md`
- `Docs/Knowledge/HarvestSystem/OBJECTS_AND_RESOLUTION.md`
- `Docs/Knowledge/HarvestSystem/KNOWN_ISSUES.md`

本轮确认的当前真相：

- 采集当前正式主链已经落成 `STT_PlayHarvest -> GA_Harvest -> HarvestWindow -> Tool Socket Trace -> HarvestResolver -> HarvestableComponent -> InventoryStatics -> BackPack`
- 正式采集目标当前只在命中窗内按本次工具挥击真实命中结果解析，不由状态树提前指定
- `HarvestToolDefinition / HarvestToolFragment / HarvestToolProfile / HarvestToolInstance` 已经形成正式工具层，不再只是历史方案边界
- `HarvestableDefinition` 负责静态定义，`HarvestableComponent` 负责 `CurrentProgress / bDepleted / bRespawnPending` 这些运行时真相
- `HarvestResolver` 当前只做统一重判定与统一结算，不直接改背包，也不直接持有节点真相
- 采集奖励当前先检查能否完整入包，再统一走 `InventoryStatics`，不能完整接收就整次失败
- `AOHarvestableTree` 已经是树节点专用子类，树倒下当前只是对象子类行为，不是公共采集系统唯一耗尽语义
- Harvest 目录当前已经真实收口为 `Core / Definition / Fragments / Items / Abilities / StateTree / System / Cue / Nodes`

按既定顺序，下一轮继续：

1. `GameplayFramework`
2. `EngineCore`
3. `ReferenceStudies`

## 17. 第七轮深提炼补记（`GameplayFramework` 第一子批次）

本轮已完成：

- `GameplayFramework`

本轮实际提炼来源：

- `GAS ReplicationPolicy分析.md`
- `GAS属性回调问题分析.md`
- `自定义GameplayEffectContext完整实现指南.md`

本轮已落地知识文档：

- `Docs/Knowledge/GameplayFramework/PROJECT_MAP.md`
- `Docs/Knowledge/GameplayFramework/DECISIONS.md`
- `Docs/Knowledge/GameplayFramework/GAS_REPLICATION_AND_EFFECT_CONTEXT.md`
- `Docs/Knowledge/GameplayFramework/KNOWN_ISSUES.md`

本轮确认的当前真相：

- `GameplayAbility ReplicationPolicy` 当前只应理解为 Ability 实例复制策略边界，不能直接推导为 Ability 内普通 C++ 成员自动同步
- `GA_Sprint` 当前虽设置了 `ReplicationPolicy = ReplicateYes`，但 `SprintSpeedBonusAmount / SprintCost / bVigorExhaustedBroadcasted` 这些成员并没有因而自动成为已验证的复制字段
- 项目已经在 `DefaultGame.ini` 正式注册 `UAOAbilitySystemGlobals`，`AllocGameplayEffectContext()` 当前统一分配 `FAOGameplayEffectContext`
- `FAOGameplayEffectContext` 当前已被真实接入 `CombatManager -> ExecCal_Damage -> AOHealthAttributeSet` 这条战斗结算链，承载暴击、格挡、招架、攻击标签、武器标签、伤害类型、命中结果等统一上下文
- `AOHealthAttributeSet::PostGameplayEffectExecute(...)` 当前直接 `check` 自定义 `EffectContext`，说明这套上下文基础设施已经是项目正式前提
- 客户端属性复制后的 UI/业务通知，当前项目稳定模式是 `OnRep_* + GAMEPLAYATTRIBUTE_REPNOTIFY + 自定义 Broadcast`，而不是只依赖引擎自动变化回调
- 历史文档里“Health 正常但 Vigor 客户端回调失败”的结论，经过当前代码核对后已不再是项目现状；`OnRep_Vigor / OnRep_MaxVigor` 现在都已补齐广播路径

按既定顺序，下一轮继续：

1. `GameplayFramework` 的下一子批次
2. `EngineCore`
3. `ReferenceStudies`

说明：

这段是第七轮结束时的阶段性记录。  
后续推进结果已由第 18 节继续接续并更新。

## 18. 第八轮深提炼补记（`GameplayFramework` 第二子批次）

本轮已完成：

- `GameplayFramework`

本轮实际提炼来源：

- `StateTree事件系统笔记.md`
- `UE5智能对象SmartObject从介绍到基础案例实现.md`

本轮已落地知识文档：

- `Docs/Knowledge/GameplayFramework/PROJECT_MAP.md`
- `Docs/Knowledge/GameplayFramework/DECISIONS.md`
- `Docs/Knowledge/GameplayFramework/STATETREE_EVENTS_AND_SMARTOBJECT_BOUNDARY.md`
- `Docs/Knowledge/GameplayFramework/KNOWN_ISSUES.md`

本轮确认的当前真相：

- `StateTree` 事件注入链在当前项目里已经正式存在：`HeroComponent / InputBufferComponent -> CombatStateTree Component -> FStateTreeEvent -> SendStateTreeEvent`
- 当前 `StateTree` 事件语义是分层承载的：输入标签走 `FStateTreeEvent.Tag`，输入类型走 `FCombatStateTreeInputEvent::InputType`
- `FCombatStateTreeInputEvent` 当前并不保存输入标签本身，因此不能把它误写成“完整 Tag + InputType 负载”
- `UAOStateTreeComponentBase`、`UAOAILogicStateTreeComponentBase`、动态加组件后的 `RestartLogic()` 补偿链，以及 `FSTT_MoveToLocation` 这些基础设施已经是项目后续扩展框架的真实前提
- `SmartObject` 当前仍未进入工程：`.uproject` 未启用相关插件，`Build.cs` 未引入相关模块，源码和内容资产里也没有正式接线
- 因此历史 `SmartObject` 文档当前只能沉淀为“框架研究和候选接线方向”，不能写成项目现状

按既定顺序，下一轮继续：

1. `EngineCore`
2. `ReferenceStudies`

说明：

到这一轮为止，`GameplayFramework` 在首轮深提炼里的五篇来源文档已经全部处理完。  
后续如果再回到这个包，应以“新增子主题”或“当前代码发生实质变化”为触发条件，而不是重复合并已有结论。
## 19. �ھ������������ǣ�`EngineCore`��

��������ɣ�

- `EngineCore`

����ʵ��������Դ��

- `C++��RTTI.md`
- `UE����.md`
- `UE��������.md`
- `UEĬ�϶����ʵ����.md`

���������֪ʶ�ĵ���

- `Docs/Knowledge/EngineCore/PROJECT_MAP.md`
- `Docs/Knowledge/EngineCore/DECISIONS.md`
- `Docs/Knowledge/EngineCore/UE_OBJECT_MODEL_REFLECTION_GC_AND_INSTANCING.md`
- `Docs/Knowledge/EngineCore/KNOWN_ISSUES.md`

��������ԭ��

- ������ƪ��ʷ����ֱ�ӵ��ɽ������ġ�
- �ȶ���ʷ���³� claim��
- �ٻص� UE5.6 ����Դ��˶� `Cast<> / UClass / UHT / GC / CDO / FObjectInitializer` ����ʵ��㡣
- ��Ҫ�ٻص�ǰ���̺˶���Щ��������Ŀ�����ʵ�÷���

����ȷ�ϵĵ�ǰ���ࣺ

- ��ǰ��Ŀ���� `UObject` ����ʱ����ʱ��Ӧ���Ȱ� UE �Լ�������ϵͳ���⣺`UClass / StaticClass / IsA / Cast<> / ExactCast<>`�������ǰѱ�׼ C++ RTTI ֱ�ӵ������¡�
- UE ������ȶ�����Ӧ�� `Դ���� -> UHT ���ɴ��� -> ����ʱԪ����ע��`��`UCLASS / USTRUCT / UPROPERTY / UFUNCTION` ����ڱ�ǣ����Ƿ��䱾�塣
- UE GC �ĺ����ǿɴ��Է�����`UPROPERTY` �Ĺؼ������������ý���ɴ��Ա����������ڰ�Ŀ�����ֱ�ӱ�� root��
- CDO �Ǵ� `RF_ClassDefaultObject` ����ʵ `UObject`���������������ȶ�����Ϊ `StaticConstructObject_Internal -> StaticAllocateObject -> ClassConstructor(FObjectInitializer) -> PostConstructInit -> InitProperties`��
- `DefaultToInstanced / EditInlineNew` ���۵��� instanced �Ӷ���ģ�ͣ������ڡ������û��Ĭ�϶��󡱡�
- ��ǰ�����Ѿ���ʵ�������� instanced �Ӷ���ģʽ��`InventoryItemFragment`��`EquipmentFeatureAction`��`HarvestToolFragment`������ Skill/Definition ��������ʽʹ�á�

���ֶ���ʶ�������Ŀ�����е㣺

- ��Ŀ����� `ItemCDO / GetItemCDO()` ��������������ǰʵ�ֲ�������ֱ�ӷ������������ϵ� `ClassDefaultObject`��
- ��˺���������桢װ�����ɼ������ܶ�����ĵ�ʱ��������ʽ���֣�
  - ����ԭ�� CDO
  - ��Ŀҵ�����ﻯ���� ��CDO�� �Ķ���ģ�����

���ȶ�˳����һ�ּ�����

1. `ReferenceStudies`

˵����
��һ�ֵ��ص㲻�ǡ���һƪ����ѧϰ�ʼǡ������ǰ���ƪ���з������ʷ�����Ͳ��Ͻ�˵���ĵײ����£��������ܳ��ڸ��õ���Ŀ������ʶ��  
����һ��Ϊֹ��������������� `EngineCore` �Ѿ��տڡ���������ٻص��������Ӧ�ԡ������ײ����⡱�򡰵�ǰ�����ڶ���ģ���ϵ�ʵ�ʱ仯��Ϊ�����������������ظ������ɽ��ۡ�

## 20. 第十轮深提炼补记（`ReferenceStudies` 第一批：`GASP笔记01-03`）

本轮已完成：

- `ReferenceStudies`

本轮实际提炼来源：

- `GASP笔记01.md`
- `GASP笔记02.md`
- `GASP笔记03.md`

本轮已落地知识文档：

- `Docs/Knowledge/ReferenceStudies/PROJECT_MAP.md`
- `Docs/Knowledge/ReferenceStudies/DECISIONS.md`
- `Docs/Knowledge/ReferenceStudies/GASP_LOCOMOTION_MOTION_MATCHING_AND_TRAVERSAL.md`
- `Docs/Knowledge/ReferenceStudies/KNOWN_ISSUES.md`

本轮整理原则：

- 不把三篇 GASP 历史笔记直接迁成“项目当前实现说明”。
- 先从历史笔记里抽出 motion matching、trajectory、traversal、motion warping、offset root bone、foot placement 这些主题。
- 再回到 UE5.6 `PoseSearch / MotionWarping / AnimationWarping` 插件源码核对稳定机制边界。
- 最后再回当前工程核对插件、模块、配置、Tag、资产和 `Source` 层调用入口。

本轮确认的当前真相：

- 这三篇文档的长期价值在于“外部 locomotion 样例的可拆机制图谱”，不在于把 GASP 样例直接当成 `AegisOdyssey` 当前现状。
- Motion Matching 更稳妥的稳定理解应回到 `Schema / Database / Query Trajectory / Pose History / AnimNode_MotionMatching` 这一组机制，而不是某一张蓝图截图。
- Traversal 更稳妥的稳定理解应回到 `环境检测 -> 中间结果结构 -> 动画选择 -> Motion Warping 执行` 这条结果驱动链。
- `MotionWarping`、`OffsetRootBone`、`FootPlacement` 都应视为支撑修正层，而不是 motion matching 主选择层本身。
- 当前工程确实已经启用 `PoseSearch`、`MotionWarping`、`AnimationWarping`、`MotionTrajectory`、`Chooser` 等插件，也已经存在 `MotionMatchingData`、`Traversal`、`Chooser` 相关资产与配置。
- 但当前 `Source` 层还没有检出明确的 `PoseSearch / MotionMatching / FootPlacement / OffsetRootBone / MotionWarpingComponent` 直接消费入口，因此不能直接写成“当前项目已正式采用整套 GASP locomotion 主链”。

本轮新增锁定的整理边界：

- `ReferenceStudies` 负责保留“可借鉴机制”，不负责宣告“外部样例现状即项目现状”。
- 后续如果要吸收这套体系，应优先按 `Trajectory`、`Schema/Database`、`Traversal 结果结构`、`Motion Warping`、`Grounding 支撑层` 这种机制粒度分批迁移，而不是整套蓝图照搬。

按既定顺序，下一轮继续：

1. `ReferenceStudies`

说明：
到这一轮为止，`ReferenceStudies` 已经完成第一批 GASP 文档的深提炼。后续继续处理这一包时，应按计划推进剩余 `高级运动系统`、`Lyra` 等外部研究材料，并继续维持“外部参考”与“项目现状”分包。

## 2026-05-19 ReferenceStudies 第二轮：高级动画层 / 距离匹配 / Warping
- 处理来源：`Notice/HistoryNotice/高级运动系统笔记01.md`、`高级运动系统笔记02.md`、`高级运动系统笔记03.md`
- 处理策略：继续留在 `ReferenceStudies`，不升格为当前项目动画系统真相包。
- 本轮锁定主题：`Animation Layer Interface`、`LinkAnimClassLayers(...)`、`Sequence Evaluator`、Distance Matching、`Stride Warping`、`Orientation Warping`、`MotionExtractorModifier`、`Rotate Root Bone`、Turn In Place、Start / Stop / Pivot、Sync Groups。
- 本轮新增主题笔记：`Docs/Knowledge/ReferenceStudies/ADVANCED_ANIMATION_LAYERS_DISTANCE_MATCHING_AND_WARPING.md`
- 本轮更新配套：`Docs/Knowledge/ReferenceStudies/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 当前项目核对结论：已确认源码层存在 crouch 与 camera crouch offset 处理；尚未在 `Source` 层检出 `LinkAnimClassLayers`、Distance Matching、Stride / Orientation Warping、Rotate Root Bone、Turn In Place 等明确入口，因此不能写成当前项目正式动画主链。

## 2026-05-19 ReferenceStudies 第三轮：Lyra 模块化角色与 GAS 接线
- 处理来源：`Notice/HistoryNotice/Lyra的GAS系统.md`
- 处理来源：`Notice/HistoryNotice/Lyra的GAS系统02.md`
- 处理来源：`Notice/HistoryNotice/Lyra的角色系统.md`
- 归档位置：`Docs/Knowledge/ReferenceStudies/LYRA_MODULAR_CHARACTER_AND_GAS_WIRING.md`
- 配套更新：`Docs/Knowledge/ReferenceStudies/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：继续留在 `ReferenceStudies`，不把 Lyra 学习笔记直接提升为当前项目设计真相。
- 本轮锁定结论：Lyra 的重点应理解为模块化角色框架下的 GAS 接线模式，包括 `PlayerState` 持久 ASC、`Pawn/Character` 作为 `AvatarActor`、`PawnExtension/Hero` 初始化链、GameFeature 扩展事件注入、`AbilitySet` 授予回收边界，以及 `InputAction -> InputTag -> ProcessAbilityInput` 输入链。
- 当前项目核对结论：已明确存在 `UAOExtPawnComponent + UAOHeroComponent` 初始化链、`AAOPlayerState/AAOCharacter::NAME_AOAbilityReady` 扩展事件、`GF_AddAbilities` 运行时授予、`UAOAbilitySet` 授予回收、`UAOAbilitySystem` 输入 Tag 输入泵，以及 `AOHealth/Combat/PrimaryAttributeSet` 的复制与 GE 生命周期处理；因此可以确认项目采用了多处 Lyra 风格对应实现，但仍不应写成“项目完全等同于 Lyra”。

## 2026-05-19 ReferenceStudies 第四轮：Lyra 相机系统 / Lyra 库存系统
- 处理来源：`Notice/HistoryNotice/Lyra的相机系统.md`
- 处理来源：`Notice/HistoryNotice/Lyra库存系统.md`
- 归档位置：`Docs/Knowledge/ReferenceStudies/LYRA_CAMERA_MODE_STACK_AND_THIRD_PERSON_PENETRATION_AVOIDANCE.md`
- 归档位置：`Docs/Knowledge/ReferenceStudies/LYRA_INVENTORY_DEFINITION_FRAGMENT_INSTANCE_AND_EQUIPMENT.md`
- 配套更新：`Docs/Knowledge/ReferenceStudies/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：继续按 `ReferenceStudies` 收束为外部样例研究，不把 Lyra 学习笔记直接提升为当前项目现状说明。
- 相机方向锁定结论：Lyra 相机更稳妥的骨架应理解为 `PlayerCameraManager + CameraComponent + CameraMode + CameraModeStack`，其核心不是单镜头硬切，而是模式栈视图混合；第三人称稳定性重点是多 `feeler` 参与的预测式防穿模，而不是把它简化成 SpringArm 自动碰撞。
- 相机方向当前项目核对结论：已核对 `UAOCameraComponent`、`UAOCameraModeStack`、`UAOCameraMode_ThirdPerson`、`UAOHeroComponent::DetermineCameraMode()`、`AAOPlayerCameraManager` 等源码入口，确认项目已采用相机模式栈、默认相机模式委托与第三人称防穿模骨架；但没有把 Lyra 的 UI Camera 优先层明确照搬为当前项目主链。
- 库存/装备方向锁定结论：Lyra 库存更稳妥的骨架应理解为 `Definition + Fragment + Instance + FastArray`；Equipment 更适合被理解为建立在库存骨架上的上层系统，而不是完全独立的另一套物品系统。
- 库存/装备方向当前项目核对结论：已核对 `UAOInventoryItemDefinition`、`UAOInventoryItemInstance`、`FAOInventoryList`、`UAOEquipmentDefinition`、`UAOEquipmentInstance`、`UAOWeaponManagerComponent`、`UAOFormalEquipmentManagerComponent`、`UAOFormalEquipmentSlotInventoryComponent` 等源码入口，确认项目保留了 Lyra 风格底层骨架，但装备、正式装备槽、FeatureAction 与技能来源物已经形成明显项目化分层；另外必须继续显式区分项目内 `ItemCDO` 命名与引擎原生 CDO 语义。

## 2026-05-19 AI 第二轮：战斗输入/旋转/翻滚执行链 + Reposition/Patrol 框架
- 处理来源：`Notice/HistoryNotice/AI状态树与战斗输入交接文档.md`
- 处理来源：`Notice/HistoryNotice/AI走位与巡逻设计方案.md`
- 处理来源：`Notice/HistoryNotice/AI决策系统最新进度交接-属性区间与翻滚扩展.md`
- 归档位置：`Docs/Knowledge/AI/COMBAT_INPUT_ROTATION_AND_ROLL_EXECUTION.md`
- 归档位置：`Docs/Knowledge/AI/REPOSITION_AND_PATROL_FRAMEWORK.md`
- 配套更新：`Docs/Knowledge/AI/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：深提炼，优先以当前源码为准，把历史交接里的“当前事实”“仍属方案”“已经过时的判断”拆开。
- 战斗输入/旋转方向锁定结论：AI 战斗执行继续默认走 `UAOHeroComponent::InjectAbilityInputCommand(...)` 统一输入桥接；Bot 注入输入后会主动补一次 `ProcessAbilityInput(...)`；`STT_SendCombatCommand` 代表一次性输入，`STT_PulseCombatCommand` 代表持续输入脉冲，`STT_RotateControlTowardTarget` 独立负责控制朝向；`GA_LightAttack`、`GA_Block`、`GA_Roll` 与 `AT_WaitRotateToDirection` 当前都以 `AController::GetControlRotation()` 为方向语义入口。
- 走位/巡逻方向锁定结论：`Reposition` 与 `Patrol` 仍应按“选点 + 移动 + 朝向”三层理解；当前源码已存在 `STT_RunEQSSelectLocation`、`STT_MoveToLocation`、`AOEnvQueryContext_PatrolAnchor`、`AAOAIPlayerBotController` 的 `PatrolAnchorLocation / PatrolTargetLocation`，因此历史方案里“这些还未正式实现”的说法已经过时；但完整高层 `Reposition / Patrol` 状态组织是否在所有敌人资产上成熟接通，仍需继续按资产和运行时链核对。

## 2026-05-19 AI 第三轮：现行导航校准 + 协作任务卡/短回合协议
- 处理来源：`Notice/HistoryNotice/AI当前进度与新会话交接说明.md`
- 处理来源：`Notice/HistoryNotice/AI战斗决策系统现行文档说明.md`
- 处理来源：`Notice/HistoryNotice/AI开发实战案例评估与后续输入策略.md`
- 归档位置：`Docs/Knowledge/AI/COLLABORATION_TASK_CARD_AND_SHORT_ROUND_PROTOCOL.md`
- 配套更新：`Docs/Knowledge/AI/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：深提炼；把交接/导航/过程文拆成“当前正文入口”“历史偏差提醒”“可复用协作协议”，不把原文整篇直接搬进知识库。
- 锁定结论一：`AI当前进度与新会话交接说明.md` 与 `AI战斗决策系统现行文档说明.md` 继续保留为历史线索，但当前接手顺序和现行正文入口应以 `PROJECT_MAP / COMBAT_DECISION_TUNING / DECISIONS / KNOWN_ISSUES` 为准；其中旧文里的 `IdealAttackDistance`、默认 `Attack / Strafe` 自动补全等说法不能再直接当当前规格。
- 锁定结论二：`AI开发实战案例评估与后续输入策略.md` 不应被写成 AI 运行时事实，更适合沉淀为协作方法资产；当前可稳定复用的协议是“第一轮任务卡 + 后续短回合（分析 -> 收窄 -> 实施 -> 验证 -> 沉淀）+ 最小必要链路阅读 + 结果回灌知识资产”。

## 2026-05-19 Collaboration 第一轮：交接文档边界治理 + 跨系统协作协议抽离
- 处理来源：上一轮从 `AI开发实战案例评估与后续输入策略.md` 提炼出的协作协议，以及当前知识库中已出现的交接文档边界判断。
- 归档位置：`Docs/Knowledge/Collaboration/TASK_CARD_AND_SHORT_ROUND_PROTOCOL.md`
- 归档位置：`Docs/Knowledge/Collaboration/HANDOFF_DOCUMENT_BOUNDARY_AND_EXTRACTION_RULES.md`
- 配套更新：`Docs/Knowledge/AI/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：把原先误放在 `AI` 包内的跨系统协作方法抽离到新的 `Collaboration` 位置，并补一篇“交接文档边界与拆分规则”，避免后续再把交接/过程/方法文混进单一系统正文。
- 锁定结论一：凡是“先看什么”“下一轮怎么接”“后续输入怎么组织”“任务怎么分回合推进”这类内容，默认优先归入 `Docs/Knowledge/Collaboration`，而不是继续留在某个系统包里。
- 锁定结论二：交接文档的完整原文继续留在 `Notice/HistoryNotice`，知识库正文只吃拆分后的稳定结构、锁定边界、误判点和可复用协作协议。

## 2026-05-19 CombatSystem 第二轮：战斗UI / 世界血条 / 目标侧跳字
- 处理来源：`Notice/HistoryNotice/战斗系统UI与目标血条-MVVM改造当前进度与新AI交接说明-2026-05-13.md`
- 处理来源：`Notice/HistoryNotice/战斗系统UI与目标血条和目标侧跳字当前进度与新AI交接说明-2026-05-14.md`
- 归档位置：`Docs/Knowledge/CombatSystem/COMBAT_UI_WORLD_HEALTH_BAR_AND_FLOATING_TEXT.md`
- 配套更新：`Docs/Knowledge/CombatSystem/PROJECT_MAP.md`、`DECISIONS.md`、`KNOWN_ISSUES.md`
- 处理方式：深提炼；把这两篇交接文收束为战斗结果观察与表现下游子主题，不重新混回战斗主链骨架。
- 锁定结论一：当前战斗 UI 主链已稳定为 `FAOCombatResultMessage -> UAOCombatMessageSubsystem -> UAOHUDViewModelComponent -> FAOCombatFeedbackViewData -> UMVVM_HUD/CombatFeedbackFeed`，UI 不再自己重解释结算真相；消息总线当前是 native multicast，客户端回放只走 `BroadcastCombatResultLocal(...)`。
- 锁定结论二：目标侧当前已明确拆成三类职责：`UAOLocalTargetHealthBarObserverComponent` 管本地观察资格，`UAOTargetHealthBarComponent` 管目标血量与头顶血条，`UAOCombatFloatingTextComponent` 管目标侧世界跳字；后续不能再把血条、观察、跳字重新揉成一类组件职责。

### 已完成新一轮深提炼（补齐剩余交接文档）
- `SkillSystem`
- `Collaboration`
- `StateTreeAI`

本轮实际提炼来源：
- `技能系统当前进度与遗留BUG交接说明-2026-05-09.md`
- `当前工作进度与项目MAP交接说明-2026-05-12.md`

本轮已落地知识文档：
- `Docs/Knowledge/SkillSystem/SESSION_HANDOFF_AND_PENDING_BUG_BOUNDARIES.md`
- `Docs/Knowledge/Collaboration/MAP_HANDOFF_EXTRACTION_AND_MANUAL_STYLE_RULES.md`
- `Docs/Knowledge/StateTreeAI/SKILL_INPUT_EVENT_AND_DUPLICATE_CONSUMPTION_RISKS.md`

注意：
这一轮没有把两篇交接文整篇搬进知识库，而是只提炼出当前仍然成立的稳定入口、跨系统规则和遗留问题边界。  
对 `技能系统当前进度与遗留BUG交接说明-2026-05-09.md`，最终锁定的当前事实主要是：

- `SkillSystem` 当前已经稳定分成 `InjectSkillSlotInputCommand*` 和 `ExecuteSkillSlotCommand*` 两条正式入口链。
- `StateTree` 直接触发技能槽命令已经正式落在 `STT_TriggerSkillSlotCommand -> ExecuteSkillSlotCommandByIndex(...)`，不再是方案层讨论。
- `SkillSlotInventoryComponent` 当前不仅是服务端投影层，客户端本地也要先补齐槽位壳，才能让统一库存交换主链识别合法技能槽落点。
- `AOSkillBarUI / AOSkillSlotUI` 当前已经有显式的 `ObservedSlotIndex` 与正式 C++ 落点入口，后续不能再把“技能槽落点身份”理解成纯蓝图随意推导。
- 历史交接文里记录过的 `ObservedPressedInputTags / ShouldForwardStateTreeInputEvent(...)` 那轮输入去重修法，当前源码里并不存在，因此只能保留为历史排查线索，不能写成现行机制。

对 `当前工作进度与项目MAP交接说明-2026-05-12.md`，最终锁定的长期有效结论主要是：

- 这类跨系统 MAP 交接文的长期价值主要在“先看哪里”“怎么拆系统边界”“说明书式文档怎么写”，而不在阶段进度本身。
- 凡是“先看什么”“下一轮怎么接”“接入说明应该回答哪些问题”这类内容，更适合沉淀到 `Docs/Knowledge/Collaboration`，而不是继续挂在某个单系统包里。
- 说明书式文档后续至少应回答：在哪里配、为什么这样配、不这样配会看到什么、结果不对时第一步先查哪里。
- `Inventory` 只提供使用入口，物品是否可用、如何使用、使用后怎么消耗仍应继续由物品实例自己决定；这条边界继续作为库存/装备类文档的长期约束。
- `Harvest / Combat / StateTree` 这几类“动画时序驱动的玩法链”后续继续整理时，应优先按主链状态持有者、事件发起者、消费者三层拆，而不是围着表象补丁推进。
