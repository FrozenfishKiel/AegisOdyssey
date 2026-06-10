# Aegis Odyssey 第一版加载资源集合草案

## 1. 文档目的

这份文档用于定义：

1. 第一版加载系统应该统计哪些资源/任务
2. 这些资源/任务应该属于哪个加载阶段
3. 哪些内容适合挡住玩家等待
4. 哪些内容不应该纳入第一版加载集合

这份文档是 [AsyncLoadingDesign.md](./AsyncLoadingDesign.md) 的配套文档。

---

## 2. 先说结论

第一版不要追求“把所有资源都纳入加载集合”，而应该先抓住真正影响玩家进入状态的关键集合。

推荐第一版只纳入这 4 类：

1. 启动全局数据集合
2. 进入世界的核心玩法集合
3. Experience / GameFeature 集合
4. 读取存档恢复任务集合

第一版范围策略正式定为：

> 优先覆盖“主角 + 当前场景战斗直接相关资源”，不做“整张地图所有可能资源”的全集式纳入。

不建议第一版就纳入：

1. 全部武器贴图
2. 全部动画资源
3. 所有 UI 子页面
4. 所有场景内可交互对象资源

原因很简单：

第一版的目标不是“全部提前加载”，而是：

> 只对真正决定“玩家能不能安全进入当前状态”的内容做可追踪加载。

---

## 3. 划分原则

第一版资源集合建议按下面这几个判断标准来定。

### 3.1 应该纳入加载集合的内容

满足以下任意一条，就适合优先纳入：

1. 没有它，玩家无法安全进入当前状态
2. 没有它，玩家进入后会立刻看到半初始化结果
3. 没有它，核心玩法流程不能开始
4. 没有它，存档恢复后的关键状态不完整
5. 它本身已经是 DataAsset / PrimaryAsset / GameFeature 这类天然可管理对象

### 3.2 不应该优先纳入第一版的内容

满足以下特点的内容，不建议第一版纳入：

1. 只是增强体验，不影响当前可玩性
2. 可以在玩家已进入世界后平滑流式补进
3. 资源体量大但与当前状态无关
4. 很难稳定统计进度，先纳入只会增加噪声

### 3.3 升级为“必须纳入加载集合”的判定标准

某类资源或任务，不是因为“它存在”就应该纳入第一版加载集合，而是因为它满足了“当前状态必须等待它”的条件。

建议用下面 5 条作为统一判断标准。

#### 标准一：没有它，玩家不能安全进入当前状态

这是最硬的判断标准。

如果缺少这个资源或任务，玩家进入后会出现：

1. 无法生成
2. 核心输入缺失
3. 核心玩法无法启动
4. 世界状态不完整

那么它就应该优先纳入加载集合。

#### 标准二：没有它，玩家一进入就会立刻感知到明显错误

这里强调的是“强感知”。

如果玩家进入后在很短时间内就会立刻发现：

1. HUD 不对
2. 核心能力不可用
3. 当前武器状态错误
4. 读档后角色状态明显不一致

那么这类资源或任务也应优先纳入。

#### 标准三：它是当前状态的直接依赖，而不是远期依赖

这个标准用于区分：

1. 当前立刻要用到的内容
2. 未来可能会用到的内容

应该优先纳入的是“当前状态直接依赖”。

不应该因为“以后也许会用到”，就一开始把整包资源都纳入加载集合。

#### 标准四：它有清晰归属，能被声明进某个集合里

例如它能够明确挂在：

1. `AOGameData`
2. `PawnData`
3. `ExperienceDefinition`
4. 读档恢复任务列表

这种资源或任务特别适合纳入第一版，因为：

1. 来源明确
2. 边界清晰
3. 后续可维护
4. 更容易做调试和排查

#### 标准五：它的完成状态能被系统判断

这条非常重要。

如果一个资源或任务被纳入加载集合，但系统并不知道它何时才算“完成”，那么它就不适合作为第一批核心加载项。

“能判断完成”可以包括：

1. 软引用加载完成
2. PrimaryAsset 句柄完成
3. GameFeature 激活完成
4. 某个恢复任务明确返回完成

