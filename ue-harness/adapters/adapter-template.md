# Project Adapter Template

## 作用

`Project Adapter` 只负责把通用 `UE Harness` 接到某个具体 Unreal Engine 项目上。

它不定义 Harness 本体，也不替代 `MCP / Project Visibility`、`Memory` 或版本历史。
它只回答一个问题：

> 这套通用 Harness 在当前项目里应该去哪里读取上下文、写入沉淀、寻找验证入口？

## 填写原则

填写 adapter 时必须遵守：

- 只写当前项目真实存在或明确约定的路径
- 不把临时猜测写成项目事实
- 不把项目特例反向写进通用 Harness 本体
- 如果某个入口不存在，要明确写“不存在”和降级方式

## 1. Project Identity

```yaml
project_name: "<UE project name>"
project_root: "<absolute or workspace-relative project root>"
engine_version: "<known Unreal Engine version, or unknown>"
primary_module: "<main runtime module name>"
editor_module: "<editor module name, if any>"
```

## 2. Source And Build Visibility

说明 AI 应该从哪里读取代码和构建信息。

```yaml
source_root: "<Source directory>"
runtime_modules:
  - "<module path>"
target_files:
  - "<Game.Target.cs path>"
  - "<Editor.Target.cs path>"
build_files:
  - "<Build.cs path>"
uproject: "<uproject path>"
plugins_root: "<Plugins directory, if any>"
```

## 3. Knowledge Visibility

说明长期稳定知识放在哪里。

```yaml
knowledge_root: "<Docs/Knowledge or equivalent>"
knowledge_packages:
  - name: "<system name>"
    project_map: "<PROJECT_MAP.md path>"
    decisions: "<DECISIONS.md path>"
    known_issues: "<KNOWN_ISSUES.md path>"
```

建议每个系统包优先遵守：

- `PROJECT_MAP.md`：系统地图、先看哪里、当前真实入口
- `DECISIONS.md`：长期设计决策
- `KNOWN_ISSUES.md`：误判点、故障边界、排查入口

## 4. Notice Visibility

说明阶段性交接、临时判断和历史原文放在哪里。

```yaml
notice_root: "<Notice root path, or absent>"
history_notice_root: "<Notice/HistoryNotice path, or absent>"
notice_index: "<history notice index path, if any>"
```

如果项目没有实体 `Notice` 目录，但存在索引或历史归属表，必须写清：

- 实体目录当前是否存在
- 索引文件在哪里
- 当前任务产出的阶段性内容应如何降级落点

## 5. Required Read Order

说明任务开始时至少应读取哪些项目上下文。

```yaml
required_harness_files:
  - "<ue-harness/core/harness-definition.md>"
  - "<ue-harness/core/rule.md>"
  - "<ue-harness/core/skill-task-flow.md>"
  - "<ue-harness/core/script-validation.md>"
  - "<ue-harness/core/mcp-project-visibility.md>"
  - "<ue-harness/core/memory.md>"
  - "<ue-harness/core/version-control.md>"
  - "<ue-harness/core/development-self-loop.md>"

required_project_files:
  - "<project knowledge entry>"
  - "<collaboration protocol>"
```

## 6. System Package Routing

说明不同任务应该优先进入哪个系统包。

```yaml
system_routing:
  - keywords:
      - "<keyword>"
    knowledge_package: "<Docs/Knowledge/...>"
    source_roots:
      - "<Source/...>"
    content_roots:
      - "<Content/... if known>"
```

## 7. Blueprint And Editor Visibility

说明涉及蓝图或编辑器操作时，AI 应该如何获得项目上下文。

```yaml
blueprint_visibility:
  source_of_truth: "<where blueprint wiring is documented>"
  content_roots:
    - "<Content path>"
  required_when_changed:
    - "C++ class"
    - "Blueprint parent class"
    - "Editor wiring location"
    - "Runtime trigger path"
```

如果当前项目没有可机器读取的蓝图接线来源，应明确降级为：

- 由实现任务补写编辑器接线说明
- 由测试文档补写人工验证步骤

## 8. Validation Visibility

说明验证结果和检查入口在哪里。

```yaml
validation:
  compile_results: "<path or command reference>"
  test_results: "<path or command reference>"
  logs:
    - "<log directory>"
  automation_tests:
    - "<test source or command>"
```

如果某类验证暂时不能自动化，应写清人工检查入口。

## 9. Version Control Visibility

说明 AI 应该如何读取项目版本历史。版本控制不是可选背景，而是 Harness 开发自循环的一部分。

```yaml
version_control:
  system: "<git, p4, svn, or absent>"
  repository_root: "<repo root path>"
  history_visibility: "<what history can be inspected>"
  required_before_changes:
    - "<status command or manual check>"
  required_before_handoff:
    - "<diff command or manual check>"
```

如果当前项目没有可读取的版本历史，应明确写清降级方式，不能假装已经理解历史演进。

## 10. Memory And Routing Rules

说明任务完成后如何判断产出进入长期知识还是阶段性 notice。

```yaml
memory_routing:
  stable_knowledge_goes_to: "<Knowledge root>"
  stage_handoff_goes_to: "<Notice or fallback path>"
  default_question: "这句话半年后还应该成立吗？"
```

默认规则：

- 长期成立的结构事实、设计边界、可复用协作协议进入 `Knowledge`
- 阶段性交接、临时判断、历史背景、未完成提醒进入 `Notice`
- 如果没有 `Notice` 实体目录，必须写入当前项目约定的 fallback

## 11. Degradation Rules

说明缺失上下文时如何安全降级。

```yaml
degradation:
  missing_version_control: "<fallback>"
  missing_notice_root: "<fallback>"
  missing_blueprint_visibility: "<fallback>"
  missing_test_results: "<fallback>"
  missing_logs: "<fallback>"
```

降级不代表跳过 Harness，而是明确当前缺失的是哪一层可见性或沉淀入口。
