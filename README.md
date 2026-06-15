# AegisOdyssey

AegisOdyssey 是一个基于 Unreal Engine 5.6 的第三人称动作 RPG 项目。这个仓库现在承载的已经不只是一个能跑起来的角色 Demo，而是在 `CommonGame/CommonUI + GAS + StateTree + GameFeatures + MVVM` 这套基础设施上，逐步把角色、战斗、库存、装备、交互、采集、制造、技能和 AI 串成一条比较完整的运行时主链。对第一次进仓库的人来说，最重要的不是把所有目录都扫一遍，而是先知道项目到底在解决什么问题、哪些目录是主线、每个系统的真相层放在哪里。

从当前配置看，项目默认启动地图是 `/Game/Levels/TestMap`，默认游戏模式是 `/Game/Games/Mode/BP_AOMainGameMode`，默认 `GameInstance` 是 `BP_AOGameInstance`，这些入口可以先从 [Config/DefaultEngine.ini](Config/DefaultEngine.ini)、[AegisOdyssey.uproject](AegisOdyssey.uproject)、[Source/AegisOdyssey/GameModes](Source/AegisOdyssey/GameModes) 和 [Source/AegisOdyssey/System](Source/AegisOdyssey/System) 对起来看。本文里出现的文件和目录路径全部都是仓库相对路径，不依赖任何本机目录结构，直接按仓库根目录展开即可。源码层面最值得先抓住的三个入口是 [Source/AegisOdyssey/Character/AOCharacter.h](Source/AegisOdyssey/Character/AOCharacter.h)、[Source/AegisOdyssey/GameModes/AOGameMode.h](Source/AegisOdyssey/GameModes/AOGameMode.h) 和 [Docs/Knowledge](Docs/Knowledge)，前两个告诉你角色和玩法框架怎么接，后一个告诉你项目已经把哪些系统沉淀成了可复用的“项目地图”。

如果你只是想快速进入状态，建议按这个顺序看：

- 先看项目骨架： [Config](Config)、[Source/AegisOdyssey/GameModes](Source/AegisOdyssey/GameModes)、[Source/AegisOdyssey/System](Source/AegisOdyssey/System)、[Source/AegisOdyssey/Character](Source/AegisOdyssey/Character)
- 先看系统地图： [Docs/Knowledge/GameplayFramework/PROJECT_MAP.md](Docs/Knowledge/GameplayFramework/PROJECT_MAP.md)、[Docs/Knowledge/CombatSystem/PROJECT_MAP.md](Docs/Knowledge/CombatSystem/PROJECT_MAP.md)、[Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md](Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md)
- 先看玩法资源： [Content/Levels](Content/Levels)、[Content/Games](Content/Games)、[Content/UI](Content/UI)
- 先看蓝图和界面怎么落地： [Source/AegisOdyssey/UI](Source/AegisOdyssey/UI)、[Source/AegisOdyssey/GameFeatures](Source/AegisOdyssey/GameFeatures)

## 文件大纲