### 3.4 最终判断口径

建议后续统一用下面这句话作为纳入标准：

> 只有当一个资源或任务同时具备“当前必须”“玩家可感知”“归属清晰”“完成可判定”这些特点时，它才值得升级为必须纳入加载集合。

这条规则的意义是：

第一版先建立可靠筛选标准，而不是先把表做得很大。

---

## 4. 基于当前项目的实际候选

结合当前工程结构，已经能看出一些天然适合做第一版加载集合的资源入口。

### 4.1 全局 GameData

当前已有：

1. `UAOGameData`
2. `Config/DefaultGame.ini` 中配置的 `DA_AOGameData`

对应入口：

1. `Source/AegisOdyssey/System/AOGameData.h`
2. `Source/AegisOdyssey/System/AOAssetManager.h`
3. `Source/AegisOdyssey/System/AOAssetManager.cpp`
4. `Content/Games/GameData/DA_AOGameData.uasset`

当前 `UAOGameData` 已经包含：

1. 全局 GameplayEffect 配置
2. `ItemCatalogDataTable`

这类资源非常适合作为第一版启动加载集合的一部分。

原因：

1. 它是全局数据入口
2. 它本来就通过 `AOAssetManager` 走启动加载
3. 它天然适合拿来作为启动阶段的真实进度来源

### 4.2 默认 PawnData

当前已有：

1. `UAOPawnData`
2. `Config/DefaultGame.ini` 中配置的 `DA_PawnData`

对应入口：

1. `Source/AegisOdyssey/Character/AOPawnData.h`
2. `Source/AegisOdyssey/System/AOAssetManager.h`
3. `Source/AegisOdyssey/System/AOAssetManager.cpp`
4. `Content/Games/PawnData/DA_PawnData.uasset`

`UAOPawnData` 当前已经关联：

1. `PawnClass`
2. `AbilitySets`
3. `DefaultAbility`
4. `InputConfig`
5. `DefaultCameraMode`
6. 属性曲线表
7. 升级曲线表

这类资源非常适合作为“进入世界核心玩法集合”的一部分。

原因：

1. 玩家生成前就需要它
2. 它是角色玩法初始化的总入口之一
3. 它关联的能力、输入、相机、成长数据都是“角色能否正常进入游戏”的核心依赖

### 4.3 ExperienceDefinition

当前已有：

1. `UAOExperienceDefinition`
2. `AOGameMode` 中设置当前 Experience 的流程
3. `AOExperienceManagerComponent` 中异步处理流程

对应入口：

1. `Source/AegisOdyssey/GameModes/AOExperienceDefinition.h`
2. `Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.h`
3. `Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.cpp`
4. `Content/Games/Mode/BP_AOGameFeatureDefinition.uasset`
5. `Content/Games/Mode/DA_GameFeatureDefinition.uasset`

当前 `UAOExperienceDefinition` 明确包含：

1. `Actions`
2. `GameFeatureNames`
3. `DefaultPawnData`

这类资源是第一版加载集合里最核心的一段。

原因：

1. 它直接决定进入某个世界后需要准备哪些玩法内容
2. 它已经天然处在 `GameMode -> GameState -> ExperienceManager` 这条启动链上
3. 它最适合成为“地图后玩法加载”的主驱动对象

### 4.4 GameFeature 集合

当前已有：

1. `GameFeatureNames`
2. `LoadAndActivateGameFeaturePlugin()`

对应入口：

1. `Source/AegisOdyssey/GameModes/AOExperienceDefinition.h`
2. `Source/AegisOdyssey/GameModes/AOExperienceManagerComponent.cpp`

这一段是当前项目里最明确的真正异步加载行为之一。

它非常适合纳入第一版进度统计。

原因：

1. 它本身已经是明确的待完成任务集合
2. 它已经有数量概念，可以天然转成阶段进度
3. 它和玩法是否 ready 的关系非常直接

### 4.5 存档恢复任务集合

当前代码里我还没有看到一套正式的存档恢复链，但从需求上看，这一项必须预留。

原因：

你已经明确提出：

