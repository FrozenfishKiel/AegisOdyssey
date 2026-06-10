---
title: Crafting System Known Issues
tags:
  - knowledge
  - crafting-system
  - known-issues
aliases:
  - Crafting System Known Issues
  - 制造系统已知问题与风险
---

# 制造系统已知问题与风险
更新时间：2026-05-28

这份文档记录的是当前真实还存在的风险，不是旧方案残留，也不是已经修完的问题。

相关文档：

- [[Crafting UI Refresh Refactor Plan]]
- [[Crafting System Decisions]]
- [[Crafting System Project Map]]
- [[Crafting Current Implemented State 2026-05-28]]
- [[Crafting Refactor Handoff 2026-05-27]]

## 1. 最容易再次看错的，是把当前系统当成旧双表架构

这是当前最大的认知风险。

现在真实架构已经是：

- `UAOPawnData::CraftingRecipeDataTable`
- `UAOCraftingComponent`
- `UMVVM_Crafting`
- Widget

如果后续有人还按下面这套旧思路查问题，基本一定会越查越偏：

- `CraftingRecipeSourceDataTable`
- `AOGameData` 里的制造配方表
- 角色来源表和全局配方表双表拼接

## 2. `CraftRecipe ... returned false` 不是命令没执行

如果控制台日志是：

`CraftRecipe request for 'Rope_Craft' on BP_Anny_C_0 returned false`

当前更准确的理解是：

- 控制台命令已经进到正式制造请求入口
- 当前 Pawn 和 `UAOCraftingComponent` 大概率都找到了
- 失败发生在正式入队前的底层校验阶段

优先排查：

1. 当前 Pawn 吃到的 `PawnData` 是否正确
2. `CraftingRecipeDataTable` 是否正确配置
3. `RecipeRowName` 是否真是表里的行名
4. 配方里的 `ItemId` 是否都能解出有效 `ItemDefinition`
5. 当前可参与制造的库存集合里是否真的有足够材料

## 3. 中文乱码仍然是当前显性风险

这轮接手过程中已经出现过乱码问题，所以这里必须留档。

当前乱码风险来源至少有两类：

- 文件编码不一致
- 源码里的中文字面量已经损坏

如果后续看到：

- UI 中文状态文案乱码
- 文档中文乱码
- 注释中文乱码

不要把它简单归类成“控件没绑好”。要同时检查编码和源码字面量。

## 4. 观察数据结构还没有完全收干净

当前方向已经比旧方案对很多了，但结构体层面仍然偏多。

现在还保留：

- `FAOCraftingRecipeListEntryViewData`
- `FAOCraftingRecipeDetailViewData`
- `FAOCraftingMaterialViewData`
- `FAOCraftingOutputViewData`
- `FAOCraftingQueueEntryViewData`

这不是当前必须立刻重构的阻塞问题，但它确实说明制造 UI 数据层还可以继续收。

## 5. `PrimaryOutputDefinition` 命名还带着旧语义

当前代码里已经不再传 `DefinitionClass`，而是直接传 `UAOInventoryItemDefinition*`，方向是对的。

但当前数据字段仍然叫 `PrimaryOutputDefinition`。这个名字更像“为了列表主显示而准备的字段”，而不是“UI 统一拿 Definition 自己查信息”的最终收敛形态。

这件事当前不影响功能，但它会持续影响后来人对结构的理解。

## 6. Widget 逻辑虽然收回 C++ 了，但蓝图壳子依然要配对

当前显示逻辑已经尽量收回 C++，但蓝图壳子依然需要满足最基本的控件绑定条件。

所以如果某个显示函数进了，UI 还是没变，要继续检查：

- 蓝图里绑定的控件名是否对
- `RecipeEntryWidgetClass`、`MaterialEntryWidgetClass`、`OutputEntryWidgetClass` 是否配置对
- 对应面板容器是否真的存在

换句话说，蓝图现在不是逻辑源，但仍然是承载层。

## 7. 当前最该防的是“又造一层中间刷新层”

这类风险后续非常容易反弹。

危险信号包括：

- 又想让 Layout 专门替制造补刷新
- 又想让 HUD 监听库存和队列后再转发一次
- 又想给 ViewModel 复制一份额外快照层
- 又想让 Widget 直接摸底层“方便一点”

这几种写法短期看像是在救火，长期一定会把这条链重新写回屎山。

## 8. 当前自动化测试还没补上

这轮已经有了比较清楚的冷启动手测链，但自动化测试还没有真正落地。

所以当前的现实是：

- 代码主链已经改了
- 编译通过了
- 但制造 UI 刷新链仍主要靠手测验证

后续如果补自动化，优先覆盖：

- 单表配方入口
- 库存变化触发刷新
- 队列变化触发刷新
- 详情区和列表区的一致性

而不是先去测蓝图排版本身。

## 9. Dedicated Server 远端客户端如果再出现“进度条晚启动、半截结束”，优先怀疑时间基准又被写回去了

这条问题本轮已经收过一次，当前不应再把它当成“未知网络玄学”。

如果后续再次出现下面这种现象：

- 队列项已经能在远端客户端显示
- 进度条不会立刻推进
- 过一会才开始动
- 只走到一半左右，这一件却已经完成

优先排查：

1. `UMVVM_Crafting::GetObservedServerWorldTimeSeconds()` 是否仍然优先走 `GameState->GetServerWorldTimeSeconds()`
2. `GetQueueEntryProgressRatio(...)` 是否又被改回直接使用客户端本地 `World->GetTimeSeconds()`
3. 客户端请求制造后的本地立即刷新，是否又被误当成“权威开始时间已经到位”

这条要记住：这类表现不是先查 `ProgressBar`，也不是先怀疑 Dedicated Server 本身不稳定，优先怀疑的是“服务端时间戳 + 客户端本地时间”又被混算了。
