"""Validate UE Harness Step 4 delivery assets.

The script is intentionally read-only. It checks that templates, checklist,
and safety scripts exist and contain the anchors needed by the Harness loop.
"""

from __future__ import annotations

import argparse
from pathlib import Path


REQUIRED_ASSETS: dict[str, tuple[str, ...]] = {
    "templates/rules-template.md": (
        "# Rules Template",
        "## 1. Rule Identity",
        "## 2. Hard Rules",
        "## 3. Documentation Rules",
        "## 4. Validation Rules",
        "## 5. Memory Rules",
    ),
    "templates/tech-doc-template.md": (
        "# Tech Doc Template",
        "## 2. 最小交付范围",
        "## 3. 上下文读取",
        "## 5. 调用链",
        "## 7. 验证与证据",
        "## 9. 记忆回流",
    ),
    "templates/test-doc-template.md": (
        "# Test Doc Template",
        "## 3. 冷启动测试步骤",
        "## 4. 预期结果",
        "## 5. 实际结果",
        "## 8. 未覆盖项",
        "## 9. 后续沉淀",
    ),
    "templates/memory-log-template.md": (
        "# Memory Log Template",
        "## 1. Memory Identity",
        "## 3. 新增稳定事实",
        "## 4. 阶段性提醒",
        "## 6. 规则升级候选",
        "## 7. 下一轮读取建议",
    ),
    "templates/delivery-checklist.md": (
        "# Delivery Checklist",
        "## 1. Context",
        "## 2. Implementation",
        "## 3. Documentation",
        "## 4. Validation",
        "## 5. Memory",
        "## 6. Handoff",
    ),
    "scripts/validate_adapter.py": (
        "REQUIRED_SECTIONS",
        "adapter validation passed",
    ),
    "scripts/validate_delivery.py": (
        "REQUIRED_ASSETS",
        "delivery assets validation passed",
    ),
}


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError(f"file is not valid UTF-8: {path}") from exc


def validate_assets(root: Path) -> list[str]:
    errors: list[str] = []

    for relative_path, required_tokens in REQUIRED_ASSETS.items():
        path = root / relative_path
        if not path.exists():
            errors.append(f"missing required asset: {relative_path}")
            continue
        if not path.is_file():
            errors.append(f"required asset is not a file: {relative_path}")
            continue

        try:
            text = read_text(path)
        except ValueError as exc:
            errors.append(str(exc))
            continue

        for token in required_tokens:
            if token not in text:
                errors.append(f"asset {relative_path} missing required token: {token}")

    return errors


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Validate UE Harness templates, checklist, and safe scripts."
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="UE Harness package root. Defaults to the parent of scripts/.",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    root = args.root.resolve()

    errors = validate_assets(root)
    if errors:
        for error in errors:
            print(error)
        return 1

    print(f"delivery assets validation passed: {root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
