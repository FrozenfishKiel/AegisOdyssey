---
title: 角色库存交互冷启动测试方案 2026-05-28
date: 2026-05-28
tags:
  - knowledge
  - interaction-system
  - inventory-equipment
  - test-plan
status: draft
---

# 角色库存交互冷启动测试方案
这次要补的，不只是“测一测角色库存能不能打开”，而是要把这轮 UI 真正怎么接出来、怎么验证没有接歪，一次性写清楚。因为你前面指出的问题是对的：如果文档只写“测上下文分发”，但不告诉接手的人到底要在编辑器里挂什么 Widget、哪个类挂到哪、什么名字必须一致、第一步从哪里开，会直接失去文档的价值。

所以这份文档这次不再写成抽象 checklist，而是按“先把 UI 接出来，再按冷启动顺序验”的方式写。你拿着这份文档，应该能回答下面这些问题：

1. 现在承接角色库存显示的页面到底是哪一个 UI。
2. `AOInventoryUI` 在这一轮到底负责什么，不负责什么。
3. 现有库存页面里，self 侧和 target 侧的面板是怎么挂出来的。
4. 编辑器里第一步要开哪个 Widget，第二步要看哪个绑定名。
5. 场上打开 AI 或尸体库存之后，怎么判断是 UI 没接、上下文没分发，还是正式数据链没走通。

相关设计文档先看这几份：

- [[CHARACTER_INVENTORY_INTERACTION_DESIGN_2026-05-28]]
- [[CHARACTER_INVENTORY_INTERACTION_DESIGN_SUPPLEMENT_2026-05-28]]
- [[INVENTORY_MAINLINES_2026-05-28]]

## 先把这轮 UI 的核心语义说死
这轮 UI 的核心其实很简单，但如果这句话不锁死，后面很容易又改歪。

现在负责“这页库存界面到底显示谁”的，不是 `AOLayout_Inventory`，不是 `AOBackPackUI`，也不是别的子面板，而是 `AOInventoryUI` 这一页本身。它做的事情可以概括成一句话：先从当前玩家和当前交互会话里，拼出两份页面显示上下文，然后把这两份上下文分别分发给同一套子面板。

也就是说，`AOBackPackUI / AOQuickBarUI / AOFormalEquipmentBarUI / AOSkillBarUI` 现在都不该自己判断“我是玩家侧还是目标侧”。它们只负责接收一个已经组好的 `FAOInventoryDisplayContext`，然后按照这个上下文去拿对应组件、绑定对应 ViewModel、刷新对应显示。

这件事为什么重要，因为它保证了三层边界不乱。

第一层，页面负责决定“显示谁”。

第二层，子面板负责“怎么把这一份上下文渲染出来”。

第三层，正式交互链仍然负责“修改谁的库存数据”。

这三层一旦混掉，就会退回到你前面指出的那种脏方案：每个子面板自己去猜目标是谁，最后所有地方都写一遍 self / target 判断，越改越乱。

## 现在这页 UI 到底是怎么构成的
这一节只回答一个问题：如果你现在要把“打开角色库存后显示角色库存 UI”这件事从编辑器里接出来，你应该理解成什么结构。

当前结构不是“新建一个角色专用库存大页面”，也不是“让 Layout 识别当前是玩家还是目标，然后切换子树”。当前结构是复用现有库存页，把它当成一个同时承载 self 侧和 target 侧的容器页。

页面承载层还是原来的交互 Session Widget 体系。相关基类是：

1. `UAOInteractionSessionWidget`
2. `UAOLayout_Inventory`
3. 页面内部的 `UAOInventoryUI`

这里真正要注意的是，`UAOLayout_Inventory` 目前仍然只是 Session Widget 容器，它本身没有承担这轮上下文分发职责。它主要还是挂在外层，承接交互会话和已有库存页内容。真正开始区分 self / target 的地方，是页面内部那一层 `AOInventoryUI`。

页面下面的内容，现在按语义应该拆成两组：

1. self 侧面板组
2. target 侧面板组

每一组里都可以复用同样的子面板类型：

