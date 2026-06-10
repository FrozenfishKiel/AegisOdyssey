?# 角色制造系统首版技术方�?
## 第一�?这份方案现在要解决什么问�?
这次要做的，不是单纯往背包 UI 里加一个“制造”按钮�?
真正要做的是，在现有项目已经成型的几条主链上，再接出一条新的、能长期扩展的玩法链。这个链至少要同时回答清楚下面几件事�?
1. 配方放在哪里，谁提供配方�?2. 玩家当前能不能做某个东西，这个判断基于哪些库存范围�?3. 制作开始以后，运行时状态放在哪里，谁来计时，谁来同步�?4. 制作完成以后，产物怎么统一入包，而不是另外发明一套“获得物品”逻辑�?5. 这套东西以后怎么从“角色自己搓东西”，平滑扩到“工作台、锻造台、炼药台”这一类可交互制造对象�?
这件事麻烦的地方，不在于“制造”这个词本身难懂，而在于它会天然跨很多层：

1. 库存�?2. 数据驱动�?3. UI / MVVM�?4. 联机同步�?5. 交互会话�?6. 后续的属性加成与外部制造上下文�?
如果现在直接按一个局部需求开写，很容易马上掉进两个坑�?
第一个坑，是把“角色制造”和“工作台制造”做成两套系统。前者走背包按钮，后者走交互对象特判，最后两边的配方、材料检索、计时、领奖、同步全都分叉�?
第二个坑，是把“能不能点制作”写成纯 UI 判断，把“制作正在进行”又写进某个 Widget 或临时数组，最后表现层和运行时真相层混在一起，联机一接上就开始失控�?
所以这份文档的目标很明确：先把首版角色制造系统的框架讲清楚，而且这个框架从第一天开始就要给后续“可制造对象”留出接入口�?
---

## 第二�?这次方案的边界先收在哪里

这次边界要收得很明确，不然方案会自然发散�?
### 2.1 这次先做角色身上的制造系�?
现阶段先落角色自身携带的制造能力�?
也就是说，当前默认制造上下文是：

1. 配方由角色提供�?2. 材料检索先只看角色当前允许参与制造的库存范围�?3. 制作完成后的产物先只进入角色库存�?4. 制作队列、计时、可否开始制作，都先围绕角色这一侧闭环�?
这不是说后续不做工作台�?
真正的意思是，首版先把“制造系统本体”搭起来，不把“交互对象会话化制造”一起硬塞进第一轮�?
### 2.2 这次不直接落工作�?/ 制造台完整实现

像锻造台、炼药台、烹饪台、加工台这种对象，这轮不做具体玩法接线�?
但方案必须从一开始就能容纳它们。也就是说，文档里不能把下面这些口径写死成“永远只有角色自己”：

1. 配方来源�?2. 材料来源�?3. 制造速度加成来源�?4. 产物去向�?
如果这四件事现在就写死，后面你一接工作台，就会发现只能推翻重来�?
### 2.3 这次不处理所有制造品类特�?
这轮先只支持最基本、最稳的制造语义：

1. 消耗材料�?2. 经过一段制造时间�?3. 产出一个或多个结果物�?
不在首版强行塞进来的内容包括�?
1. 制造失败率�?2. 随机词条�?3. 暴击产出�?4. 制作取消返还�?5. 多阶段加工�?6. 制作中断损失�?7. 世界对象必须占位才能继续制作�?
这些都不是不能做，而是现在放进首版，只会让结构失焦�?
不过这里要补一条当前已经明确下来的规则�?
首版可以直接把“制造中途不可取消”当成正式玩法规则立住。这样第一轮就不需要额外做取消返还，也不需要做取消回滚链路�?
### 2.4 这次不改动统一入包主链

这一点必须锁死�?
当前项目已经明确了：任何“玩家获得物品”的新系统，都应该优先接统一入包入口，而不是在来源系统里各自再写一套发货逻辑�?
现有知识库已经把这条边界钉得很死�?
1. 新系统如果想让玩家“获得物品”，应先�?`TryAddInventoryBatchToActor(...)`�?2. 不应在采集、拾取、制作等来源系统里各自再写一套“获得提示”逻辑�?3. 通知语义跟着“真正进入玩家背包”走，不跟着“来源玩法名字”走�?
所以这次制造系统的产物发放，也必须走同一条链�?
---

## 第三�?先把现有项目里已经能复用的骨架看清楚

这次方案不是从零画白板。项目里其实已经有几条非常关键的现成模式，制造系统应该主动复用它们�?
### 3.1 统一入包主链已经存在，而且口径已经稳定

当前统一入包入口是：

1. `UAOInventoryStatics::CanActorFullyAcceptInventoryBatch(...)`
2. `UAOInventoryStatics::TryAddInventoryBatchToActor(...)`

这条链的意义，不只是“有个函数能塞东西进背包”，而是它已经定义了新系统接库存时的标准姿势�?
1. 先做完整容量预演�?2. 能完整接收才真正入包�?3. 由接�?Actor 自己暴露库存组件�?4. 由库存优先级决定最终进哪个容器�?
对于制造系统来说，这一点非常关键�?
因为它意味着“制作按钮能不能点”并不需要自己重新发明一套最终产物容量判断。只要产物最终还是走统一入包，那么可点击性校验和最终提交逻辑就应该建立在同一�?`ReceiveBatch` 语义上�?
### 3.2 背包是当前角色主接收容器，这一点已经锁�?
现阶段角色获得物品的主接收容器是 `UAOBackPackComponent`�?
这件事对首版制造系统有两个直接影响�?
第一，角色制造完成后的默认产物去向不需要再讨论，先统一回背包�?
第二，后续如果某个制造对象要把产物投进它自己的库存，那应该被视为“后续上下文扩展”，而不是首版默认行为�?
也就是说，首版方案里要区分清楚：

1. “产物默认去角色背包”是当前落地事实�?2. “产物未来也可以去制造对象库存”是预留扩展口�?
### 3.3 交互对象的会话化同步模式已经存在

当前容器交互不是 UI 直接读对象真相，而是已经走了这样一条链�?
`Interact Ability -> Object-side ExecuteInteraction -> InteractionSessionComponent -> SessionModel -> Session Snapshot -> UI`

这条链的价值很大，因为它已经证明了项目对“与某个对象建立一段玩法会话”这件事，有现成的同步和 UI 组织方式�?
这对后续制造台很重要�?
虽然这轮不做工作台，但方案现在就应该顺着这条路想，而不是另起一套“制造台专属同步系统”。后面如果制造对象要参与制造，最自然的落点就是：

1. 交互对象创建制造会话�?2. 会话模型整理当前制造上下文快照�?3. `OwnerOnly` 会话状态同步给当前交互玩家�?4. 制�?UI 消费会话快照，而不是直接读对象真相�?
这条思路现在先记住，首版角色制造先不落地它，但方案必须与它兼容�?
### 3.4 MVVM 在库存侧的用法已经有一个稳定模�?
当前项目�?`UMVVM_InventoryMenu` 已经承担了几类库存型观察数据�?
1. 背包列表�?2. 快捷栏列表�?3. 正式装备栏投影列表�?4. 容器会话里的观察快照列表�?
这说明项目当前对 MVVM 的态度很明确：

1. ViewModel 负责�?UI 提供快照�?2. 真相层不放在 Widget�?3. 同一类玩法数据，允许先用一个够用的 ViewModel 模板承载，不急着为了“概念纯粹”立刻拆成很多个专用类型�?
这对制造系统的启发是：

制�?UI 也应该有自己明确�?ViewModel / 快照层，但首版不要把它做成一堆互相纠缠的�?Widget 自己拼状态�?
### 3.5 项目现有数据驱动�?DataAsset �?DataTable 并用，物品身份已经明显走总表 ID �?
当前项目里很多核心玩法定义都已经明显往 `UPrimaryDataAsset` 靠：

1. `UAOPawnData`
2. `UAOAbilitySet`
3. `UAOSkillDefinition`
4. `UAOHarvestableDefinition`
5. `UAOHarvestToolProfile`
6. `UAOAIDecisionProfile`

同时项目也不是完全不用表。它有表，而且物品身份这条链已经很明确地落在全局物品总表 `FAOItemCatalogRow` 上，主要承担 `ItemId -> ItemDefinitionClass` 的映射职责�?
这里有一条必须严格遵守的原则，需要单独写死：

**物品有且只有一个物�?ID，也就是 `ItemId`�?*

这句话落到系统设计上，真正的意思是�?
1. 物品总表是全项目物品身份的唯一收口入口�?2. 任何玩法系统都不能再发明第二套物品身份�?3. 任何玩法系统如果需要拿到物品定义，都必须通过 `ItemId` 去总表查询�?4. 制造系统也必须严格遵守这条边界�?
这说明一个很重要的设计倾向�?
1. 结构复杂、需要挂引用、后续会扩字段的玩法定义，适合 `PrimaryDataAsset`�?2. 纯编号映射、规则行配置、批量配表入口，适合 `DataTable`�?
所以这次制造系统更合理的首版解法，不是把“单条配方”先做成 `DataAsset`，而是明确拆成两张表来看：

1. 全局物品总表继续负责 `ItemId -> ItemDefinitionClass`�?2. 制造配方表负责“这条配方要哪些 ItemId、产出哪�?ItemId、耗时多久、显示什么”�?
也就是说，物品身份沿用现有总表 ID 链，制造系统自己只新增专用配方表和配方来源表，不另起一套平行物品标识体系�?
---

## 第四�?这套制造系统的核心，不是配方本身，而是制造上下文

很多制造系统一上来就盯着“配方长什么样”。这当然重要，但还不是最关键的�?
这套系统真正的核心，是制造上下文�?
因为同一个配方，在不同上下文里，真正参与判断的东西可能都不一样：

1. 谁提供这张配方�?2. 材料从哪些库存里扣�?3. 速度加成从谁身上取�?4. 产物最后送到谁身上�?5. 当前是否允许制造�?
如果不先把“上下文”建出来，后面所有逻辑都会悄悄写死在角色身上�?
### 4.1 什么叫制造上下文

可以先用一句话把它讲清楚：

制造上下文，就是“这一次制造到底是在谁的规则下发生”的那一层运行时描述�?
首版角色制造里，这个上下文很简单：

1. 配方来源是角色�?2. 材料来源是角色制造上下文声明允许参与制造的库存集合�?3. 速度加成来源是角色当前可读的制造速度属性�?4. 产物接收方是角色�?
但后面一接制造台，这四个来源就可能变化：

1. 配方来源可能变成工作台�?2. 材料来源可能变成角色库存集合 + 工作台仓储�?3. 速度加成可能来自角色天赋 + 工作台模块加成�?4. 产物接收方可能是角色背包，也可能是工作台自身容器�?
所以制造系统从第一版开始，就应该围绕“上下文可变、制造规则不变”去设计�?
### 4.2 为什么这一步不能偷�?
因为一旦现在偷懒，把所有逻辑都写成：

1. 角色有一张配方表�?2. 角色制造上下文允许的库存集合里找材料�?3. 角色身上读速度�?4. 角色背包收产物�?
那后面你要接工作台时，就不是“补一个对象侧制造能力”了，而是要把整条制造链拆开重写�?
这类重写通常最伤，因为它会同时打穿�?
1. 数据表设计�?2. UI 列表加载方式�?3. 材料检索逻辑�?4. 制作中状态同步�?5. 完成时产物投递�?
所以这次方案里，角色制造虽然先做，但语义上不能把“角色”写成系统唯一宿主�?
### 4.3 制造上下文对外最好只暴露几类决议，不要长成一个万能对�?
“制造上下文”这个抽象是必须要有的，但它也不能一抽出来就变成一个什么都往里塞的胖对象�?
更稳的做法，是让它只对制造系统暴露几类稳定职责：

1. 当前能提供哪些配方�?2. 当前哪些库存可以作为材料来源�?3. 当前产物应该投递给谁�?4. 当前总制造时长倍率是多少�?5. 当前还需要满足哪些上下文级额外限制�?
这五类东西里，前四类是主链，最后一类是给后面扩“工作台限定”“场景限定”“状态限定”这一类条件留的口子�?
这里真正重要的不是“是不是非要做一个接口类”，而是职责边界要先想清楚。制造系统本体应该只问这些决议结果，不应该直接知道“角色等级怎么查”“工作台模块怎么叠加”“哪个库存来自谁”这种来源细节�?
首版角色制造可以先不把这层做得特别花。完全可以由角色制造组件在请求开始时组装一个轻量运行时上下文，把上面几类决议结果整理好，再交给制造主链继续走�?
但哪怕首版实现先朴素一点，这个分层口径也最好先立住。否则后面一接对象制造，上下文来源、材料来源、接收方来源都会直接反向侵入制造核心逻辑�?
---

## 第五�?首版建议的对象模�?
这一章开始正式进入结构设计�?
这里不会直接把类名写成不可更改的死方案，但会先把对象分层讲清楚。只要分层对了，后面具体命名其实可以微调�?
### 5.1 配方定义层：首版建议直接落专�?Recipe DataTable，不建议先拆成单�?DataAsset

原因很实际�?
一条真正可扩展的制造配方，未来几乎一定会挂很多引用和规则字段。除了最基础的材料和产物，它通常还会长出下面这些东西�?
1. 配方显示名�?2. 配方图标�?3. 配方分类�?4. 制造时长�?5. 产物列表�?6. 材料列表�?7. 解锁条件�?8. 制造站限制�?9. 批量制造规则�?10. 额外校验标签�?11. UI 描述与排序信息�?
但这不自动等于首版就该做�?`DataAsset`�?
因为你们项目现在已经有一条真实在用的全局物品总表链路，采集等奖励入口本身就在�?`ItemId` 驱动物品解析。制造系统如果首版绕开这条现成链，反而会把“配方系统”和“物品总表系统”做成两套并行身份来源�?
所以更贴合当前项目现状的首版做法是�?
1. 单条配方本体直接落在专用 `Recipe DataTable`�?2. 配方行里材料与产物先�?`ItemId + Count` 表达�?3. 运行时再通过现有全局物品总表�?`ItemId` 解析�?`ItemDefinitionClass`�?4. 角色或工作台“能提供哪些配方、何时开放”再由独立的配方来源表承载�?
这样既保留了你一开始说的“配方以表的形式给出”，也和当前项目已经存在的物品总表设计保持同一条口径�?
### 5.2 配方目录层：角色侧建议单独有“制造表”资�?
你提到“角色身上的制作表会随着等级逐渐开放”，这个表建议单独设计成一个“配方目录资产”，而不是把所有解锁规则写进单条配方里�?
原因在于，配方本体和“谁在什么条件下能看到它”不是一回事�?
同一条配方：

1. 可能对角色是 10 级解锁�?2. 对某个工作台是默认开放�?3. 对某个特殊玩法模式是禁用�?
如果把这些规则写回配方本体，配方就会被不同来源上下文污染�?
更干净的分层应该是�?
1. `Recipe DataTable` 只描述“做什么、要什么、多久做完、产出什么”�?2. `Recipe Source Table` 描述“谁能提供哪些配方、何时开放”�?
对于首版角色制造，建议有一个角色制造表资产，里面至少包含：

1. 配方行标识或配方 ID�?2. 解锁等级�?3. 是否默认显示�?4. 排序权重�?5. 所属页签或分类�?
这样后面工作台要接进来，也只是换一个来源资产，而不是推翻配方本体�?
### 5.3 配方目录除了“有没有”，最好顺手区分“看不看得见�?
这一层看起来�?UI 细节，其实不是�?
因为“角色当前不能做某个配方”至少有两种完全不同的语义：

