---
title: Character Inventory Interaction Design Supplement 2026-05-28
date: 2026-05-28
tags:
  - knowledge
  - interaction-system
  - inventory-equipment
  - supplement
aliases:
  - 角色库存交互方案补充 2026-05-28
status: proposed
---

# 角色库存交互方案补充

这份补充文档只记录两个已经敲定、但本轮不前置处理的收口点。

主方案见：
- [[CHARACTER_INVENTORY_INTERACTION_DESIGN_2026-05-28]]

## 1. Chest 中心命名收口后置

当前仍然保留了少量 `Chest` 中心命名，例如：
- `TransferChestSlotToInventorySlot(...)`
- `TransferInventorySlotToChestSlot(...)`

这件事本身成立，后面也应该收。

但本轮结论已经锁定：
- 不把这类命名清理当作“角色库存交互正式接入”的前置阻塞项
- 先把“角色作为容器对象”的正式功能链落完
- 等功能语义稳定后，再顺手统一做命名收口

这么做的原因不是忽略命名问题，而是避免在功能边界还没完全站稳前，先做一轮低收益改名，扩大改动面。

## 2. 会话复制层本轮保持容器会话语义

当前 `UAOInteractionSessionComponent` 的复制与客户端重建逻辑，仍然只按 `UAOContainerInteractionSessionModel` 处理。

这在本轮不是缺陷，而是当前方案的直接落点。

本轮结论已经锁定：
- 这条不是“暂时不动”，而是按当前方案本来就不该动
- 因为本次角色库存交互明确继续复用 `UAOContainerInteractionSessionModel`
- 所以当前会话复制层继续只认容器会话，符合本次设计边界

后面如果顺手整理，这一层最多做：
- 文义收口
- 注释澄清

这次需求里不为了“更抽象”去额外发明第二套角色专属会话复制结构。

## 3. 当前执行口径

所以接下来的实现顺序是：
1. 继续直接做“角色作为可交互容器对象”的正式接入
2. 上面两个点先记账，不抢在功能前面处理
3. 等角色库存交互代码落完，再顺手做低风险收口

## 4. 当前已落地情况

这部分只记录截至本轮为止已经落地并确认过的代码事实，避免后续继续把“方案中的目标”误当成“代码里的现状”。

### 1. 角色已经正式接入“可交互容器对象”语义

当前 `AAOCharacter` 已经正式接入：
- `IInteractableTarget`
- `IInventoryInterface`
- 角色侧 `InteractionBounds` 交互触发盒

也就是说，角色现在不是靠临时调试入口假装打开库存，而是已经具备了站进现有普通交互链路的正式对象能力。

### 2. 角色开包继续复用现有容器会话

当前角色库存交互没有新开第二套角色专用会话模型。  
实际仍然复用 `UAOContainerInteractionSessionModel`，并且当前观察目标仍然固定为角色的 `CharacterBackPackComponent`。

这也意味着当前已落地口径仍然是：
- 角色作为容器对象成立
- 目标侧当前先只接角色背包
- 还没有扩成目标侧多容器展示

### 3. 调试开包入口已经归回 `AAOPlayerController`

本轮已经确认：
- `DebugOpenCharacterInventory` 的职责留在 `AAOPlayerController`
- 服务端调试请求与目标解析也留在 `AAOPlayerController`
- `AAOCharacter` 不再承担调试开包入口职责

这条收口很重要，因为它把“正式角色职责”和“开发期调试入口”重新分开了，避免后面继续往角色类里堆临时控制逻辑。

### 4. 角色类里的历史调试 helper 残留已经清掉

之前 `AAOCharacter.cpp` 里在 `GetDefaultInventoryInteractionOption()` 后面残留了一段历史调试直开 helper 脏代码。  
本轮已经把这段残留彻底删掉，当前这里已经直接接回 `PossessedBy()`，不再挂着旧的调试会话启动残片。

这一步的意义不是“代码好看一点”，而是明确：
- 角色类只保留正式角色 / 交互容器职责
- 调试入口不再偷偷残留在角色类内部

### 5. 自动化测试与编译验证已经对齐当前方案

