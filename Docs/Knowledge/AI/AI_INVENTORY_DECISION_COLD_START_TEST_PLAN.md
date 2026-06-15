---
title: AI Inventory Decision Cold Start Test Plan
tags:
  - knowledge
  - ai
  - test-plan
  - inventory-decision
aliases:
  - AI库存决策冷启动测试方案
  - Inventory Decision Cold Start Test Plan
---

# AI 库存决策冷启动测试方案

更新时间：2026-06-14  
适用范围：当前项目内基于 `UAOAIDecisionComponent + StateTree` 的库存决策主链验证。  
目标：从 0 开始，把“AI 是否会把库存决策真正转成可执行库存操作”按链路分层验证清楚。  
不适用范围：具体某个敌人资产的数值调参、自动化测试脚本实现、未来未落地方案。

## 1. 测试身份

```yaml
feature: "AI 库存决策主链"
goal: "验证 AI 能否从库存评估、统一提交、StateTree 消费一路走到真实库存操作执行"
system: "UAOAIDecisionComponent + Inventory Decision StateTree"
environment: "UE Editor / PIE / 当前 AI 知识库对应实现"
date: "2026-06-14"
```

## 2. 先回答三个冷启动问题

### 2.1 去哪里测

不要先去角色控制器上找临时指令，也不要先盯某个 Task。

这条链当前应优先在“实际被 AI Controller Possess 的敌方 Pawn / Character 资产”上测，因为：

1. `UAOAIDecisionComponent` 的宿主在 `AAOCharacter` 上创建。  
   代码锚点：`Source/AegisOdyssey/Character/AOCharacter.cpp:65`
2. `AAOAIPlayerBotController::OnPossess()` 会扫描 Pawn 上的 `UAOStateTreeComponentBase` 并在树资产存在时 `RestartLogic()`。  
   代码锚点：`Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp:115-123`
3. `UAOAILogicStateTreeComponentBase` 会在组件初始化时补默认树，并监听“已提交的库存决策结果”。  
   代码锚点：`Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp:23-38, 59-109`

因此，测试入口不是“某个单独函数”，而是：

**一个被 `AAOAIPlayerBotController` 控制、并且 Pawn 上挂有 AI 逻辑 StateTree 组件的敌方角色资产。**

### 2.2 这条链真正的宿主是谁

按职责拆开看：

1. `AAOCharacter`
   持有 `UAOAIDecisionComponent`，这是决策运行时状态和统一提交队列的宿主。
2. `AAOAIPlayerBotController`
   负责在 Possess 后启动 Pawn 身上的 StateTree 逻辑。
3. `UAOAILogicStateTreeComponentBase`
   负责把“已提交的库存决策结果”转成 StateTree Event。
4. `STE_UpdateInventoryDecision`
   负责收集和计算库存决策，不负责最终执行库存操作。
5. `STT_UseResolvedInventoryItem`
   负责把已经提交/解析出的库存决策结果落成真实库存调用。

### 2.3 这次到底要验证什么

不是验证“标签有没有配上”，而是验证下面整条链是否闭合：

1. StateTree Evaluator 收集事实并算出 `FAOAIInventoryDecisionResult`
2. 结果缓存进 `UAOAIDecisionComponent`
3. 决策组件把库存决策提交到统一决策层
4. `UAOAILogicStateTreeComponentBase` 监听到“已提交库存决策变更”
5. 它发出 `AI_Event_InventoryDecision_Updated` 或 `AI_Event_InventoryDecision_Cleared`
6. 对应 StateTree Task 消费当前已提交库存决策
7. 最终走到 `UAOQuickBarComponent::SetActivateIndex(...)` 或 `UAOInventoryComponent::TryUseItemAtSlot(...)`

## 3. 前置条件

### 3.1 项目与环境

1. 能正常打开当前工程。
2. 能进入拥有 AI 敌人的测试地图。
3. 能在 PIE 下观察目标 AI 的 Controller、Pawn、StateTree 组件、Inventory 组件和 QuickBar 组件。

### 3.2 必须先确认的资产接线

进入测试前，先在目标敌人蓝图或实例上确认：

1. Pawn 最终类型继承自 `AAOCharacter`。
2. AI Controller 类是 `AAOAIPlayerBotController` 或其派生类。
3. Pawn 身上存在 `UAOAILogicStateTreeComponentBase` 或其派生 StateTree 组件。
4. 该 StateTree 组件要么显式配置了树资产，要么 `DefaultStateTree` 不为空。
5. Pawn 身上存在库存相关组件，并且运行时确实有可供使用的道具或快捷栏条目。

### 3.3 测试素材要求

为了验证“AI 主动从库存里拿东西出来用”，测试角色至少要满足下面之一：

