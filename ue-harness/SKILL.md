---
name: ue-harness
description: Use when working on Unreal Engine project tasks, UE C++ or Blueprint changes, editor wiring, UE technical documentation, validation, project knowledge updates, or multi-agent execution where Harness discipline should be followed.
---

# UE Harness

Use this as the entry point for UE development tasks. The goal is not to make a longer prompt; the goal is to make the task enter Harness before execution.

Harness means the agent works inside rules, project visibility, validation, memory, version history, and a development self-loop.

## Entry Gate

Before implementation, read only the files needed for the task, but do not skip the gate:

1. Read `core/harness-definition.md` to keep the definition aligned.
2. Read `core/rule.md` and `core/skill-task-flow.md` for the hard rules and task loop.
3. Read `core/script-validation.md`, `core/mcp-project-visibility.md`, `core/memory.md`, `core/version-control.md`, and `core/development-self-loop.md` for validation, visibility, memory, history, and loop requirements.
4. Choose the current project adapter from `adapters/`; for this repo, use `adapters/examples/aegis-odyssey.md`.
5. Read the relevant `Knowledge` package from the adapter routing before editing code or writing documents.

If a required context source is missing, say which Harness layer is missing and follow the adapter degradation rule. Do not silently continue as if the context was available.

## Direct Execution Mode

When the user explicitly asks to "直接改", "直接开干", "不要停", "先把问题解决", "continue fixing", or otherwise makes execution speed more important than process narration, Harness enters Direct Execution Mode.

In Direct Execution Mode:

1. Do not stop before implementation just to present a plan, ask for confirmation, wait for subagents, or complete documentation ceremony.
2. Keep the entry gate minimal: `git status --short`, the adapter identity if already known, and only the code/Knowledge files needed for the immediate edit.
3. Execute the smallest complete code closure first, then report validation degradation at handoff.
4. Missing subagents, missing docs templates, unavailable compile tools, or unavailable editor visibility are not pre-implementation blockers. They are final handoff notes unless they make safe code editing impossible.
5. If a user change directly conflicts with the intended edit, stop and ask. Otherwise continue.

Harness should help the agent move safely; it must not become a process lock that prevents code from being changed.

## Task Flow

For every UE task:

1. Identify the minimum delivery.
2. Read the adapter, relevant rules, relevant Knowledge files, and current version state.
3. Map affected C++ modules, Blueprint/editor wiring, tests, logs, docs, and memory targets.
4. Execute the smallest safe change.
5. Verify with scripts, compile/test output, logs, or explicit manual steps.
6. Update technical docs, test docs, or memory only when the output belongs there.
7. Use `templates/delivery-checklist.md` before handoff.

Do not claim completion if validation is missing. State the degradation instead.

For deletion-heavy refactors or responsibility-boundary simplifications, add a Deleted Symbol Checklist before handoff. Search for removed types, fields, functions, config keys, Blueprint-facing names, and log labels with `rg`. Do not consider the code path closed while removed source symbols still appear in affected source roots, except for intentionally retained lower-layer runtime types.

If `apply_patch` fails because of invalid UTF-8, mojibake comments, or context drift, do not retry the same fragile patch repeatedly. After one failed retry, switch to a safer strategy:

1. Rebuild the file from a clean source such as `git show HEAD:path` when appropriate.
2. Apply only the necessary semantic edits line-by-line.
3. Re-run scans for swallowed `UPROPERTY` / `UFUNCTION` macros and deleted symbols.
4. Never leave a header where comments consume Unreal reflection macros or declarations.

## Execution Strategy

Multi-agent support is optional. SubAgent 只是执行策略，不能把多 agent 当成 Harness 本体。

When multi-agent execution is available:

- 主 agent 是唯一代码语义写入者。
- SubAgents may inspect, comment, draft docs, draft tests, run checks, and propose memory updates.
- Cheaper models may handle comments, documentation, test-step drafting, checklist review, and archive suggestions.
- Higher-capability models should handle requirement judgment, architecture, code implementation, conflict resolution, and final synthesis.

When multi-agent execution is unavailable, the main agent must simulate the same perspectives in order: requirement, design, implementation, validation, documentation, memory.

Read `core/multi-agent-strategy.md` before delegating or simulating delegation.

## Validation

Use the safe scripts when applicable:

```powershell
python scripts/validate_adapter.py adapters/examples/aegis-odyssey.md
python scripts/validate_delivery.py --root .
```

These scripts are read-only structural checks. They do not replace UE compile, automation tests, editor validation, Blueprint wiring checks, or log review.

## Documentation And Memory

Use the templates as assets, not as the Harness definition:

- `templates/tech-doc-template.md` for technical docs.
- `templates/test-doc-template.md` for cold-start validation docs.
- `templates/memory-log-template.md` for memory回流.
- `templates/rules-template.md` when creating project or subsystem rules.
- `templates/delivery-checklist.md` before handoff.

Routing rule:

- Stable facts that should still hold months later go to `Knowledge`.
- Stage handoff, temporary judgment, unresolved context, or historical notes go to `Notice`.
- If the project has no concrete Notice location, mark the item as `Notice candidate` instead of writing it into stable Knowledge.

## Stop Conditions

Stop and ask or report degradation when:

- The adapter cannot identify the project context.
- Code, Blueprint wiring, tests, logs, or version history needed for the task are unavailable.
- A user change conflicts with the intended edit.
- Verification cannot be run and no safe manual verification path can be written.

Stopping at a missing Harness layer is part of Harness discipline; it is not a failure of execution.

Exception: in Direct Execution Mode, missing validation tools are not a stop condition after safe code edits are possible. Continue with static validation and report exactly which compile/test step could not run.