当前已知对应的自动化测试文件是：
- `Source/AegisOdyssey/TestProject/CharacterInventoryInteractionTests.cpp`

当前测试覆盖的是这一轮最小闭环里的几个关键事实：
- 角色实现正式接口
- 角色库存解析仍回到背包
- 角色存在交互触发盒
- 存活敌人默认拒绝
- 死亡敌人允许打开

另外，本轮在清掉角色类历史调试残留后，已经重新编译通过：
- `AegisOdysseyEditor Win64 Development`

这说明当前收口至少在编译层面已经重新站稳。

### 6. 当前还没顺手处理的尾巴

这轮没有继续扩 scope 去做下面这些额外整理，它们仍然留在后续收口清单里：
- 少量 `Chest` 中心命名的进一步统一
- 会话复制层文义收口
- `AOCharacter.cpp` 里更早位置的历史中文乱码注释清扫

这些问题里，前两个已经在本补充方案前文记账；最后一个属于代码卫生问题，不影响这轮角色库存交互正式链路已经落地的事实。

## 5. 角色库存 UI / MVVM 方案补充

这一节只讨论一个问题：

**当玩家打开角色身上的背包以后，UI 该怎么显示目标角色当前的库存数据，并且继续符合项目现有 MVVM 架构。**

这部分看起来像是在做“角色专用库存 UI”，但本质上仍然没有脱离当前已经存在的容器交互 UI 主链。

### 1. 这次 UI 方案的核心口径

这次必须继续坚持项目现有 MVVM 口径：

- 打开哪个 UI，就由哪个 UI 访问自己对应的数据
- UI 只消费当前会话暴露给它的数据，不直接读取对象真相
- 对象真相仍然在对象自己的正式库存组件上

也就是说，后续无论是：
- 角色打开别的角色库存
- 角色自己打开自己的库存
- 以后扩成角色身上的别的库存 UI 或非库存 UI

都不能让 Widget 自己跳过会话，直接去 Actor 身上东拼西凑找数据。

### 2. 这轮 UI 目标不是只看背包，而是“除制造系统外都显示”

这次口径已经重新锁定，不再是“目标侧先只看角色背包”。

后续 UI 阶段的正式目标应当是：

- 打开角色库存以后，继续复用现有容器交互界面
- 目标侧把角色当前已有的库存相关 UI 模块按现有体系显示出来
- 当前唯一明确排除的是制造系统

也就是说，后续目标不是做一个“只够看背包”的过渡版角色界面，而是：

**除了制造系统以外，角色当前已经存在并且属于库存体系的那些 UI，都应该在目标侧按现有架构显示出来。**

这里要特别注意两层含义不要混：

第一层是“当前代码事实”。  
当前正式会话模型落地时，目标库存观察入口仍然先锁在 `CharacterBackPackComponent`，这一点在本方案前文“当前已落地情况”里已经单独记录了。

第二层是“下一阶段 UI 方案目标”。  
下一阶段不应该把目标侧永远收死在“只看背包”这个阶段，而是应该让现有库存类 UI 模块一起站回同一条正式会话链。

所以这里不是要否定前面的代码事实，而是明确后续 UI 施工目标已经扩大：

- 背包要显示
- QuickBar 这类已有库存 UI 要显示
- 正式装备这类已有库存 UI 要显示
- 其他已经站在库存体系里的现成 UI，也都应纳入
- 制造系统暂时明确不纳入这一轮角色库存 UI

### 3. 不新开角色专用数据链，继续复用现有容器会话 UI 链

当前现有正式链路已经是：

`InteractionSessionComponent -> UAOContainerInteractionSessionModel -> ObservedContainerSlots / ContainerViewModel -> UAOContainerUI`

这条链对箱子成立，对角色也继续成立。

因此这次 UI 方案明确不做：

- 角色专用库存真相快照
- 角色专用库存复制结构
- 角色专用库存 Widget 直连 Actor
- 角色专用库存 Mutation 分叉入口

目标角色的库存显示，仍然应该来自当前会话模型暴露出来的正式观察数据，而不是 UI 直接读取 `AAOCharacter`。