1. 这条配方对当前角色根本还没开放，不应该出现在列表里�?2. 这条配方已经进入当前角色的制造表，但还没达到解锁条件，可以显示成未解锁状态�?
这两种东西如果混成一个布尔值，后面列表表现、引导提示、升级解锁反馈都会很别扭�?
所以角色制造表这一层，除了配方行标识和解锁条件，最好还要顺手把“默认可见性”一起管住。首版哪怕只做很简单的等级解锁，列表语义也建议至少区分�?
1. 完全隐藏�?2. 可见但未解锁�?3. 已解锁可参与制造�?
这样后面你要补“角色升级后新解锁提示”或者“工作台上展示高阶未解锁配方”时，数据层不用再返工�?
### 5.4 材料项与产物项建议复用统一入包语义

制造系统里最容易另起炉灶的一层，是“产物条目”和“材料条目”的结构�?
这里建议尽量往现有库存语义靠，尤其是产物侧�?
首版建议�?
1. 材料项使�?`ItemId + Count` 作为基础表达�?2. 产物项也先使�?`ItemId + Count` 作为基础表达�?
不要在首版就引入“制造产物必须经过另一套中间物品描述结构”，也不要让制造系统自己再发明第三套物品身份键�?
理由很简单�?
当前统一入包入口 `FAOInventoryReceiveBatch` 本身已经是围绕“定义项批量入库 / 实例项批量入库”设计的。首版制造系统更自然的做法，是先把配方里�?`ItemId` 通过全局物品总表翻译�?`ItemDefinitionClass`，再组装�?`DefinitionEntries` 走统一入包�?
这件事的好处很大�?
1. 容量判断直接复用�?2. 入包行为直接复用�?3. 获取通知直接复用�?4. 后面制造产物进入角色背包，不需要额外写 UI 提示主链�?
### 5.5 制造运行时状态层必须独立出来

这层非常关键�?
配方定义和制造运行时不是一回事。定义层只是静态配置，真正联机最敏感的是运行时状态�?
首版建议明确有一层“制造任务运行时状态”，至少描述下面这些内容�?
1. 当前在做哪条配方�?2. 本次制造的请求发起者是谁�?3. 本次制造上下文是谁�?4. 开始时间�?5. 结束时间或剩余时间�?6. 当前状态�?7. 本次将要产出的结果快照�?8. 本次消耗过的材料快照�?
这层数据不应该散�?Widget、临时数组、计时器闭包或�?Blueprint 本地变量里�?
它应该是明确的、可复制的、服务端权威的运行时真相层�?
### 5.6 制造能力宿主层首版建议落在角色组件，而不是背包组件本�?
首版角色制造系统建议新增一个独立的角色组件，比如“制造组件”这一类东西，而不是直接把制造逻辑塞进 `BackPackComponent`�?
原因有三层�?
第一，背包组件的职责当前已经很清楚，就是库存容器。它应该继续负责�?
1. 容量�?2. 入包�?3. 移动�?4. 使用�?5. 统一库存快照�?
而不应该再额外承担“角色当前能做哪些配方”“正在做什么”“制作速度怎么算”这种玩法语义�?
第二，制造能力天然更接近角色玩法能力，而不是库存本体能力�?
第三，后续工作台要接进来时，如果制造本体是一个独立组件或独立运行时对象，就更容易把“角色制造”和“对象制造”统一进同一个框架�?
---

## 第六�?首版建议的数据结构怎么�?
这一章把数据层拆得再具体一点�?
### 6.1 单条配方定义建议包含哪些字段

单条配方定义建议至少包含下面这些字段�?
第一组，�?UI 与导航字段：

1. 配方 ID�?2. 配方显示名�?3. 配方描述�?4. 配方图标�?5. 配方分类标签�?6. 配方排序权重�?
第二组，是制造本体字段：

1. 制造时长基值�?2. 材料列表�?3. 产物列表�?4. 是否允许批量制造�?5. 单次制造产出是否固定�?
第三组，是规则字段：

1. 所需解锁标签或条件�?2. 是否需要特定制造上下文标签�?3. 是否允许在移动中制造�?4. 是否允许并行制造�?
首版不一定要一次把这些字段都写满，但结构上建议预留�?
### 6.2 角色制造表建议包含哪些字段

角色制造表建议承担“角色能从哪里看到哪些配方”的职责�?
它至少可以包含：

1. 配方行标识或配方 ID�?2. 解锁等级�?3. 是否默认显示�?4. 分类页签�?5. 排序权重�?
如果后面角色还有职业、专精、阵营之类限制，也可以继续长，但首版建议先把“等级解锁”立住�?
### 6.3 首版解锁规则建议先收成等级解锁，不要一上来做通用条件�?
这件事建议现在就收口�?
因为一旦首版想把解锁规则一步做成“任意条件组合”，你很快就会把制造系统牵到很多还不该一起进场的边界里：

1. 任务进度�?2. 阵营或职业限制�?3. GameplayTag 条件树�?4. 编辑器配置校验�?5. 联机下解锁状态同步�?
这些都不是制造主链的第一优先级�?
所以首版更稳的口径是：

1. 配方本体不负责角色解锁规则�?2. 角色制造表首版只正式支持“等级解锁”�?3. 其他复杂条件先只做字段预留，不承诺这一轮全部落地�?
这并不保守，反而更利于把边界钉住。因为等级解锁本身就已经能覆盖你这次最明确的角色成长开放需求，而且它不会把制造系统过早绑定到别的成长子系统上�?
等后面真要扩展成职业、专精、任务线或者工作台条件时，也应该优先扩“配方来源侧的解锁求值逻辑”，而不是回头污染单条配方定义�?
### 6.4 为什么角色制造表不建议直接绑�?PawnData 的现有等级表�?
这是个容易顺手写进现�?`PawnData` 的地方，但我不建议首版这么做�?
原因不是“绝对不能”，而是当前 `PawnData` 里已经承担了很多角色基础构成与成长配置。制造表如果直接并进去，短期看方便，长期很容易把“角色战斗基础数据”和“角色生活玩法配方开放”搅在一起�?
更稳妥的做法是：

1. `PawnData` 可以持有一个“角色制造表资产引用”�?2. 具体配方开放列表依然由独立制造表资产维护�?
这样以后不同角色模板、不同职业模板、不同玩法模式要换制造表时，修改边界会更清楚�?
### 6.5 材料和产物为什么首版更适合先认 ItemId，再通过总表解析 DefinitionClass

首版建议材料和产物都先用 `ItemId` 识别，而不是在制造系统里直接塞一�?`ItemDefinitionClass`�?
原因很现实�?
当前库存体系本身确实是围绕：

1. `UAOInventoryItemDefinition`
2. `UAOInventoryItemInstance`

但你们项目现在也已经明确补了一条全局物品总表链，�?`ItemId` 作为跨系统物品身份入口在使用。制造系统如果首版继续沿用这条链，反而更统一�?
而且这里不是“更统一”这么简单，而是必须遵守的项目原则：

1. 物品身份唯一键只�?`ItemId`�?2. 物品总表负责收揽物品身份�?3. 任何系统都只能通过 ID 去查物品�?
如果现在改成“制造系统直接配 DefinitionClass，不�?ItemId”，你马上会遇到两件事：

1. 制造系统和采集、掉落等现有 ID 驱动入口的物品身份口径不一致�?2. 后面配表和跨系统对接时，又要补一�?Recipe -> Definition -> ItemId 的反向对齐�?
所以首版制造系统更合理的做法是�?
1. 配方静态配置直接挂 `ItemId`�?2. UI 显示、材料检索、产物发放前，再统一通过全局物品总表解析�?`ItemDefinitionClass`�?3. 制造系统自己不维护第二�?`ItemId -> DefinitionClass` 映射�?4. 制造系统内部也不引入第二套“制造专用物品键”�?
这样能把“物品身份”继续锁在现有总表里，把“制造规则”锁在配方表里，两边职责更干净�?
---

## 第七�?首版运行时链路应该怎么�?
这一章开始讲“玩家点一下制造”以后，到底应该发生什么�?
### 7.1 制造入口不是按钮，而是一条标准请求链

无论是背包界面里点某个可制造物，还是以后制造台里点某条配方，制造入口都不应该被理解成“某个按钮直接开始倒计时”�?
更准确的理解应该是：

玩家发起了一次制造请求，这个请求要经过统一的可制造性判断，成功后才会生成一条制造中的运行时任务�?
首版角色制造建议链路如下：

1. UI 选择当前配方�?2. ViewModel 刷新这条配方的材料、耗时、可制造性描述�?3. 玩家点击制造�?4. 客户端只做表现层禁点和本地展示，不直接认为制造成功�?5. 服务端组件收到制造请求�?6. 服务端再次按运行时真相做一次轻量校验�?7. 校验通过后，先扣材料，再创建制造任务�?8. 任务到时完成后，服务端统一发放产物�?
这里要注意，用户原话里提到“不需要太过于在内部做太多判断”。这句话我理解的是：

1. 不要把系统写成到处兜底、到处重算、到处特判的臃肿逻辑�?2. 但这不等于服务端可以完全不做最终校验�?
联机场景下，服务端至少还是要守住最基本的真相边界�?
### 7.2 为什么要先扣材料，再进入制造中

这个顺序建议锁死�?
首版不建议做“先开始计时，完成时再扣材料”�?
原因很直接�?
如果完成时才扣材料，你马上会遇到这些问题�?
1. 制造过程中玩家把材料挪走怎么办�?2. 制造过程中别的系统消耗了同一批材料怎么办�?3. 制造期�?UI 上材料足够，但完成时突然失败，玩家体验会很奇怪�?
而“开始时先扣材料”这条链虽然看起来保守，但结构更稳：

1. 制造一旦开始，这次任务就已经拥有了自己的成本前提�?2. 后续只需要关心计时和发货�?3. 不需要在完成点重新争抢材料状态�?
所以首版建议：

1. 可点按钮前，UI 基于快照禁点�?2. 真正入队时，服务端正式扣材料�?3. 材料扣除成功，这条队列任务才允许进入制造队列�?
### 7.3 制造完成时为什么必须再做一次最终入包能力判�?
你在需求里说，如果库存已满，也应该禁止制作�?
这条判断当然要在 UI 上先做，而且要尽量准确�?
但服务端完成时，仍然建议保留最终的“能否完整接收产物”判断，而且直接复用统一入包入口的语义�?
原因很实际�?
因为从玩家点下开始制作，到制作真正结束，中间是有时间差的�?
只要有时间差，就会发生下面这些事情：

1. 玩家中途往背包里塞了别的东西�?2. 其他玩法在这段时间内给玩家发了奖励�?3. 背包容量状态在制作结束时已经变了�?
所以首版建议这样处理：

1. 开始前，UI 用当前快照判断是否可做�?2. 开始时，服务端做一次正式材料扣除和产物可接收校验�?3. 完成时，服务端在发货前再走一次统一入包校验�?
至于完成时如果发现容量已不够怎么处理，首版需要明确口径�?
我建议首版不要把这件事做成“物品凭空丢失”，也不要把它做成“服务端偷偷塞进背包”�?
更贴合当前需求的首版口径是：

1. 理论上开始前已经禁止这类任务�?2. 如果制造期间容量发生变化导致完成时无法入包，直接把本次产物以掉落物的形式丢到世界里�?
这个口径的好处很直接�?
1. 不需要再补一套“待领取 / 待投递”状态机�?2. 不需要额外做重新领取入口�?3. 完成点结果是确定的，不会停在半完成状态�?4. 顺手也能把“丢弃物�?/ 掉落物生成”这条链一起立住�?
当然，这也意味着掉落链必须做成正式能力，而不是制造系统里的临时特判。也就是说，制造完成后如果无法完整入包，应该走统一的掉落物生成口径�?
当前这条掉落规则也可以顺手收成更具体的首版边界：

1. 掉落点在角色脚下�?2. 掉落物默认持�?`5` 分钟�?3. 掉落物可以被拾取�?4. 首版不做所有权保护�?
### 7.4 首版直接按正式制造队列设计，但规则先收死为“不可取消�?
这一轮讨论后，这里的口径要改成正式队列方案�?
首版不再按“单任务过渡版”设计，而是直接按制造队列设计�?
但同时规则也要明确：

1. 队列是正式需求�?2. 中途不可取消�?3. 不做取消返还�?4. 队列上限先收�?`5` 条�?
这样一来，虽然队列本身比单任务更复杂，但有几件最烦的事反而先被主动砍掉了�?
1. 不需要处理取消回滚�?2. 不需要处理中途返还材料�?3. 不需要处理取消后�?UI 状态回退�?
### 7.5 队列阶段真正要立住的是任务记录模型和调度规则

既然首版明确要做队列，那运行时结构就不该再围�?`CurrentTask` 这种单任务心智来写�?
更合理的做法是：

1. 组件内部正式维护任务队列�?2. 每条任务都是完整记录，而不是一堆散字段�?3. 当前执行中的任务和排队中的任务，共用同一套任务结构�?
每条任务至少要带�?
1. 配方行标识或配方 ID�?2. 入队顺序�?3. 当前状态�?4. 开始时间�?5. 结束时间�?6. 产物快照�?7. 扣料快照�?8. 完成后的投递结果�?
在“不可取消”的前提下，队列调度反而更容易收口�?
1. 新任务入队时按当前规则决定能否进入队列�?2. 当前任务完成后，下一条任务自动推进�?3. 不存在玩家中途撤销导致的大量回滚�?
此外，当前还可以顺手把中断规则直接定死：

1. 角色死亡时，直接清空制造队列�?2. 角色下线时，直接清空制造队列�?3. 切图时，直接清空制造队列�?4. 角色或宿主被销毁时，直接清空制造队列�?5. 以上情况都不返还已经扣掉的材料�?
---

## 第八�?制造时间和加成应该怎么设计，才不会后面又重�?
你特别提到制造时间会受到加成，而且这个加成不一定来自角色，也可能来自可制作对象�?
这正好再次说明，“制造上下文”比“角色自己制造”更重要�?
### 8.1 制造时间不要直接写成“角色属�?* 某个比例�?
首版不建议把制造速度设计成“只认角色某个字段”的硬编码逻辑�?
更合理的写法应该是：

1. 配方定义有基础制造时长�?2. 制造上下文对外提供一个“总制造速度修正值”或“总制造时长倍率”�?3. 最终耗时统一由制造系统按同一公式结算�?
这件事表面上只是“以后好扩展”，但其实它还能减少这次首版里的耦合�?
因为这样首版角色制造时，你只需要让角色上下文返回它自己的当前制造加成即可。以后工作台接进来，也只是让工作台上下文在同一接口里再叠加一层环境加成�?
### 8.2 如果是角色制造，就直接落“角色总制造加成”这一类总属�?
你这里已经把口径说明确了，那方案也应该跟着收正�?
如果是角色身上的制造系统，更直接的做法就是给角色一个“总制造加成”属性，语义上和 `Attack` 这类总属性一致，由当前角色身上的实际属性链去结算出一个总值，然后制造系统直接读取这个总值�?
也就是说，首版这里可以直接收成：

1. 配方定义有基础制造时长�?2. 角色侧有一个总制造加成属性�?3. 最终制造时长由基础时长和这个总属性统一结算�?
这样做的好处是：

1. 不需要制造系统自己管理一堆分散乘区�?2. 角色制造的数值来源很清楚�?3. 和当前角色属性体系的思路一致�?
### 8.3 当前既然派生属性问题已经收口，这层就可以直接接角色总制造加�?
既然你已经明确当�?`GAS` 派生属性的问题已经解决了，那这层方案也不该继续保留“先别接属性链”的旧保守说法�?
更直接的口径应该是：

1. 角色制造系统读取角色当前总制造加成�?2. 这个总制造加成和其他总属性一样，走现有属性链路�?3. 制造系统只负责消费最终结果，不自己参与属性派生逻辑�?
---

