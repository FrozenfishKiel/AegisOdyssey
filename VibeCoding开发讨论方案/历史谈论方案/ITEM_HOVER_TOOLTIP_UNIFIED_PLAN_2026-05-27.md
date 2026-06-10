# 物品悬浮信息框统一方案
更新时间：2026-05-27

这份文档只讨论“鼠标移动到物品上时自动显示信息框，移开后自动消失”这件事，不讨论右键菜单动作，不讨论 Tooltip 具体美术和动画，也不讨论与当前需求无关的 UI 样式问题。

它的目标不是给库存单独补一个悬浮框，也不是给制造系统单独补一个悬浮框，而是把这条能力收成一套真正可复用的通用链路。

---

## 第一章 这次真正要解决的核心问题

这次需求表面上是一个 Tooltip，实际上要解决的是三件事：

1. 同一个 Tooltip 要同时支持库存格子、制造材料格子、制造产出格子、制造配方格子，以及后续任何能稳定拿到物品定义的条目。
2. Tooltip 不能像当前右键菜单那样把 `SourceInventory` 设计成硬前提，否则制造材料这类没有库存来源链的条目又会被迫单独写一套特殊逻辑。
3. Tooltip 必须由一个全局唯一宿主管理生命周期，确保任意时刻只显示一个，并且新悬浮能立即覆盖旧悬浮，旧悬浮离开时不会把新的误关掉。

所以这次要收的不是“库存 Tooltip”或者“制造 Tooltip”，而是：

**一个基于 `UAOInventoryItemDefinition*` 的通用悬浮信息框系统。**

---

## 第二章 这次锁定的总原则

### 2.1 Tooltip 的接入规则不按系统分，而按数据能力分

这次不再按“库存系统 / 制造系统 / 技能系统”去决定谁能用 Tooltip。

统一规则只有一句话：

**凡是某个 UI 条目在当前代码路径里能够稳定解析出 `UAOInventoryItemDefinition*`，就允许接入这套 Tooltip；凡是拿不到 `Definition` 的，不接。**

这条规则意味着：

- 背包格子可以接
- 快捷栏格子可以接
- 正式装备槽可以接
- 技能槽里如果能解析到来源物品 `Definition`，也可以接
- 制造配方条目可以接
- 制造材料条目可以接
- 制造产出条目可以接
- 后续新增 UI 条目，只要能给出 `Definition`，也默认可以复用

它不再依赖“这个格子是不是 `UAOInventoryUI` 子类”，也不依赖“这个格子有没有 `SourceInventory`”。

### 2.2 Tooltip 的正式输入只认 Definition，不认 SourceInventory

这次 Tooltip 的正式输入口径锁定为：

- `UAOInventoryItemDefinition* ItemDefinition`
- `FVector2D ScreenSpacePosition`
- 一个用于生命周期管理的本地悬浮来源标识

也就是说，Tooltip 只关心“当前要显示哪一个物品定义”，不关心这个定义来自：

- 背包实例
- 快捷栏投影
- 正式装备槽
- 制造材料观察数据
- 制造产出观察数据

这条原则是为了避免再次把 Tooltip 做成“库存系统专属能力”。

### 2.3 Tooltip 只显示本地已知真相，不参与网络同步

Tooltip 是纯本地 UI 状态。

这次不做：

- 悬浮目标的 RPC
- 当前悬浮物品的复制
- 服务端权威的 Tooltip 状态同步

网络层真正要保证正确的是：

- 本地背包实例能正确拿到 `ItemInstance -> ItemDefinition`
- 本地制造观察数据能正确拿到 `MaterialEntry / OutputEntry -> ItemDefinition`

Tooltip 只消费客户端已经观察到的结果，不再额外发明第二套同步语义。

---

## 第三章 Tooltip 该写在哪里

### 3.1 不写在 InventoryUI 主链里

当前右键菜单之所以能走 `UAOInventoryUI -> SourceInventory -> ContextMenuViewModel`，是因为它本身就是按“库存来源上下文”设计的。

Tooltip 不适合沿用这条链，原因很直接：

- 制造材料条目不是 `UAOInventoryUI` 子类
- 制造材料条目也没有 `SourceInventory`
- 如果 Tooltip 继续绑 `SourceInventory`，制造系统又会被迫单独补特例