```text
AegisOdyssey
├─ AegisOdyssey.uproject
├─ Config/                         # 地图、模式、输入、标签、AssetManager 等项目级配置
├─ Source/AegisOdyssey/            # 项目主模块 C++ 源码
│  ├─ GameModes/                   # Experience / GameMode / GameState / WorldSettings
│  ├─ Character/                   # 角色本体与核心运行时组件挂载点
│  ├─ Player/                      # Controller / LocalPlayer / PlayerState / Spawn
│  ├─ Animation/                   # 动画实例、Notify、战斗/采集命中窗
│  ├─ Camera/                      # 第三人称相机组件、相机模式、相机管理器
│  ├─ AbilitySystem/               # GAS Ability / Attribute / Effect / Task
│  ├─ Combat/                      # 战斗辅助结构
│  ├─ Inventory/                   # 背包、物品定义、库存实例、统一入包
│  ├─ Equipment/                   # 快捷栏、武器、正式装备栏
│  ├─ Items/                       # 世界物品 Actor 与物品目录辅助结构
│  ├─ Interaction/                 # 交互对象、会话、容器样板
│  ├─ Harvest/                     # 采集动作、工具、对象、Resolver、节点生命周期
│  ├─ Crafting/                    # 制造组件与制造观察数据
│  ├─ SkillSystem/                 # 技能定义、实例、槽位、运行时总入口
│  ├─ StateTree/                   # 通用 StateTree 组件、条件、任务
│  ├─ UI/                          # CommonUI + MVVM + Widget + 世界血条
│  ├─ GameFeatures/                # GameFeature Action 与 Experience 相关接线
│  └─ System/                      # AssetManager / GameData / GameInstance 等系统底座
├─ Content/                        # 地图、蓝图、UI、技能、美术与第三方资源包
├─ Docs/                           # 设计稿、进度记录、知识文档、系统项目地图
│  ├─ Knowledge/                   # 按系统整理的长期知识库
│  └─ superpowers/                 # 方案、计划、设计文档
├─ Plugins/                        # 本地插件与随仓库携带的插件源码
└─ Tools/                          # 辅助排查脚本
```

## 模块导航

- **Gameplay Framework / 项目总线**  
  这条线决定项目怎么起、角色怎么拿到 PawnData、Experience 怎么加载、GameFeature 怎么把能力和 UI 接进来。它的核心设计是把“项目启动、能力装配、界面接线、资源加载”从单个角色或单个地图里抽出来，统一收口到 `GameMode + Experience + AssetManager + GameFeature` 这一层；设计思想是先把玩法底座做成可装配、可替换、可扩展的框架，再让具体战斗、交互、采集这些系统往上挂；核心目的是避免项目规模变大以后，所有能力都硬绑在角色蓝图或关卡蓝图里。先看 [Source/AegisOdyssey/GameModes](Source/AegisOdyssey/GameModes)、[Source/AegisOdyssey/System](Source/AegisOdyssey/System)、[Source/AegisOdyssey/GameFeatures](Source/AegisOdyssey/GameFeatures)、[Plugins/GameFeatures/AOGameCore](Plugins/GameFeatures/AOGameCore) 和 [Docs/Knowledge/GameplayFramework/PROJECT_MAP.md](Docs/Knowledge/GameplayFramework/PROJECT_MAP.md)。如果你想先抓“项目为什么不是一个普通第三人称模板”，这里就是第一站，尤其是 [Source/AegisOdyssey/GameModes/AOGameMode.h](Source/AegisOdyssey/GameModes/AOGameMode.h)、[Source/AegisOdyssey/GameModes/AOExperienceDefinition.h](Source/AegisOdyssey/GameModes/AOExperienceDefinition.h)、[Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.h](Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.h)、[Source/AegisOdyssey/System/AOAssetManager.h](Source/AegisOdyssey/System/AOAssetManager.h) 和 [Source/AegisOdyssey/AOAbilitySystemGlobals.h](Source/AegisOdyssey/AOAbilitySystemGlobals.h) 这几处。

- **角色、玩家与输入主入口**  
  项目很多系统最后都会收口到角色本体，所以 [Source/AegisOdyssey/Character](Source/AegisOdyssey/Character) 和 [Source/AegisOdyssey/Player](Source/AegisOdyssey/Player) 是第二个必看区域。这里的核心设计是让 `AOCharacter` 成为主要运行时挂载点，把战斗、库存、装备、技能、制造、交互这些长期能力都以组件方式并列挂上去；设计思想是尽量把“角色能做什么”和“系统怎么实现”拆开，角色负责承载，子系统负责各自的真相；核心目的是让玩家角色、AI 角色和未来的新 Pawn 能复用同一套骨架，而不是每次都重新拼一套功能集合。[Source/AegisOdyssey/Character/AOCharacter.h](Source/AegisOdyssey/Character/AOCharacter.h) 直接把背包、快捷栏、正式装备、技能、制造、战斗、交互、持久状态标签、AI 决策这些组件挂在一个角色身上；[Source/AegisOdyssey/Character/AOHeroComponent.h](Source/AegisOdyssey/Character/AOHeroComponent.h)、[Source/AegisOdyssey/Character/AOInputBufferComponent.h](Source/AegisOdyssey/Character/AOInputBufferComponent.h)、[Source/AegisOdyssey/Input](Source/AegisOdyssey/Input)、[Source/AegisOdyssey/Player/AOPlayerController.h](Source/AegisOdyssey/Player/AOPlayerController.h) 和 [Source/AegisOdyssey/Player/AOLocalPlayer.h](Source/AegisOdyssey/Player/AOLocalPlayer.h) 则负责把输入、控制权和本地玩家上下文接回来。想理解“一个角色到底承接了哪些玩法能力”，直接从这里读最省时间。