## 第九�?材料检索范围这件事，首版必须先把口径说�?
你在需求里特别强调了一点：材料检索不一定只看角色库存，某些可制作对象也可以纳入范围�?
这句话很重要，因为它直接决定了“材料来源”这层不能写死�?
### 9.1 角色制造的材料来源就按扫描角色身上全部库存来设�?
这一点按当前最新口径改�?
角色制造的材料来源，直接按扫描角色身上全部库存来设计�?
也就是说，首版这里不再坚持“只认一组显式声明的角色库存集合”，而是明确收成�?
1. 角色当前挂载的全部库存都参与材料检索�?2. 制造系统按统一规则汇总这些库存里的材料数量�?3. 服务端扣料时也按同一套全量扫描结果执行�?
### 9.2 扫描角色全部库存以后，真正要收清楚的是扣料顺序和去重规则

既然材料来源明确是角色身上全部库存，那这里真正需要收口的，就不再是“扫不扫全部库存”，而是“扫描后按什么顺序扣”�?
否则同一份材料分散在多个库存里时，前后端很容易对扣料结果产生歧义�?
所以这里建议补一条统一规则�?
1. 角色全部库存参与扫描�?2. 材料检索时按角色库存组件的固定注册顺序汇总�?3. 材料扣除时沿用同一固定注册顺序执行�?
### 9.3 材料扣除也必须和检索范围共用同一套来源决议结�?
这是另一个很容易做歪的点�?
有些系统会在 UI 上扫描一遍材料来源，显示“你材料够了”，然后真正开始制造时再重新现场乱找一遍，最后扣到的甚至不是 UI 当时看到的那一批�?
这会制造非常差的体验，也会给联机校验带来不必要的歧义�?
首版建议�?
1. 先基于当前制造上下文做一次材料需求解析�?2. 得到一份明确的“本次计划从哪些库存、哪些槽位、扣多少”的扣料计划�?3. UI 可制造性显示基于这份计划�?4. 服务端真正开始制造时，也基于同一套决议逻辑重新生成并确认扣料计划�?
也就是说，材料检索与材料扣除不是两套思路，而是一套逻辑的“预演视图”和“正式执行”�?
---

## 第十�?首版 MVVM 应该怎么接，才不会把真相层写�?UI

制�?UI 是这次很容易失控的一层�?
因为它同时要显示�?
1. 配方列表�?2. 当前选中配方详情�?3. 材料需求与当前持有量�?4. 是否可点击�?5. 制造中状态�?6. 剩余时间�?
如果没有清楚�?ViewModel 分层，最后一定会散到 Widget 里�?
### 10.1 首版建议至少拆成两层视图数据

我建议首版至少拆成两层�?
第一层，是“可制造配方列表快照”�?
它负责回答：

1. 当前有哪些配方可见�?2. 每条配方的显示信息是什么�?3. 是否已解锁�?4. 是否可点击�?5. 当前排序与分类�?
第二层，是“当前选中配方详情快照”�?
它负责回答：

1. 当前选中哪条配方�?2. 需要哪些材料�?3. 每种材料当前拥有多少�?4. 哪些材料不足�?5. 当前制造时长是多少�?6. 当前点击为什么被禁用�?
如果首版直接把所有状态散在一堆按钮和格子上，后面连“为什么现在不能做”都很难统一解释�?
### 10.2 制造中的运行时状态也应该有单独快�?
当前项目在库存、正式装备栏、交互容器这些地方，都已经证明了一个模式是可行的：

运行时真相层在服务端或组件里，UI 消费的是明确快照�?
制造中状态也应该遵守这件事�?
所以建议再单独有一份“当前制造状态快照”，至少包含�?
1. 是否有进行中任务�?2. 当前配方是谁�?3. 开始时间�?4. 结束时间�?5. 剩余时间�?6. 当前状态文本�?7. 当前是否允许取消�?
这份快照不一定非要在首版就做得非常庞大，但它必须存在，否则倒计时和状态文案最后都会长�?Widget�?
### 10.3 为什么首版不建议复用 `UMVVM_InventoryMenu`

虽然项目�?`UMVVM_InventoryMenu` 很好用，但我不建议首版把制造系统也硬塞进它�?
原因在于，`InventoryMenu` 的快照语义已经很明确了，它服务的是一类“槽位型物品列表观察”�?
而制造系统的核心观察数据并不是“库存槽位列表”，而是�?
1. 配方列表�?2. 材料需求列表�?3. 制造状态�?
如果把这些再硬塞�?`InventoryMenu`，短期也许能少建几个类型，长期只会把 ViewModel 语义做乱�?
所以首版更稳的方向是：

1. 库存显示继续复用现有库存 ViewModel�?2. 制造系统单独引入自己的 ViewModel / 数据快照类型�?
### 10.4 首版 UI 刷新驱动建议以“当前选中配方 + 当前库存变化”为�?
首版角色制造里，当前详情刷新主要受两件事驱动：

1. 玩家切换了当前选中配方�?2. 角色库存发生了变化�?
这正好是比较稳的首版刷新边界�?
也就是说，当前制造详情页不需要每帧去猜状态。更合适的方式是：

1. 选中配方改变时，重算详情快照�?2. 相关库存变动时，重算详情快照�?3. 制造中剩余时间如果要显示秒级刷新，再单独针对制造任务状态做轻量更新时间推进�?
这样能把“数据变化触发刷新”和“计�?UI 自身刷新”分开，不会让整页逻辑一直抖�?
---

## 第十一�?联机同步问题真正难在哪里

你把联机同步列成难点是对的，而且这是这次方案里必须提前挑明的部分�?
这件事难的地方，不在于“会不会�?RPC”，而在于哪些数据应该同步，哪些不应该同步�?
### 11.1 配方静态定义不应该当成高频同步对象

配方本体是静态资产，不是高频运行时状态�?
它不应该被理解成�?
1. 每次打开制造界面都复制整份配方�?2. 每次库存变化都把配方细节再同步一次�?
更合理的思路是：

1. 静态配方定义由本地资产与引用提供�?2. 联机真正同步的是当前角色已解锁哪些配方、当前选中哪个、当前是否有进行中任务、当前制造任务状态是什么�?
这能明显减轻运行时同步负担，也更符合项目现有“静态定义走资产、运行时状态走组件 / 会话快照”的风格�?
### 11.2 首版角色制造仍然建议服务端权威，但网络侧尽量少�?RPC

这条建议非常明确�?
首版角色制造不要做客户端预测制造完成，也不要做本地伪造制造成功�?
客户端在这条链上主要承担�?
1. 显示配方�?2. 根据当前快照禁点�?3. 显示倒计时�?4. 在必要时发送制造请求�?
服务端承担：

1. 最终校验�?2. 扣材料�?3. 建立制造任务或推进入队�?4. 计时完成�?5. 统一入包或掉落�?6. 更新运行时状态�?
这里还要补一条当前已经明确的网络原则�?
不是说完全不能写 `RPC`，而是能靠状态同步表达的，就尽量不要额外�?`RPC`�?
更具体一点说�?
1. 制造请求这种“客户端主动发起动作”的入口，必要时还是要有请求�?`RPC`�?2. 但队列状态、当前进行中任务、剩余时间、完成结果这些持续状态，优先走组件复制或状态同步�?3. 不要为了图省事，把一连串运行时状态变化全都改成事件型 `RPC` 推过去�?
### 11.3 首版建议制造任务状态走组件复制，不要先做会话专属同�?
因为这轮先只做角色自身制造，不是工作台会话制造，所以首版建议制造运行时状态直接挂在角色制造组件上，并作为角色侧可复制状态同步�?
这样做的好处是：

1. 打开背包就能看到当前角色自己的制造状态�?2. 不依赖当前是否有某个交互会话�?3. 重连和重新打开 UI 时更容易恢复�?
以后工作台制造要接会话化同步时，再在对象制造上下文那一侧走 `InteractionSessionComponent -> SessionModel -> Snapshot` 这条链即可�?
也就是说�?
1. 角色制造首版先用角色组件复制状态�?2. 对象制造后续再走交互会话复制状态�?
这不是分裂，而是因为两者的持有者本来就不一样�?
### 11.4 为什么“库存变化导致可制造性变化”这层同步不能偷�?
制造系�?UI 上最常见的一个错误体验，就是本地明明看见材料够了，点下去却又失败，或者本地看见不能做，结果其实早就能做�?
这说明“可制造性”虽然是表现层概念，但它的计算输入是运行时真相�?
首版建议把这个问题看成两层：

1. 客户端快照层：根据当前本地已知库存和制造状态更新禁点�?2. 服务端真相层：开始制作时仍然做最终确认�?
这里不需要为了“严格正确”就把每个细节做成复杂预测同步，但也不能完全放弃最终真相校验�?
---

## 第十二章 首版方案里最值得提前指出的几个明显风�?
这部分我单独拎出来讲，因为你要求里明确说了，要指出明显风险�?
### 12.1 风险一：把首版角色制造写成永久只能角色自�?
这是最大风险之一�?
如果现在设计时不抽“制造上下文”，后面工作台一来，重构面会非常大�?
这个风险不是未来才会爆，它一旦在首版类结构里写死，后面每多写一个函数，重构成本都会变高�?
所以这轮方案里最重要的防线就是：

1. 配方来源可变�?2. 材料来源可变�?3. 速度来源可变�?4. 产物去向可变�?
首版虽然都先默认是角色，但系统接口不能把这些默认值写成唯一真相�?
### 12.2 风险二：把制造能力直接塞进背包组�?
这会让库存系统和玩法系统耦合得太紧�?
一旦后面工作台或其他对象也要拥有制造能力，你就会发现不得不把角色背包逻辑又抽回来�?
而且背包组件当前已经承担了很多职责，再把制造也压进去，只会让模块边界变浑�?
### 12.3 风险三：队列规则没收清楚，结果做成一半任务系统、一半取消系�?
现在既然队列已经是正式需求，那风险点就不再是“做不做队列”，而是“队列规则没有先写死”�?
最关键的边界就是：

1. 队列要做�?2. 中途不可取消�?3. 不做取消返还�?
如果这三条不先锁死，最后最容易出现的不是“队列难”，而是系统同时背着�?
1. 队列调度�?2. 取消回滚�?3. 材料返还�?4. UI 状态回退�?
### 12.4 风险四：扫描角色全部库存却没有统一扣料顺序和去重规�?
你现在已经明确角色制造支持扫描全部库存，那风险点也应该跟着换�?
真正危险的不是“扫描全部库存”本身，而是扫描了全部库存以后，没有把扣料顺序、重复统计和来源决议规则收清楚�?
如果这层不钉死，后面最容易出现�?
1. UI 显示材料够了，但服务端扣到的是另一批来源�?2. 不同库存里同类物品的扣除顺序前后不一致�?3. 多人环境下本地显示和服务端结算理解不一致�?
### 12.5 风险五：制造系统自己参与角色总制造加成的派生计算

既然你已经明确角色总制造加成会�?`Attack` 一样作为正式总属性存在，那风险点就不该再写成“不要接属性链”�?
现在真正要防的是：制造系统自己去参与这个总属性怎么派生、怎么叠层、怎么刷新�?
更稳的边界应该是�?
1. 属性系统负责把角色总制造加成算出来�?2. 制造系统只消费最终总值�?
### 12.6 风险六：把可制造性完全当�?UI 判断，服务端不守最终边�?
用户体验上可以强调禁点，但联机场景下，服务端仍然需要守住最低限度的真相�?
1. 材料是否仍足够�?2. 当前是否已有进行中任务�?3. 产物是否仍有完整接收空间�?
### 12.7 风险七：在制造系统里旁路物品总表，偷偷形成第二套物品身份

这是必须提前点名的高风险�?
如果制造系统内部为了省事，开始直接存私有定义映射、缓存第二份身份表、或者再发明一套制造专用物品键，短期看也许少了一步查表，长期一定会把“物品有且只有一�?`ItemId`”这条原则打穿�?
这个风险一旦发生，后面最容易出现的问题就是：

1. 制造系统里的物品身份和采集、掉落、入包等现有系统不一致�?2. 配表入口开始分裂成“总表驱动”和“制造特供驱动”�?3. 同一个物品在不同系统里的认知开始漂移�?
所以这条边界必须锁死：

1. 物品身份唯一键只�?`ItemId`�?2. 物品总表是唯一合法的身份收口点�?3. 制造系统只能消费这条链，不能旁路、不能镜像、不能补第二套身份�?
否则这条链只要一进网络环境，迟早会被边界情况打穿�?
---

## 第十三章 首版建议的具体落地顺�?
既然这轮先写方案，不写代码，那落地顺序也要先说清楚。这样后面真的开始做，不会又回到一堆平行问题同时开工的状态�?
这里我不想只写抽象步骤了，直接把实现拆成几个可以落地推进的阶段�?
### 13.1 第一阶段：先把静态配置和数据入口立住

第一步先做：

1. 专用 `Recipe DataTable`�?2. 角色配方来源表�?3. 材料项与产物项的 `ItemId + Count` 结构�?4. 配方显示、分类、排序字段�?5. 角色总制造加成需要读取的配置入口�?
这一阶段的目标，不是让制造能跑，而是让“数据怎么配置”先有一个稳定口径�?
这一阶段做完以后，应该已经能回答清楚�?
1. 配方怎么配�?2. 角色怎么拿到可用配方�?3. 制造系统怎么通过 `ItemId` 查物品�?
### 13.2 第二阶段：把角色制造组件、队列状态和入队扣料主链立住

第二步再做：

1. 角色制造组件�?2. 制造队列运行时状态�?3. 上下文决议结果的组装入口�?4. 队列上限 `5` 的规则�?5. 入队即扣料的服务端请求入口�?6. 扫描角色全部库存并按固定注册顺序扣料的实现�?
这一阶段要把“制造真相层到底放哪”“队列怎么进”“材料怎么扣”先钉死�?
这一阶段做完以后，应该至少已经能做到�?
1. 请求进入制造队列�?2. 成功入队就扣材料�?3. 队列状态在服务端成立并可同步�?
### 13.3 第三阶段：把计时推进、完成入包、满包掉落和清空规则接完�?
第三步再做：

1. 当前制造任务自动推进�?2. 完成时统一入包�?3. 满包时掉落到角色脚下�?4. 掉落物持�?`5` 分钟、可拾取、不做所有权保护�?5. 死亡 / 下线 / 切图 / 销毁时清空队列且不返还材料�?
这一阶段做完以后，制造系统主链才算真的闭环�?
### 13.4 第四阶段：补角色制�?UI / MVVM 和最小联机同步面

第四步再做：

1. 配方列表快照�?2. 当前选中配方详情快照�?3. 基于角色库存变化的可制造性刷新�?4. 基于当前制造队列状态的制造中显示�?5. 最小必要请求型 `RPC`�?6. 其余持续状态优先走组件复制或状态同步�?
这一阶段重点是让 UI 和联机表现都消费清晰快照，而不是自己猜�?
### 13.5 第五阶段：再考虑后续对象制造上下文怎么�?
等角色制造主链稳了，再继续往对象制造扩�?
这个阶段才去做：

1. 可制造对象如何提供配方�?2. 可制造对象如何暴露材料来源�?3. 可制造对象如何通过交互会话给当前玩家同步制�?UI 所需快照�?
这样扩展时，你是在“已有制造系统上接新上下文”，而不是“重新发明第二套制造系统”�?
---