所以这次明确不把 Tooltip 的宿主写死在 `UAOInventoryUI` 上。

### 3.2 也不写在 CraftingWidgetBase 或某个单独槽位类里

如果把 Tooltip 写在：

- `UAOCraftingWidgetBase`
- `UAOCraftingRecipeListEntryWidget`
- `UAOBackPackSlot`
- `UAOQuickBarSlot`

都会立刻变成某个局部系统专属能力。

这和本次“按 Definition 通用接入”的目标冲突。

### 3.3 Tooltip 应挂在 HUD 全局唯一宿主下

当前锁定口径是：

- Tooltip ViewModel 挂在 HUD 主链
- Tooltip Widget 由 HUD 侧统一持有或统一打开
- 各个格子/条目只发“显示 / 隐藏 Tooltip”的请求

这样做的好处是：

- 天然保证全局唯一
- 不会出现背包一份、制造一份、技能栏一份各自互不知情
- 生命周期和清理逻辑可以统一收口
- 后续新增接入点时，不需要再新造第二个 Tooltip 宿主

---

## 第四章 Tooltip 的数据模型怎么定义

### 4.1 Definition 需要新增 Description

当前 `UAOInventoryItemDefinition` 已经稳定承载：

- `DisplayName`
- `SemanticTags`
- `Fragments`

这次新增 Tooltip 信息后，正式补充：

- `FText Description`

这个 `Description` 直接定义在 `UAOInventoryItemDefinition` 上，不额外拆成“短描述 / 长描述”两套字段。

### 4.2 长文本处理走最简单口径

这次对长描述的处理明确锁定为：

- `Description` 在数据层永远保存完整原文
- C++ 不做截断
- C++ 不做人为换行
- C++ 不做摘要
- C++ 不做“超过多少字就裁剪”
- Tooltip ViewModel 只原样透传完整 `FText`
- 实际显示时的自动换行、最大宽度、最大行数、裁剪、省略号，全部交给 UI 表现层

这条规则的目的，是让数据层始终只负责“提供完整文本真相”，不把显示策略写死在逻辑层里。

### 4.3 Tooltip ViewModel 只持有显示快照

Tooltip ViewModel 应只持有当前显示需要的快照，例如：

- 当前是否可见
- 当前显示名称
- 当前显示描述
- 当前图标 Brush
- 当前是否有有效图标
- 当前屏幕弹出位置
- 当前活动悬浮来源标识

这里持有的是“当前悬浮显示快照”，不是库存或制造系统里的长期业务真相。

### 4.4 Tooltip 图标仍然从 Definition Fragment 取

图标继续沿用当前现有规则：

- 从 `UAOInventoryItemDefinition` 上查 `UAOFragment_InventoryIcon`
- 查到就显示
- 查不到就只显示文字

不单独再为 Tooltip 发明第三套取图标规则。

---

## 第五章 Tooltip 的生命周期怎么管理

### 5.1 任意时刻只能有一个活动 Tooltip

这次锁死为：

- HUD 下只有一个正式活动 Tooltip
- 新悬浮请求到来时，直接覆盖旧 Tooltip 的显示快照
- 不允许同时保留多个 Tooltip

### 5.2 显示时机

当前用户已拍板的交互口径是：

- 鼠标进入条目时，在当前位置附近弹出 Tooltip
- Tooltip 显示后不跟随鼠标持续移动

这意味着 Tooltip 不需要做鼠标实时追踪，只需要在 `MouseEnter` 当帧拿一次屏幕位置并固定下来即可。

### 5.3 隐藏时机

Tooltip 应在下面几种情况下关闭：

1. 鼠标离开当前活动条目
2. 当前活动条目在刷新或重建中被销毁
3. 当前条目解析不到有效 `Definition`
4. 新条目进入并直接覆盖旧条目

### 5.4 必须有“当前活动来源”校验

这次必须防止下面这种错误：

1. 鼠标先进入 A
2. 再快速进入 B
3. A 之后才触发 `MouseLeave`
4. A 的离开把本应继续显示的 B 给关掉

因此 Tooltip 的隐藏逻辑不能做成“谁来都能关”，而必须做成：

- 只有当前活动来源自己离开时，才允许关闭当前 Tooltip
- 如果已经被新的来源覆盖，旧来源的 Hide 请求应直接忽略

