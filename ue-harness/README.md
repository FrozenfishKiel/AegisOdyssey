# UE Harness

这是 `UE Harness` 能力包的正式骨架目录。

当前状态：

- 已完成 `Step 1：先搭能力包骨架`
- 已完成 `Step 2：再写 Harness 本体`
- 已完成 `Step 3：再写项目接线层`
- 已完成 `Step 4：补模板、检查清单和安全脚本`
- 下一步进入 `Step 5：最后再写入口 Skill 和执行策略`

## 目录说明

- `SKILL.md`
  入口 skill 的承载文件。
- `core/`
  Harness 本体定义层。
- `templates/`
  模板与检查清单层。
- `adapters/`
  项目接线层。
- `scripts/`
  安全验证脚本层。

## 当前实施边界

当前已经固定四层内容：

1. 通用 Harness 本体放在哪里
2. 接线层、模板层、脚本层分别放在哪里
3. 当前 UE 项目如何通过 `Project Adapter` 接入通用 Harness
4. 模板、检查清单和安全脚本如何支撑可重复验证与记忆回流

下一步按方案进入 `Step 5：最后再写入口 Skill 和执行策略`。