## 第十四章 这次方案最后想锁住的几句核心口�?
这部分我直接收成最重要的几条结论，后面真开始实现时，应该默认以这些口径为准�?
第一，首版先做角色自身制造，但系统设计不能把“角色”写成唯一宿主。真正稳定的抽象应该是制造上下文，而不是角色特判�?
第二，首版单条配方本体建议直接落专用 `Recipe DataTable`，物品身份继续严格复用现有全局物品总表�?`ItemId -> ItemDefinitionClass` 链。角色能提供哪些配方、何时开放，再由独立配方来源表组织�?
第三，材料与产物静态配置首版先直接�?`ItemId + Count`，运行时再通过全局物品总表解析�?`ItemDefinitionClass`。这里必须严格遵守“物品有且只有一�?`ItemId`，总表负责收揽，通过 ID 查物品”的原则。产物最终统一翻译�?`FAOInventoryReceiveBatch`，并�?`TryAddInventoryBatchToActor(...)`。制造系统不另起一条发货主链�?
第四，首版角色制造直接按正式制造队列设计，当前规则先收死为“入队就扣、队列上�?`5`、中途不可取消、异常清空不返还材料”。先把入队、扣料、推进、完成、掉�?入包、同步这条主链做稳�?
第五，UI 上的“能不能点制作”可以尽量靠当前快照禁点，但服务端仍然必须守住最基本的真相边界。不要把制造成功与否完全外包给表现层�?
第六，如果是角色身上的制造系统，就直接让角色提供一个总制造加成属性，语义上和 `Attack` 这类总属性一致。制造系统只消费最终总值，不自己参与属性派生计算�?
第七，角色制造的材料来源按扫描角色身上全部库存来设计，但必须把扫描顺序、扣料顺序和去重规则写死。当前首版口径先收成“按角色库存组件固定注册顺序汇总和扣除”�?
第八，制造完成时如果无法完整入包，就把产物掉到角色脚下，默认持续 `5` 分钟，可被拾取，首版不做所有权保护�?
第九，后续工作台制造最自然的扩展方向，不是再写第二套制造系统，而是让工作台通过现有交互会话模式，接入同一条制造主链�?
## 第十五章 首阶段角色制造冷启动测试手册
这一章现在不再写成“测试提纲”，而是直接写成拿到工程以后可以照着做的冷启动操作手册�?
目标只有一个：

**让第一次接手这轮功能的人，知道先开哪个资产、先配哪几张表、进 PIE 以后先敲什么命令、每一步该看什么现象、错了先查哪�?*

这章默认读者是�?
1. 还没接过这一轮制造系�?2. 不知�?`CraftRecipe Recipe_Test_Plank` 这种命令到底在测什�?3. 看到 `returned false` 时，不知道该先回头看资产、看 `PawnData`，还是看代码入口

如果你现在就处于这个状态，可以直接只走下面这条线，不用先通读整章�?
1. 先看 `15.3`，确认从哪个资产开始点
2. 再看 `15.4`，把最小三张表配出�?3. 再看 `15.5`，把�?PIE 前的前置条件对齐
4. 然后只跑 `15.6` 这一条首条成功主�?5. 如果失败，立刻回�?`15.6.5` 的冷启动排查表，不要先跳去改 UI

这里不新增设计结论，只把已经拍板的首阶段边界翻成可执行步骤�?
这一章真正要验证的是下面这些事：

1. 角色自身制造能不能被真实触发�?2. 入队时是不是先扣材料�?3. 制造完成后是不是先走统一入包�?4. 入不下的时候是不是掉到角色脚下�?5. 队列是不是按不可取消、异常清空不返还材料的规则工作�?6. 当前最小联机真相是不是走服务端权威 + `OwnerOnly` 队列同步�?
### 15.1 程序员阅读导�?如果你这轮是�?review，或者准备接着往下做，不建议上来就开编辑器乱点�?
按下面这个顺序进，最快：

1. 先读本章 `15.2`、`15.3`、`15.4`，先知道这轮到底从哪个资产开始点，怎么把首条制造链触发起来�?2. 再回看本方案 `7.1`、`7.2`、`7.3`、`7.4`、`7.5`，把入队、扣料、完成、掉落、异常清队这些规则再对齐一遍�?3. 再看本方�?`9.1`、`9.2`、`9.3`，确认材料池现在只扫描角色身上全部库存，而且按库存组件固定注册顺序扣料�?4. 再看本方�?`11.2`、`11.3`、`11.4`，确认这轮是服务端权威，客户端不能把 `RequestEnqueueRecipe(...)` 的返回值直接当成“已经成功入队”�?5. 这轮如果�?review 新增正式制造消费层，先看这�?Widget：`UAOCraftingWidgetBase` -> `UAOCraftingRecipeListWidget` / `UAOCraftingRecipeDetailWidget` / `UAOCraftingQueueWidget`，再回看它们是否都统一通过 `UAOCraftingWidgetBase::GetCraftingViewModel()` 取同一份制�?ViewModel�?6. 这轮新增的承载层要优先补看两处：`UAOLayout_Inventory` 里稳定提供的 `CraftingPanelWidget` 承载槽，以及 `UAOCraftingPanelWidget` 作为正式制造面板聚�?Widget 的接线�?7. 再看 ViewModel 直接消费接口：`UMVVM_Crafting::GetRecipeBlockReasonText(...)`、`UMVVM_Crafting::GetSelectedRecipeBlockReasonText()`、`UMVVM_Crafting::GetActiveQueueEntry(...)`、`UMVVM_Crafting::GetQueueEntryRemainingSeconds(...)`、`UMVVM_Crafting::GetQueueEntryProgressRatio(...)`、`UMVVM_Crafting::GetActiveQueueRemainingSeconds()`、`UMVVM_Crafting::GetActiveQueueProgressRatio()`�?8. 如果这轮�?review 单条可点击制造项蓝图接线，记住当前状态解释口径已经收口到 `UAOCraftingQueueEntryWidget`。蓝图应该直接读 `UAOCraftingQueueEntryWidget::GetBlockReason()` 返回�?`EAOCraftingRecipeBlockReason`，再决定自己显示什么文字、图标或遮罩�?9. 再补�?`UAOCraftingQueueWidget` �?`UAOCraftingQueueEntryWidget` 的职责边界：前者负责容器、数组和批量创建子项，后者负责单条可点击条目；`SelectButton` 现在明确要求�?`UButton`，不要再按旧思路�?`UCommonButtonBase`�?10. 这轮如果�?review 新增 UI 入口和入口稳定化，再看这条链：`UAOMainUI::GetCraftingViewModel()` -> `UAOHUDViewModelComponent::GetCraftingViewModel()` -> `UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)` -> `UMVVM_Crafting::RequestEnqueueSelectedRecipe()` / `UMVVM_Crafting::RequestEnqueueRecipe(...)`�?11. 再看制造耗时怎么读取速度加成：`UAOCraftingComponent::ResolveCraftingDurationSeconds(...)` -> `UAOCraftingComponent::ResolveTotalCraftingSpeedBonus()` -> `UAOCombatAttributeSet::GetCraftingSpeedBonus()`�?12. 最后再按下面这个顺序跳代码�?
`AAOPlayerController::CraftRecipe(...)`

`UAOMainUI::GetCraftingViewModel()`

`UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)`

`UMVVM_Crafting::RequestEnqueueSelectedRecipe()`

`UMVVM_Crafting::RequestEnqueueRecipe(FName)`

`UAOLayout_Inventory::GetCraftingPanelWidget()`

`UAOCraftingPanelWidget::NativeConstruct()`

`UAOCraftingPanelWidget::HandleCraftingViewModelChanged()`

`UAOCraftingQueueWidget::RefreshActiveEntryTiming()`

`UAOCraftingQueueWidget::StartTimingRefresh()`

`UAOCraftingQueueWidget::StopTimingRefresh()`

 `UAOCraftingComponent::RequestEnqueueRecipe(...)`

`UAOCraftingComponent::TryEnqueueRecipeOnAuthority(...)`

`UAOCraftingComponent::IsRecipeUnlockedForOwner(...)`

`UAOCraftingComponent::ResolveRecipeRuntimeData(...)`

`UAOCraftingComponent::BuildMaterialConsumePlan(...)`

`UAOCraftingComponent::ExecuteMaterialConsumePlan(...)`

`UAOCraftingComponent::ResolveCraftingDurationSeconds(...)`

`UAOCraftingComponent::ResolveTotalCraftingSpeedBonus()`

`UAOCraftingComponent::HandleActiveCraftingFinished(...)`

`UAOCraftingComponent::DropCraftingOutputsToWorld(...)`

关联代码位置�?
1. [AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:32)
2. [AOMainUI.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp:84)
3. [AOCombatFeedbackBlueprintLibrary.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.cpp:80)
4. [MVVM_Crafting.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp:68)
5. [MVVM_Crafting.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h:32)
6. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:54)
7. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:129)
8. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:177)
9. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:211)
10. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:266)
11. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:357)
12. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:409)
13. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:480)
14. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:750)
15. [AOCombatAttributeSet.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h:124)
16. [AOCombatAttributeSet.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.cpp:187)
17. [AOCraftingWidgetBase.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h:12)
18. [AOCraftingWidgetBase.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp:10)
19. [AOCraftingRecipeListWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp:9)
20. [AOCraftingRecipeDetailWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.cpp:9)
21. [AOCraftingQueueWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingQueueWidget.cpp:9)
22. [MVVM_Crafting.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp:118)
23. [AOLayout_Inventory.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.h:12)
24. [AOLayout_Inventory.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.cpp:10)
25. [AOCraftingPanelWidget.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.h:14)
26. [AOCraftingPanelWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.cpp:11)
27. [AOCraftingQueueWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingQueueWidget.cpp:11)

### 15.2 这轮最短验证路径到底是什�?这轮先别急着做两件事�?
1. 不要先接正式制�?UI�?2. 不要先接工作台或交互会话�?
因为这两件事一旦混进来，你很快就会分不清，到底是制造真相层没通，还是 UI / 会话接线没通�?
当前最短、最稳的验证路径已经固定成下面这条：

1. 配好 `AOGameData` 上的两张全局表�?2. 配好当前测试角色最终使用的 `PawnData` 上的角色配方来源表�?3. 给测试角色背包准备一套刚好够做一次的材料�?4. 进入 PIE�?5. 打开控制台�?6. 输入�?
`CraftRecipe 配方行名`

7. 看材料是不是立刻减少�?8. 看制造队列是不是新增条目�?9. 等制造时间结束�?10. 看产物是进入库存，还是掉到角色脚下�?
这一条链现在不依赖正�?Widget，也不依赖交互对象，只验证制造主链本身�?
当前这个调试入口挂在�?
1. [AOPlayerController.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h:24)
2. [AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:32)

它的职责非常窄，只负责把控制台输入转成一次真实的配方入队请求，不承担正式 UI 入口职责�?
### 15.3 拿到工程以后，先从哪个资产开始点
如果你现在问“我第一次进工程，到底先点哪个资产”，顺序就按下面这个来�?
#### 15.3.1 第一个先开 `DA_AOGameData`
项目配置里当前能看到默认 `AOGameData` 资产路径�?
1. [DefaultGame.ini](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Config/DefaultGame.ini:14)
2. 当前默认路径�?`/Game/Games/GameData/DA_AOGameData.DA_AOGameData`

在内容浏览器里优先找�?
`/Game/Games/GameData/DA_AOGameData`

这一步先别管别的，先只看这份资产里下面两项有没有配：

1. `ItemCatalogDataTable`
2. `CraftingRecipeDataTable`

这两个字段代码入口在�?
1. [AOGameData.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h:39)
2. [AOGameData.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.h:42)

如果这里没配对，后面任何制造测试都不用继续了�?
#### 15.3.2 第二个再开当前测试角色真正吃到�?`PawnData`
项目配置里能看到一份默�?`PawnData`�?
1. [DefaultGame.ini](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Config/DefaultGame.ini:15)
2. 路径�?`/Game/Games/PawnData/DA_PawnData.DA_PawnData`

但真正进地图以后，角色最后吃的是哪份 `PawnData`，不一定永远就是这份默认资产�?
当前决议顺序是：

1. 优先看当前运行时是不是已经给 Pawn 指定�?`PawnData`
2. 如果没有，再回退到当�?`Experience` �?`DefaultPawnData`

对应代码在：

1. [AOGameMode.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/GameModes/AOGameMode.cpp:154)
2. [AOExtPawnComponent.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOExtPawnComponent.h:45)
3. [AOExtPawnComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOExtPawnComponent.cpp:234)

所以你在真正测试前，不要“随便挑一份看起来�?PawnData 的资产就改”�?先确认测试地图里的角色最后吃的是哪一份，再动手�?
#### 15.3.3 第三个才看角色配方来源表
确认好当前测试角色实际吃到的 `PawnData` 以后，再去这�?`PawnData` 里找�?
`CraftingRecipeSourceDataTable`

字段入口在：

1. [AOPawnData.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.h:71)
2. [AOPawnData.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp:104)

这一项如果没配，最典型的现象就是：

1. 控制台命令能敲进去�?2. `CraftingComponent` 也能找到�?3. 但服务端会在“配方是否已解锁”这一步直接拒绝�?
### 15.4 这轮至少要配哪三张表
首阶段最小闭环至少依赖三张表，少一张都别往后测�?
#### 15.4.1 全局物品总表
这张表承担的是：

`ItemId -> ItemDefinitionClass`

也就是制造系统最后怎么把一个物�?ID 解析成真实库存定义�?
当前入口�?
1. `DA_AOGameData.ItemCatalogDataTable`
2. [AOGameData.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp:38)

这张表必须满足：

1. 产物 `ItemId` 能查�?2. 材料 `ItemId` 也能查到
3. 查到的行�?`ItemDefinitionClass` 不为�?
这张表现在对应的行结构是�?
- [FAOItemCatalogRow](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Items/AOItemCatalogTypes.h:11)

第一轮建议直接新建一张最小测试表，例如：

- 资产名：`DT_ItemCatalog_CraftingTest`
- 行结构：`FAOItemCatalogRow`

最小至少放两行�?
1. 材料行，例如行名 `Material_Wood`
2. 产物行，例如行名 `Output_Plank`

这两行至少要填：

1. `ItemId`
2. `ItemDefinitionClass`

第一轮最小测试时，可以先约定成下面这种最简单示例：

1. `Material_Wood` -> `ItemId = 1001`
2. `Output_Plank` -> `ItemId = 1002`

这里要注意：

1. 这张表的“行名”对当前制造主链不是关键输�?2. 真正关键的是 `ItemId` �?`ItemDefinitionClass` 配对正确
3. 后面配方表里填材料和产物时，引用的是这里�?`ItemId`

#### 15.4.2 全局制造配方表
这张表负责：

1. 一条配方消耗哪�?`ItemId`
2. 一条配方产出哪�?`ItemId`
3. 基础制造时长是多少

当前入口�?
1. `DA_AOGameData.CraftingRecipeDataTable`
2. [AOCraftingRecipeTypes.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h:22)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:215)

第一轮最好先配一条最小配方：

1. 只消耗一种材�?2. 只产出一种结果物
3. 制造时长先写短一点，比如 `1` �?`3` �?4. 配方行名用容易认的名字，不要等到进控制台时再�?
这张表现在对应的行结构是�?
- [FAOCraftingRecipeRow](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h:22)

第一轮建议直接新建一张最小测试表，例如：

- 资产名：`DT_CraftingRecipe_Test`
- 行结构：`FAOCraftingRecipeRow`

最小建议先只建一行：

1. 行名：`Recipe_Test_Plank`

这一行至少把下面四项填完整：

1. `DisplayName`，例�?`测试木板`
2. `BaseCraftDurationSeconds`，例�?`2.0`
3. `MaterialEntries`
4. `OutputEntries`

最小示例可以直接写成：

1. `MaterialEntries` 只放一项：`ItemId = 1001`，`Count = 1`
2. `OutputEntries` 只放一项：`ItemId = 1002`，`Count = 1`

这里要特别强调一遍：

控制台命�?`CraftRecipe` 需要的是“配方表的行名”，不是物品显示名，也不�?`ItemId`�?
也就是说，你第一轮进 PIE 时，控制台里应该敲的是：

`CraftRecipe Recipe_Test_Plank`

不是�?
1. `CraftRecipe 测试木板`
2. `CraftRecipe 1002`
3. `CraftRecipe Output_Plank`

#### 15.4.3 角色配方来源�?这张表负责：