也就是说，Tooltip 宿主必须维护一个“当前活动来源标识”。

### 5.5 Tooltip 自己不能抢鼠标

Tooltip Widget 本身必须避免吃掉鼠标命中，否则会出现：

1. 鼠标进入物品格子
2. Tooltip 弹出后挡住格子
3. 格子立刻收到 `MouseLeave`
4. Tooltip 又被关闭
5. 鼠标重新回到格子
6. Tooltip 再次打开

最终表现就是闪烁抖动。

所以 Tooltip Widget 在显示层必须是“不拦鼠标命中”的。

---

## 第六章 各类条目怎么接入

### 6.1 库存类格子

库存类格子的 Tooltip 数据来源统一为：

`ItemInstance -> GetItemCDO() -> UAOInventoryItemDefinition`

也就是说，背包、快捷栏、正式装备槽、技能槽这类本质上持有物品实例或来源物品实例的条目，只需要在悬浮时把当前解析出的 `Definition` 交给 Tooltip 链路即可。

### 6.2 制造类条目

制造类条目的 Tooltip 数据来源统一为当前观察快照里已经准备好的 `Definition`：

- 配方条目：主产物 `PrimaryOutputDefinition`
- 材料条目：`MaterialData.ItemDefinition`
- 产出条目：`OutputData.ItemDefinition`

这样 Tooltip 接入继续遵守当前制造系统已经收好的边界：

- Widget 只消费观察快照
- 不再在 Tooltip 逻辑里反查底层制造组件
- 不再为 Tooltip 额外发明一条“从 RecipeRowName 再重新查表”的链路

### 6.3 Definition 为空时的统一口径

如果当前条目能被显示，但此时拿不到有效 `UAOInventoryItemDefinition*`，统一按下面规则处理：

- 不显示 Tooltip
- 如果当前正有旧 Tooltip，直接关闭旧 Tooltip

不为“Definition 缺失”额外发明一套纯字符串兜底 Tooltip 规则。

---

## 第七章 当前方案已经明确解决的问题

### 7.1 不再依赖 SourceInventory

这次 Tooltip 的正式输入不再把 `SourceInventory` 当成硬条件，因此制造材料条目和库存格子可以共用一套链路。

### 7.2 不再按业务系统拆多套 Tooltip

这次 Tooltip 的接入规则只认 `Definition`，不认“这个条目属于哪个系统”。

### 7.3 生命周期统一归 HUD 管

这次通过 HUD 全局唯一宿主管理 Tooltip，避免多处各自持有实例导致的重复弹出、遗留不清、关闭串台。

### 7.4 长文本不会被逻辑层写死

这次 `Description` 永远保留完整 `FText` 原文，逻辑层不做截断，避免后续 UI 需求一变又得返工改数据层。

### 7.5 多人联机边界清楚

Tooltip 是纯本地显示行为，不需要额外同步；真正需要可信的是客户端已经观察到的物品实例或制造观察数据。

---

## 第八章 这次明确不做的事

本次方案明确不包含：

- Tooltip 的美术布局细节
- Tooltip 的动画
- Tooltip 的渐隐渐现策略
- Tooltip 的高级富文本排版
- Tooltip 的滚动描述框行为
- 基于不同物品类型展示完全不同布局
- 继续复用右键菜单的 `SourceInventory` 语义

这些都不属于当前方案的必要前提。

---

## 第九章 这轮方案的最终结论

这次不是在库存里补一个悬浮框，也不是在制造里补一个悬浮框。

这次真正要落地的是：

**一套以 `UAOInventoryItemDefinition*` 为唯一正式内容来源、由 HUD 全局唯一宿主管理、可被任意可解析 `Definition` 的 UI 条目复用的通用 Tooltip 链路。**

最终口径锁定为：

- Tooltip 按 Definition 接入，不按系统接入
- Tooltip 不依赖 `SourceInventory`
- Tooltip 只显示本地已知真相，不做网络同步
- Tooltip 一次只显示一个
- 鼠标进入时在当前位置附近弹出
- Tooltip 显示后不跟随鼠标
- 鼠标离开或来源失效时关闭
- `UAOInventoryItemDefinition` 新增 `FText Description`
- `Description` 保存完整长文本，C++ 不做截断，显示策略交给 UI