1. 主界面读取存档时应该加载
2. 但这不是写死入口，而是属于“状态尚未恢复完整，不应让玩家进入”的一种情况

所以第一版方案里，必须预留“读档恢复任务集合”的位置。

即使现在还没有完整的 SaveGame 系统，也应该把这一类任务设计成可挂接。

---

## 5. 第一版推荐纳入的加载集合

下面是推荐的 V1 清单。

### 5.1 启动阶段加载集合

推荐纳入：

1. `DA_AOGameData`
2. `ItemCatalogDataTable`
3. 启动阶段必须存在的全局 GameplayEffect 类引用
4. 如果主界面初始化依赖某些核心输入配置，也可纳入最小输入集合

当前项目里最明确的入口：

1. `UAOAssetManager::StartInitialLoading()`
2. `UAOAssetManager::GetGameData()`

第一版建议：

启动阶段不要塞太多内容，先把“全局数据总表”这一路打通。

### 5.2 进入世界核心玩法集合

推荐纳入：

1. 当前世界对应的 `ExperienceDefinition`
2. `DefaultPawnData`
3. `DefaultPawnData` 关联的 `AbilitySets`
4. `DefaultPawnData` 关联的 `InputConfig`
5. `DefaultPawnData` 关联的成长/属性曲线表
6. 当前进入该世界必须依赖的相机模式类
7. 当前场景战斗直接相关的敌人配置、技能配置、战斗必需 UI 与必需演出资源

原因：

这批内容直接决定：

1. 玩家能否正常生成
2. 玩家是否具备基础输入
3. 玩家是否具备核心能力
4. 基础属性和成长数据是否 ready
5. 当前场景一进入就会参与的战斗内容是否 ready

### 5.3 Experience / GameFeature 集合

推荐纳入：

1. `ExperienceDefinition.GameFeatureNames`
2. `ExperienceDefinition.Actions`
3. 由 Experience 明确声明的关键软资源列表

这里特别重要的一点是：

第一版应该把 `ExperienceDefinition` 扩展成“声明当前关卡/玩法必需资源集合”的核心承载体。

也就是说，未来不应该只靠它放：

1. `GameFeatureNames`
2. `Actions`

还应该允许它描述：

1. 当前场景必须提前准备的 PawnData
2. 当前场景必须提前准备的能力集合
3. 当前场景必须提前准备的 UI / Gameplay 配置资源

### 5.4 读取存档恢复任务集合

推荐纳入的不是某个固定资源，而是一组恢复任务：

1. 当前要进入哪个世界
2. 玩家当前使用哪个 Pawn / Profile / Loadout
3. 关键物品实例是否恢复完成
4. 关键能力状态是否恢复完成
5. 关键数值状态是否恢复完成
6. 必须先恢复的 UI / 交互状态是否完成

这一段更适合设计成“任务集合”，而不只是“资源集合”。

因为读档本身往往不只是加载资产，还包含：

1. 反序列化
2. 状态重建
3. 实例恢复
4. 世界对象绑定

---

## 6. 第一版不建议纳入的内容

为了避免系统一开始就失控，以下内容不建议在 V1 大规模纳入。

### 6.1 大量泛用动画资源

例如：

1. 全部攻击动画
2. 全部翻滚动画
3. 全部敌人战斗动画

原因：

1. 体量大
2. 很多并不是当前立刻必须
3. 更适合跟随角色、武器或技能按需流式加载

### 6.2 非当前状态必需的 UI 子页面

例如：

1. 背包完整界面
2. 宝箱子页面
3. 复杂角色展示页

原因：

玩家进入战斗世界时，不一定要先等这些 UI 准备完。

### 6.3 非当前使用武器或技能的完整资源包

例如：

1. 所有武器的贴图和 Mesh
2. 所有技能图标和演出资源
3. 所有敌人配置

原因：

第一版应该优先纳入“当前会立刻用到”的集合，而不是“未来可能会用到”的全集。

### 6.4 场景内普通交互物资源全集

例如：

1. 所有箱子 UI
2. 所有掉落物定义
3. 所有可交互静态资源

这些更适合后续按系统逐步细化。