1. 角色当前开放哪些配�?2. 每条配方要求多少等级才能解锁

当前入口�?
1. `PawnData.CraftingRecipeSourceDataTable`
2. [AOCraftingRecipeSourceTypes.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeSourceTypes.h:26)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:177)

第一轮建议直接配成最简单的状态：

1. 这条配方确实在表�?2. `UnlockLevel` 先设�?`1`
3. 测试角色等级也保证是 `1` 或更�?
这张表现在对应的行结构是�?
- [FAOCraftingRecipeSourceRow](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeSourceTypes.h:26)

它内部真正承载“解锁哪条配方”的条目结构是：

- [FAOCraftingRecipeUnlockEntry](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeSourceTypes.h:8)

第一轮建议直接新建一张最小测试表，例如：

- 资产名：`DT_CraftingRecipeSource_Test`
- 行结构：`FAOCraftingRecipeSourceRow`

最小建议先只建一行，例如�?
1. 行名：`Default`

这一行里至少�?`RecipeEntries` 填一条：

1. `RecipeRowName = Recipe_Test_Plank`
2. `UnlockLevel = 1`
3. `bVisibleBeforeUnlock = true`
4. `SortOrder = 0`

这里要特别说明当前代码口径：

1. `CraftingRecipeSourceDataTable` 现在是“遍历整张表的所有行”，不是按某个固定行名精确查�?2. 所以这张表里第一轮最关键的是 `RecipeEntries` 里写�?`RecipeRowName`
3. 这张表自己的表行名例�?`Default`、`CharacterDefault`，当前不是控制台输入，也不是制造主链的精确匹配�?
对应代码可直接看�?
1. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:79)
2. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:312)

### 15.4.4 这三张表第一轮怎么串起�?如果你第一次接手，不要自己脑补引用方向，先按这条最小链来：

1. �?`DT_ItemCatalog_CraftingTest` 里建材料和产物的 `ItemId`
2. �?`DT_CraftingRecipe_Test` 里用这些 `ItemId` 写一�?`Recipe_Test_Plank`
3. �?`DT_CraftingRecipeSource_Test` 里把 `Recipe_Test_Plank` 放进 `RecipeEntries`
4. �?`DT_ItemCatalog_CraftingTest` 挂到 `DA_AOGameData.ItemCatalogDataTable`
5. �?`DT_CraftingRecipe_Test` 挂到 `DA_AOGameData.CraftingRecipeDataTable`
6. �?`DT_CraftingRecipeSource_Test` 挂到“测试角色实际吃到的那份 `PawnData`”的 `CraftingRecipeSourceDataTable`

第一轮你至少要能回答清楚这三个问题：

1. 材料和产物的 `ItemId` 在哪张表里定�?2. `Recipe_Test_Plank` 这条配方在哪张表里定�?3. 当前测试角色为什么能看到并解�?`Recipe_Test_Plank`

### 15.5 �?PIE 前的前置准备
这一步最容易被省掉，然后后面不管看到什么，都会下意识觉得“代码有 bug”�?
你在�?PIE 之前，先把下面这些事情一条条确认完�?
#### 15.5.1 确认测试地图里用的是标准角色�?这轮最小入口要求：

1. 玩家控制器是 `AAOPlayerController` 或其蓝图派生
2. Pawn �?`AAOCharacter` 或其蓝图派生

因为控制台命令挂�?`AAOPlayerController` 上，而制造组件挂�?`AAOCharacter` 上�?
如果你测试地图里跑的是别�?Controller 或别�?Pawn，最典型现象是：

1. `CraftRecipe` 命令不存�?2. 或者命令存在，但日志提�?`controlled pawn is not AAOCharacter`

对应代码�?
1. [AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:40)
2. [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:70)

#### 15.5.2 确认角色身上真的有制造组�?虽然当前代码已经�?`UAOCraftingComponent` 挂在 `AAOCharacter` 上了，但你在测试蓝图里最好还是确认一次�?
现在标准挂载点是�?
1. [AOCharacter.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.h:69)
2. [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:70)

如果你测的是旧测试蓝图，或者地图里根本不是这套标准角色，后面所有制造步骤都会白跑�?
#### 15.5.3 确认背包里已经有测试材料
这一轮最稳的做法还是下面这套�?
1. 先给角色背包放刚好够做一次的材料
2. 第一轮先不要混多个同类材料分散在多个库存组件�?3. 第一轮也不要先测极限堆叠

当前角色主接收容器是 `UAOBackPackComponent`，挂载点在：

1. [AOCharacter.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.h:61)
2. [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:63)

如果你要临时往角色库存塞材料，当前代码侧真正入库存的底层入口是�?
1. `UAOInventoryComponent::AddItemDefinition(...)`
2. [AOInventoryComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp:621)

但这一步现在还没有为制造专门做编辑器按钮，所以实际准备材料时，你可以按项目里现有最方便的方式处理，比如�?
1. 直接在测试角色初始库存里放进�?2. 或者进游戏后通过你们现有的拾�?/ 给物品调试手段先拿到�?
这里重点不是“你到底用哪种方式塞进去”，而是保证制造开始前，材料确实已经在角色自己的库存组件里�?
#### 15.5.4 确认 `AOGameData` 已经加载到正确资�?这一步最好也确认一次，不要想当然�?
项目配置里当前能看到两处历史路径�?
1. [DefaultEngine.ini](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Config/DefaultEngine.ini:87)
2. [DefaultGame.ini](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Config/DefaultGame.ini:14)

你现在实际应优先�?`DefaultGame.ini` 里的这份来确认：

`/Game/Games/GameData/DA_AOGameData`

因为当前代码�?`UAOGameData::Get()` 是走 `AOAssetManager` 去拿运行时资产：

1. [AOAssetManager.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOAssetManager.cpp:182)
2. [AOGameData.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/System/AOGameData.cpp:57)

### 15.6 第一条成功主链，照着怎么�?下面这组步骤就是按“第一次接手的人也能照着做”的粒度写的�?
这一组只验证一件事�?
**一条合法配方，能不能从请求入队一直走到完成发货�?*

#### 15.6.1 进入编辑器前
1. 确认 `DA_AOGameData` 里已经配�?`ItemCatalogDataTable`
2. 确认 `DA_AOGameData` 里已经配�?`CraftingRecipeDataTable`
3. 确认测试角色最终吃到的 `PawnData` 里已经配�?`CraftingRecipeSourceDataTable`
4. 确认这三张表里存在一条最小可测配�?5. 确认测试角色背包里已经有刚好够一次的材料

如果你不想自己临场起名，第一轮就直接按下面这组最小测试资源去建：

1. `DT_ItemCatalog_CraftingTest`
2. `DT_CraftingRecipe_Test`
3. `DT_CraftingRecipeSource_Test`
4. 配方行名固定先用 `Recipe_Test_Plank`
5. 材料 `ItemId` 固定先用 `1001`
6. 产物 `ItemId` 固定先用 `1002`

#### 15.6.2 进入 PIE �?1. 启动 PIE
2. 确认当前玩家控制的是标准角色
3. 打开控制�?4. 输入�?
`CraftRecipe Recipe_Test_Plank`

5. 按回车执�?
#### 15.6.3 命令执行后，第一时间先看什�?命令执行后，先别急着盯产物�?
先看这三件事�?
1. 材料是不是立刻减少了
2. 制造队列是不是新增了一条任�?3. 日志�?`CraftRecipe request for '配方行名' on 角色�?returned true/false` 打的是哪种结�?
如果这里直接�?`false`，就先别继续等计时结束了，说明当前入队前就已经被拦了�?
#### 15.6.3.1 `CraftRecipe` 这个命令到底是在测什�?这条控制台命令不是正式玩法入口，也不是“直接发一个成品到背包”的作弊命令�?
它现在只承担一件事�?
1. 把控制台里输入的“配方行名”转成一次制造入队请求�?2. 让这次请求走当前角色制造系统的正式主链�?
也就是说，`CraftRecipe Rope_Craft` 测的不是“Rope 这个物品能不能凭空生成”，而是�?
1. 当前玩家控制�?Pawn 是不是标�?`AAOCharacter`
2. 角色身上是不是有 `UAOCraftingComponent`
3. 当前角色实际吃到�?`PawnData` 是否开放了 `Rope_Craft`
4. `DA_AOGameData.CraftingRecipeDataTable` 里是否真的存�?`Rope_Craft`
5. 当前角色允许参与制造的库存集合里，材料是否够扣
6. 这条请求最终能不能成功入队

所以如果你敲完命令“看起来什么都没发生”，不要第一反应理解成命令没执行�?
更准确的理解应该是：

1. 命令已经执行到制造入口了
2. 但制造主链在“写入队列之前”就把这次请求挡下来�?
#### 15.6.3.2 `returned false` 现在真正表示什�?当前控制台日志：

`CraftRecipe request for '配方行名' on 角色�?returned false`

在你现在这种单机 / ListenServer 风格的本地权威调试里，应直接理解成：

1. `AAOPlayerController::CraftRecipe(...)` 已经拿到了角色和制造组�?2. `UAOCraftingComponent::RequestEnqueueRecipe(...)` 已经被真正调�?3. `UAOCraftingComponent::TryEnqueueRecipeOnAuthority(...)` 在服务端权威判定里返回了 `false`
4. 所以这次请求没有成功入�?5. 因为没有入队，所以不会扣料、不会开始计时、不会有产物、队列里也不会新增条�?
也就是说，这�?`false` 日志不是“命令不存在”，也不是“控制器没接到输入”�?
它表示的是：

**制造入口已经跑到权威入队判定，但前置条件没过�?*

这里还要补一条以后容易误读的边界�?
`UAOCraftingComponent::RequestEnqueueRecipe(...)` 在纯客户端语义下会先发一�?`ServerRequestEnqueueRecipe(...)` 再直接返�?`true`，所以将来如果你在多人客户端单独看这条布尔值，它未必等于最终服务端是否真的入队成功�?
但你这次日志打出�?`false`，说明当前这次测试不是“客户端先发 RPC 再乐观返�?true”那种情况，而是已经在本地权威分支里被明确挡下来了�?
#### 15.6.3.3 `returned false` 时先按什么顺序排�?当前代码里，`TryEnqueueRecipeOnAuthority(...)` 会在下面这些位置直接返回 `false`�?
1. `Owner` 无效，或当前不是权威端，或制造队列已�?2. `IsRecipeUnlockedForOwner(...)` 失败
3. `ResolveRecipeRuntimeData(...)` 失败
4. `BuildMaterialConsumePlan(...)` 失败
5. `ExecuteMaterialConsumePlan(...)` 执行失败

但结合当前这套制造系统的真实配置链，第一轮排查不要平均撒网，直接按下面顺序走�?
1. 先确�?`Rope_Craft` 这条字符串是不是**配方表行�?*
2. 再确认测试角色实际吃到的是哪�?`PawnData`
3. 再确认那�?`PawnData.CraftingRecipeSourceDataTable` 里有没有开�?`Rope_Craft`
4. 再确�?`DA_AOGameData.CraftingRecipeDataTable` 里是否真的存�?`Rope_Craft`
5. 再确认当前角色允许参与制造的库存集合里，是否真的有足够材�?6. 最后才怀疑扣料执行期失败或队列已�?
原因很简单：

当前最常见的失败，不是制造逻辑本身没跑，而是资源链没接对�?
尤其是下面三种情况，最容易表现成“命令敲了但什么都没发生”：

1. 配方来源表没有开放这条配�?2. 配方总表里根本没有这条行
3. 材料根本没进角色允许参与制造的库存集合

#### 15.6.3.4 �?`CraftRecipe Rope_Craft` 这种案例，当前最该先怀疑什�?如果你现在敲的是�?
`CraftRecipe Rope_Craft`

而日志是�?
`CraftRecipe request for 'Rope_Craft' on BP_Anny_C_0 returned false`

那在没有额外日志细分之前，当前最稳的判断不是“制造代码坏了”，而是�?
1. `BP_Anny_C_0` 确实拿到了制造入�?2. �?`Rope_Craft` 没能通过“已解锁 + 配方存在 + 材料可扣”这一层基础校验

按当前项目的真实风险排序，优先怀疑：

1. `BP_Anny_C_0` 实际吃到�?`PawnData` 不是你以为的那份
2. 那份 `PawnData` 上的 `CraftingRecipeSourceDataTable` 没有开�?`Rope_Craft`
3. `DA_AOGameData.CraftingRecipeDataTable` 里不存在 `Rope_Craft`
4. 当前角色库存里没有足够材�?
不要先从“产物为什么没出来”开始查�?
因为现在连入队都没成功，产物、计时、完成发货这些后段逻辑还根本没开始跑�?
#### 15.6.4 然后再看什�?如果已经成功入队，再继续看：

1. 队列里是不是有一条任务从 `Queued` 进入 `Active`
2. 这条任务是不是带了正确的开始时间和结束时间
3. 到了结束时间以后，产物是不是进入库存

如果这一步通过，才算“最基础成功制造主链”通过�?
#### 15.6.5 冷启动排查表：看到什么现象，就先回哪一�?第一次接手时，最常见的问题不是“哪里都坏了”，而是还没把故障定位到正确层�?
先按现象分层，不要一上来就同时改表、改蓝图、改 C++�?
**现象一：控制台里根本没�?`CraftRecipe` 这个命令**

先查�?
1. 当前玩家控制器是不是 `AAOPlayerController` 或其蓝图派生
2. 测试地图是不是走的标准角色链
3. 当前是不是你以为的那张测试地�?
第一回头位置�?
1. [AOPlayerController.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.h:24)
2. [AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:32)

**现象二：命令能敲，日志打印了 `returned false`**

先不要怀疑完成发货、掉落生成和 UI�?
先按这个顺序查：

1. 控制台里写的是不是配方表行名，例�?`Recipe_Test_Plank`
2. `DA_AOGameData.CraftingRecipeDataTable` 里是否真的有这条�?3. 当前测试角色最终吃到的 `PawnData` 到底是哪�?4. 那份 `PawnData.CraftingRecipeSourceDataTable` 里有没有开放这条配�?5. 角色允许参与制造的库存集合里，材料是否真的足够

第一回头位置�?
1. [AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:32)
2. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:54)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:129)
4. [AOPawnData.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp:104)

**现象三：日志�?`returned true`，但你肉眼觉得“没发生任何事�?*

这时不要只盯着产物�?
先查三件事：

1. 材料有没有在入队瞬间减少
2. 队列里有没有新增条目
3. 这条条目是不是很快从 `Queued` 切到 `Active`

如果这三件事都发生了，只是你还没等到完成，那说明主链已经跑起来了�?
第一回头位置�?
1. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:266)
2. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:357)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:409)

**现象四：能入队，也能计时，但完成后没有产�?*

这时才开始查完成结算层：

1. 统一入包是不是失败了
2. 失败后有没有走掉落兜�?3. 掉落物是否真的生成在角色脚下

第一回头位置�?
1. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:423)
2. [AOInventoryStatics.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp:64)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:480)

**现象五：命令主链能通，但正�?UI 还是空的**

先把它当成“承载层 / ViewModel 接线问题”，不要回头怀疑制造逻辑还没实现�?
第一回头位置�?
1. [AOLayout_Inventory.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.cpp:39)
2. [AOCraftingPanelWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.cpp:11)
3. [AOCraftingWidgetBase.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp:24)
4. [MVVM_Crafting.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp:68)

