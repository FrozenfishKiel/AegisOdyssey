# AI战斗决策系统现行文档说明

## 当前应该看哪几份

如果你要按照项目**当前已经落地的实现**来理解和继续使用 AI 战斗决策系统，请优先看下面两份：

1. [AI当前进度与新会话交接说明.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI当前进度与新会话交接说明.md)
2. [AI战斗决策参数调参与计算说明.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI战斗决策参数调参与计算说明.md)

它们的分工是：

- `AI当前进度与新会话交接说明.md`
  负责讲现在项目推进到哪、这套系统和项目其他部分是什么关系、后续接手时先看什么。
- `AI战斗决策参数调参与计算说明.md`
  负责讲当前实现的公式、参数、距离响应曲线、StateTree 接法、辅助条件与调试节点。

---

## 哪些文档属于历史阶段稿

下面这些文档可以保留作历史参考，但**不要把它们当成当前最终规则**：

- [AI第四阶段战斗决策系统使用与实现说明.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI第四阶段战斗决策系统使用与实现说明.md)
- [AI第四阶段战斗决策与攻击欲望设计方案.md](D:/UE_Project/AO/AegisOdyssey/AegisOdyssey/Notice/AI第四阶段战斗决策与攻击欲望设计方案.md)

原因很简单：

- 它们形成于当前实现完全收口之前
- 里面保留了一些历史讨论语义
- 个别字段和流程已经被后续实现替换

---

## 当前已经明确过期的历史点

如果你在旧文档里看到下面这些内容，请以新文档为准：

- `IdealAttackDistance`
  现在已经不再是正式配置入口，当前距离语义统一由武器上的 `AIAttackRange` 提供。
- 默认 `Attack / Strafe` 自动补全
  现在已经移除。每个角色都必须明确配置自己的 `DecisionProfile -> IntentDefinitions`。
- “攻击距离不够就直接取消攻击意图”
  现在已经不是主逻辑。距离更多是惩罚或奖励因子，通过距离响应曲线进入计算。

---

## 当前这套系统的最小完整组成

如果你想确认“整套系统”是否已经说明完整，可以按这个清单看：

1. 武器侧距离入口：`AOWeaponDefinition.AIAttackRange`
2. 决策配置入口：`UAOAIDecisionProfile.IntentDefinitions`
3. 决策运行时核心：`UAOAIDecisionComponent`
4. 事实采集 Evaluator：`UpdateCurrentTarget`
5. 决策输出 Evaluator：`UpdateCombatDecision`
6. 标签分支 Condition：`AI Decision Intent Matches`
7. 数值门槛 Condition：`AI Decision Value In Range`
8. 执行回写 Task：`Commit AI Decision Intent`
9. 记忆清理 Task：`Reset AI Decision State`
10. 调试打印 Task：`Debug Print On Screen`
11. StateTree 选择层接法
12. 距离响应曲线调参方法

上面这些内容，现在都已经在现行文档中覆盖到了。