1. `AOBackPackUI`
2. `AOQuickBarUI`
3. `AOFormalEquipmentBarUI`
4. `AOSkillBarUI`

这四种子面板不是“玩家专用版”和“目标专用版”两套类。它们仍然是同一套类，只是实例挂在不同位置，接收到不同的显示上下文。

## 编辑器里这次 UI 应该怎么接
这一节直接回答“UI 怎么构建”。

### 第一步，先确认你接的页面根不是 `AOLayout_Inventory`
这一轮最容易理解错的地方就在这里。你在 UMG 编辑器里真正要检查的，不是 `AOLayout_Inventory` 有没有新增一堆 target 判断，而是它内部有没有一层 `AOInventoryUI` 作为页面内容根。

换句话说，外层还是 `UAOInteractionSessionWidget` 体系负责承接会话，但 self / target 上下文分发这件事，必须落在页面里的 `AOInventoryUI` 实例上。

如果你打开库存页蓝图时发现，self 和 target 相关面板都直接挂在 Layout 层上，而且页面里没有对应的 `AOInventoryUI` 内容根，那这轮接线方向就已经错了。

### 第二步，确认这页内容根是 `AOInventoryUI`
接着看库存页面里的内容树。承担库存页主体内容的那个 Widget，父类应该是 `UAOInventoryUI`，不是普通 `UUserWidget`，也不是别的中间 Layout 类。

因为这一轮代码里，`AOInventoryUI` 才有下面这些能力：

1. 维护 `SelfInventoryDisplayContext`
2. 维护 `TargetInventoryDisplayContext`
3. 监听外层 `UAOInteractionSessionWidget` 的会话变化
4. 在会话变化后调用 `RefreshInventoryPageContexts()`
5. 把上下文分发给 self / target 两侧子面板

如果当前库存页内容根不是 `AOInventoryUI`，那代码里这一轮加的上下文分发能力根本接不上。

### 第三步，把 self 侧四个子面板挂到 `AOInventoryUI`
现在开始看真正的绑定名。`AOInventoryUI` 这轮通过 `BindWidgetOptional` 直接暴露了八个槽位，四个是 self 侧，四个是 target 侧。

self 侧需要确认下面四个名字在页面蓝图里能绑定上：

1. `SelfBackPackPanel`
2. `SelfQuickBarPanel`
3. `SelfFormalEquipmentPanel`
4. `SelfSkillPanel`

这四个位置分别应该挂：

1. `AOBackPackUI`
2. `AOQuickBarUI`
3. `AOFormalEquipmentBarUI`
4. `AOSkillBarUI`

这里重要的不是视觉摆放，而是“名字要对、类要对”。因为 `AOInventoryUI::RefreshInventoryPageContexts()` 就是按这四个成员变量去分发 self 上下文的。名字不对，哪怕你视觉上摆了一个很像的面板，也不会真正被代码接管。

### 第四步，把 target 侧四个子面板挂到 `AOInventoryUI`
然后是 target 侧。同样要检查这四个名字：

1. `TargetBackPackPanel`
2. `TargetQuickBarPanel`
3. `TargetFormalEquipmentPanel`
4. `TargetSkillPanel`

这里挂的类仍然是同一套：

1. `AOBackPackUI`
2. `AOQuickBarUI`
3. `AOFormalEquipmentBarUI`
4. `AOSkillBarUI`

这一步最关键的理解是：target 侧不是“另一套类”，而是“同一套类的另一组实例”。也正因为如此，这轮方案才是轻的。代码不需要再发明一套目标库存专用 Widget，只需要让页面给第二组实例喂 target 上下文。

### 第五步，目标头部信息如果要显示，就从 `AOInventoryUI` 取
如果这页库存 UI 里要显示目标名称、目标是否死亡之类的头部信息，这轮也不应该再去蓝图里手写查找逻辑。

页面级别现在已经提供了最小查询入口：

1. `GetTargetInventoryDisplayName()`
2. `IsTargetInventoryOwnerDead()`
3. `HasTargetInventoryDisplayContext()`