### 15.6A 制作系统 UI 冷启动怎么�?这一章不是测制造逻辑对不对，而是专门回答�?
1. 正式制�?UI 现在挂在哪里
2. ViewModel 从哪里拿
3. 蓝图应该绑定哪些 C++ Widget �?4. 第一次把 UI 接起来时，先验证什么、后验证什�?
如果你当前的问题是“我连面板都还没接上，不知道从哪开始”，先看这一章，不要直接跳去�?`15.7` 的手工制造用例�?
#### 15.6A.1 当前正式 UI 的真实承载链
当前代码里，正式制�?UI 不是直接从某�?Widget 蓝图自己去找角色组件，而是已经收口成这条链�?
1. `AAOCharacter` 身上�?`UAOCraftingComponent`
2. `UAOHUDViewModelComponent` 把制造组件绑定到 HUD �?3. `UMVVM_Crafting` 持有制造列表、详情、队列三类观察数�?4. `UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)` 作为蓝图 / Widget 获取入口
5. `UAOCraftingWidgetBase` 统一监听 `UMVVM_Crafting`
6. `UAOCraftingPanelWidget` 作为承载面板，下面再挂列�?/ 详情 / 队列三个�?Widget
7. `UAOLayout_Inventory` 作为库存主布局，把正式制造面板承载进�?
先读这些入口�?
1. [AOHUDViewModelComponent.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/AOHUDViewModelComponent.h:76)
2. [MVVM_Crafting.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h:11)
3. [AOCombatFeedbackBlueprintLibrary.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h:36)
4. [AOCraftingWidgetBase.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h:11)
5. [AOCraftingPanelWidget.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.h:14)
6. [AOLayout_Inventory.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.h:12)

#### 15.6A.2 先认清每�?UI 类各自负责什�?当前这几�?C++ 类的职责不要混：

1. `UAOCraftingWidgetBase`
作用：所有制作系�?Widget 的公共基类，统一获取 `UMVVM_Crafting`，统一监听观察变化，统一触发刷新�?
2. `UAOCraftingRecipeListWidget`
作用：只负责“当前有哪些配方”和“当前选中了哪条配方”�?
3. `UAOCraftingRecipeDetailWidget`
作用：只负责“当前选中配方的详情、阻断原因、能不能点制作、点击制作怎么发请求”�?
4. `UAOCraftingQueueWidget`
作用：只负责“当前队列长什么样”和“当前活动项剩余时间 / 进度比”�?
5. `UAOCraftingPanelWidget`
作用：承载上面三个子 Widget，本体不自己持有第二份制造状态�?
6. `UAOLayout_Inventory`
作用：库存主布局，当前正式制造面板挂在这里�?
#### 15.6A.3 第一次把蓝图 UI 接起来，最小要建哪些蓝�?如果你现在完全从零把正式制造面板接起来，建议最小先有四层蓝图：

1. 一个面板蓝图，父类�?`UAOCraftingPanelWidget`
2. 一个配方列表蓝图，父类�?`UAOCraftingRecipeListWidget`
3. 一个配方详情蓝图，父类�?`UAOCraftingRecipeDetailWidget`
4. 一个制造队列蓝图，父类�?`UAOCraftingQueueWidget`

第一轮先不要把这三块功能糊进一个普�?`UUserWidget`�?因为当前 C++ 已经�?ViewModel 监听和缓存链分层了，直接绕开这些父类会把接线重新打散�?如果要按“主 Widget 直管纯蓝图子项”的方式落地，子项蓝图就只负责样式和控件命名，不再自己写列表循环、材料循环或队列循环�?
第一轮建议直接建这几个蓝图资产：

1. `WBP_CraftingPanel`
2. `WBP_CraftingRecipeList`
3. `WBP_CraftingRecipeDetail`
4. `WBP_CraftingQueue`
5. `WBP_CraftingRecipeEntry`
6. `WBP_CraftingMaterialEntry`
7. `WBP_CraftingOutputEntry`
8. `WBP_CraftingQueueEntry`

其中前四个是主面板和三个主子区块，后四个是纯蓝图子项承载体。主 Widget 负责 `CreateWidget` �?`AddChild`，子项蓝图只负责显示单条数据�?
#### 15.6A.4 面板蓝图第一轮怎么绑子 Widget
先做面板蓝图�?
1. 新建 `WBP_CraftingPanel`
2. 父类�?`UAOCraftingPanelWidget`
3. 在蓝图里放三个子控件�?4. 子控件分别使用：
   `UAOCraftingRecipeListWidget` 派生蓝图
   `UAOCraftingRecipeDetailWidget` 派生蓝图
   `UAOCraftingQueueWidget` 派生蓝图
5. 这三个子控件变量名必须对�?C++ �?`BindWidgetOptional` 字段名：
   `RecipeListWidget`
   `RecipeDetailWidget`
   `QueueWidget`

如果名字没对上，`UAOCraftingPanelWidget` 不会自动拿到这三个子 Widget�?
先查�?
1. [AOCraftingPanelWidget.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.h:35)

#### 15.6A.5 列表蓝图第一轮怎么�?列表蓝图第一轮不要先做复杂样式，先验证“能拿到列表，能切换选中”：

1. 父类�?`UAOCraftingRecipeListWidget`
2. 不要自己去找 `CraftingComponent`
3. 不要自己在蓝图里存第二份配方数组
4. 只通过 `GetRecipeList()` 取列�?5. 点击某条配方时，只调�?`SelectRecipe(RecipeRowName)`
6. 当前选中态只通过 `GetSelectedRecipeRowName()` 判断
7. 如果要验证“没有蓝图循环”，就检查列表蓝图里没有 `ForEach CachedRecipeList`、没�?`CreateWidget` 整表循环，只保留“每条数据创建一�?`WBP_CraftingRecipeEntry`”的逻辑

也就是说，列表蓝图的第一轮目标不是“做出最终视觉效果”，而是确认�?
1. 它拿到的�?`UMVVM_Crafting` 给的那份列表
2. 点选行为真的把 `SelectedRecipeRowName` 写回 ViewModel

#### 15.6A.6 详情蓝图第一轮怎么�?详情蓝图第一轮先只验证三件事�?
1. 能显示当前选中配方详情
2. 能显示阻断原因文�?3. “制作”按钮能走正式请求入�?
按这个方式接�?
1. 父类�?`UAOCraftingRecipeDetailWidget`
2. 显示层只�?`GetRecipeDetail()`
3. 阻断提示只读 `GetBlockReasonText()`
4. 按钮可点性只�?`CanEnqueueSelectedRecipe()`
5. 点击“制作”按钮时，只�?`RequestEnqueueSelectedRecipe()`
6. 材料区只验证会创�?`WBP_CraftingMaterialEntry`
7. 产物区只验证会创�?`WBP_CraftingOutputEntry`
8. 详情蓝图里不应有 `ForEach CachedRecipeDetail.MaterialEntries` �?`ForEach CachedRecipeDetail.OutputEntries` 的整表驱动逻辑

不要在详情蓝图里�?
1. 自己手写“材料够不够”的判断
2. 自己拼第二套“为什么不能做”的文案
3. 自己再传一遍配方行名给组件

#### 15.6A.7 队列蓝图第一轮怎么�?队列蓝图第一轮先分成两部分看�?
1. 整条队列列表
2. 当前活动项的剩余时间和进度条

按这个方式接�?
1. 父类�?`UAOCraftingQueueWidget`
2. 队列列表只读 `GetQueueList()`
3. 是否有活动项只读 `HasActiveEntry()`
4. 当前活动项只�?`GetActiveEntry()`
5. 剩余时间只读 `GetActiveEntryRemainingSeconds()`
6. 进度比只�?`GetActiveEntryProgressRatio()`
7. 队列蓝图里只验证会创�?`WBP_CraftingQueueEntry`
8. 队列蓝图里不要再�?`ForEach CachedQueueList` 的整表循�?
这里要记住一个边界：

当前活动项时间推进不是让蓝图自己 Tick 算，而是 C++ �?`UAOCraftingQueueWidget` 已经在做轻量定时刷新�?
#### 15.6A.8 正式制造面板怎么挂进库存布局
当前正式承载位置不是 HUD 根面板任意一角，而是库存布局 `UAOLayout_Inventory`�?
第一轮接法：

1. 打开库存布局蓝图，对�?`UAOLayout_Inventory`
2. 确认蓝图里有一个承载制造面板的控件变量
3. 这个变量名必须对�?C++ 字段名：`CraftingPanelWidget`
4. 这个控件实例应当是你刚做�?`WBP_CraftingPanel`
5. 如果看不到这层承载关系，先别改列表或详情逻辑，先确认面板槽有没有接到正确父类

对应入口�?
1. [AOLayout_Inventory.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.h:35)
2. [AOLayout_Inventory.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.cpp:39)

#### 15.6A.9 ViewModel 到底从哪里拿，不要再自己�?当前推荐入口有两条：

1. `UAOMainUI::GetCraftingViewModel()`
2. `UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)`

�?`Crafting` 这一�?C++ Widget 基类默认走的是第二条�?
- [AOCraftingWidgetBase.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp:24)

�������������ʽ������ͼ���Ѿ��̳�����Щ `UAOCrafting*Widget` ���࣬��һ�ֲ�Ҫ�Լ��ٶ��⻺��һ�� `UMVVM_Crafting` ������`UAOCraftingRecipeListWidget` �� `UAOCraftingRecipeDetailWidget` �������Ǳ�����ʽ UI ·�ߣ��б���״̬�İ��� `UAOCraftingRecipeListEntryWidget` �� `StateText` �󶨺� `GetBlockReason()` ˢ�£���Ҫ�Ѷ��� `UAOCraftingQueueWidget` д�ص�ǰ��ʽʵ�֡�
#### 15.6A.10 ������ʱ��ô�ж��ǡ�UI û���ϡ����ǡ�����û����
��һ�ν���ʱ����Ҫһ�����հ׾�˵������ϵͳûʵ�֡����Ȱ��ֲ��Ų飺

1. ��������������֣�
�Ȳ� `AOLayout_Inventory` �� `CraftingPanelWidget` �Ƿ��ԡ�

2. ��������ֵ��б� / ����������Ϊ�գ�
�Ȳ� `UAOCraftingPanelWidget` ������� `BindWidgetOptional` �����Ƿ���롣

3. �����������ֵ�û���κ����ݣ�
�Ȳ� `UAOCraftingWidgetBase` �Ƿ��õ��� `UMVVM_Crafting`��

4. ����õ��� `UMVVM_Crafting` ���б�Ϊ�գ�
�Ȳ��ɫ��ʵ `PawnData` �Ƿ����� `CraftingRecipeSourceDataTable`��

5. ����б��������ݵ�״̬�İ���ˢ�£�
�Ȳ� `UAOCraftingRecipeListEntryWidget` �� `StateText` �󶨺� `GetBlockReason()` �Ƿ�����������ͬһ�� `UMVVM_Crafting`��

#### 15.6A.11 UI ��������һ���ֹ���֤˳��
��һ�νӺ� UI ���Ȱ�����˳���飬��Ҫһ�����Ͳ�����������쳣�жϣ�

1. �򿪿�沼�֣�ȷ����ʽ���������֡�
2. ȷ���б��������������鶼�����ص�������
3. ȷ���б����ܿ��� `Recipe_Test_Plank`��
4. ��� `Recipe_Test_Plank`��ȷ�������������л���
5. ȷ������������İ�����ʾ��
6. �������������ȷ���������ߵ���ʽ��ӡ�
7. ȷ���б������״̬�İ�����ű仯������ `StateText` / `GetBlockReason()` ������
8. ����һ�������ϡ��ȼ�����������ס���䷽��ȷ���б���������������������Ϣһ�¡�

ֻ����һ��ͨ���󣬲�ֵ�ü�������ʽ�͸���������

#### 15.6A.12 ���� C++ �Ѿ�׼���á���ͼ���ڱ���󶨵Ŀؼ�
��һ��֮����ʽ���� UI ����ֻͣ�ڡ������� ViewData������ô������Ⱦ����ȫ����ͼ�Լ�дѭ������

��ǰ C++ �Ѿ���ʼ����

1. ���� `CachedRecipeList`
2. ���� `CachedRecipeDetail.MaterialEntries`
3. ���� `CachedRecipeDetail.OutputEntries`
4. ��̬������Ӧ�� Widget
5. �ѻ����ı�����ť�ɵ��ԡ���ǰѡ���䷽�����ԭ��ˢ���� Widget
6. �б������� `UAOCraftingRecipeListEntryWidget` ����ˢ�� `StateText` �� `GetBlockReason()` ��Ӧ״̬��

������ͼ������Ҫ�������£�

1. �������ؼ�
2. ָ��ÿ������ Widget ��

��ǰ��Ҫ������ͼ��������ʽ�����ֻ����Щ��

1. `UAOCraftingRecipeListWidget`
2. `UAOCraftingRecipeDetailWidget`
3. `UAOCraftingPanelWidget`

����㻹�ڿ��ɶ���ר��ع飬��ֻ����������ʷ·������Ҫ����д����ʽ�����ߡ�
先看配方列表�?
- 父类：`UAOCraftingRecipeListWidget`
- 容器控件名：`RecipeListContainer`
- 子项类字段：`RecipeEntryWidgetClass`
- 子项蓝图父类：`UAOCraftingRecipeListEntryWidget`
- 子项内需要预留的控件名：
  `SelectButton`（当前要求使�?`UButton`，不再要�?`UCommonButtonBase`�?  `RecipeIconImage`
  `RecipeNameText`
  `DurationText`
  `SelectedIndicator`
  `WBP_CraftingRecipeEntry` 只是一条配方行的蓝图模板，不应该自己再去读整份配方表�?  当前列表项这一层先只承担配方行模板职责，重点是把图标、名称、制造时长和选中态稳定显示出来，不要再把“可制作状态解释”挂在这一层�?  如果蓝图里还留着旧的 `CraftableStateText`、`BlockReasonText` 文本控件，不要再期待这一层的 C++ 自动往里写值；这轮状态解释口径已经收回到队列单项�?
再看配方详情�?
- 父类：`UAOCraftingRecipeDetailWidget`
- 文本控件名：
  `RecipeNameText`
  `RecipeDurationText`
  `RecipeBlockReasonText`
- 按钮控件名：
  `EnqueueButton`
- 容器控件名：
  `MaterialListContainer`
  `OutputListContainer`
- 子项类字段：
  `MaterialEntryWidgetClass`
  `OutputEntryWidgetClass`
- 材料子项蓝图父类：`UAOCraftingRecipeMaterialEntryWidget`
- 材料子项控件名：
  `ItemIconImage`
  `ItemNameText`
  `CountText`
  `StatusText`
  `SatisfiedIndicator`
- 产物子项蓝图父类：`UAOCraftingRecipeOutputEntryWidget`
- 产物子项控件名：
  `ItemIconImage`
  `ItemNameText`
  `CountText`
  材料与产物子项都只做单条展示，不要在蓝图里再循环数组�?
再看制造队列：

- 父类：`UAOCraftingQueueWidget`
- 容器控件名：`QueueListContainer`
- 子项类字段：`QueueEntryWidgetClass`
- 子项蓝图父类：`UAOCraftingQueueEntryWidget`
- 子项内需要预留的控件名：
  `SelectButton`（当前要求使�?`UButton`，不再要�?`UCommonButtonBase`�?  `RecipeIconImage`
  `RecipeNameText`
  `StateText`
  `DurationText`
  `RemainingText`
  `ProgressBar`
  队列子项负责单个可点击条目，不要在蓝图里自己轮询整条列表；如果要做“是否可制造”的特殊表现，直接读 `UAOCraftingQueueEntryWidget::GetBlockReason()` 返回�?`EAOCraftingRecipeBlockReason`�?
这些名字如果没对上，当前 C++ 会出现下面这些现象：

