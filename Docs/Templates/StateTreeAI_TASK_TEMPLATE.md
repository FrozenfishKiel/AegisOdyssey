# StateTree AI 任务模板

更新时间：2026-05-14
适用范围：所有落在 `StateTree AI` 域内的分析、修复、重构前诊断、行为回归排查任务。

## 建议固定前缀

```md
按现有项目风格处理。
先分析影响范围，再实施改动。
不要修改无关模块。
如果存在明显风险，先指出再继续。
优先给出最小改动方案。
```

## 标准任务模板

```md
任务：
- [一句话描述这次要解决的问题]

问题背景：
- [当前现象]
- [复现条件]
- [最近一次观察到的日志、视频现象或调试结论]

任务目标：
- [修复后应该恢复什么行为]
- [如果是分析任务，就写清要确认什么真相]

本次范围：
- [只看哪些模块]
- [优先检查哪些类 / 文件]
- [明确这次不扩散到哪些系统]

当前约束：
- [例如：不重构整套 AI 系统]
- [例如：不修改无关角色逻辑]
- [例如：先不改具体 StateTree 资产，只查 C++ 运行时链路]

期望产出：
- [例如：给出根因分析]
- [例如：给出最小改动修复方案]
- [例如：指出需要补的验证点]

输出要求：
- [要求区分明确依据 / 基于当前资料的推断 / 仍需确认的缺口]
- [如果是修复任务，要求先分析再改]
- [如果是高风险任务，要求给验证清单]

验收标准：
- [至少 2-4 条可验证结果]

已知风险：
- [最担心的回归点]
- [最可能误判的点]

优先参考知识：
- `Docs/Knowledge/StateTreeAI/PROJECT_MAP.md`
- `Docs/Knowledge/StateTreeAI/AI_RULES.md`
- `Docs/Knowledge/StateTreeAI/DECISIONS.md`
- `Docs/Knowledge/StateTreeAI/KNOWN_ISSUES.md`

相关代码：
- [列出 3-8 个最相关文件]
```

## 示例一：目标丢失后不回退

```md
任务：
- 排查 AI 丢失当前目标后仍停留在追击逻辑的问题

问题背景：
- 敌人目标死亡或离开后，AI 仍然表现为追击态
- 当前观察到移动任务结束后，没有正常回到无目标分支

任务目标：
- 确认当前目标失效后，相关状态是否被正确清理
- 给出最小改动修复方案

本次范围：
- 只看 `CurrentTarget` 维护、StateTree evaluator、MoveTo 相关逻辑
- 优先检查 `AAOAIPlayerBotController`、`STE_UpdateCurrentTarget`、`STT_MoveToTarget`
- 不扩散到整套感知系统重构

当前约束：
- 不重构整个 AI 感知系统
- 不修改无关角色逻辑
- 优先修运行时状态边界

期望产出：
- 说明目标状态在哪一层丢失或没清理
- 给出最小改动修复建议
- 列出需要验证的回归点

输出要求：
- 区分当前代码能直接确认的依据与推断
- 先分析目标状态链路，再给改动建议
- 给出最小验证清单

验收标准：
- 目标死亡后不会继续追旧目标
- 没有目标时相关条件分支能正确切换
- 正常追击现有目标的行为不回归

已知风险：
- 清目标可能影响 Focus
- 依赖 `CurrentTarget` 的其他节点可能出现联动变化

优先参考知识：
- `Docs/Knowledge/StateTreeAI/PROJECT_MAP.md`
- `Docs/Knowledge/StateTreeAI/AI_RULES.md`
- `Docs/Knowledge/StateTreeAI/DECISIONS.md`
- `Docs/Knowledge/StateTreeAI/KNOWN_ISSUES.md`

相关代码：
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`
- `Source/AegisOdyssey/Character/Enemies/AOEnemyBotController.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Evaluator/STE_UpdateCurrentTarget.cpp`
- `Source/AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_MoveToTarget.cpp`
```

## 示例二：StateTree 组件没启动

```md
任务：
- 排查敌人 Pawn 被接管后 StateTree 没有正确启动的问题

问题背景：
- 某些敌人生成后不执行预期 AI 行为
- 怀疑 StateTree 组件没有拿到树或没有在 Possess 后正确重启

任务目标：
- 确认树资产来源、组件生命周期与重启链路是否正确

本次范围：
- 只看 StateTree 组件生命周期和 Possess 后重启链
- 优先检查 `UAOStateTreeComponentBase`、`UAOAILogicStateTreeComponentBase`、`AAOAIPlayerBotController::OnPossess`
- 不先扩散到具体行为节点资产

当前约束：
- 不修改无关行为节点
- 不先改资产内容
- 先完成运行时链路确认

期望产出：
- 指出树资产在当前实例上是怎么被设置的
- 指出逻辑为什么没有启动或重启
- 给出最小修复建议

输出要求：
- 区分明确依据与推断
- 先确认生命周期链路，再给修复建议
- 给出最小验证清单

验收标准：
- 组件能拿到期望的 StateTree 资产
- Possess 后会执行正确重启链路
- 现有正常工作的 AI 不被破坏

已知风险：
- GameFeature 动态加组件时序可能影响结果

优先参考知识：
- `Docs/Knowledge/StateTreeAI/PROJECT_MAP.md`
- `Docs/Knowledge/StateTreeAI/AI_RULES.md`
- `Docs/Knowledge/StateTreeAI/DECISIONS.md`
- `Docs/Knowledge/StateTreeAI/KNOWN_ISSUES.md`

相关代码：
- `Source/AegisOdyssey/StateTree/AOStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.cpp`
- `Source/AegisOdyssey/Player/AAOAIPlayerBotController.cpp`
```
