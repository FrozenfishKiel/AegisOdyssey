---
title: Debug Give Item Command Design 2026-05-28
date: 2026-05-28
tags:
  - knowledge
  - inventory-equipment
  - debug
  - command
  - multiplayer
status: proposed
---

# 调试发货指令方案

这份方案只讨论一件事：给当前项目补一条开发期调试指令，用来把指定 `ItemId` 的物品按数量发到某个对象身上的库存里，效果类似 `Minecraft /give`。

这次不写代码，只锁方案。

而且这条方案不是只为了 AI。
它的目标从一开始就定成“对对象生效”，所以玩家、AI、以及后面任何真正挂了库存组件的 Actor，都应该走同一条指令主链。

## 这次到底要解决什么问题

当前 AI 自主使用库存的链路已经开始成形，但测试时有一个非常现实的问题：

AI 会不会用库存，和 AI 一开始有没有库存，是两件完全不同的事。

如果每次为了测一个库存动作，都要手工进编辑器、临时摆物品、或者绕一圈别的玩法系统给目标塞物品，测试成本就会很高，而且很容易把“AI 决策问题”和“测试准备问题”混在一起。

所以这里需要一条开发期调试能力，专门解决“如何稳定、低成本地往指定对象身上发物品”这个前置问题。

这条能力的目标非常克制：

1. 输入 `ItemId`
2. 输入数量
3. 可选输入目标对象
4. 最终发起一次正式入包请求

它不应该顺手变成新库存系统，也不应该去替库存组件兜底包满、非法槽位、或者特殊来源逻辑。

## 这条能力的边界先锁清楚

### 1. 它是调试命令，不是新玩法系统

这条能力当前的定位是开发与测试期调试命令。

它的职责只有一层：

把“控制台输入”翻译成“一次正式入包请求”。

它不承担：

- 新的掉落系统职责
- 新的奖励结算职责
- 新的制造发货职责
- 新的 GM 权限系统职责

它更不应该变成一条独立的第二入包主链。

### 2. 它只认 `ItemId`，不认名字

这件事和当前项目物品身份边界一致。

当前项目里，物品身份统一收口在 `ItemId -> ItemDefinitionClass` 这条链上，入口是：