所以头部显示应该直接绑定这几个接口，而不是在蓝图里重新去找当前 Session、拿 Actor、再自己判断死活。否则表面上看只是“顺手写个标题”，实则又在 UI 层分叉出第二套解析逻辑。

### 第六步，制造面板继续留在 self 侧，不要往 target 侧扩
你前面已经定过边界了，所以这里直接写死：制造系统这轮不属于 target 侧显示范围。

这意味着库存页面里如果本来就有 Crafting 相关区域，它继续按 self 侧语义存在，不需要因为现在能打开角色库存，就额外给 target 侧做一块制造交互区域。否则这轮范围会立刻失控。

## 这套 UI 接好以后，数据到底怎么流
这一节是为了让接手的人知道，为什么这样接不会变成两套真相。

页面初始化时，`AOInventoryUI::NativeConstruct()` 会先做两件事：

1. `BindOwningInventoryPageSession()`
2. `RefreshInventoryPageContexts()`

第一件事是往上找到外层的 `UAOInteractionSessionWidget`，并监听当前交互会话变化。第二件事是立刻构建当前页面的 self / target 显示上下文，并把它们分发给子面板。

self 上下文来自当前 owning pawn，本质上是从玩家自己身上找：

1. `BackPackComponent`
2. `QuickBarComponent`
3. `FormalEquipmentSlotInventoryComponent`
4. `FormalEquipmentManagerComponent`
5. `SkillComponent`
6. `SkillSlotInventoryComponent`

target 上下文则不是自己去查 Actor，而是从当前 `UAOContainerInteractionSessionModel` 读出来。也就是说，角色目标侧数据的正式来源，仍然是“当前容器会话正在观察谁、观察到了哪些组件”。

这一点非常关键。它说明 UI 并没有另起一条目标同步链。它只是把现有容器会话已经观察到的目标组件重新打包成 `FAOInventoryDisplayContext`，再交给 target 侧子面板。

## 子面板现在各自应该怎么理解
这部分不是源码解析，而是告诉接手的人后面看代码时要带着什么预期。

### `AOBackPackUI`
`AOBackPackUI` 现在的职责非常简单。它接收一个显示上下文，拿其中的 `BackPackComponent`，再从这个组件取 `InventoryViewModel` 并刷新格子。

它不应该自己去判断当前是不是 target，也不应该自己去会话里找目标 Actor。当前保留的 self fallback，只是为了兼容旧的“玩家自己开背包”链路不要瞬间全空，不是为了让它继续承担目标解析职责。

### `AOQuickBarUI`
`AOQuickBarUI` 和背包是同一语义。它应该读 `DisplayContext.QuickBarComponent`，再绑定 QuickBar 对应的 ViewModel。

如果 target 侧 QuickBar 没有显示，优先先查这页 UI 有没有把 `TargetQuickBarPanel` 绑定上，再查当前目标是否真的有 `QuickBarComponent`，不要反过来先怀疑拖拽逻辑或快捷键逻辑。

### `AOFormalEquipmentBarUI`
正式装备栏比背包多一层，它既需要正式装备库存组件，也需要正式装备管理组件，因为格子类型和显示重建都依赖它们。

所以这块如果 target 侧显示异常，要同时看两件事：页面有没有把 target 上下文正常分发下来，以及目标侧的 `FormalEquipmentInventory` 和 `FormalEquipmentManager` 是否都存在。只看其中一边不够。

### `AOSkillBarUI`
技能栏这块稍微特殊一点，因为它除了显示，还带技能观察刷新和输入映射刷新。但这一轮它的显示身份仍然必须由页面级上下文决定。

它现在接到新上下文后，会重新绑定当前显示对象的技能观察委托，再刷新技能槽。如果打开目标 A 再打开目标 B 时，技能栏残留的是目标 A 的内容，那首先应该怀疑的是它没正确重新绑定新显示上下文，而不是先怀疑技能系统本身。

## 先按这个顺序做冷启动，不要跳
这部分才是测试顺序。不是所有东西混着点，而是固定顺序，一个问题一个问题排。

