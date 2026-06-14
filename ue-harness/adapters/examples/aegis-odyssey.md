# Aegis Odyssey Adapter Example

## 作用

这是 `AegisOdyssey` 当前仓库接入通用 `UE Harness` 的 adapter 示例。

本文件只描述当前项目的接线方式，不反向定义 Harness 本体。

## 1. Project Identity

```yaml
project_name: "AegisOdyssey"
project_root: "D:/UE_Project/AO/AegisOdyssey/AegisOdyssey"
engine_version: "5.6"
primary_module: "AegisOdyssey"
editor_module: "none; AegisOdysseyEditor.Target.cs reuses AegisOdyssey"
```

## 2. Source And Build Visibility

```yaml
source_root: "Source"
runtime_modules:
  - "Source/AegisOdyssey"
target_files:
  - "Source/AegisOdyssey.Target.cs"
  - "Source/AegisOdysseyEditor.Target.cs"
build_files:
  - "Source/AegisOdyssey/AegisOdyssey.Build.cs"
uproject: "AegisOdyssey.uproject"
plugins_root: "Plugins"
content_root: "Content"
config_root: "Config"
saved_root: "Saved"
```

主要源码子系统入口包括：

- `Source/AegisOdyssey/AbilitySystem`
- `Source/AegisOdyssey/AI` 当前未发现独立目录，AI 相关入口主要分散在 `Player`、`StateTree`、`Character` 与知识库 `Docs/Knowledge/AI`
- `Source/AegisOdyssey/Combat`
- `Source/AegisOdyssey/Crafting`
- `Source/AegisOdyssey/Harvest`
- `Source/AegisOdyssey/Interaction`
- `Source/AegisOdyssey/Inventory`
- `Source/AegisOdyssey/Equipment`
- `Source/AegisOdyssey/SkillSystem`
- `Source/AegisOdyssey/StateTree`
- `Source/AegisOdyssey/UI`
- `Source/AegisOdyssey/TestProject`

## 3. Knowledge Visibility

```yaml
knowledge_root: "Docs/Knowledge"
knowledge_index:
  - "Docs/Knowledge/KNOWLEDGE_BASE_PLAN.md"
  - "Docs/Knowledge/HISTORY_NOTICE_INDEX.md"
```

当前已存在的主要知识包：

```yaml
knowledge_packages:
  - name: "AI"
    root: "Docs/Knowledge/AI"
    project_map: "Docs/Knowledge/AI/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/AI/DECISIONS.md"
    known_issues: "Docs/Knowledge/AI/KNOWN_ISSUES.md"
  - name: "Collaboration"
    root: "Docs/Knowledge/Collaboration"
    project_map: "absent"
    decisions: "absent"
    known_issues: "absent"
  - name: "CombatSystem"
    root: "Docs/Knowledge/CombatSystem"
    project_map: "Docs/Knowledge/CombatSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/CombatSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/CombatSystem/KNOWN_ISSUES.md"
  - name: "CraftingSystem"
    root: "Docs/Knowledge/CraftingSystem"
    project_map: "Docs/Knowledge/CraftingSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/CraftingSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/CraftingSystem/KNOWN_ISSUES.md"
  - name: "EngineCore"
    root: "Docs/Knowledge/EngineCore"
    project_map: "Docs/Knowledge/EngineCore/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/EngineCore/DECISIONS.md"
    known_issues: "Docs/Knowledge/EngineCore/KNOWN_ISSUES.md"
  - name: "GameplayFramework"
    root: "Docs/Knowledge/GameplayFramework"
    project_map: "Docs/Knowledge/GameplayFramework/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/GameplayFramework/DECISIONS.md"
    known_issues: "Docs/Knowledge/GameplayFramework/KNOWN_ISSUES.md"
  - name: "HarvestSystem"
    root: "Docs/Knowledge/HarvestSystem"
    project_map: "Docs/Knowledge/HarvestSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/HarvestSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/HarvestSystem/KNOWN_ISSUES.md"
  - name: "InteractionSystem"
    root: "Docs/Knowledge/InteractionSystem"
    project_map: "Docs/Knowledge/InteractionSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/InteractionSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/InteractionSystem/KNOWN_ISSUES.md"
  - name: "InventoryEquipment"
    root: "Docs/Knowledge/InventoryEquipment"
    project_map: "Docs/Knowledge/InventoryEquipment/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/InventoryEquipment/DECISIONS.md"
    known_issues: "Docs/Knowledge/InventoryEquipment/KNOWN_ISSUES.md"
  - name: "MultiplayerSystem"
    root: "Docs/Knowledge/MultiplayerSystem"
    project_map: "Docs/Knowledge/MultiplayerSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/MultiplayerSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/MultiplayerSystem/KNOWN_ISSUES.md"
  - name: "ReferenceStudies"
    root: "Docs/Knowledge/ReferenceStudies"
    project_map: "Docs/Knowledge/ReferenceStudies/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/ReferenceStudies/DECISIONS.md"
    known_issues: "Docs/Knowledge/ReferenceStudies/KNOWN_ISSUES.md"
  - name: "SkillSystem"
    root: "Docs/Knowledge/SkillSystem"
    project_map: "Docs/Knowledge/SkillSystem/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/SkillSystem/DECISIONS.md"
    known_issues: "Docs/Knowledge/SkillSystem/KNOWN_ISSUES.md"
  - name: "StateTreeAI"
    root: "Docs/Knowledge/StateTreeAI"
    project_map: "Docs/Knowledge/StateTreeAI/PROJECT_MAP.md"
    decisions: "Docs/Knowledge/StateTreeAI/DECISIONS.md"
    known_issues: "Docs/Knowledge/StateTreeAI/KNOWN_ISSUES.md"
```