### 6.5 当前地图全部远端内容的全集式纳入

第一版不建议把“当前地图理论上会用到，但玩家进入后短时间不会接触”的全部内容都算进加载集合。

例如：

1. 远处区域的敌人资源
2. 当前阶段不会立刻参与战斗的交互物
3. 当前阶段不会立刻用到的次级 UI

原因：

1. 这会显著拉长加载时间
2. 会让第一版边界变得模糊
3. 不符合当前已经确定的 `A` 方案范围

---

## 7. 第一版推荐的最小可落地结构

为了让这套集合能快速落地，第一版建议只做下面这几层。

### 7.1 启动集合

内容：

1. `AOGameData`
2. `ItemCatalogDataTable`
3. 必需全局 GE 类

来源：

1. `AOAssetManager`

### 7.2 世界进入集合

内容：

1. 当前 `ExperienceDefinition`
2. 当前 `DefaultPawnData`
3. PawnData 关联的核心 AbilitySet / InputConfig / CurveTable
4. 当前场景战斗直接相关的敌人 / 技能 / 战斗 UI / 必需演出资源

来源：

1. `AOGameMode`
2. `AOExperienceManagerComponent`
3. `AOAssetManager`

### 7.3 Experience 集合

内容：

1. `GameFeatureNames`
2. `Actions`
3. 后续补进来的软资源列表

来源：

1. `AOExperienceDefinition`
2. `AOExperienceManagerComponent`

### 7.4 读档恢复集合

内容：

1. 当前读档恢复任务列表
2. 每个任务的完成状态

来源：

1. 后续 SaveGame / Profile / Inventory 恢复流程

---

## 8. 为什么这样划分最适合当前项目

因为当前项目已经天然具备以下结构：

1. `AOAssetManager` 适合承接启动加载
2. `AOExperienceManagerComponent` 适合承接地图后玩法加载
3. `AOGameMode / AOGameState` 已经把“进入可玩状态前必须等 Experience 完成”的逻辑立住了

也就是说，第一版其实不需要发明全新体系，只需要把已有入口整理成可统计的任务集合。

---

## 9. 第一版资源集合的扩展方向

当第一版打通后，可以逐步扩展：

1. 当前地图专属 UI 集合
2. 当前地图专属敌人配置集合
3. 当前存档专属装备与技能集合
4. 当前玩家位置附近的大型流式资源集合

扩展原则仍然不变：

> 优先纳入“影响当前安全进入状态”的内容，再考虑体验增强型内容。

---

## 10. 当前建议的下一步

建议下一步继续补一份更具体的设计稿：

1. “第一版资源集合到代码结构的映射说明”

这份后续文档建议明确回答：

1. `AOGameData` 里的哪些字段要真正转成启动加载项
2. `PawnData` 里的哪些字段要转成世界进入加载项
3. `ExperienceDefinition` 应该新增哪些字段来声明待加载集合
4. `读档恢复任务集合` 应该如何抽象成统一接口

---

## 11. 当前阶段的接入边界

当前项目仍在持续推进：

1. 技能开发
2. 采集开发

因此，第一版资源集合的接入策略必须服从以下约束：

> 不能因为加载系统整理资源集合，而阻塞技能和采集功能继续推进。

### 11.1 当前建议的接入方式

当前阶段只建议：

1. 先定义资源集合结构
2. 先确定哪些类别将来应该纳入
3. 先接入已经天然存在的入口，如 `AOGameData`、`PawnData`、`ExperienceDefinition`、`GameFeature`
4. 为技能与采集相关资源预留挂接位置，但不要求当前一次性整理完成

### 11.2 当前不建议的接入方式

当前阶段不建议：

1. 为了加载系统去重构技能资源组织方式
2. 为了加载系统去重构采集资源组织方式
3. 强制要求当前所有技能资源都补齐到加载集合
4. 强制要求当前所有采集资源都补齐到加载集合
5. 把“资源集合整理完整”作为当前技能/采集开发的前置条件

### 11.3 当前推荐结论

第一版资源集合在当前阶段应以“可扩展、可挂接、低侵入”为原则推进，而不是追求一次性整理完整。