1. 快捷栏里已有可用条目，且该条目满足当前库存决策候选条件。
2. 普通库存里已有可用物品，且物品可以被 `TryUseItemAtSlot(...)` 成功使用。

如果这两条都不满足，那么后面即使库存决策评估链是通的，也不会出现真实使用行为。

## 4. 冷启动测试步骤

这一节按“从外到内、从宿主到执行”来走，不跳步。

### 4.1 第一层：先确认测试宿主和树是否真的启动

#### 步骤

1. 打开工程。
2. 进入包含目标 AI 的测试地图。
3. 找到一个具体的敌方 AI 实例，确认它的 Pawn 类、Controller 类和挂载组件。
4. 启动 PIE。
5. 在运行时确认该 AI 已被 `AAOAIPlayerBotController` Possess。
6. 确认 Pawn 上的 `UAOAILogicStateTreeComponentBase` 已经拿到 StateTree 资产并启动逻辑。

#### 预期结果

1. 目标 Pawn 持有 `UAOAIDecisionComponent`。
2. 目标 Pawn 被 `AAOAIPlayerBotController` 控制。
3. Possess 后 StateTree 被重启，运行态不是空树或停树状态。
4. 如果组件本身没显式配树，但 `DefaultStateTree` 已配，则运行时仍能拿到树。

#### 失败意味着什么

1. 如果 Pawn 上没有 `UAOAIDecisionComponent`，说明测的宿主就错了，这条链后面都不用看。
2. 如果 Controller 不是 `AAOAIPlayerBotController`，说明你没有走当前方案定义的启动入口。
3. 如果 StateTree 没有资产或没有启动，后面看不到任何库存决策行为是正常的。

### 4.2 第二层：先确认 AI 当前真的具备做库存决策的外部条件

#### 步骤

1. 让 AI 进入会产生战斗行为的场景，确保它有目标、在战斗链里工作。
2. 检查该 AI 当前库存和快捷栏是否真的有可用物品。
3. 确认这些物品当前场景下具备可用性，而不是因为冷却、数量、槽位、前置条件导致不可用。
4. 记录“这次期望 AI 用的到底是什么物品或哪类候选”。

#### 预期结果

1. AI 不是纯空载库存。
2. 至少有一个候选物品理论上能被选中并执行。
3. 当前战斗场景能触发库存决策评估，而不是一直停留在无目标或无战斗输入状态。

#### 失败意味着什么

1. 如果库存本身没有候选物品，后续“AI 不会主动用装备”不是实现 bug，而是测试样本无效。
2. 如果物品存在但当前不可用，后续更可能卡在候选筛选或执行校验层，而不是 StateTree 事件层。

### 4.3 第三层：验证 `STE_UpdateInventoryDecision` 是否真的算出了结果

#### 步骤

1. 在运行时打开 StateTree 调试或对应调试观察入口。
2. 重点观察 `STE_UpdateInventoryDecision` 的实例数据。
3. 确认它是否更新了：
   - `PendingInventoryDecision`
   - `bHasPendingInventoryDecision`
   - `PendingInventoryDecisionCoordinationMode`
   - `CurrentSubmittedInventoryDecision`
   - `bHasCurrentSubmittedInventoryDecision`
4. 重点确认这次评估是否产出了带 `UseCommand` 或 `ResolvedTarget` 的 `FAOAIInventoryDecisionResult`。

#### 代码锚点

1. 计算结果构建：`Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateInventoryDecision.cpp:593-638`
2. 缓存进决策组件：`Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateInventoryDecision.cpp:641-646`
3. 回读当前已提交库存决策：`Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateInventoryDecision.cpp:648-649`

#### 预期结果

1. Evaluator 在有效战斗窗口内能够产出 `PendingInventoryDecisionResult`。
2. 这个结果不是只有标签，还应包含实际执行所需的 `UseCommand`，或已经解析出的 `ResolvedTarget`。
3. 如果命中快捷栏候选，结果中应带出快捷栏槽位信息。

#### 失败意味着什么

1. 如果这里根本没有 `PendingInventoryDecisionResult`，问题在评估层，不要先怪 Task。
2. 如果有 `bHasAction` 但没有有效 `UseCommand / ResolvedTarget`，说明决策结果不完整，执行层就算进来也会失败。
3. 当前文件在 `651-656` 仍保留旧的 pending 输出残留，这里是现阶段高风险点。如果表现怪异，这一段要优先怀疑。

### 4.4 第四层：验证结果有没有进入统一决策组件，而不是留在 STE 里自嗨

#### 步骤