## 4. Notice Visibility

```yaml
notice_root: "absent in current workspace scan"
history_notice_root: "absent in current workspace scan"
notice_index: "Docs/Knowledge/HISTORY_NOTICE_INDEX.md"
```

当前仓库扫描没有发现实体 `Notice` 或 `Notice/HistoryNotice` 目录。

但 `Docs/Knowledge/HISTORY_NOTICE_INDEX.md` 明确记录了历史 notice 的归属与处理方式，说明项目知识库曾经以 `Notice/HistoryNotice` 作为历史档案来源。

因此当前降级规则为：

- 稳定事实继续进入 `Docs/Knowledge`
- 阶段性交接、临时判断、历史背景原则上应进入 `Notice/HistoryNotice`
- 在实体 `Notice/HistoryNotice` 目录恢复或创建前，不应把临时内容误写入 `Docs/Knowledge` 正文
- 如果必须记录本轮阶段性内容，先在交付说明中标记为 `Notice candidate`，等待项目确认落点

## 5. Required Read Order

Harness 本体必读：

```yaml
required_harness_files:
  - "ue-harness/core/harness-definition.md"
  - "ue-harness/core/rule.md"
  - "ue-harness/core/skill-task-flow.md"
  - "ue-harness/core/script-validation.md"
  - "ue-harness/core/mcp-project-visibility.md"
  - "ue-harness/core/memory.md"
  - "ue-harness/core/version-control.md"
  - "ue-harness/core/development-self-loop.md"
```

当前项目协作规则优先读：

```yaml
required_project_files:
  - "Docs/Knowledge/KNOWLEDGE_BASE_PLAN.md"
  - "Docs/Knowledge/HISTORY_NOTICE_INDEX.md"
  - "Docs/Knowledge/Collaboration/TASK_CARD_AND_SHORT_ROUND_PROTOCOL.md"
  - "Docs/Knowledge/Collaboration/HANDOFF_DOCUMENT_BOUNDARY_AND_EXTRACTION_RULES.md"
```

系统任务进入时，应先读对应系统包的：

- `PROJECT_MAP.md`
- `DECISIONS.md`
- `KNOWN_ISSUES.md`

## 6. System Package Routing

```yaml
system_routing:
  - keywords: ["AI", "StateTree", "Bot", "Patrol", "Reposition", "Combat Decision"]
    knowledge_package: "Docs/Knowledge/AI"
    source_roots:
      - "Source/AegisOdyssey/StateTree"
      - "Source/AegisOdyssey/Player"
      - "Source/AegisOdyssey/Character"
  - keywords: ["Combat", "Damage", "Defense", "Hit", "Health Bar", "Floating Text"]
    knowledge_package: "Docs/Knowledge/CombatSystem"
    source_roots:
      - "Source/AegisOdyssey/Combat"
      - "Source/AegisOdyssey/ExecCal"
      - "Source/AegisOdyssey/UI"
  - keywords: ["Skill", "Ability Slot", "Skill Slot", "Gameplay Ability"]
    knowledge_package: "Docs/Knowledge/SkillSystem"
    source_roots:
      - "Source/AegisOdyssey/SkillSystem"
      - "Source/AegisOdyssey/AbilitySystem"
  - keywords: ["Inventory", "BackPack", "Equipment", "QuickBar", "Formal Equipment"]
    knowledge_package: "Docs/Knowledge/InventoryEquipment"
    source_roots:
      - "Source/AegisOdyssey/Inventory"
      - "Source/AegisOdyssey/Equipment"
      - "Source/AegisOdyssey/Items"
  - keywords: ["Interaction", "Container", "Chest", "Session", "Mutation"]
    knowledge_package: "Docs/Knowledge/InteractionSystem"
    source_roots:
      - "Source/AegisOdyssey/Interaction"
      - "Source/AegisOdyssey/Inventory"
  - keywords: ["Harvest", "Gather", "Tool", "Harvestable"]
    knowledge_package: "Docs/Knowledge/HarvestSystem"
    source_roots:
      - "Source/AegisOdyssey/Harvest"
      - "Source/AegisOdyssey/Items"
  - keywords: ["GAS", "Replication", "EffectContext", "SmartObject", "Gameplay Framework"]
    knowledge_package: "Docs/Knowledge/GameplayFramework"
    source_roots:
      - "Source/AegisOdyssey/AbilitySystem"
      - "Source/AegisOdyssey/GameFeatures"
      - "Source/AegisOdyssey/GameModes"
  - keywords: ["Reflection", "GC", "CDO", "UObject", "Instancing"]
    knowledge_package: "Docs/Knowledge/EngineCore"
    source_roots:
      - "Source/AegisOdyssey"
  - keywords: ["Crafting", "Workbench", "Recipe"]
    knowledge_package: "Docs/Knowledge/CraftingSystem"
    source_roots:
      - "Source/AegisOdyssey/Crafting"
  - keywords: ["Multiplayer", "Replication", "Network"]
    knowledge_package: "Docs/Knowledge/MultiplayerSystem"
    source_roots:
      - "Source/AegisOdyssey"
```