这里还要再往前收一层：

后面即便目标侧出现的不再只是“背包那一块”，也不能把问题重新做回“每个 Widget 自己去目标角色身上找自己要的数据”。  
正确方向仍然是：

- 当前会话知道自己打开的是谁
- 当前会话负责暴露这次界面需要消费的目标侧数据
- 对应 Widget 只消费这次会话给它的数据

### 4. “打开谁的库存”和“改谁的数据”必须继续是一回事

这是这次 UI 方案的红线，必须单独记清楚。

无论是：
- 玩家打开自己角色的库存
- 玩家打开 AI 队友角色的库存
- 玩家通过调试入口打开一个额外允许查看的目标角色库存

最终被显示和被修改的，必须始终是同一套目标库存数据。

不能出现下面这种分裂：

- UI 显示的是角色 A 的会话快照
- 但右键、拖拽、使用、交换最后改到的是角色 B 或玩家自己本地那套库存

所以这一层正式语义必须继续是：

- 当前会话指向哪个目标角色背包
- UI 就显示哪个目标角色背包
- Mutation 就修改哪个目标角色背包

而不是因为“请求者是玩家自己”就偷偷切回玩家本人的另一套数据链。

### 5. 当前 UI / ViewModel 分工应该怎么理解

这一轮继续按现有职责分层：

#### 1. `UAOContainerInteractionSessionModel`

负责：

- 保存当前交互目标
- 解析当前目标背包
- 维护 `ObservedContainerSlots`
- 维护目标侧 `ContainerViewModel`

它是当前目标侧库存观察数据的正式会话来源。

#### 2. `UAOContainerUI`

负责：

- 绑定玩家自己的 `InteractionSessionComponent`
- 监听当前会话是否切到 `UAOContainerInteractionSessionModel`
- 消费目标侧 `ObservedContainerSlots` / `ContainerViewModel`
- 把目标侧格子按当前会话快照重建出来

它不是目标角色真相层，只是当前目标侧容器 UI 的消费层。

#### 3. `UAOInventoryUI`

负责：

- 统一库存交互请求入口
- 统一右键菜单动作请求入口
- 统一通过当前交互会话提交 mutation

也就是说，目标角色库存 UI 打开以后，里面发生的拖拽、右键、使用、交换，仍然应该继续回到 `AOInventoryUI` 这一套正式请求链。

#### 4. `UMVVM_InventoryMenu`

当前仍然只应该被理解为：

- 一份给库存类 UI 消费的 ViewModel
- 具体由谁持有、给谁灌数据，取决于当前 UI 所依附的那条正式链

对目标侧这些库存类显示来说，它们对应的 ViewModel 也应该继续由正式数据宿主持有并刷新，而不是由 UI 自己凭空造一份。

### 5.1 不动玩家自开背包入口，但必须坚持“面向的是同一套数据”

这次方案不碰玩家自己打开自己背包的现有入口。

原因不是它不重要，而是因为这里有很明确的边界：

- 本轮讨论的是“角色打开角色库存”的 UI 方案
- 不是重做玩家自开背包的入口形态
- 更不是顺手把玩家现有背包打开链重构一遍

但这不代表可以放松数据一致性要求。

这一层必须继续锁死：

- 不管玩家是通过自己原有的背包入口打开
- 还是通过角色对角色交互打开目标库存

只要最后指向的是同一个角色的同一套库存真相，显示和修改都必须落到那同一套数据上。

也就是说，这里不动“入口形式”，但必须坚持“面向的数据对象完全一致”，这条仍然是红线。

### 6. 多人同步口径在 UI 层不能变形

这部分非常重要，因为 UI 很容易被误写成“看起来能显示，但同步语义已经变了”。

当前必须继续坚持：

- 目标角色库存真相仍在服务端目标库存组件
- 服务端会话模型整理目标侧观察快照
- `InteractionSessionComponent` 继续用 `OwnerOnly` 会话状态同步给当前查看者
- 查看者本地 UI 只消费自己收到的当前会话快照

所以这次 UI 改动绝不能演变成：