如果后续按这份方案落代码，那么库存、制造、装备、技能等所有能稳定解析到 `Definition` 的条目，都应能够共用同一套 Tooltip 能力，而不再继续复制“各系统各一套悬浮框”的老路。

---

## 第十章 程序员阅读导航

这一章只给接手程序员做 review 导航，口径以当前代码事实为准，不把设计预期写成“已经有某个蓝图资源”。

建议按下面顺序读：

1. 先看 Tooltip 的全局宿主和回退链路，确认当前实现是 HUD 统一持有 ViewModel 与 Widget。
2. 再看 Tooltip ViewModel 的显示/隐藏规则，确认它只认 `Definition + ScreenSpacePosition + SourceToken`，并且只处理本地 UI 状态。
3. 再看库存类与制造类接入点，确认各条目只是把自己当前能解析到的 `Definition` 提交给 HUD 链路，而不是各自再造一份 Tooltip。
4. 最后看正式装备槽和技能槽，确认它们虽然各自有专用 Widget，但 Tooltip 入口仍然复用库存基类口径。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp`
  - `UAOMainUI::EnsureItemHoverTooltipWidget`
  - 当前代码支持 `ItemHoverTooltipWidgetClass` 可选配置；未配置时回退到 `UAOItemHoverTooltipWidget::StaticClass()`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::ShowTooltip`
  - `UMVVM_ItemHoverTooltip::HideTooltip`
  - `UMVVM_ItemHoverTooltip::ForceHideTooltip`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.cpp`
  - `UAOItemHoverTooltipWidget::UAOItemHoverTooltipWidget`
  - `UAOItemHoverTooltipWidget::RefreshDisplay`
  - `UAOItemHoverTooltipWidget::BuildDefaultWidgetTreeIfNeeded`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
  - `UAOInventoryUI::ShowHoverTooltip`
  - `UAOInventoryUI::HideHoverTooltip`
  - `UAOInventoryUI::ResolveHoverTooltipItemDefinition`
- `Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp`
  - `UAOCraftingRecipeListEntryWidget::ResolveHoverTooltipItemDefinition`
  - `UAOCraftingRecipeListEntryWidget::ShowHoverTooltip`
  - `UAOCraftingRecipeListEntryWidget::HideHoverTooltip`
- `Source/AegisOdyssey/UI/Widgets/Skill/AOSkillSlotUI.cpp`
  - `UAOSkillSlotUI::ResolveHoverTooltipItemDefinition`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.cpp`
  - `UAOFormalEquipmentSlotUI::ResolveHoverTooltipItemDefinition`

## 第十一章 前置准备

这一轮手测默认目标是确认“统一 Tooltip 链路是否已经接到现有 UI 条目上”，不是验证某个特定蓝图皮肤是否存在。

手测前先确认：

1. 进入的界面是当前主 HUD 路径，而不是某个临时测试 UI。
2. 不预设必须存在蓝图 Tooltip 资源；当前正确口径是：
   - 如果 `ItemHoverTooltipWidgetClass` 已在 HUD 侧配置，就走配置类；
   - 如果未配置，也应回退到原生 C++ `UAOItemHoverTooltipWidget`，至少保证链路可工作。
3. 测试角色身上准备四类可观察目标：
   - 背包/快捷栏里的普通物品
   - 制造界面的配方、材料、产出条目
   - 技能栏里可由 `SourceItemInstance` 解析出来源物品的技能槽
   - 正式装备栏里已放入装备实例的槽位