## 7. Blueprint And Editor Visibility

```yaml
blueprint_visibility:
  source_of_truth: "not fully machine-readable in current adapter"
  content_roots:
    - "Content"
  required_when_changed:
    - "C++ class"
    - "Blueprint parent class"
    - "Editor wiring location"
    - "Runtime trigger path"
```

当前降级规则：

- 如果任务涉及蓝图或编辑器接线，交付时必须补“蓝图交互说明”
- 如果 AI 无法读取蓝图接线真相，不允许假装已经确认
- 测试文档必须补人工验证步骤

## 8. Validation Visibility

```yaml
validation:
  compile_results: "not fixed; provide command/output per task"
  test_results: "Source/AegisOdyssey/TestProject contains automation test sources"
  logs:
    - "Saved"
  automation_tests:
    - "Source/AegisOdyssey/TestProject"
  tools:
    - "Tools/inspect_inventory_component_defaults.py"
```

当前已发现测试源码入口：

- `Source/AegisOdyssey/TestProject/AIDecisionQueueTests.cpp`
- `Source/AegisOdyssey/TestProject/CharacterInventoryInteractionTests.cpp`
- `Source/AegisOdyssey/TestProject/HarvestLifecycleTests.cpp`
- `Source/AegisOdyssey/TestProject/MVVM_CraftingTests.cpp`
- `Source/AegisOdyssey/TestProject/TestHarvestLifecycleActors.cpp`
- `Source/AegisOdyssey/TestProject/TestReplicationPawnComponent.cpp`

每次任务应在交付中明确：

- 是否编译
- 是否运行测试
- 测试结果从哪里来
- 哪些验证暂时只给出人工步骤

## 9. Version Control Visibility

```yaml
version_control:
  system: "git"
  repository_root: ".git"
  history_visibility: "local git status, diff, log, blame when needed"
  required_before_changes:
    - "git status --short"
  required_before_handoff:
    - "git status --short"
    - "git diff -- <changed files>"
```

当前项目已经存在 `.git`，所以 Harness 在进入实现前应先识别工作区状态，避免覆盖用户已有改动。

涉及历史原因、旧方案迁移、知识库归档或回归风险时，应读取相关 `git log` / `git diff` / `git blame`，而不是只根据当前文件内容推断。

## 10. Memory And Routing Rules

```yaml
memory_routing:
  stable_knowledge_goes_to: "Docs/Knowledge"
  stage_handoff_goes_to: "Notice/HistoryNotice if present; otherwise Notice candidate in final handoff"
  default_question: "这句话半年后还应该成立吗？"
```

当前项目的稳定沉淀规则：

- 系统结构、长期边界、当前真实入口进入对应 `Docs/Knowledge/<System>/PROJECT_MAP.md`
- 长期设计决策进入对应 `DECISIONS.md`
- 误判点、历史偏差、故障定位进入对应 `KNOWN_ISSUES.md`
- 跨系统协作方法进入 `Docs/Knowledge/Collaboration`
- 外部参考研究进入 `Docs/Knowledge/ReferenceStudies`

阶段性内容当前处理规则：

- 不直接塞进知识库正文
- 标记为 `Notice candidate`
- 等待项目确认 `Notice/HistoryNotice` 实体落点

## 11. Degradation Rules

```yaml
degradation:
  missing_version_control: "current project has .git; use git history when needed"
  missing_notice_root: "mark stage-only output as Notice candidate; do not write into Knowledge as stable fact"
  missing_blueprint_visibility: "require explicit blueprint/editor wiring note and manual verification steps"
  missing_test_results: "state tests not run or unavailable; provide cold-start manual test steps"
  missing_logs: "state logs unavailable; do not infer runtime truth from code alone"
```

## 12. Adapter Boundary

这个 adapter 示例只记录当前项目的接线事实。

如果后续项目结构变化，应更新本文件；但不能把 AegisOdyssey 的项目特例写回 `ue-harness/core` 作为通用规则。