- 把目标角色完整库存详情常驻复制给所有相关客户端
- Widget 自己跨过会话直接抓目标角色库存组件
- 为角色库存 UI 额外发明第二条专用同步链

### 6.1 为什么 UI 不直读目标角色组件，仍然能及时刷新

这个问题必须单独写清楚，因为它是最容易把人带偏的一点。

很多人看到“UI 不直接去读 `AAOCharacter` 身上的库存组件”时，第一反应会是：

**那底层库存一变，UI 还怎么立刻跟着变？**

答案是：当前项目里，箱子库存本来就不是靠 Widget 直读真相来刷新，而是靠一条已经落地的“底层变更 -> 会话刷新 -> 会话快照同步 -> UI 重建”链。

也就是说，当前实时更新依赖的不是“UI 有没有直接拿到目标组件指针”，而是“目标库存有没有正确接进会话观察链”。

### 6.2 箱子当前就是这样跑通的

当前箱子库存的刷新链已经是：

1. 服务端 `UAOContainerInteractionSessionModel` 绑定当前观察库存的变更事件。
2. 底层库存变化后，触发 `HandleObservedInventoryChanged()`。
3. 会话模型调用 `RefreshObservedContainer()` 重新抓取最新槽位快照。
4. 会话模型调用 `SyncCurrentSessionToReplication()`，把新的会话快照同步回 `InteractionSessionComponent`。
5. `InteractionSessionComponent` 通过 `OwnerOnly` 把最新 `ReplicatedSessionState` 发给当前查看者。
6. 客户端 `RebuildClientSessionFromReplicatedState()` 重建本地容器会话。
7. 本地 `UAOContainerUI` 监听 `GetOnContainerDataChanged()`，再调用 `RefreshContainerSlots()` 重建格子。

这一套链现在对箱子已经成立，而且这正是当前箱子 UI 能及时更新的原因。

换句话说，箱子当前也不是：

- UI 直接抓 `AAOChest::ChestInventory`
- 然后自己盯着它变化

它走的本来就是会话快照驱动 UI。

### 6.3 所以后续角色库存 UI 成不成立，真正看的不是 Widget 能不能直连角色

后续做角色库存 UI 时，真正要检查的不是：

- `UAOContainerUI` 能不能自己 `Cast<AAOCharacter>` 后直接去拿角色背包

而是下面这几件事有没有完整成立：

1. 目标角色背包有没有被当前 `UAOContainerInteractionSessionModel` 正确观察。
2. 目标角色背包变化时，是否同样会触发 `OnInventoryObservedChanged`。
3. 触发后，是否同样会走 `RefreshObservedContainer()`。
4. 刷新后，是否同样会走 `SyncCurrentSessionToReplication()`。
5. 客户端本地 `UAOContainerUI` 是否同样只消费这份新的会话快照并刷新格子。

如果这五步成立，角色库存 UI 就会像箱子一样及时更新。

如果这五步不成立，就算 Widget 手里直接拿到了角色组件指针，也只是把同步问题藏进 UI 里，而不是解决它。

### 6.4 这条判断标准要锁成后续实现红线

所以这部分正式口径可以直接锁成一句话：

**角色库存 UI 是否成立，判断标准不是 Widget 能不能直接读到目标角色库存组件，而是目标角色背包能不能完整接进当前箱子这条既有“观察刷新 -> 会话快照 -> OwnerOnly 同步 -> UI 重建”链。**

这句话很重要，因为它直接决定了后续代码该往哪一层改：

- 正确方向是补强“角色背包接进既有观察链”
- 错误方向是让 UI 自己越权去补底层同步缺口

### 7. 后续如果角色还有别的 UI，也继续按“开哪个 UI 就消费哪个会话数据”扩

你这次特别提醒得对，角色身上以后不一定只有背包 UI。

所以这轮方案要提前把扩展口径锁死：

- 角色以后可以有别的库存 UI
- 角色以后也可以有非库存 UI
- 但这些 UI 仍然都应该通过当前正式会话链拿各自对应的数据

正确扩展方向是：