## 第一轮，先确认代码和蓝图接线都站住
这一轮先不看运行时拖拽，先看结构是否站住。

第一步，编译工程，至少保证当前 UI 改动没有把工程打崩。  
第二步，打开库存页面蓝图，确认页面内容根是 `AOInventoryUI`。  
第三步，确认 self / target 两侧八个子面板绑定名都对上。  
第四步，确认 target 侧挂的不是“别的临时 Widget”，而是和 self 侧相同的四种子面板类。  
第五步，确认目标头部如果存在，是从 `AOInventoryUI` 暴露接口取值，而不是蓝图里另写一套查找。

这一轮通过的标准不是“看起来有个布局”，而是“页面的分发入口确实已经落到 `AOInventoryUI` 上”。

## 第二轮，只测普通交互入口能不能把这页 UI 打开
这一轮开始进 PIE，但只测入口。

第一步，找一个实际使用 `AAOCharacter` 的目标。  
第二步，确认它身上的 `InventoryInteractionOptions` 已经配好。  
第三步，确认对应 option 的 `InteractionWidgetClass` 指向的是当前库存交互页，而不是空。  
第四步，优先拿死亡角色做正向测试，不要先拿存活敌人。  
第五步，靠近目标，通过普通交互选项触发打开库存。

这一步预期是：你能打开的不是一页新 UI，而是现有库存交互页，并且里面能看到 self 侧和 target 侧的库存语义区域。

如果这里根本打不开，先查 `InventoryInteractionOptions` 和 `InteractionWidgetClass`，不要先去查子面板代码。因为页面还没打开时，子面板是否能刷新根本还不是主问题。

## 第三轮，打开后先看“显示的是谁”
真正进到这页 UI 后，第一件要测的不是拖拽物品，而是看 self 和 target 两边读的是不是不同对象。

最好的做法是先手动把数据准备成一眼能看出差别的样子。比如：

1. 玩家背包放物品 A，目标背包放物品 B。
2. 玩家物品栏放物品 C，目标物品栏放物品 D。
3. 玩家正式装备栏和目标正式装备栏至少有一个槽位不一样。
4. 玩家技能栏和目标技能栏也尽量摆出明显差异。

然后打开目标库存，依次看：

1. `SelfBackPackPanel` 显示的是不是玩家自己的背包。
2. `TargetBackPackPanel` 显示的是不是目标的背包。
3. `SelfQuickBarPanel` 和 `TargetQuickBarPanel` 是否也分别读了不同对象。
4. 正式装备栏和技能栏是否也是同样逻辑。

这一轮最重要的验收点不是“页面有东西”，而是“左右两边读的不是同一份东西”。

## 第四轮，专门测切换目标刷新
这一轮是为了抓“第一次能显示，第二次切目标就残留”的问题。

第一步，准备两个库存差异明显的目标 A 和目标 B。  
第二步，先打开目标 A 的库存，记住 target 侧背包、物品栏、正式装备栏、技能栏内容。  
第三步，关闭会话。  
第四步，立即打开目标 B 的库存。  
第五步，再看 target 侧是否全部切成目标 B 的内容。

这一步还要顺手看目标标题和死亡状态。如果标题还停在 A，或者死亡状态和 B 对不上，那问题就不是某一个子面板自己刷新慢，而是页面级 target 上下文根本没有刷新干净。

## 第五轮，专门测 target 侧空面板
这轮是为了避免你后面调试时被自动隐藏骗掉。

准备一个目标，让它某些区域故意为空，比如没有技能、没有正式装备、没有 QuickBar 内容。然后打开它的库存。

预期现象不是“整个面板没了”，而是“面板还在，只是里面没内容”。因为这轮方案明确要求，空面板也保留显示，便于调试分辨问题。

如果 target 某个区域整个消失，优先查这个面板有没有真正绑定到 `AOInventoryUI` 的 target 槽位上，再查蓝图里是否有额外可见性判断，而不是先怀疑目标身上没组件。

## 第六轮，最后才测正式交互链有没有歪
前面显示确认完了，最后才开始测操作。因为只有先证明“显示的是谁”，后面测“改的是谁”才有意义。