- **战斗系统**  
  这个项目的战斗主线不是每种攻击各算各的，而是尽量把命中采集和统一结算重新收回同一条链。它的核心设计是“命中采集可以分散，但命中结算必须统一”，所以攻击、技能、投射体、范围伤害最后都要把战斗上下文送回同一套结果结构；设计思想是先统一真相，再统一表现，先让上下文、结算、结果消息稳定，再让跳字、Cue、UI、镜头反馈去消费；核心目的是让战斗系统后续扩技能、扩武器、扩防御语义时，不会因为每条链都有一套私有逻辑而失控。核心入口在 [Source/AegisOdyssey/AOAbilityTypes.h](Source/AegisOdyssey/AOAbilityTypes.h)、[Source/AegisOdyssey/AOCombatResultMessage.h](Source/AegisOdyssey/AOCombatResultMessage.h)、[Source/AegisOdyssey/AbilitySystem](Source/AegisOdyssey/AbilitySystem)、[Source/AegisOdyssey/ExecCal](Source/AegisOdyssey/ExecCal)、[Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.h](Source/AegisOdyssey/Character/AOCharacterCombatManagerComponent.h) 和 [Docs/Knowledge/CombatSystem/PROJECT_MAP.md](Docs/Knowledge/CombatSystem/PROJECT_MAP.md)。如果你在找轻攻击、重攻击、翻滚、格挡、受击这些现成实现，可以直接看 [Source/AegisOdyssey/AbilitySystem/Abilities](Source/AegisOdyssey/AbilitySystem/Abilities)；如果你在找“伤害上下文、格挡/弹反、跳字、Cue、最终消息”这些统一语义，就回到根层的 `AOAbilityTypes`、`AOCombatResultMessage` 和属性/执行计算那条线。

- **库存、快捷栏与正式装备栏**  
  这部分已经不是单一背包组件，而是拆成了“统一入包、库存内使用、快捷栏装备、正式装备栏长期穿戴”几条互相关联但职责不同的链。它的核心设计是把“物品进入系统”“物品在库存里被消费”“物品成为当前装备状态”“物品成为长期穿戴状态”分开建模；设计思想是不再把背包、快捷栏、武器管理器、正式装备管理器混成一套万能库存，而是让每一层只持有自己那部分运行时真相；核心目的是让物品系统既能支持 RPG 式长期装备，又能支持动作游戏式快捷切换，还能保持 UI 和数据流清楚。源码入口在 [Source/AegisOdyssey/Inventory](Source/AegisOdyssey/Inventory)、[Source/AegisOdyssey/Equipment](Source/AegisOdyssey/Equipment)、[Source/AegisOdyssey/UI/ViewModel/Inventory](Source/AegisOdyssey/UI/ViewModel/Inventory)、[Source/AegisOdyssey/UI/Widgets/Inventory](Source/AegisOdyssey/UI/Widgets/Inventory) 和 [Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md](Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md)。如果你想搞清楚物品怎么进背包，先看 [Source/AegisOdyssey/Inventory/AOInventoryStatics.h](Source/AegisOdyssey/Inventory/AOInventoryStatics.h)、[Source/AegisOdyssey/Inventory/AOInventoryComponent.h](Source/AegisOdyssey/Inventory/AOInventoryComponent.h)、[Source/AegisOdyssey/Inventory/AOBackPackComponent.h](Source/AegisOdyssey/Inventory/AOBackPackComponent.h)；如果你想看快捷栏和武器链，去 [Source/AegisOdyssey/Equipment/AOQuickBarComponent.h](Source/AegisOdyssey/Equipment/AOQuickBarComponent.h)、[Source/AegisOdyssey/Equipment/AOWeaponManagerComponent.h](Source/AegisOdyssey/Equipment/AOWeaponManagerComponent.h)；如果你想看正式装备栏，重点看 [Source/AegisOdyssey/Equipment/Formal](Source/AegisOdyssey/Equipment/Formal)。

