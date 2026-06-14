# Multi-Agent Strategy

## 定位

多 agent 在这套能力包里必须明确降级为“执行策略”。

它不是 Harness 的定义本体，而是承载 Harness 的一种方式。

原文里 `SubAgent` 很重要，但它只是五个核心组件之一，不能取代：

- Rule
- Skill
- Script
- MCP
- Memory
- Version Control
- Development Self-Loop

## 作用

多 agent 的价值在于：

- 复杂任务可以拆成不同角色视角
- 不同职责可以并行或分段协同
- AI 不必永远只站在“写代码”的单一视角工作

在 UE 场景里，常见的角色视角可以包括：

- 需求视角
- 设计视角
- 实现视角
- 测试视角
- 归档视角

## 关键边界

这套能力包里，多 agent 只能解决“怎么承载 Harness”，不能反过来定义“什么是 Harness”。

换句话说：

- 如果平台支持多 agent，Harness 可以通过多 agent 承载
- 如果平台不支持多 agent，Harness 仍然应该成立

因此，多 agent 不是前提条件，只是增强手段。

## 当前推荐执行策略

结合当前团队目标，推荐策略可以是：

- 主 agent 负责代码实现和最终汇总
- 子 agent 负责注释、文档、测试、检查、归档建议
- 主 agent 作为唯一代码语义写入者
- 子 agent 优先使用较低成本模型承担旁路任务

但这些内容必须被理解为：

- 策略层
- 调度层
- 成本层

而不是 Harness 本体定义。