1. 列表为空，但 `CachedRecipeList` 其实已经有数�?2. 详情区有缓存，但材料/产物列表不生�?3. 队列缓存有条目，但界面没有子�?4. 按钮存在，但没被 C++ 接到正式“制作请求”入�?
#### 15.6A.13 这轮新增的最小子�?Widget 职责
这轮现在收口后的做法，不是继续用普�?`UUserWidget` + `GetWidgetFromName(TEXT(...))` 去硬找控件了，而是改成�?
1. �?Widget 继续�?C++
2. 子项回到 typed C++ Widget
3. 这些 typed 子项不额外拆成新的独立文件，而是并回现有 `RecipeList / Detail / Queue` 的头源文�?4. �?Widget 负责 `CreateWidget`
5. 子项自己负责 `BindWidget`、`Set...Data(...)` 和单条显示刷�?
这一轮手工测试时，子项继承关系和控件名要按下面这组对�?
1. 配方列表�?父类：`UAOCraftingRecipeListEntryWidget`
容器控件：`RecipeListContainer`
子项类字段：`RecipeEntryWidgetClass`
子项控件名：
`SelectButton`
`RecipeIconImage`
`RecipeNameText`
`DurationText`
`SelectedIndicator`
作用：显示单条配方列表项，必须能同时看见图标、名称、制造时长和选中态；`SelectButton` 当前统一�?`UButton` 接。这一层先不要再承担“可制作状态”解释，状态枚举入口已经收口到单条制造队列项�?
2. 配方详情材料�?父类：`UAOCraftingRecipeMaterialEntryWidget`
容器控件：`MaterialListContainer`
子项类字段：`MaterialEntryWidgetClass`
子项控件名：
`ItemIconImage`
`ItemNameText`
`CountText`
`StatusText`
`SatisfiedIndicator`
作用：显示单条材料需求，必须能看见图标、名称、拥�?需求数量和满足状态�?
3. 配方详情产物�?父类：`UAOCraftingRecipeOutputEntryWidget`
容器控件：`OutputListContainer`
子项类字段：`OutputEntryWidgetClass`
子项控件名：
`ItemIconImage`
`ItemNameText`
`CountText`
作用：显示单条产物，必须能看见图标、名称和数量�?
4. 制造队列项
父类：`UAOCraftingQueueEntryWidget`
容器控件：`QueueListContainer`
子项类字段：`QueueEntryWidgetClass`
子项控件名：
`SelectButton`
`RecipeIconImage`
`RecipeNameText`
`StateText`
`DurationText`
`RemainingText`
`ProgressBar`
作用：显示单条可点击制造条目，必须能看见图标、名称、状态、总时长、剩余时间和进度条；如果蓝图要显示“可制�?不可制造”或阻断文案，统一�?`GetBlockReason()` 自己解释�?
这轮真正承担“动态创�?+ 数据灌入”职责的是下面这几类�?Widget�?
1. `UAOCraftingRecipeListWidget`
作用：创建配方列表子项，并把点击转回 `SelectRecipe(...)`�?
2. `UAOCraftingRecipeDetailWidget`
作用：创建材�?/ 产物子项，并承载详情头部与“制作”按钮逻辑�?
3. `UAOCraftingQueueWidget`
作用：创建队列子项，并持续刷新活动项剩余时间与进度�?
所以蓝图现在不用再自己做：

1. `ForEach CachedRecipeList`
2. `ForEach MaterialEntries`
3. `ForEach OutputEntries`
4. `ForEach CachedQueueList`
5. 手动 `CreateWidget`
6. 手动 `AddChild`

这些主循环已经开始往 C++ 收�?测试时要专门看两件事�?
1. 列表、详情、队列三块的�?Widget 是否都只通过自己�?`Get*List()` �?`Get*Detail()` 入口取数据�?2. 子项蓝图是否只是单条展示模板，而不是偷偷在 `Blueprint Update Animation`、`Tick` 或事件图里自己做整表循环�?3. `WBP_CraftingRecipeEntry`、`WBP_CraftingMaterialEntry`、`WBP_CraftingOutputEntry`、`WBP_CraftingQueueEntry` 是否都只是一条数据的模板，没有再自己读整张表、整组数组或整条队列�?
#### 15.6A.14 这一轮之后，蓝图层还应该做什么，不应该做什�?现在蓝图层建议只保留下面这些事：

1. 设计布局
2. 配字体、颜色、间距、图标、底�?3. 绑定 `BindWidget` 对应的控件名
4. 指定 `RecipeEntryWidgetClass / MaterialEntryWidgetClass / OutputEntryWidgetClass / QueueEntryWidgetClass`
5. 子项蓝图内部放好约定名字的按钮、文本、进度条、选中标记
6. 如果要做额外动画或过渡效果，�?`BP_RefreshFromCraftingViewModel()` 里补表现层逻辑
7. 冷启动时先确认每个子项蓝图都继承了正确的 typed 父类，没有把子项又退回成普�?`UUserWidget`

不建议再在蓝图里做下面这些事�?
1. 自己循环 `CachedRecipeList` 造列表项
2. 自己循环材料和产物数组造子�?3. 自己存第二份“当前选中配方�?4. 自己重新计算活动项剩余时间和进度�?5. 自己拼第二套“能不能制作”的判断
6. 自己在子项蓝图里回调主列表去补数�?7. 自己在队列子项里轮询整条队列刷新自己

当前这轮的目标很明确�?
**把“数据到 UI 内部的主更新逻辑”尽量收�?C++，让蓝图主要只负责设计和样式�?*

### 15.7 每一组手工测试怎么�?下面这些是建议测试顺序，不要一上来就测最复杂的掉落和异常清队�?
#### 15.7.1 先测一次最基础成功制�?1. 先确认你现在测的就是 `Recipe_Test_Plank`
2. �?`15.6` 的完整步骤做一�?2. 记录材料初始数量
3. 执行 `CraftRecipe 配方行名`
4. 观察材料立即减少
5. 观察队列新增
6. 等待任务完成
7. 观察产物进入库存

通过标准�?
1. 服务端接受请�?2. 材料是入队时扣，不是完成时扣
3. 产物最终进入库�?
如果失败，先查：

1. 控制台命令入口：[AOPlayerController.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Player/AOPlayerController.cpp:32)
2. 请求入口：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:54)
3. 服务端入队判定：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:129)

#### 15.7.2 再测“入队就扣，不是完成才扣�?1. 让角色材料只够做一�?2. 记录当前材料数量
3. 执行 `CraftRecipe 配方行名`
4. 不要等任务完成，立刻检查材�?5. 确认材料已经减少
6. 等任务完成后，再确认没有被重复扣第二�?
如果失败，先查：

1. 扣料计划生成：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:266)
2. 扣料执行：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:357)
3. 槽位消耗：[AOInventoryComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp:826)

#### 15.7.3 再测“材料不够时不能入队�?1. 把角色材料减到小于配方需�?2. 执行 `CraftRecipe 配方行名`
3. 确认不会新增队列�?4. 确认剩余材料不会再被�?
这一步的目标不是�?UI 提示，而是确认服务端真相层没有塞进非法任务�?
#### 15.7.4 再测“等级不足时不能入队�?1. 先在 `DT_CraftingRecipe_Test` 里再补一条真实配方行，例�?`Recipe_Test_HighLevel`
2. 再在 `DT_CraftingRecipeSource_Test` �?`RecipeEntries` 里把这条配方加进�?3. 把这条新条目�?`UnlockLevel` 设成高于当前角色等级，例如角色当前是 `1`，这里先�?`99`
4. 执行 `CraftRecipe Recipe_Test_HighLevel`
3. 确认入队失败
4. 确认材料不被�?
如果失败，先查：

1. 解锁判定：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:177)
2. 配方来源表读取：[AOPawnData.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOPawnData.cpp:104)

#### 15.7.5 再测“正常完成后统一入包�?1. 给角色留出足够库存空�?2. 执行一条合法制�?3. 等待完成
4. 确认产物进的是角色库�?5. 确认不是制造系统自己私发一份结果，而是复用统一入包�?
如果失败，先查：

1. 完成发货：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:423)
2. 统一入包：[AOInventoryStatics.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp:64)

#### 15.7.6 最后再测“完成时入不下则掉到角色脚下�?这一组现在要带着问题意识去测，因为当前实现和方案口径还没有完全对齐�?
建议这样做：

1. 先保证角色当前能够成功入�?2. 执行一条合法制�?3. 让任务进�?`Active`
4. 在制造完成前，用其他方式把角色库存塞�?5. 等待制造结�?6. 去角色脚下找掉落�?7. 确认它可拾取，默认持�?`5` 分钟

如果你发现根本不能入队，不是你步骤错了，而是当前实现还在入队前做完整容量拦截�?
先查�?
1. 入队前容量拦截：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:148)
2. 完成后掉落分支：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:426)
3. 掉落物生成：[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:480)

#### 15.7.7 再测“中途不可取消�?这一条当前不是看某个按钮有没有禁用，而是看系统有没有提供撤销链�?
现在按这个口径验�?
1. 让一条制造任务进�?`Active`
2. 在任务运行期间，不提供任何取消按钮或取消命令
3. 让它自然完成，或者走异常中断

只要当前没有正式取消链，就符合首阶段边界�?
#### 15.7.8 最后测“异常清队且不返还材料�?1. 让一条任务已经入队，最好已经进�?`Active`
2. 记录当前剩余材料数量
3. 在制造过程中触发角色死亡，或者明确触发一�?`Reset / UnPossessed / EndPlay`
4. 确认制造队列清�?5. 确认之前扣掉的材料不返还

如果失败，先查：

1. [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:183)
2. [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:196)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:70)
4. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:560)

#### 15.7.9 阶段三专项一：满包完成后掉落
这一条是本轮最容易误判的场景。先记住测试顺序，不要反过来�?
先开和先配的顺序�?
1. 先开 `DA_AOGameData`
2. 先确�?`CraftingRecipeDataTable` 里已经有一条最小可测配�?3. 再确认测试角色最终吃到的 `PawnData` 里已经配�?`CraftingRecipeSourceDataTable`
4. 再准备一套刚好够做一次的材料，避免入队阶段就因为别的原因失败

实际操作�?
1. �?PIE 之前，先让角色当前库存还留有一点空�?2. 在控制台输入 `CraftRecipe 配方行名`，先把任务成功入�?3. 立刻让角色把库存塞满，优先用你们现有的给物品、拾取或测试调试手段补满
4. 等待制造时间结�?5. 去角色脚下找掉落�?6. 确认掉落物可拾取，默认生命期�?`5` 分钟

预期现象�?
1. 请求已经成功入队
2. 完成时如果角色库存已满，产物不会悄悄消失
3. 产物会落到角色脚�?4. 掉落物的交互文案是“拾取�?
失败先查�?
1. 先看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:442) 的完成发货入�?2. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:548) 的掉落生�?3. 再看 [AOInventoryStatics.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp:64) 的统一入包结果
4. 如果根本连入队都没成功，先回�?`15.7.1` �?`15.7.4`，不要直接判定为掉落问题

#### 15.7.10 阶段三专项二：死�?/ 异常清队绑定
这一条不要从“UI 有没有变”看，直接看角色生命周期触发后，队列和材料是不是按规则收口�?
先开和先配的顺序�?
1. 先确认测试角色是标准 `AAOCharacter`
2. 再确�?`AAOCharacter` 上挂�?`UAOCraftingComponent`
3. 再确认当前任务已经入队，最好已经进�?`Active`

实际操作�?
1. 先记录当前制造队列里有几条任务、剩余材料是多少
2. 让角色在制造过程中死亡，或者直接触发一�?`Reset`
3. 如果你要测的是非死亡的异常清队，就用 `UnPossessed` �?`EndPlay` 这条链去验证
4. 观察制造队列是否立刻清�?5. 再确认之前已经扣掉的材料没有返还

预期现象�?
1. 角色异常退出当前上下文以后，队列被清空
2. 当前这轮已经扣掉的材料不回滚
3. 不会出现“人死了，队列还挂着”的残留任务

失败先查�?
1. 先看 [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:179) �?`UnPossessed()`
2. 再看 [AOCharacter.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Character/AOCharacter.cpp:188) �?`Reset()`
3. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:108) �?`EndPlay()`
4. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:81) �?[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:70) 的计时器清理与队列重�?
#### 15.7.11 阶段三专项三：交付失败兜�?这一条是专门测“完成后没法完整交付时，系统是不是还有兜底出口”�?
先开和先配的顺序�?
1. 先准备一条产物明确的配方
2. 再确认这条配方的产物 `ItemId` 能在全局物品总表里查�?`ItemDefinitionClass`
3. 再确认测试角色当前库存空间不够一次完整接�?
实际操作�?
1. 先让任务成功入队
2. 等到接近完成前，把角色库存补到无法再完整接收产物
3. 等待任务结束
4. 先看产物是不是没有直接丢�?5. 再看角色脚下有没有掉落物
6. 最后确认掉落物能拾�?
预期现象�?
1. 交付失败以后，系统不会把结果卡死在完成�?2. 兜底路径应当把产物送到世界�?3. 世界里掉出来的物品仍然能被拾�?
失败先查�?
1. 先看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:462) �?[AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:482) 的失败交付分�?2. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:548) 的掉落生成是否返�?`true`
3. 再看 [AOItem.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Items/AOItem.cpp:1) 里掉落物是否真的可交�?4. 如果没有任何世界掉落，先回看 [AOInventoryStatics.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Inventory/AOInventoryStatics.cpp:64) 的统一入包结果，确认到底是入包失败还是兜底没走�?
#### 15.7.12 阶段三专项四：扣料失败分�?这一条要专门把“扣料失败”跟“配方没解锁”“材料不足”分开看，别把失败原因混在一起�?
先开和先配的顺序�?
1. 先确认还是同一�?`PawnData`
2. 再确�?`CraftingRecipeSourceDataTable` 里这条配方是解锁�?3. 再确�?`CraftingRecipeDataTable` 里这条配方确实存�?4. 再准备一组材料，故意让其中某一项在扣料执行时失�?
实际操作�?
1. 先执行一条平时能入队的配�?2. 在扣料前后人为制造库存变化，让某个材料槽位在执行时变成不�?3. 再次触发 `CraftRecipe 配方行名`
4. 观察这次是“直接没进队”，还是“已经准备扣料但最终被回滚�?5. 对照当前材料数量，确认失败后有没有按分流预期处理

预期现象�?
1. 如果是入队前就发现不够，队列不应新增
2. 如果是扣料执行阶段失败，已经扣掉的那部分应当按回滚路径处�?3. 不应该出现“失败了但材料多扣了一份”这种脏状�?
失败先查�?
1. 先看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:273) 的扣料计划生�?2. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:364) 的扣料执�?3. 再看 [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:426) 附近的失败后分流
预期现象�?
1. 选中的配方能成功入队时，队列会出现新条目�?2. 选中的配方不可入队时，返回值要和直接点这条配方的结果一致�?3. 如果 `SelectedRecipeRowName` 为空，`RequestEnqueueSelectedRecipe()` 应该直接失败，不应制造空请求�?4. block reason 文本只应来自 `UMVVM_Crafting`，不要在 Widget 里手工拼接一套新文案�?5. 单条可点击制造项的特殊状态应当只建立�?`UAOCraftingRecipeListEntryWidget` / `GetBlockReason()` 上，不应再依赖旧�?`CraftableStateText` / `BlockReasonText` 直出字段�?6. `WBP_CraftingQueueEntry` 的点击控件必须代表可入队����ɻع���֤ʱ�ٿ� `UButton`�����ҵ������Ȼ���ȶ��������������·��7. �������Ŀ��ʣ��ʱ�䡢���ȱȶ�Ӧ������ͬһ�� ViewModel ���գ��������б�����ϸ�����и��Զ����Ե�״̬��8. ��ͼ����ֻ�е���ȡ����չʾ�����������б������顢���е�ѭ���߼���������ͼ����
失败先查�?
1. ViewModel 获取链路：`AOMainUI.cpp` / `AOCombatFeedbackBlueprintLibrary.cpp` / `AOCraftingWidgetBase.cpp`
2. 态更新：`MVVM_Crafting.cpp`
3. 阻断原因、队列快照和剩余时间：`MVVM_Crafting.cpp`
4. 真正入队逻辑：`AOCraftingComponent.cpp`
1. [AOMainUI.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp:84)
2. [AOCombatFeedbackBlueprintLibrary.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.cpp:80)
3. [MVVM_Crafting.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h:32)
4. [MVVM_Crafting.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp:68)
5. [AOCraftingWidgetBase.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.cpp:24)
6. [AOCraftingRecipeListWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp:9)
7. [AOCraftingRecipeDetailWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeDetailWidget.cpp:9)
8. [AOCraftingQueueWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingQueueWidget.cpp:9)