- **交互系统与容器会话**  
  当前交互系统已经不是“交互按钮直接弹 UI”这种直连结构，而是显式引入了交互会话、会话模型和对象侧真相层。它的核心设计是让交互对象自己决定会话如何建立、数据如何暴露、修改何时提交，而不是让 UI 越权驱动世界对象；设计思想是把“交互入口”“对象规则”“会话状态”“UI 观察”拆开，每层都只管自己的边界；核心目的是让容器、工作台、按钮、门、AI 互动这些对象都能复用同一套交互主链，而不是每一种对象都再发明一套临时流程。先看 [Source/AegisOdyssey/Interaction](Source/AegisOdyssey/Interaction) 和 [Docs/Knowledge/InteractionSystem/PROJECT_MAP.md](Docs/Knowledge/InteractionSystem/PROJECT_MAP.md)。比较有代表性的样板是 [Source/AegisOdyssey/Interaction/Abilities/AOGameplayAbility_Interact.h](Source/AegisOdyssey/Interaction/Abilities/AOGameplayAbility_Interact.h)、[Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.h](Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.h)、[Source/AegisOdyssey/Interaction/Session](Source/AegisOdyssey/Interaction/Session)、[Source/AegisOdyssey/Interaction/Containers/AOChest.h](Source/AegisOdyssey/Interaction/Containers/AOChest.h) 和 [Source/AegisOdyssey/Interaction/Containers/AOContainerInventoryComponent.h](Source/AegisOdyssey/Interaction/Containers/AOContainerInventoryComponent.h)。如果你后面要接按钮、箱子、工作台或别的交互对象，最好先把这一套会话语义看顺。

- **采集系统**  
  采集链现在已经比较完整，而且是这个项目里很有代表性的“动作状态 -> 命中窗 -> 服务端重判定 -> 正式入包 -> 节点生命周期”范式。它的核心设计是把采集视为正式动作链，而不是点一下资源节点就直接掉物品；设计思想是把动作发起、工具语义、命中判定、服务端重判定、奖励结算、节点 depleted/respawn 生命周期分层处理；核心目的是让采集系统可以像战斗系统那样继续扩树、岩石、灌木、工具种类和表现层，而不把所有逻辑堆在单个 Actor 里。先看 [Source/AegisOdyssey/Harvest](Source/AegisOdyssey/Harvest) 和 [Docs/Knowledge/HarvestSystem/PROJECT_MAP.md](Docs/Knowledge/HarvestSystem/PROJECT_MAP.md)。里面最关键的入口是 [Source/AegisOdyssey/Harvest/StateTree/STT_PlayHarvest.h](Source/AegisOdyssey/Harvest/StateTree/STT_PlayHarvest.h)、[Source/AegisOdyssey/Harvest/Abilities/GA_Harvest.h](Source/AegisOdyssey/Harvest/Abilities/GA_Harvest.h)、[Source/AegisOdyssey/Harvest/System/AOHarvestResolver.h](Source/AegisOdyssey/Harvest/System/AOHarvestResolver.h)、[Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.h](Source/AegisOdyssey/Harvest/Core/AOHarvestableComponent.h) 以及 [Source/AegisOdyssey/Harvest/Nodes](Source/AegisOdyssey/Harvest/Nodes)。现在树、灌木、岩石这些对象族都已经有对应的 C++ 节点类，不再只是文档里的概念。