- [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
- [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)
- [AOItemCatalogTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Items/AOItemCatalogTypes.h)

所以这条调试命令也必须严格复用这条边界：

- 输入只认 `ItemId`
- 命令层自己不维护第二份映射
- 查表失败就直接失败

### 3. 它只对 Actor 目标生效，不扩到任意 UObject

虽然你的口语需求里说的是“对象”，但当前项目正式统一入包入口本来就是：

- `UAOInventoryStatics::TryAddInventoryBatchToActor(AActor* TargetActor, ...)`

也就是：

- [AOInventoryStatics.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.h)
- [AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)

所以这条命令在现阶段最准确的边界，不是“任意 UObject”，而是“任意运行时 Actor 目标”。

这已经覆盖了你当前最关心的目标：

- 玩家 Pawn
- AI Pawn
- 未来任何真正挂了库存组件的容器型 Actor

这个边界既不缩水，也不乱扩。

## 为什么推荐挂在 `PlayerController Exec`，而不是另起一套调试系统

这里有三个理论选择：

1. `PlayerController Exec + Server RPC + 统一入包入口`
2. `CheatManager` 调试命令体系
3. `WorldSubsystem / DebugManager` 全局调试体系

当前最合适的是第一个。

原因不是“它最优雅”，而是“它最符合项目现状”。

项目已经有现成的调试命令入口：

- [AOPlayerController.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h)
- [AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)

其中已经有一个 `UFUNCTION(Exec)`：

- `CraftRecipe(FName RecipeRowName)`

同时项目也已经有很明确的“客户端输入，服务端权威处理”模式，例如：

- `Server_ExecuteInteractionRequest(...)`

这说明当前工程对“调试控制台入口挂 PlayerController，真正权威处理落在服务端”这件事，是有现成土壤的。

如果这时候为了一个 `/give` 命令，先搭一套新的 `CheatManager` 或调试子系统，反而是在把一件本来很清楚的事情做复杂。

所以这里不追求调试基础设施的宏大统一，优先追求：

- 快速可接入
- 与现有结构一致
- 联机权威边界清楚

## 推荐的命令形态

建议命令名不要叫 `Give`，而叫：

- `GiveItem`

原因很简单：

- `Give` 过于宽泛
- 后面很可能还会有别的调试发放语义
- `GiveItem` 一眼就知道是物品发放

推荐命令形态如下：

```text
GiveItem <ItemId> <Count> [TargetActorNameOrPath]
```

例如：

```text
GiveItem 1001 3
GiveItem 1001 3 BP_Enemy_Test_C_0
GiveItem 1001 3 /Game/Levels/TestMap.TestMap:PersistentLevel.BP_Enemy_Test_C_0
```

这条语法的关键点有三个：

1. `ItemId` 必填
2. `Count` 必填
3. 目标对象可选

如果不传目标对象，默认目标就是当前控制对象，也就是当前 `PlayerController` 的 `GetPawn()`。

这条默认规则非常重要，因为它保证了最常见测试路径足够短：

- 给自己发货，不需要再写对象参数
- 给 AI 发货，才额外写目标对象

## 目标对象解析规则

这部分是方案里最容易写松的地方，所以要锁得很明确。

你已经选定了目标口径：

- 默认给当前控制对象
- 允许显式传 `ActorName` 或路径

基于这个口径，我建议把服务端目标解析规则收成两段式。

### 第一段：优先按显式路径解析

如果第三个参数看起来像完整对象路径，就优先按路径解析。

这样做的好处是：

- 精确
- 可重复
- 不受重名 Actor 影响

它更适合稳定测试脚本和文档化验收。

### 第二段：路径失败后，再按当前 World 中的 Actor 名精确匹配

如果不是路径，或者路径解析失败，再按当前 World 里的 Actor 名做精确匹配。

这里有两个硬约束建议直接锁死：

1. 不做模糊匹配
2. 找到多个同名 Actor 直接失败

原因很实际。

调试命令最怕的不是“失败”，而是“误命中”。
宁可名字不唯一时报错，也不要为了方便去做 contains、前缀匹配、模糊匹配，然后把物品发错对象。

所以目标解析的正确口径应该是：

- 路径命中，直接用
- 名字唯一精确命中，才用
- 找不到，失败
- 找到多个，失败

## 物品解析规则

物品解析必须继续严格站在当前项目已经锁好的身份边界上。

当前总表行结构是：

- `FAOItemCatalogRow`

当前核心字段是：

- `ItemId`
- `ItemDefinitionClass`

对应位置：

- [AOItemCatalogTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Items/AOItemCatalogTypes.h)

运行时解析入口是：

- `UAOGameData::FindItemCatalogRowById(int32 ItemId)`

对应位置：

- [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)

所以调试命令的物品解析规则应该明确写成：

1. 用 `ItemId` 查 `AOGameData.ItemCatalogDataTable`
2. 拿到 `FAOItemCatalogRow`
3. 从中取 `ItemDefinitionClass`
4. 组装 `FAOInventoryReceiveBatch.DefinitionEntries`
5. 再统一走正式入包入口

也就是说，这条命令仍然不是“直接往库存里塞 ItemDefinitionClass”。

它只是把调试输入先翻译成了当前项目已经统一的物品身份链。

## 正式发货主链怎么走

这部分是整个方案最核心的地方。

这条调试命令虽然是开发期能力，但真正发货时必须和正式玩法系统站在同一条主链上。

当前项目里已经锁定：

- 新系统如果想让对象“获得物品”，应优先接 `TryAddInventoryBatchToActor(...)`

这条结论本身也已经写在知识库里：

- [DECISIONS.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Docs/Knowledge/InventoryEquipment/DECISIONS.md)
- [PROJECT_MAP.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md)

所以这条调试命令的正式主链应该锁成：

`Exec 输入 -> PlayerController 本地请求 -> Server RPC -> 服务端解析目标 Actor -> 服务端按 ItemId 查物品总表 -> 组装 FAOInventoryReceiveBatch -> UAOInventoryStatics::TryAddInventoryBatchToActor(...)`

这条链的意义非常大。

它保证了：

- 调试命令不是特殊库存入口
- AI、玩家、采集、制造、拾取，最终都仍然往同一条正式入包语义收束
- 后面如果统一入包逻辑有改动，这条调试命令也会自动跟着对齐

## 多人网络同步方案怎么定

你已经把这件事最敏感的边界直接定掉了：

- 任意客户端都能输入
- 客户端输入后，允许请求服务端给任意可解析目标对象发货

既然这个边界已经是明确设计，而不是犹豫中的风险项，那方案里就不应再遮遮掩掩。

当前推荐的多人语义应当写得非常直白：

### 1. 客户端本地绝不直接改库存

这一点不能松。

因为当前真正入包能力本来就应当由权威端执行，`UAOInventoryComponent::TryAddInventoryBatch(...)` 这类正式变更也天然更适合在服务端发生。

如果客户端本地直接塞库存，再想补同步，反而会把调试能力做成一条歪链。

### 2. 客户端只负责发请求

客户端输入 `GiveItem` 时，不直接做目标解析后的正式入包。

它只负责把这几个信息发给服务端：

- `ItemId`
- `Count`
- 可选目标字符串

### 3. 服务端负责所有权威解析与正式入包

服务端收到请求后，统一完成下面这些事：

- 解析目标 Actor
- 查 `ItemId`
- 组装 `FAOInventoryReceiveBatch`
- 调用 `TryAddInventoryBatchToActor(...)`

这才符合当前项目的网络边界。

### 4. 当前开发期不加管理员限制

这个点要明确写在方案里，而不是藏成默认行为。

因为现在不是“不小心忘了做权限”，而是你明确要求：

- 当前测试开发阶段，先不限制管理员

所以方案里应该正式定性为：

当前这条命令属于开发期调试能力，权限边界有意放宽。
后续如果项目进入更严格测试或正式环境，再单独补权限收口，不在本轮方案内。

## 命令层不该替库存系统管什么

这部分要单独写清楚，不然后面很容易有人顺手往命令里加“帮忙兜底”的逻辑。

这条命令当前不负责：

- 包满时自动找别的容器
- 自动拆分成多个批次
- 自动掉地上
- 自动重试
- 绕过 `CanFullyAcceptInventoryBatch(...)`
- 绕过对象上现有库存组件优先级
- 对不同物品来源写不同通知语义

一句话概括就是：

它只负责发起请求，不负责替正式库存系统改规则。

如果目标对象背包已满，或者对象没有可接收入包的库存组件，失败就是正确结果。

## 命令结果与日志反馈应该怎么做

这条能力是开发期调试命令，所以可观察性不能太差。

建议结果反馈至少覆盖两层：

### 1. 请求方反馈

输入命令的那一端，应该能明确知道：

- 请求有没有发出去
- 服务端有没有接受
- 最终成功还是失败

### 2. 服务端日志反馈

服务端侧也应有清晰日志，至少带出：

- 发起请求的是谁
- 目标 Actor 是谁
- `ItemId` 是多少
- `Count` 是多少
- 失败卡在目标解析、物品解析，还是正式入包

这样后面联机测 AI 时，才不会陷入“客户端看起来没反应，但不知道到底卡哪”的状态。

## 这条方案与现有系统的关系

这条命令不是新库存来源系统，它只是复用现有库存主链的一层调试翻译器。

和现有系统的关系应该明确写成下面这样：

### 1. 与制造系统的关系

制造系统继续走制造自己的入包准备与权威请求链。
这条调试命令不参与制造语义。

### 2. 与采集系统的关系

采集系统继续按采集结果构造奖励批次，再走统一入包。
这条调试命令不参与采集结算语义。

### 3. 与 AI 测试的关系

AI 测试只是这条命令的重要使用场景之一。
它不是 AI 专属入口。

### 4. 与玩家测试的关系

玩家对象也应该走同一条命令，因为“对象获得物品”的正式主链本来就不该被 AI 和玩家拆成两套。

## 首版不做什么

为了避免方案膨胀，这里把首版明确不做的事直接锁掉。

首版不做：

- 批量多种物品一次输入
- 模糊搜索目标对象
- 用物品名或显示名查物品
- 自动补全 ItemId
- 管理员权限限制
- UI 面板式发货工具
- 非 Actor 目标
- 发货结果持久化或命令历史

这样首版边界会很稳。

## 最终推荐方案

最终建议直接锁成下面这条方案：

1. 在 `AAOPlayerController` 上增加新的 `Exec` 调试命令入口。
2. 命令名使用 `GiveItem`。
3. 语法为 `GiveItem <ItemId> <Count> [TargetActorNameOrPath]`。
4. 不传目标时，默认给当前控制对象。
5. 传目标时，服务端优先按显式路径解析，失败后再按当前 World 中的 Actor 名做精确匹配。
6. 客户端输入命令时，只提交一条服务端请求，不直接改库存。
7. 服务端收到请求后，统一完成目标解析、`ItemId` 查表、`FAOInventoryReceiveBatch` 组装与正式入包。
8. 正式入包继续只走 `UAOInventoryStatics::TryAddInventoryBatchToActor(...)`。
9. 包满、对象无库存、`ItemId` 无效等情况，都按正式主链失败返回，不额外在命令层兜底。
10. 当前开发测试阶段，不限制管理员权限，任意客户端都可以请求服务端给任意可解析目标对象发货。

## 这份方案对应的程序员阅读导航

如果后面开始实现，这条链最值得先看的文件顺序建议是：

1. [AOPlayerController.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h)
2. [AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
3. [AOInventoryStatics.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.h)
4. [AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)
5. [AOInventoryComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.h)
6. [AOInventoryComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp)
7. [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
8. [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)
9. [AOItemCatalogTypes.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Items/AOItemCatalogTypes.h)

这条顺序正好对应：

- 命令入口
- 服务端权威请求
- 正式入包主链
- 物品总表解析

后面无论是写实现，还是写测试方案，都不容易看乱。

## 本轮测试看什么

这轮不是在验证一个新库存系统，而是在验证一条新的调试命令有没有老老实实复用现有正式入包主链。

测试时重点只看四件事：

1. `GiveItem` 命令能不能从 `AAOPlayerController` 正常进入服务端权威执行。
2. 目标对象解析是否符合方案：不传目标默认给当前控制对象；传目标时支持显式路径和唯一 Actor 名。
3. 物品解析是否严格走 `ItemId -> AOGameData -> ItemDefinitionClass`。
4. 最终入包是否严格走 `UAOInventoryStatics::TryAddInventoryBatchToActor(...)`，而不是偷偷走第二套入口。

如果这四件事都对，再去测 AI 是否会使用物品，测试边界才不会混淆。

## 程序员阅读导航

如果你是第一次接手这条命令，建议按下面顺序看：

1. 先看 [AOPlayerController.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h)，确认控制台入口、Server RPC、结果回传和服务端执行辅助函数都定义在哪。
2. 再看 [AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)，从 `GiveItem(...)` 一路顺着看到 `Server_GiveItemRequest_Implementation(...)` 和 `TryExecuteGiveItemOnAuthority(...)`。
3. 再看 [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)，确认 `FindItemCatalogRowById(...)` 仍然是当前唯一的 `ItemId` 解析入口。
4. 最后看 [AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)，确认真正入包还是统一走 `TryAddInventoryBatchToActor(...)`。

如果测试现象不对，也建议先用这个顺序回代码，不要一上来就怀疑 AI 决策。

## 前置准备

开始手工测试前，先把下面几件事准备好：

1. 确认工程已经编过当前代码，至少 `AegisOdysseyEditor` 目标能通过编译。
2. 确认 `AOGameData` 已经配置了真实可用的 `ItemCatalogDataTable`，并且你要测试的 `ItemId` 在总表里能解出有效 `ItemDefinitionClass`。
3. 确认测试目标对象身上真的挂了可接收统一入包的库存组件；如果目标没有库存组件，失败就是预期结果。
4. 如果要测“默认给自己”，确认当前本地 PlayerController 已经实际控制了一个 Pawn。
5. 如果要测“给某个 AI”，先在运行时拿到这个 AI 的唯一 Actor 名，或者直接准备它的完整对象路径。
6. 如果要测联机，至少准备一个 Listen Server 和一个可输入控制台命令的客户端。

## 手工测试步骤

### 1. 默认目标是否给当前控制对象

1. 启动游戏并进入可控制角色的测试关卡。
2. 打开控制台，输入 `GiveItem 1001 3`。
3. 观察日志。
4. 预期先看到本地请求日志，再看到服务端结果日志，最后看到客户端结果回传日志。
5. 再去看当前控制对象的背包或相关库存 UI，确认新增了 `ItemId=1001` 对应的物品，数量增加 `3`。
6. 如果失败，先看日志是否提示 `controlled pawn was not found`；如果是，先排查当前控制器是否真的持有 Pawn。

关联代码位置：
[AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
入口依次是 `GiveItem(...)`、`Server_GiveItemRequest_Implementation(...)`、`TryExecuteGiveItemOnAuthority(...)`。

### 2. 显式 Actor 名是否能唯一命中

1. 在场上放一个挂了库存组件的 AI，并记下它运行时的唯一 Actor 名，例如 `BP_Enemy_Test_C_0`。
2. 打开控制台，输入 `GiveItem 1001 2 BP_Enemy_Test_C_0`。
3. 观察日志，确认服务端结果里写明命中的目标对象名称。
4. 再去看这个 AI 身上的库存数据或调试 UI，确认对应物品确实加到了该 AI 身上，而不是玩家自己身上。
5. 如果失败，先看日志是否提示 `target actor 'xxx' was not found`。
6. 如果场上有多个同名 Actor，预期直接失败，并看到 `multiple actors matched target name`，这属于方案内正确行为，不要把它改成模糊匹配。

关联代码位置：
[AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
重点看 `TryExecuteGiveItemOnAuthority(...)` 里按 `TActorIterator` 做唯一精确匹配的部分。

### 3. 显式对象路径是否优先于名字匹配

1. 复制一个场上目标 Actor 的完整对象路径。
2. 打开控制台，输入 `GiveItem 1001 1 <完整对象路径>`。
3. 观察日志，确认命令成功，并且目标对象是路径指向的那个实例。
4. 如果路径可用但名字重名，预期仍然应该成功命中路径对象，而不是落回名字歧义失败。
5. 如果失败，先看是不是路径对象不在当前 World；当前实现只接受当前世界内的 Actor。

关联代码位置：
[AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
重点看 `StaticFindObject(...)` 路径解析成功后又做了一次 `GetWorld()` 校验。

### 4. 非法 ItemId / Count 是否只在命令层快速失败

1. 打开控制台，输入 `GiveItem -1 3`。
2. 预期本地直接报 `ItemId is invalid`，不会继续走正式发货。
3. 再输入 `GiveItem 1001 0`。
4. 预期本地直接报 `Count must be greater than 0`。
5. 再输入一个不存在于总表中的合法整数，例如 `GiveItem 999999 1`。
6. 预期请求会发到服务端，但服务端结果会报 `could not resolve to a valid item definition`。

关联代码位置：
[AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
本地参数兜底在 `GiveItem(...)`，总表解析失败在 `TryExecuteGiveItemOnAuthority(...)`。

### 5. 无库存 / 包满时是否沿用正式入包失败

1. 选择一个没有库存组件的 Actor，或者把目标库存先人为填满。
2. 输入 `GiveItem 1001 1 <目标名或路径>`。
3. 预期命令最终失败，并提示 `could not accept the requested inventory batch`。
4. 这时不要在命令里补自动掉地、自动拆批次、自动找其他容器，这些都不属于这条命令的职责。

关联代码位置：
[AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)
真正的接收判定仍然是 `TryAddInventoryBatchToActor(...)` 按库存组件顺序尝试完整接收。

### 6. 联机下客户端请求是否仍由服务端权威入包

1. 启动一局联机测试，让一个非服务端客户端进入游戏。
2. 在这个客户端控制台输入 `GiveItem 1001 1`。
3. 观察服务端日志，确认服务端收到了请求并执行了 `Server_GiveItemRequest_Implementation(...)`。
4. 再观察客户端日志，确认客户端收到了 `ClientReportGiveItemResult(...)` 的成功或失败结果。
5. 最后看库存结果，确认真实物品变化来自服务端同步，而不是客户端本地幻觉。

关联代码位置：
[AOPlayerController.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h)
网络入口是 `Server_GiveItemRequest(...)` 和 `ClientReportGiveItemResult(...)`。

## 验收口径

这条命令本轮可以认为“通过验收”，至少要同时满足下面几点：

1. `GiveItem <ItemId> <Count>` 能稳定给当前控制对象发货。
2. `GiveItem <ItemId> <Count> <ActorName>` 在目标名唯一时能稳定命中指定对象。
3. `GiveItem <ItemId> <Count> <ObjectPath>` 能稳定优先按路径命中。
4. 非法 `ItemId`、非法 `Count`、目标不存在、目标无库存、目标包满，都能返回明确失败信息。
5. 联机下客户端不会本地直接改库存，真实发货仍然由服务端权威执行。
6. 整条链最终仍然复用 `UAOInventoryStatics::TryAddInventoryBatchToActor(...)`，没有长出第二条库存入口。

如果只满足“能加物品”，但失败场景和联机场景都没测，这条命令还不能算测完。

## 关联代码位置

本轮最关键的代码入口是：

1. [AOPlayerController.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h)
2. [AOPlayerController.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp)
3. [AOGameData.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h)
4. [AOGameData.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp)
5. [AOInventoryStatics.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.h)
6. [AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)

对应职责分别是：

1. 命令声明与 RPC 暴露
2. 命令请求、服务端权威执行、目标解析、结果日志
3. `ItemId` 到总表的正式查询入口
4. 统一入包入口与接收组件选择逻辑

## 最终补记：QuickBar 前置入包与通知语义收口

这段补记对应的是后续已经最终锁定、可直接进入实施阶段的库存接收改动。

背景很简单：

当前项目里，`QuickBar` 不是没有库存能力，而是当前主动关闭了“统一入包参与资格”。

也就是说，现状不是“QuickBar 不能装物品”，而是：

1. `QuickBar` 本身仍然是 `UAOInventoryComponent` 体系里的库存组件。
2. 它仍然具备 `CanFullyAcceptInventoryBatch(...)` / `TryAddInventoryBatch(...)` 这套通用入包能力。
3. 当前只是通过 `bAllowUnifiedInventoryIntake = false`，把它排除在 `TryAddInventoryBatchToActor(...)` 的统一入包选择之外。

这个事实必须先写清楚，不然后面很容易误判成“要想让 QuickBar 接货，就得新做一套入包库存”。

这不是当前结论。

## 已锁定的改动目标

这次要做的，不是新增“物品栏专用入包系统”，也不是重写制作、采集、调试发货这些来源入口。

这次锁定的目标很克制：

1. 保持现有统一入包主链不变。
2. 让 `QuickBar` 重新参与统一入包选择。
3. 在统一入包选择器里，先对 `QuickBar` 做一次前置尝试。
4. 如果 `QuickBar` 无法完整接收整批物品，再继续回退到原本的优先级排序链。
5. `BackPack` 继续作为前置尝试失败后的主回退接收容器。

用一句话概括，就是：

`QuickBar first try -> fail -> original priority-based intake`

## 这次为什么定成“前置尝试”，而不是改成新的主规则

这里已经明确排除了几条更重的方向：

- 不新增新的入包库存组件
- 不把 QuickBar 改造成第二套掉落系统
- 不重写统一入包的总体语义
- 不去碰制造、采集、AI 调试发货这些来源系统

这样收的好处是：

1. 改动点集中在统一入包选择器。
2. 现有所有“外部系统先构造 `FAOInventoryReceiveBatch` 再统一入包”的来源入口都天然复用。
3. 当 `QuickBar` 放不下时，仍然自然回退到当前背包链，不需要新兜底规则。

## 当前已经确认的边界与风险判断

### 1. QuickBar 的“能不能使用物品”不是这层该管的事

这点已经锁定。

如果某个物品进了 `QuickBar` 但本来就不可用，那仍然应该由 `TryUseItemAtSlot(...)` / `CanUseItemAtSlot(...)` 这些现有使用链负责裁定。

这一层改动只讨论“先进哪个库存组件”，不顺手讨论“进了以后该不该能用”。

### 2. 获取物品通知必须从 BackPack 语义收回到“进入库存成功”语义

这点已经通过代码复核确认，而且本轮已经定性为必须一起改，不再只是记录事实。

当前 `UAOInventoryComponent::TryAddInventoryBatch(...)` 在成功入包后会统一构造 `FAOInventoryAcquisitionMessage`，但真正是否向世界级 `UAOInventoryMessageSubsystem` 广播，仍然受：

- `ShouldBroadcastInventoryAcquisitionNotifications()`

这条组件级开关控制。

当前默认实现返回 `false`，而 `BackPack` 明确重载成了 `true`。

这意味着当前代码里：

1. “进入库存成功”与“发获取物品通知”在当前代码里还没有彻底收成同一层语义。
2. 如果 `QuickBar` 开始参与前置接货，而通知仍然只挂在 `BackPack`，那获取物品提示就会直接失效。

所以这次已经锁定：

1. 去掉 `ShouldBroadcastInventoryAcquisitionNotifications()` 这条组件级开关。
2. 只要统一库存接收真正成功，就允许统一构造并广播 `FAOInventoryAcquisitionMessage`。
3. HUD 侧继续沿用现有 `bIsLocalRelevant` 过滤，只显示和本地玩家相关的获取物品提示。

也就是说，这次不是让 `QuickBar` 单独补一份通知逻辑，而是把通知语义从“进入 BackPack 才提示”正式收回到“进入库存就提示”。

### 3. 正式装备卸下回 BackPack 不在本轮范围内

这条也已经锁定。

当前正式装备卸下回 `BackPack`，属于现有显式背包语义的一部分，本轮不改它。

所以这次不要顺手去碰：

- 正式装备卸下回流目标
- 任何显式 `FindComponentByClass<UAOBackPackComponent>()` 的旧链路

这轮只处理统一入包选择器和获取物品通知语义，不扩边界。

## 后续实施时的阅读顺序

如果后面开始落这条方案，建议先按下面顺序看：

1. [AOInventoryStatics.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp)
2. [AOInventoryComponent.h](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.h)
3. [AOInventoryComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp)
4. [AOQuickBarComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Equipment/AOQuickBarComponent.cpp)
5. [AOBackPackComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOBackPackComponent.cpp)
6. [AOInventoryMessageSubsystem.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryMessageSubsystem.cpp)
7. [AOHUDViewModelComponent.cpp](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/AOHUDViewModelComponent.cpp)

这条顺序对应的就是：

1. 统一入包如何挑接收组件
2. QuickBar 为什么当前没有参与统一入包
3. 获取物品通知为什么当前错误地挂在 BackPack 语义上