4. 测试时默认在本地客户端观察 UI 结果即可；Tooltip 是本地 UI 状态，不涉及 RPC、复制或服务端权威 Tooltip 同步。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp`
  - `UAOMainUI::NativeConstruct`
  - `UAOMainUI::EnsureItemHoverTooltipWidget`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.cpp`
  - `UAOItemHoverTooltipWidget::BuildDefaultWidgetTreeIfNeeded`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::ShowTooltip`
  - `UMVVM_ItemHoverTooltip::HideTooltip`

## 第十二章 手工测试步骤

### 12.1 全局宿主与本地 UI 边界

1. 进入游戏并打开主 HUD。
2. 将鼠标悬停到任一可解析 `Definition` 的条目上，确认只出现一份 Tooltip。
3. 快速从条目 A 移到条目 B，确认 B 可以直接覆盖 A，且不会出现 A 的延迟 `MouseLeave` 把 B 误关掉。
4. 观察整轮行为时，不需要也不应该依赖 RPC 或复制现象；这里只验证本地 UI 状态。
5. 若项目没有配置专用蓝图 Tooltip 类，也应看到原生 C++ Tooltip Widget 的回退结果，而不是整条链路失效。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp`
  - `UAOMainUI::EnsureItemHoverTooltipWidget`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::ShowTooltip`
  - `UMVVM_ItemHoverTooltip::HideTooltip`
  - `UMVVM_ItemHoverTooltip::ForceHideTooltip`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.cpp`
  - `UAOItemHoverTooltipWidget::UAOItemHoverTooltipWidget`
  - `UAOItemHoverTooltipWidget::RefreshDisplay`

### 12.2 库存格子

1. 打开背包或其他库存类界面，把鼠标移动到一个有物品实例的格子上。
2. 确认 Tooltip 可以显示该物品的名称、描述，以及有图标时的图标。
3. 将鼠标移开，确认 Tooltip 关闭。
4. 再把鼠标移到一个空格子或当前无法解析出 `ItemInstance -> GetItemCDO()` 的格子上，确认不显示 Tooltip；如果上一格仍留着旧 Tooltip，应被关掉。
5. 如果项目当前 UI 上同时有背包与快捷栏，交叉移动鼠标，确认两者不是各自弹一份，而是共用 HUD 下那一份 Tooltip。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
  - `UAOInventoryUI::NativeOnMouseEnter`
  - `UAOInventoryUI::NativeOnMouseLeave`
  - `UAOInventoryUI::ResolveHoverTooltipItemDefinition`
  - `UAOInventoryUI::ShowHoverTooltip`
  - `UAOInventoryUI::HideHoverTooltip`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::ShowTooltip`

### 12.3 制造配方 / 材料 / 产出条目

1. 打开制造界面。
2. 先把鼠标移到配方条目上，确认 Tooltip 读取的是当前观察快照里的 `PrimaryOutputDefinition`，而不是临时回底层组件重新查表。
3. 再把鼠标移到材料条目上，确认 Tooltip 读取的是 `MaterialData.ItemDefinition`。
4. 再把鼠标移到产出条目上，确认 Tooltip 读取的是 `OutputData.ItemDefinition`。
5. 对任一当前 `Definition` 为空的制造条目，确认不显示 Tooltip；如果前一条目留有旧 Tooltip，应一并关闭。
6. 快速在配方、材料、产出三个条目间来回切换，确认全程仍只有一份活动 Tooltip。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp`
  - `UAOCraftingRecipeListEntryWidget::NativeOnMouseEnter`
  - `UAOCraftingRecipeListEntryWidget::NativeOnMouseLeave`
  - `UAOCraftingRecipeListEntryWidget::ResolveHoverTooltipItemDefinition`
  - `UAOCraftingRecipeListEntryWidget::ShowHoverTooltip`
  - `UAOCraftingRecipeListEntryWidget::HideHoverTooltip`
- `Source/AegisOdyssey/Crafting/Components/AOCraftingComponent.cpp`
  - `UAOCraftingComponent::BuildRecipeListViewData`
  - `UAOCraftingComponent::BuildRecipeDetailViewData`
  - `UAOCraftingComponent::ResolvePrimaryOutputDefinition`
  - `UAOCraftingComponent::FindItemDefinitionByItemId`

### 12.4 技能槽