- **制造系统**  
  制造系统当前重点不是复杂度，而是链路已经比较清楚：`PawnData -> CraftingComponent -> MVVM -> Widget`。它的核心设计是把配方来源、底层真相、观察数据和界面消费拆成四层，避免 UI 自己去拼配方判断；设计思想是让制造组件直接维护“当前能做什么、材料够不够、队列里有什么”这些运行时事实，再把观察结果交给 MVVM；核心目的是让制造后续不管扩配方、扩队列、扩观察来源，还是改 UI，都能沿着同一条主线推进。先看 [Source/AegisOdyssey/Crafting](Source/AegisOdyssey/Crafting)、[Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h](Source/AegisOdyssey/UI/ViewModel/MVVM_Crafting.h)、[Source/AegisOdyssey/UI/Widgets/Crafting](Source/AegisOdyssey/UI/Widgets/Crafting) 和 [Docs/Knowledge/CraftingSystem/PROJECT_MAP.md](Docs/Knowledge/CraftingSystem/PROJECT_MAP.md)。如果你要看配方定义，去 [Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h](Source/AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h)；如果你要看底层真相层，去 [Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h](Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.h)。

- **技能系统**  
  当前技能系统已经从“按键直连几个 Ability”演进成了 `SkillDefinition -> SkillSource -> SkillInstance -> SkillSlot -> SkillComponent -> ASC` 这套结构。它的核心设计是把技能的静态定义、运行时身份、装备槽关系和真正的 Ability 执行分开；设计思想是技能不是一个纯数据表条目，也不是一个单纯按键映射，而是一条从物品来源、实例化、装配、授予到执行的完整链；核心目的是让技能既能接库存来源、又能接快捷栏输入，还能被 AI、StateTree 和未来更多系统稳定消费。建议先看 [Source/AegisOdyssey/SkillSystem](Source/AegisOdyssey/SkillSystem) 和 [Docs/Knowledge/SkillSystem/PROJECT_MAP.md](Docs/Knowledge/SkillSystem/PROJECT_MAP.md)，然后回头对照 [Source/AegisOdyssey/Inventory/Fragments/AOFragment_SkillSource.h](Source/AegisOdyssey/Inventory/Fragments/AOFragment_SkillSource.h)、[Source/AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h](Source/AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h)、[Source/AegisOdyssey/SkillSystem/Core/AOSkillInstance.h](Source/AegisOdyssey/SkillSystem/Core/AOSkillInstance.h)、[Source/AegisOdyssey/SkillSystem/Components/AOSkillComponent.h](Source/AegisOdyssey/SkillSystem/Components/AOSkillComponent.h) 和 [Source/AegisOdyssey/SkillSystem/Abilities](Source/AegisOdyssey/SkillSystem/Abilities)。如果你想看现成技能案例，直接进 `Abilities`；如果你想看运行时总入口和槽位管理，就盯 `AOSkillComponent`。

- **StateTree AI 与敌人决策**  
  这个项目的 AI 不是把所有逻辑都堆进 StateTree 资产里，而是把运行时状态、决策语义和 StateTree 消费关系分层处理。它的核心设计是让 Controller 或专门组件持有目标、巡逻、决策等运行时真相，再让 StateTree 只负责消费这些真相并组织行为；设计思想是把“谁知道当前世界状态”和“谁负责进入某个行为状态”分开，避免 StateTree 既算事实又执行业务；核心目的是把 AI 从难以排查的黑盒资产重新拉回可定位、可复用、可增量调试的 C++ 主链。先看 [Source/AegisOdyssey/StateTree](Source/AegisOdyssey/StateTree)、[Source/AegisOdyssey/Character/Enemies](Source/AegisOdyssey/Character/Enemies)、[Source/AegisOdyssey/Player/AAOAIPlayerBotController.h](Source/AegisOdyssey/Player/AAOAIPlayerBotController.h)、[Docs/Knowledge/StateTreeAI/PROJECT_MAP.md](Docs/Knowledge/StateTreeAI/PROJECT_MAP.md) 和 [Docs/Knowledge/AI/PROJECT_MAP.md](Docs/Knowledge/AI/PROJECT_MAP.md)。如果你是在排查“树有没有启动、状态为什么没切、目标为什么没更新”，优先看 [Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h](Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.h)、[Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h](Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h) 和 `AAOAIPlayerBotController`；如果你是在排查“某个决策为什么没被消费”，再去看各类 Evaluator、Task 和 Condition。