- 当前打开的会话 Widget 决定本次要展示哪类数据
- 会话模型按当前 Widget 需要暴露对应 ViewModel / 快照
- Widget 消费那份正式暴露出来的数据

而不是：

- 先做一个“角色万能大 UI”
- 然后每个子面板自己去角色 Actor 身上单独扒数据

### 7.1 目标侧抬头和展示信息不能再写死成 Chest / Character 分叉

这条也已经锁定为必须这样做。

## 6. 2026-05-28 晚间纠偏：当前 UI / MVVM 方案必须回收并重写

这次已经正式确认，前一版“角色库存 UI”虽然把功能跑通了，但实现方向是错的，必须回收。

### 1. 这次确认要回收的错误改动

- `UAOInventoryUI` 不应该承担“当前到底看自己还是看目标对象”的解析职责。
- `UAOInventoryUI` 不应该提供 `ResolveInventoryDataSourceActor / FindTargetComponent` 这类 UI 越权入口。
- `AOBackPackUI / AOQuickBarUI / AOFormalEquipmentBarUI / AOSkillBarUI` 不应该自己翻目标 Actor 身上的组件。
- `AOFormalEquipmentSlotUI / AOSkillSlotUI` 不应该自己绕回目标 Actor 取运行时真相。

### 2. 这次重写后的正式口径

- `UAOContainerInteractionSessionModel` 成为目标侧库存 UI 的正式数据入口。
- 目标角色的背包、QuickBar、正式装备栏、技能栏数据，都从当前会话模型统一暴露。
- `UI` 只消费当前会话模型给出来的数据或 ViewModel，不再自己做目标解析。
- 槽位控件如果还需要运行时能力对象，由上层面板注入，不再自己 `FindComponentByClass`。

### 3. 这次实际落地的收口点

- `UAOInventoryUI` 已移除：
  - `EAOInventoryWidgetDataSourceMode`
  - `ResolveInventoryDataSourceActor`
  - `GetCurrentInteractionTargetActor`
  - `GetInventoryDataSourceMode`
  - `FindTargetComponent`
- `UAOInventoryUI` 新增：
  - `GetOwningContainerSessionModel()`
- `UAOContainerInteractionSessionModel` 新增正式 getter：
  - `GetObservedBackPackComponent()`
  - `GetObservedQuickBarComponent()`
  - `GetTargetQuickBarViewModel()`
  - `GetObservedFormalEquipmentSlotInventory()`
  - `GetTargetFormalEquipmentViewModel()`
  - `GetObservedFormalEquipmentManager()`
  - `GetObservedSkillComponent()`
  - `GetObservedSkillSlotInventory()`
  - `GetObservedSkillSlotViewDataList()`
- `AOBackPackUI / AOQuickBarUI / AOFormalEquipmentBarUI / AOSkillBarUI` 已改成只从当前容器会话模型拿目标侧数据。
- `AOFormalEquipmentSlotUI / AOSkillSlotUI` 已改成由上层注入正式运行时对象，不再自己向目标 Actor 取数据。

### 4. 这次重写后必须继续坚持的红线

- 打开谁的库存，会话就指向谁。
- 显示谁的数据，就改谁的数据。
- 不允许 UI 私自切回“玩家自己那套组件真相”。
- 不允许为了角色库存 UI 再发明第二套同步或第二套 Mutation 入口。

后续目标侧抬头、标题、名字、死亡状态这类展示信息，应该抽成“当前会话目标的展示语义”，而不是继续在 Widget 里写死：

- 如果是 `AAOChest` 就显示一套
- 如果是 `AAOCharacter` 就手写另一套

这不是说现在就要发明一个很大的新系统，而是要先把方向锁正：

- 展示层读取的是“当前会话目标的展示数据”
- 不要在具体 Widget 里继续堆 `Chest / Character / 以后别的对象` 的硬分支

当前已经锁定的最小展示集只有：

- 目标名字
- 是否死亡

阵营、头像、职业之类后面再扩，但扩的时候也仍然应该站在这条“展示语义抽象”上，而不是重新写死对象特判。

### 8. 当前阶段的推荐执行口径

所以下一阶段如果正式开始写 UI，当前推荐口径已经锁定为：