1. 在运行时观察 `UAOAIDecisionComponent` 的库存决策运行时状态。
2. 确认 Evaluator 调用 `CacheInventoryEvaluation(...)` 后，决策组件内部确实接收到了这次库存评估结果。
3. 确认当前有没有“已提交”的库存决策结果，而不只是评估期的临时结果。
4. 如果项目里有统一决策调试入口，确认库存决策最终是以统一决策项形式进入队列或当前提交态。

#### 代码锚点

1. 当前提交结果写入与广播：`Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:335-352`
2. 库存统一决策标签：`Source/AegisOdyssey/AOGameplayTags.cpp:51`
3. 当前已提交库存结果读取：`Source/AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.cpp:515-524`

#### 预期结果

1. 库存决策不是停留在 STE 私有变量里，而是进入 `UAOAIDecisionComponent`。
2. 当前提交态能读到 `CurrentSubmittedInventoryDecisionResult`。
3. 统一提交使用的是 `AI_Decision_Inventory_UseItem`，不是绕开主链直接手搓执行。

#### 失败意味着什么

1. 如果 Evaluator 有结果，但 `UAOAIDecisionComponent` 没接到，说明问题在“评估结果 -> 决策组件”这一层。
2. 如果决策组件有缓存但没有已提交结果，说明问题在“缓存/评估”与“统一提交”之间。
3. 如果有人直接在别处绕过 `UAOAIDecisionComponent` 执行库存，那就是脱离方案。

### 4.5 第五层：验证 StateTree 桥接组件是否只消费“已提交结果”

#### 步骤

1. 运行时确认 `UAOAILogicStateTreeComponentBase` 成功绑定到了 `UAOAIDecisionComponent::OnSubmittedInventoryDecisionChanged()`。
2. 观察库存提交变化时，桥接组件是否发出对应的 StateTree Event。
3. 区分两种事件：
   - `AI_Event_InventoryDecision_Updated`
   - `AI_Event_InventoryDecision_Cleared`

#### 代码锚点

1. 事件绑定：`Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp:59-84`
2. 事件发送：`Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp:97-108`

#### 预期结果

1. StateTree 桥接层只监听“已提交库存决策变更”。
2. 有动作时发送 `Updated`。
3. 清空时发送 `Cleared`。
4. 事件 Payload 是 `FAOAIInventoryDecisionResult`，不是只有一个标签。

#### 失败意味着什么

1. 如果决策组件已经有提交结果，但 StateTree 完全收不到事件，问题在桥接层。
2. 如果桥接层监听的是 pending 而不是 submitted，那就违反当前方案边界。

### 4.6 第六层：验证执行 Task 是否消费了“已提交库存决策”

#### 步骤

1. 找到树中负责实际使用库存结果的状态与 Task。
2. 确认命中的 Task 是 `STT_UseResolvedInventoryItem`。
3. 观察它进入状态时是否成功解析：
   - `CurrentSubmittedInventoryDecision`
   - `UseCommand`
   - `ResolvedTarget`
4. 确认它成功进入 `Succeeded`，而不是因为解析失败或执行失败直接 `Failed`。

#### 代码锚点

1. Task 入口与执行：`Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_UseResolvedInventoryItem.cpp:13-60`
2. 当前已提交结果优先消费：`Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_UseResolvedInventoryItem.cpp:79-112`

#### 预期结果

1. Task 优先消费当前已提交库存决策。
2. 如果当前实例数据没有，则会回查 `UAOAIDecisionComponent` 当前提交态。
3. 执行成功后会调用 `CommitExecutedInventoryAction(...)` 回写执行事实。

#### 失败意味着什么

1. 如果任务没有命中，问题更可能在上游事件或树条件。
2. 如果任务命中了但 `ResolveUseCommand(...)` 失败，问题在提交结果不完整。
3. 如果任务命中了且解析成功，但执行失败，问题在库存执行层而不是决策层。

### 4.7 第七层：验证是否真的落到库存组件调用

#### 步骤

1. 如果结果走快捷栏，确认最终落到 `UAOQuickBarComponent::SetActivateIndex(...)`。
2. 如果结果走普通库存，确认最终落到 `UAOInventoryComponent::TryUseItemAtSlot(...)`。
3. 检查目标槽位是否合法。
4. 检查调用后是否真的出现了预期的游戏内效果，而不是只在决策层显示“想用”。

#### 代码锚点

1. 快捷栏执行：`Source/AegisOdyssey/Character/Enemies/AI/AOAIInventoryRuntimeUseLibrary.cpp:60-71`
2. 普通库存执行：`Source/AegisOdyssey/Character/Enemies/AI/AOAIInventoryRuntimeUseLibrary.cpp:77-85`

#### 预期结果

1. 快捷栏路径会切到目标槽位。
2. 普通库存路径会真正调用 `TryUseItemAtSlot(...)`。
3. AI 行为层面可以看到“主动使用库存物品”的实际表现。

#### 失败意味着什么