实际操作�?
1. �?PIE，先确认当前角色已经持有制造组件，并且 HUD 侧能拿到 `UMVVM_Crafting`�?2. 如果你是在蓝图里测，先通过 `UAOMainUI::GetCraftingViewModel()` �?`UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)` 取到同一�?ViewModel�?3. �?UI 中先点选一条可制造配方，�?`SelectedRecipeRowName` 不是 `None`�?4. 直接调用 `RequestEnqueueSelectedRecipe()`，不要手写配方名�?5. 观察结果是否�?`RequestEnqueueRecipe(SelectedRecipeRowName)` 一致�?6. 再切一条不同配方，重复一次，确认它确实读的是当前选中的那条，而不是旧缓存�?7. 打开正式制�?UI 时，确认 `AOCraftingRecipeListWidget` �?`AOCraftingRecipeDetailWidget` 取到的是同一�?`UMVVM_Crafting`，不要一个走 HUD 入口、一个走别的缓存�?8. 选中一条被材料、等级或其他规则挡住的配方，确认明细侧能显示 `GetSelectedRecipeBlockReasonText()` 对应的文本，而不是只靠按钮灰掉�?9. 同时确认列表项里�?`RecipeIconImage` 会跟着当前数据变化；如果列表项蓝图绑定了基�?`GetBlockReason()` 的状态文案、图标或遮罩，也要确认它们不是旧缓存�?10. 再专门挑一条会在列表里显示特殊状态的配方，确认列表项蓝图拿到的是 `EAOCraftingRecipeBlockReason`，而不是继续依赖旧�?`CraftableStateText` / `BlockReasonText` 文本直出�?11. 打开 `WBP_CraftingQueueEntry` 检�?`SelectButton` 的实际类型，确认现在接的�?`UButton`，不是旧�?`Common Button` 体系�?12. 直接点击一条可入队单项，确认点击后仍然能走�?`RequestEnqueueRecipe(...)` �?`RequestEnqueueSelectedRecipe()` 的正式入队入口，而不是只更新本地表现�?13. 再点一条被材料、等级或其他规则挡住的单项，确认蓝图表现读到的是新的 `GetBlockReason()` 结果，而不是沿用上一条的可制造状态�?14. 队列里只要存在活动项，`UAOCraftingQueueWidget` 应该能读�?`GetActiveQueueEntry(...)`，并同时展示剩余时间和进度比�?15. 让活动任务继续推进一小段时间，确认队�?UI 读取的是更新后的 `GetActiveQueueRemainingSeconds()` �?`GetActiveQueueProgressRatio()`，不是旧快照�?16. 如果你是按本轮“主 Widget 直管纯蓝图子项”来接的，再额外确认一次：列表、详情、队列主 Widget 的蓝图图表里没有整表 `ForEach`，只有按条目创建子项和刷新单条子项的逻辑�?
如果你怀疑已经回退成蓝图循环，直接按下面查�?
1. 列表蓝图里不应出�?`ForEach CachedRecipeList`、`CreateWidget` 整表循环或自己缓存第二份列表
2. 详情蓝图里不应出�?`ForEach CachedRecipeDetail.MaterialEntries` / `ForEach CachedRecipeDetail.OutputEntries`
3. 队列蓝图里不应出�?`ForEach CachedQueueList`
4. 子项蓝图里不应再去读整张配方表或整条队列，只能吃单条数据
5. `WBP_CraftingRecipeEntry`、`WBP_CraftingMaterialEntry`、`WBP_CraftingOutputEntry`、`WBP_CraftingQueueEntry` 只能是单条模板，不能自己再做列表逻辑

预期现象�?
1. 选中的配方能成功入队时，队列会出现新条目�?2. 选中的配方不可入队时，返回值要和直接点这条配方的结果一致�?3. 如果 `SelectedRecipeRowName` 为空，`RequestEnqueueSelectedRecipe()` 应该直接失败，不应制造空请求�?4. block reason 文本只应来自 `UMVVM_Crafting`，不要在 Widget 里手工拼接一套新文案�?5. 单条可点击制造项的特殊状态应当只建立�?`UAOCraftingQueueEntryWidget::GetBlockReason()` 上，不应再依赖旧�?`CraftableStateText` / `BlockReasonText` 直出字段�?6. `WBP_CraftingQueueEntry` 的点击控件应当是 `UButton`，并且点击后仍然能正常触发后续入队链路�?7. 活动队列条目、剩余时间、进度比都应该来自同一�?ViewModel 快照，不允许列表、明细、队列各自读各自的状态�?8. 蓝图子项只承担单条展示，不允许把列表、详情、队列的循环逻辑塞回子项图表�?
失败先查�?
1. ViewModel 获取链路：`AOMainUI.cpp` / `AOCombatFeedbackBlueprintLibrary.cpp` / `AOCraftingWidgetBase.cpp`
2. 选择态更新：`MVVM_Crafting.cpp`
3. 阻断原因、队列快照和剩余时间：`MVVM_Crafting.cpp`
4. 真正入队逻辑：`AOCraftingComponent.cpp`

#### 15.7.14 本轮专项六：先确认库存布局把正式制造面板接到承载槽�?这一条只测承载，不先测制造逻辑。先确认 `AOLayout_Inventory` 真的把正式制造面板挂到了 `CraftingPanelWidget`，再往里看�?Widget�?
先开和先配的顺序�?
1. 先确认测试地图跑的是 `UAOLayout_Inventory`
2. 再确认这个布局蓝图里绑定了 `CraftingPanelWidget`
3. 再确�?`CraftingPanelWidget` 的实际实例是 `UAOCraftingPanelWidget`
4. 再确认面板里能拿到配方列表、配方详情和队列这三�?5. 再确认三个主区块各自下挂的是纯蓝图子项模板，而不是把整张表直接画在一�?Widget �?
实际操作�?
1. �?PIE 后先打开库存界面，不要先敲制造命�?2. 看正式制造面板是不是已经跟着库存布局一起出�?3. 检�?`CraftingPanelWidget` 这个槽位是不是空挂着
4. 打开面板后，确认列表、详情和队列三块都在同一层面板里
5. 展开列表、详情、队列三个主 Widget 的蓝图层，确认它们都没有自己写整表循�?6. 再发起一条能入队的配方，确认面板里看到的队列变化不是别的缓存

预期现象�?
1. 库存布局打开后，正式制造面板会跟着出现
2. `CraftingPanelWidget` 不会是一个没接线的空�?3. 面板内部的列表、详情和队列都能被正常看�?4. 列表、详情、队列分别通过纯蓝图子项承载单条数据，不需要蓝图自己写 `ForEach` 拼整�?
失败先查�?
1. `[AOLayout_Inventory.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.h:12)`，看 `GetCraftingPanelWidget()`
2. `[AOLayout_Inventory.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.cpp:10)`，看 `NativeOnInitialized()`
3. `[AOCraftingPanelWidget.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.h:14)`，看 `GetRecipeListWidget()` / `GetRecipeDetailWidget()` / `GetQueueWidget()`
4. `[AOCraftingPanelWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.cpp:11)`，看 `NativeConstruct()` / `HandleCraftingViewModelChanged()`

#### 15.7.15 本轮专项七：活动队列的轻量时间推进刷�?这一条只测活动条目的时间推进，不要拿它代替整套队列入队验证�?
先开和先配的顺序�?
1. 先确认队列里已经有一条正在进行的活动任务
2. 再确认队�?Widget 不是只在构造时读一次快�?3. 再确认活动任务是从同一�?`UMVVM_Crafting` 里读出来�?
实际操作�?
1. 让角色先入队一条制造任�?2. 打开正式制造面板里的队列区�?3. 记下当前活动条目的剩余时间和进度�?4. 原地等待大约 `0.2` �?`0.3` 秒，不要重新发起新命�?5. 再看一次剩余时间和进度�?6. 如果 UI 上能直接看出变化，再继续等一小段时间，确认它是连续推进，不是一次性跳�?7. 切走面板再切回来，确认刷新不是只靠初次构�?
预期现象�?
1. 活动条目的剩余时间会持续变小
2. 进度比会跟着轻微变化
3. 重新打开或切换面板以后，队列仍然能读到当前活动条目的最新时�?
失败先查�?
1. `[AOCraftingQueueWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingQueueWidget.cpp:11)`，看 `NativeConstruct()` / `RefreshActiveEntryTiming()` / `StartTimingRefresh()` / `StopTimingRefresh()`
2. `[AOCraftingPanelWidget.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingPanelWidget.cpp:11)`，看 `HandleCraftingViewModelChanged()`
3. `[MVVM_Crafting.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.cpp:118)`，看 `GetActiveQueueEntry(...)` / `GetQueueEntryRemainingSeconds(...)` / `GetQueueEntryProgressRatio(...)`

#### 15.7.16 本轮专项六：制造耗时要读�?`CraftingSpeedBonus`
这一条是专门验证这轮新增的复制属性和耗时缩放，不要只看队列有没有动�?
先确认代码链路：

1. [AOCombatAttributeSet.h](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h:124)
2. [AOCombatAttributeSet.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.cpp:187)
3. [AOCraftingComponent.cpp](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp:750)

实际操作�?
1. 先准备一条基础制造时长能看出来差异的配方�?2. 记录未加成时这条配方的预计制造时长�?3. 让测试角色获得一组明确会修改 `CraftingSpeedBonus` 的属性来源，再重新发起同一条配方�?4. 对比入队后队列里该条目的预计结束时间，确认它比未加成时更短�?5. 如果没有其他加成源，优先确认 `CraftingSpeedBonus` 复制到了当前角色对应�?`UAOCombatAttributeSet`�?6. 再确�?`ResolveCraftingDurationSeconds(...)` 确实是通过 `ResolveTotalCraftingSpeedBonus()` 去读这个属性，而不是读了别的旧字段�?
预期现象�?
1. `CraftingSpeedBonus` 改变后，新入队任务的制造时长应随之缩放�?2. 没有加成时，时长应回到基础值，不应莫名其妙变短�?3. 复制到别的客户端后，看到的队列预计结束时间应与服务端一致�?
失败先查�?
1. 属性复制：`AOCombatAttributeSet.cpp`
2. 速度汇总：`AOCraftingComponent.cpp`
3. UI 观察数据是否只是旧快照：`MVVM_Crafting.cpp`

### 15.8 验收口径
这一轮不要把“命令能敲”“屏幕有反应了”当成通过�?
首阶段真正要过的是下面这些口径：

1. 命令触发后，服务端必须先判定、先扣料、再入队，不允许完成时才补扣�?2. 配方是否合法，必须由服务端真相层收口，不能只靠表现层假设�?3. 制造完成后，产物必须先尝试走统一入包，而不是制造系统私发一份结果�?4. 如果完成时入不下，产物目标行为应该是掉到角色脚下，默认持�?`5` 分钟，可被拾取�?5. 当前最小同步只要求拥有者能看到正确队列变化，不要求这一轮先把完�?UI 做完�?6. 异常中断后，队列必须清空，而且这轮规则是不返还材料�?7. 阶段三新增的四个专项场景必须都能按本章步骤复现并说清失败点，不能只靠“看起来差不多”�?8. 本轮新增�?ViewModel 入口必须能从 `UAOMainUI::GetCraftingViewModel()` �?`UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(...)` 取到同一�?`UMVVM_Crafting`，并�?`RequestEnqueueSelectedRecipe()` 的结果要和直接点配方的结果一致�?9. 本轮新增�?`CraftingSpeedBonus` 复制属性必须能影响 `UAOCraftingComponent::ResolveCraftingDurationSeconds(...)` 的结果，且多人视图看到的预计完成时间不能各看各的�?10. 本轮新增正式制�?Widget 必须都能从同一�?`UMVVM_Crafting` 取到 block reason、活动队列条目、剩余时间和进度比，不允许一�?Widget 自己拼一套状态�?11. `UAOCraftingRecipeDetailWidget`、`UAOCraftingQueueWidget` 的手工验证必须能复现到对应的 UI 反馈，不允许只验证命令入队，不验证正式消费层展示�?12. 本轮新增�?`CraftingPanelWidget` 承载槽必须能�?`UAOLayout_Inventory` 打开时稳定承载正式制造面板，不能表现成“布局开了但面板没接上”�?13. 本轮新增�?`UAOCraftingQueueWidget` 轻量时间推进必须能在活动任务期间持续刷新，不能只在构造时读一次剩余时间�?14. 本轮蓝图接线必须遵守“主 Widget 直管纯蓝图子项”的收口方式，列表、详情、队列的子项都只能做单条展示，不能自己再写整表循环�?15. 本轮新增�?typed 子项必须分别继承 `UAOCraftingRecipeListEntryWidget`、`UAOCraftingRecipeMaterialEntryWidget`、`UAOCraftingRecipeOutputEntryWidget`、`UAOCraftingQueueEntryWidget`，并在蓝图里对齐本章列出的控件名，不能退回普�?`UUserWidget`�?16. 本轮单条可点击制造项如果要做“可制作 / 不可制作 / 未解�?/ 材料不足 / 队列满”等特殊表现，验收时必须确认它只建立�?`UAOCraftingQueueEntryWidget::GetBlockReason()` 返回�?`EAOCraftingRecipeBlockReason` 上，不再依赖旧的 `CraftableStateText` / `BlockReasonText` 文本直出�?17. 本轮验收�?`WBP_CraftingQueueEntry` �?`SelectButton` 必须实际使用 `UButton`，并且点击后仍然能稳定驱动后续入队链路�?18. 本轮验收时必须看见图标与状态表现同步正确，尤其是详情项�?`ItemIconImage`、队列单项的 `RecipeIconImage`、基�?`GetBlockReason()` 的蓝图表现，以及活动项的 `RemainingText` / `ProgressBar`�?19. 本轮验收时必须确认没有回退成蓝图整表循环，至少要检查列表、详情、队列三个主 Widget 的事件图里不再有 `ForEach` 造整表子项的逻辑�?
### 15.9 这一章怎么�?如果你现在是第一次接这个功能，建议按下面顺序走，不要跳步�?
1. 先开 `DA_AOGameData`，确�?`ItemCatalogDataTable` �?`CraftingRecipeDataTable`�?2. 再确认测试地图里的角色最终吃的是哪份 `PawnData`�?3. 再给这份 `PawnData` �?`CraftingRecipeSourceDataTable`�?4. 再准备一条最小配方和一套刚好够一次的材料�?5. 然后�?PIE，用 `CraftRecipe 配方行名` 跑第一条成功主链�?6. 成功主链过了以后，再测材料不足、等级不足�?7. 再测正常完成入包�?8. 再测满包完成后掉落、死�?/ 异常清队、交付失败兜底、扣料失败分流�?9. 再额外测一遍本轮新增的 `RequestEnqueueSelectedRecipe()` 路径，确�?UI 里当前选中的配方就是最终入队的那条�?10. 再额外测一�?`CraftingSpeedBonus`，确认改属性后队列预计耗时确实变化�?11. 每一步失败时，先按本章写的“先查哪”回头，不要直接跳去�?UI 或改工作台�?
这样走的好处是：

每一步失败时，你都知道自己现在卡在哪一层�?
不会把下面这些问题混成一团：

1. 资源没配
2. 角色吃错 `PawnData`
3. 控制器不�?`AAOPlayerController`
4. 请求入口没进
5. 服务端拒绝入�?6. 扣料失败
7. 完成发货失败
8. 掉落没生�?