1. 继续沿用当前容器交互会话 Widget 入口，不新开角色专用打开链。
2. 目标侧应显示角色当前已有的库存类 UI 模块，制造系统明确排除在这一轮之外。
3. 玩家自己打开自己背包的现有入口不在这一轮内改动，但显示和修改最终指向的仍然必须是同一套库存真相。
4. 目标侧数据继续只来自正式会话暴露的观察数据，不允许 Widget 自己绕过会话去扒目标 Actor。
5. 所有交互动作继续回到 `AOInventoryUI + SubmitCurrentInteractableMutation(...)` 这条正式链。
6. 目标侧展示信息按“当前会话目标展示语义”抽象，不再继续堆 `Chest / Character` 写死分支。
7. 目标失效时直接正式关会话，不做 UI 层兜底假状态。
8. 不因为“这是角色库存”就在 UI 层发明第二套真相、第二套同步、第二套请求入口。

这八条如果后面实现时被打破，基本就可以直接视为脱离本方案。

## 6. 2026-05-28 晚间纠偏：当前 UI / MVVM 方案必须回收并重写

这一节是对本轮已经发生的错误方向做正式记账。  
不是讨论“也许可以这样”，而是明确：**当前这轮 UI / MVVM 改动语义已经明显跑偏，必须回收并重写方案。**

### 6.1 问题定性

这次跑偏的根本原因，不是“代码风格不喜欢”，而是**职责层级放错了**。

当前错误方向是：

- 让 `BackPackUI / QuickBarUI / FormalEquipmentBarUI / SkillBarUI` 这些子 UI 自己判断当前显示玩家还是对象
- 让子 UI 自己决定去哪个 Actor 身上找哪个组件
- 让 UI 底层通过枚举或目标解析逻辑，自行拼出“当前该显示谁的数据”

这和本方案原本锁定的方向不一致。

本方案真正要的不是：

- “每个子 Widget 都有一套目标判断逻辑”

而是：

- “打开角色库存和打开箱子一样，都是先建立一份正式会话”
- “这次会话需要暴露哪些数据、从谁身上取，由会话模型 / ViewModel 决定”
- “UI 只消费 ViewModel 暴露的数据，不自己决定目标对象，不自己拼装数据真相”

所以这次问题的严重性在于：

- 它不是小范围实现偏差
- 它已经开始把职责下沉到 UI 子控件
- 它会直接破坏项目现有 MVVM 口径

### 6.2 这次必须推翻的错误理解

这次已经确认，后续实现时**必须明确推翻**下面这些错误理解：

1. 不是 `UI` 决定自己当前绑定哪一个“库存上下文”。
2. 不是子 `Widget` 自己判断当前显示“玩家库存”还是“对象库存”。
3. 不是 `Widget` 自己绕过会话去目标 Actor 身上扒数据。
4. 不是为了显示目标角色库存，就给每个子栏位都发明一套“当前目标模式 / 自己模式”判断。
5. 不是 `UI` 先强制判断目标对象是否具备某组件，再决定能不能显示。

正确口径已经重新锁死为：

- **SessionModel 定义这次会话语义**
- **ViewModel 负责持有和切换这次会话的数据上下文**
- **ViewModel 负责向对象直接取数据，并裁剪这次允许暴露的数据**
- **UI 只绑定 ViewModel 暴露的数据**

### 6.3 新职责边界

后续重写方案时，职责边界正式收成下面三层：

#### 1. 交互 / 会话层

负责回答：

- 这次打开的是谁
- 当前操作者是谁
- 当前会话是否允许继续存在
- 当前会话允许暴露哪些库存相关数据

这一层不负责具体显示，但它决定整次界面的“看谁”和“给看什么”。

#### 2. ViewModel 层

这是这次纠偏里最重要的一条。

后续必须改成：

- ViewModel 持有这次会话的数据上下文
- ViewModel 直接向目标对象取需要的数据
- ViewModel 决定背包、物品栏、QuickBar、正式装备、技能栏哪些要暴露
- ViewModel 决定取不到时返回空、隐藏、或不可交互

也就是说：