- **UI、CommonUI 与 MVVM**  
  `UI` 是当前源码里文件最多、也最容易让人一眼看花的目录，但结构其实并不乱。它的核心设计是 `Layout -> ViewModel -> Widget` 这条观察链，加上世界血条、战斗反馈、库存菜单这些面向不同场景的 UI 分支；设计思想是让 UI 更多承担观察、组织和展示职责，而不直接持有玩法真相；核心目的是在后期不断改界面、加面板、换交互方式时，尽量不把玩法系统和界面逻辑重新搅成一团。建议先把 [Source/AegisOdyssey/UI/Layouts](Source/AegisOdyssey/UI/Layouts)、[Source/AegisOdyssey/UI/ViewModel](Source/AegisOdyssey/UI/ViewModel)、[Source/AegisOdyssey/UI/Common](Source/AegisOdyssey/UI/Common)、[Source/AegisOdyssey/UI/Widgets](Source/AegisOdyssey/UI/Widgets) 和 [Source/AegisOdyssey/UI/WorldHealthBar](Source/AegisOdyssey/UI/WorldHealthBar) 这几层关系看清楚，再回头看 [Content/UI](Content/UI) 里的资产。项目里很多玩法 UI 已经走 CommonUI + MVVM 这一套，所以如果你遇到“为什么不是 Widget 直接读组件”的写法，不是绕，而是项目有意把观察层和真相层分开了。

- **动画、相机、物品与测试支撑层**  
  这几块不一定会单独被拿出来讲，但主系统几乎都会经过它们。它们的核心设计更像公共桥接层：动画负责把玩法时序切成可控窗口，相机负责把角色状态翻译成第三人称观察体验，物品 Actor 负责把数据世界和场景世界接起来，测试目录负责保留可复现的验证样板；设计思想是把这些容易散落在各处的横切能力集中管理；核心目的是让主系统不用各自复制一份通知窗、相机逻辑、世界物品壳子或者回归脚本。[Source/AegisOdyssey/Animation](Source/AegisOdyssey/Animation) 里放的是动画实例和各种 Notify/NotifyState，像 [Source/AegisOdyssey/Animation/NotifyState/AOCombatWindow.h](Source/AegisOdyssey/Animation/NotifyState/AOCombatWindow.h)、[Source/AegisOdyssey/Animation/NotifyState/AOHarvestWindow.h](Source/AegisOdyssey/Animation/NotifyState/AOHarvestWindow.h)、[Source/AegisOdyssey/Animation/NotifyState/AOInputBufferWindow.h](Source/AegisOdyssey/Animation/NotifyState/AOInputBufferWindow.h) 这些窗口类，实际上就是战斗、采集、输入缓冲这些玩法进入正式结算前的关键桥。 [Source/AegisOdyssey/Camera](Source/AegisOdyssey/Camera) 则承接第三人称相机组件、相机模式和相机管理器，先看 [Source/AegisOdyssey/Camera/AOCameraComponent.h](Source/AegisOdyssey/Camera/AOCameraComponent.h)、[Source/AegisOdyssey/Camera/AOCameraMode_ThirdPerson.h](Source/AegisOdyssey/Camera/AOCameraMode_ThirdPerson.h)、[Source/AegisOdyssey/Camera/AOPlayerCameraManager.h](Source/AegisOdyssey/Camera/AOPlayerCameraManager.h) 就能抓住主线。 [Source/AegisOdyssey/Items](Source/AegisOdyssey/Items) 更像世界物品 Actor 和物品目录辅助层，里面的 [Source/AegisOdyssey/Items/AOItem.h](Source/AegisOdyssey/Items/AOItem.h)、[Source/AegisOdyssey/Items/AOWeapon.h](Source/AegisOdyssey/Items/AOWeapon.h)、[Source/AegisOdyssey/Items/AOHarvestTool.h](Source/AegisOdyssey/Items/AOHarvestTool.h) 负责把“物品作为世界对象”这件事接起来。最后还有 [Source/AegisOdyssey/TestProject](Source/AegisOdyssey/TestProject)，这里集中放了一些回归和样板测试入口，比如制造、采集、交互和 AI 决策相关测试，排查问题时很适合拿来当现成参照。