这一轮至少覆盖四类现有操作：

1. 右键
2. 拖拽
3. 交换
4. 使用

建议最少做三组动作。

第一组，从 target 侧拿一个物品到 self 侧。  
第二组，从 self 侧拖一个物品到 target 侧。  
第三组，找一个当前规则允许使用的物品，做一次使用或右键动作。

每做完一组动作，都不要只看界面即时变化。必须关掉会话再重新打开，看数据是不是正式落地了。因为这轮最怕的不是界面不变，而是界面看起来变了，重开后才发现只是显示缓存动了。

## 多人冷测试至少怎么收
多人这次不要求一口气测完所有边界，但至少别完全不测。因为这个功能天然牵涉“发起打开的人”和“库存所属对象”不是同一个对象。

最小建议是做 listen server 或双端 PIE，收三件事：

1. 任意客户端发起正常交互时，最终是否由服务端决定并建立会话。
2. 打开后 target 侧看到的数据，在服务端和客户端语义是否一致。
3. 做一次最基本的 target 到 self 或 self 到 target 的物品移动后，服务端和各客户端看到的结果是否一致。

如果联机下出现本地能打开、服务端不认，或者本地界面动了、同步后回滚，那优先查正式交互链和服务端请求，不要先查 UI。

## 这轮最容易错的地方，直接写给接手的人
第一，页面根不是 `AOInventoryUI`。  
这个错一旦发生，后面所有“self / target 分发”代码都等于白写。

第二，target 侧挂的是别的临时 Widget，不是和 self 侧同类的子面板。  
这会直接把复用链打断。

第三，八个绑定名有一个没对上。  
视觉上看起来像挂上了，运行时其实成员还是空。

第四，标题或死亡状态在蓝图里自己查，不走 `AOInventoryUI` 接口。  
这会悄悄长出第二套 target 解析逻辑。

第五，看到 target 侧是空，就误判成没接上。  
先分清是“空但还显示”，还是“整个面板消失”。

第六，拖拽后界面变了，就以为数据已经改对。  
必须关掉重开再看。

## 这次通过验收的最低标准
这轮如果要算真正闭环，至少要满足这些点：

1. 页面内容根是 `AOInventoryUI`。
2. self / target 两侧四类子面板都已经挂上，并且绑定名正确。
3. 打开角色库存后，显示的是现有库存页，不是新起的角色专用页。
4. self 侧与 target 侧能显示不同对象的数据。
5. 切换不同目标后，target 侧会完整刷新。
6. 目标标题和死亡状态与当前目标一致。
7. target 空面板仍然保留显示。
8. 右键、拖拽、交换、使用仍然走原正式数据链。
9. 关闭重开后，修改结果仍然正确落在对应对象上。

## 关联代码位置
这次真正在支撑这页 UI 的关键位置就是这些：

1. `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h`
2. `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
3. `Source/AegisOdyssey/UI/Common/Inventory/AOBackPackUI.h`
4. `Source/AegisOdyssey/UI/Common/Inventory/AOBackPackUI.cpp`
5. `Source/AegisOdyssey/UI/Common/Inventory/AOQuickBarUI.h`
6. `Source/AegisOdyssey/UI/Common/Inventory/AOQuickBarUI.cpp`
7. `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.h`
8. `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.cpp`
9. `Source/AegisOdyssey/UI/Widgets/Skill/AOSkillBarUI.h`
10. `Source/AegisOdyssey/UI/Widgets/Skill/AOSkillBarUI.cpp`
11. `Source/AegisOdyssey/UI/AOInteractionSessionWidget.h`
12. `Source/AegisOdyssey/UI/AOInteractionSessionWidget.cpp`
13. `Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.h`
14. `Source/AegisOdyssey/UI/Layouts/AOLayout_Inventory.cpp`
15. `Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h`
16. `Source/AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.cpp`
17. `Source/AegisOdyssey/Character/AOCharacter.h`
18. `Source/AegisOdyssey/Character/AOCharacter.cpp`