- 能不能取到数据，是对象 / ViewModel 的事
- 不是 UI 先做强制判断

#### 3. UI 层

UI 的职责必须继续压回纯显示：

- 绑定 ViewModel
- 显示 ViewModel 暴露出的列表、槽位、展示信息
- 把交互请求继续交回正式请求链

UI 不再负责：

- 判断目标是谁
- 判断当前显示自己还是别人
- 判断去哪个 Actor 身上找哪个组件

### 6.4 如果同一个界面要同时显示玩家侧和目标侧，正确做法是什么

这一点这次也重新锁定。

如果一个库存界面同时显示：

- 玩家自己一侧
- 当前目标对象一侧

那也不是 UI 自己临时猜测左右两边分别是谁。

正确做法是：

- 这次会话对应的 ViewModel 同时暴露“自己一侧的数据”和“目标一侧的数据”
- 左边控件绑定 ViewModel 的自己侧字段
- 右边控件绑定 ViewModel 的目标侧字段

也就是说：

- 不是 Widget 自己持有 `SelfContext / TargetContext`
- 而是 ViewModel 持有并组织这些上下文
- Widget 只绑定 ViewModel 的不同字段

这条必须与项目现有 MVVM 口径保持一致。

### 6.5 这次必须回收的改动列表

下面这些方向已经被正式判定为越界改动，后续必须回收：

1. `AOInventoryUI` 上新增的“数据源模式枚举驱动整套目标解析”这一思路要回收，不再作为后续正式方案中心。
2. `BackPackUI / QuickBarUI / FormalEquipmentBarUI / SkillBarUI` 分别重写“当前显示谁”的做法要回收。
3. 子 UI 通过统一 `FindTargetComponent<>` 直接去目标 Actor 身上抓组件的思路要回收。
4. “打开角色库存”被实现成“每个子 UI 自己解析当前目标 Actor”的方向要回收。
5. 以 UI 层目标解析来替代会话模型 / ViewModel 组织数据的思路要回收。

这里说的“回收”，不是指否定“目标对象取数据”这件事本身。

真正要回收的是：

- **让 UI 子控件承担目标解析和数据源选择职责**

真正保留的是：

- **会话打开链本身**
- **正式 mutation 请求链本身**
- **角色作为可交互容器对象这条总语义**

### 6.6 后续重写方案时必须体现的改动点

为了让后续实现不再跑偏，新的 UI / MVVM 修正版方案必须明确写出下面这些改动点：

1. 目标对象是谁，由会话模型定义，不由子 UI 定义。
2. 当前会话允许暴露哪些库存数据，由 ViewModel 定义，不由子 UI 定义。
3. ViewModel 负责向对象取背包、物品栏、QuickBar、正式装备、技能栏等数据。
4. UI 只绑定 ViewModel 暴露出的字段，不自己决定数据源。
5. 如果目标对象某类数据不存在，或当前会话不允许显示，由 ViewModel 返回空/隐藏语义，UI 不先做强制过滤。
6. 如果同一个界面同时显示玩家侧和目标侧，左右两侧的数据组织也必须由 ViewModel 提供，而不是由左右子控件各自猜测。
7. 后续任何“对象抬头信息 / 名字 / 死亡状态 / 阵营 / 头像”之类展示数据，也继续站在“会话目标展示语义 + ViewModel 暴露字段”这条线上，不重新掉回 `Chest / Character` 写死特判。

### 6.7 当前检查口径

这次你要检查我有没有真正理解，最直接就看我后面是不是继续坚持下面这几条：

1. **UI 不决定看谁。**
2. **UI 不决定取哪份库存。**
3. **UI 不负责找目标 Actor 身上的组件。**
4. **ViewModel 负责持有并切换这次会话的数据上下文。**
5. **对象数据能不能取到，是对象 / ViewModel 的事，不是 UI 先做强制判断。**
6. **打开角色库存与打开箱子，入口语义一致，差异只体现在会话暴露的数据内容，而不是底层 UI 架构分叉。**
## 7. 2026-05-28 深夜补充：AOInventoryUI 才是这页库存界面的上下文分发层