1. 打开技能栏，确保至少有一个技能槽当前可从 `SourceItemInstance` 解析到来源物品定义。
2. 将鼠标移到该技能槽上，确认 Tooltip 能显示，并且数据来源是 `CurrentViewData.SourceItemInstance->GetItemCDO()`。
3. 再将鼠标移到没有有效 `SourceItemInstance` 的技能槽上，确认不显示 Tooltip。
4. 在技能槽与背包格子之间快速切换悬停，确认两边共享一份 HUD Tooltip，而不是各自维护独立生命周期。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/Skill/AOSkillSlotUI.cpp`
  - `UAOSkillSlotUI::ResolveHoverTooltipItemDefinition`
  - `UAOSkillSlotUI::ResolveCurrentSkillSlotViewData`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
  - `UAOInventoryUI::ShowHoverTooltip`
  - `UAOInventoryUI::HideHoverTooltip`
- `Source/AegisOdyssey/SkillSystem/Components/AOSkillComponent.cpp`
  - `UAOSkillComponent::GetSkillSlotViewData`
  - `UAOSkillComponent::BuildSkillSlotViewData`

### 12.5 正式装备槽

1. 打开正式装备栏，确保至少一个槽位里当前已有装备实例。
2. 将鼠标移到该正式装备槽上，确认 Tooltip 能显示，并且数据来源最终仍是 `CurrentEntry.Instance->GetItemCDO()`。
3. 再移到空正式装备槽上，确认不显示 Tooltip。
4. 在正式装备槽、背包格子、技能槽之间快速切换悬停，确认不会出现多个 Tooltip 并存，也不会出现旧来源把新来源误关掉。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.cpp`
  - `UAOFormalEquipmentSlotUI::ResolveHoverTooltipItemDefinition`
  - `UAOFormalEquipmentSlotUI::ResolveInventoryItemContextMenuRequest`
- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
  - `UAOInventoryUI::ShowHoverTooltip`
  - `UAOInventoryUI::HideHoverTooltip`

### 12.6 显示层行为

1. 对任一可显示 Tooltip 的条目执行悬停。
2. 确认 Tooltip 出现后不会持续跟随鼠标移动；它只使用进入当帧算出的屏幕位置。
3. 确认 Tooltip 本身不抢鼠标命中，不会因为盖住原条目而触发闪烁开关。
4. 如果物品没有图标 Fragment，确认 Tooltip 仍可只显示文字，不应因为缺图标整条链路失败。
5. 如果物品描述较长，确认当前代码路径不会在 C++ 侧截断、摘要或强制换行；显示效果由 UI 层自行处理。

关联代码位置：

- `Source/AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.cpp`
  - `UAOInventoryUI::ShowHoverTooltip`
- `Source/AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.cpp`
  - `UAOCraftingRecipeListEntryWidget::ShowHoverTooltip`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.cpp`
  - `UAOItemHoverTooltipWidget::UAOItemHoverTooltipWidget`
  - `UAOItemHoverTooltipWidget::RefreshDisplay`
  - `UAOItemHoverTooltipWidget::ResolveTooltipCanvasPosition`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::SetItemDescription`
  - `UMVVM_ItemHoverTooltip::SetHasValidItemIcon`

## 第十三章 验收口径

这一轮 Tooltip 相关验收，建议只按下面口径收，不额外发散到美术资源或网络层：

1. HUD 下始终只有一份活动 Tooltip 宿主；不同接入点共用同一套生命周期。
2. 当前代码允许 HUD 配置蓝图 Tooltip 类，但不能把“蓝图类一定已存在”写成既成事实；未配置时必须还能回退到原生 C++ Tooltip Widget。
3. 库存格子、制造配方条目、制造材料条目、制造产出条目、技能槽、正式装备槽都能按各自现有代码路径解析出 `Definition` 时显示 Tooltip。
4. 任一条目当前拿不到有效 `Definition` 时，不显示 Tooltip；若旧 Tooltip 还在，应能被正确关闭。
5. 新来源覆盖旧来源时，旧来源的延迟 Hide 请求不会误关当前 Tooltip。
6. Tooltip 只属于本地 UI 状态；本轮验收不要求 RPC、复制、服务端权威同步等网络语义。
7. Tooltip 的内容源仍然是 `UAOInventoryItemDefinition`，描述文本走完整 `FText`，图标仍从 `UAOFragment_InventoryIcon` 取；缺图标时允许只显示文字。

关联代码位置：

- `Source/AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.cpp`
  - `UMVVM_ItemHoverTooltip::ShowTooltip`
  - `UMVVM_ItemHoverTooltip::HideTooltip`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp`
  - `UAOMainUI::EnsureItemHoverTooltipWidget`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.cpp`
  - `UAOItemHoverTooltipWidget::BuildDefaultWidgetTreeIfNeeded`
  - `UAOItemHoverTooltipWidget::RefreshDisplay`