1. 如果前面都正常，最后这里失败，说明是“库存执行层”问题，不是决策问题。
2. 如果槽位非法，优先回查 `ResolvedTarget` 或候选解析。
3. 如果函数调用成功但游戏表现没有变化，要去看具体物品使用逻辑，而不是再回头改决策架构。

## 5. 推荐的排查顺序

如果当前现象是：**角色不会主动拿库存里的装备来跟我对打**，默认排查顺序固定为：

1. 先确认测试样本里真的有可用物品。
2. 再看 `STE_UpdateInventoryDecision` 是否产出了有效 `PendingInventoryDecisionResult`。
3. 再看 `UAOAIDecisionComponent` 是否出现 `CurrentSubmittedInventoryDecisionResult`。
4. 再看 `UAOAILogicStateTreeComponentBase` 是否发出 `AI_Event_InventoryDecision_Updated`。
5. 再看 `STT_UseResolvedInventoryItem` 是否进树并执行成功。
6. 最后再看 `SetActivateIndex(...)` / `TryUseItemAtSlot(...)` 是否真被调用。

不要反过来一上来先怀疑标签，也不要先去角色控制器上加临时指令。

## 6. 每层失败对照表

### 6.1 宿主层失败

现象：AI 根本没进入正确宿主链。  
优先看：

1. Pawn 是否是 `AAOCharacter` 系。
2. Controller 是否是 `AAOAIPlayerBotController`。
3. StateTree 组件是否真的挂在 Pawn 上。

### 6.2 启动层失败

现象：树没跑起来。  
优先看：

1. StateTree 资产是否为空。
2. `DefaultStateTree` 是否为空。
3. Possess 后是否真的执行了 `RestartLogic()`。

### 6.3 评估层失败

现象：没有库存决策结果。  
优先看：

1. 当前是否满足战斗/目标前提。
2. 当前库存里是否有可用候选。
3. `STE_UpdateInventoryDecision` 是否只算出 0 分或没有候选。

### 6.4 提交层失败

现象：STE 里有结果，但决策组件没有当前提交结果。  
优先看：

1. `CacheInventoryEvaluation(...)` 之后统一决策是否真的提交。
2. 提交标签是否还是 `AI_Decision_Inventory_UseItem`。
3. 是否有旧逻辑在别处清掉当前提交态。

### 6.5 桥接层失败

现象：决策组件有已提交结果，但 StateTree 没反应。  
优先看：

1. `UAOAILogicStateTreeComponentBase` 是否成功绑定。
2. 是否正确发送了 `Updated / Cleared`。
3. Payload 是否为完整 `FAOAIInventoryDecisionResult`。

### 6.6 执行层失败

现象：Task 进了，但没有实际使用物品。  
优先看：

1. `ResolveUseCommand(...)` 是否成功。
2. `ResolvedTarget` 槽位是否合法。
3. `TryExecuteResolvedTarget(...)` 或 `TryExecuteUseCommand(...)` 是否返回失败。

## 7. 当前已知风险点

### 7.1 `STE_UpdateInventoryDecision` 仍保留旧 pending 输出残留

当前文件在：

- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateInventoryDecision.cpp:651-656`

仍在把 `PendingInventoryDecisionResult` 同步回实例数据。  
这不一定必然出 bug，但它是当前最容易让人误把“评估期结果”当成“统一提交结果”的高风险入口。

因此测试时要特别区分：

1. `PendingInventoryDecision`
2. `CurrentSubmittedInventoryDecision`

如果现象是“StateTree 看起来有候选，但就是不执行”，优先检查是不是只看到了 pending，没有形成 submitted。

## 8. 手工验证记录模板

执行测试时，建议至少按下面格式记录一次：

```yaml
test_map: ""
test_enemy: ""
controller_class: ""
state_tree_component: ""
state_tree_asset: ""
inventory_has_candidate: true
quickbar_has_candidate: true
evaluator_has_pending_result: true
decision_component_has_submitted_result: true
bridge_sent_updated_event: true
task_entered: true
runtime_execute_succeeded: true
final_behavior_observed: ""
suspect_layer: ""
notes: ""
```

## 9. 未覆盖项

本方案当前未覆盖：

1. 多人联机下的库存决策同步。
2. 存档恢复后的库存决策状态恢复。
3. 复杂道具副作用、动画、能力链的逐项验证。
4. 全量敌人资产的逐个蓝图接线巡检。

## 10. 关联文档

1. [PROJECT_MAP.md](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Docs/Knowledge/AI/PROJECT_MAP.md)
2. [DECISIONS.md](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Docs/Knowledge/AI/DECISIONS.md)
3. [KNOWN_ISSUES.md](/D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Docs/Knowledge/AI/KNOWN_ISSUES.md)