- **配置、资源与知识文档**  
  如果你更关心“这个仓库除了源码还带了什么”，那另外三块也值得记住。它们的核心设计不是简单的资料堆放，而是把项目级配置、内容资产和长期知识沉淀分成三层：`Config` 负责规则入口，`Content` 负责运行时资源，`Docs/Knowledge` 负责解释这些系统为什么会长成现在这样；设计思想是让“代码如何跑”“资源如何挂”“团队如何理解系统”分别有稳定落点；核心目的是降低接手成本，避免后面每次看不懂都只能去翻历史提交。 [Config](Config) 里是地图、输入、GameplayTag、AssetManager 和类重定向这些项目级配置；[Content](Content) 里除了项目自己的地图、蓝图和 UI，也混有不少第三方资源包、模板资源和演示资产，第一次看时建议优先关注 [Content/Levels](Content/Levels)、[Content/Games](Content/Games)、[Content/UI](Content/UI) 和 [Content/Skill](Content/Skill)；[Docs/Knowledge](Docs/Knowledge) 则是这个项目最有价值的长期导航层，很多系统都已经整理出了 `PROJECT_MAP / DECISIONS / KNOWN_ISSUES` 三件套，想接手某个系统时先读这里，通常比直接在源码里盲搜更快。

## 最后怎么用这份 README

如果你是第一次来看这个项目，比较推荐的读法不是“从上往下全看完”，而是先用上面的模块导航找到你关心的那条主线，再沿着对应的 `PROJECT_MAP -> 源码入口 -> Content/蓝图资产` 继续往下走。这个仓库的体量已经超过“靠记忆扫一遍就能全抓住”的阶段了，所以 README 的目标也不是代替源码，而是先把入口、边界和阅读顺序交代清楚，让你能更快找到真正该看的地方。只要你是从仓库根目录开始阅读，这份 README 里的所有路径都可以直接对应到仓库里的实际位置。
## 全局控制台指令

当前项目里已经接入的全局控制台指令主要有下面 3 条。它们都属于调试/排查入口，不是主玩法流程的一部分。

- `AegisOdyssey.DumpLoadedAssets`
  无参数。打印 `UAOAssetManager` 当前 `LoadedAssets` 池里已经加载且仍在内存中的资源，适合排查资源是否真的被加载进来。源码位置：[AOAssetManager.cpp](Source/AegisOdyssey/System/AOAssetManager.cpp)
- `AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>`
  全局开关当前世界里 `UAOAILogicStateTreeComponentBase` 挂载的 AI 决策 StateTree。`true` 会对已有组件执行 `RestartLogic`，`false` 会执行 `StopLogic`。参数兼容 `true/false`、`1/0`、`on/off`。源码位置：[AOGlobalConsoleCommands.cpp](Source/AegisOdyssey/System/AOGlobalConsoleCommands.cpp)
- `AegisOdyssey.AI.SetDebugPanelEnabled <true|false>`
  全局开关当前世界的 AI 调试 Slate 面板。打开后，HUD 调试观察链会在 `Saved/AIDebug` 目录下为这一轮观察新建一份按时间命名的 `txt` 日志文件，并在观察期间持续追加每次快照，便于回看 AI 决策变化。参数同样兼容 `true/false`、`1/0`、`on/off`。源码位置：[AOGlobalConsoleCommands.cpp](Source/AegisOdyssey/System/AOGlobalConsoleCommands.cpp)