前面那一轮纠偏，先解决的是“子 UI 自己翻目标 Actor 找数据”这个明显跑偏的问题。  
但如果后面继续往下做“同页同时显示自己库存和目标库存”，只停在这一层还不够，因为真正缺的已经不再是同步，而是这页界面的显示上下文组织。

这件事现在正式锁定：

- 不是 `Layout` 去决定这页库存界面内部每一块显示谁
- 也不是每个 `AOBackPackUI / AOQuickBarUI / AOFormalEquipmentBarUI / AOSkillBarUI` 自己判断当前到底代表谁
- 而是 `AOInventoryUI` 自己承担这页库存界面的上下文分发层职责

### 7.1 这次补充真正要解决的是什么

当前目标侧数据链已经成立：

- 目标角色库存数据由正式 `SessionModel` 暴露
- 目标侧子面板已经不再自己回目标 Actor 身上抓组件

但后面真正要做的是：

**同一个库存界面里，同时摆下两套数据上下文。**

也就是：

- 一套给玩家自己这一边
- 一套给当前会话目标这一边

如果这时候再让子面板自己判断“我是玩家区还是目标区”，最后还是会重新滑回老路。

### 7.2 AOInventoryUI 后面应该真正持有什么

后面正式实现时，`AOInventoryUI` 这页界面应该持有两套明确上下文：

- `SelfInventoryContext`
- `TargetInventoryContext`

这两套上下文里放的不是散乱状态，而是这页会真正显示出来的正式数据引用，例如：

- Backpack 对应的 ViewModel / InventoryComponent
- QuickBar 对应的 ViewModel / InventoryComponent
- FormalEquipment 对应的 ViewModel / InventoryComponent / Manager
- Skill 对应的 SkillViewDataList / SkillComponent / SkillSlotInventory

这样一来，这页界面的语义就是：

- 玩家区子面板吃 `SelfInventoryContext`
- 目标区子面板吃 `TargetInventoryContext`

### 7.3 为什么这层职责不能再下沉到 Layout

这里要专门记清楚，避免后面又有人把这层职责放错。

`Layout` 的职责更像是：

- 承载整张界面
- 管理打开、关闭、切页、界面堆栈

它不应该再往下管：

- 这张库存页内部左边是谁
- 右边是谁
- 每个子面板该绑定哪一份库存上下文

这些已经不是页面容器问题，而是库存页自身语义问题。  
所以这里不能继续往 `AOLayout_Inventory` 下沉，后面真正组织“自己这一边”和“目标这一边”的，应该是 `AOInventoryUI` 这页界面本身。

### 7.4 子面板还要再收一次：从“自取型”改成“注入型”

后面正式写 UI 的时候，下面这些子面板：

- `AOBackPackUI`
- `AOQuickBarUI`
- `AOFormalEquipmentBarUI`
- `AOSkillBarUI`

不应该继续自己做这些事：

- 自己决定当前显示玩家还是目标
- 自己去拿当前 `SessionModel`
- 自己推导当前该绑定哪份 ViewModel / InventoryComponent

它们应该统一变成“由上层注入”的模式：

- 上层 `AOInventoryUI` 先组织好这页要显示的上下文
- 再把对应那一份上下文分发给对应子面板

也就是说：

- 不是一份 `AOBackPackUI` 内部再切“我是玩家背包还是目标背包”
- 而是同类控件实例化两份
- 一份绑定玩家上下文
- 一份绑定目标上下文

同类控件，两份实例，不同绑定。

### 7.5 这次补充后的正式检查红线

后面如果继续推进这页 UI，必须继续检查下面几条有没有被打破：

1. `AOInventoryUI` 负责组织这页界面的显示上下文。
2. `Layout` 不负责库存页内部左右两边的数据上下文分配。
3. 子面板不自己决定“我是自己区还是目标区”。
4. 子面板不自己找 `SessionModel`，也不自己回 Actor 身上抓组件。
5. 玩家区和目标区是“两份面板实例 + 两份绑定上下文”，不是“一份面板内部切模式”。
6. 界面里显示谁的数据，就改谁的数据，这条红线仍然不能破。
