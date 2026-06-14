"""Validate a concrete UE Harness Project Adapter.

The script is intentionally read-only. It checks structure and required
visibility sections, but it does not infer project truth or modify files.
"""

from __future__ import annotations

import argparse
from pathlib import Path


REQUIRED_SECTIONS = (
    "## 1. Project Identity",
    "## 2. Source And Build Visibility",
    "## 3. Knowledge Visibility",
    "## 4. Notice Visibility",
    "## 5. Required Read Order",
    "## 6. System Package Routing",
    "## 7. Blueprint And Editor Visibility",
    "## 8. Validation Visibility",
    "## 9. Version Control Visibility",
    "## 10. Memory And Routing Rules",
    "## 11. Degradation Rules",
)

REQUIRED_TOKENS = (
    "project_name:",
    "knowledge_root:",
    "notice_root:",
    "required_harness_files:",
    "blueprint_visibility:",
    "validation:",
    "version_control:",
    "memory_routing:",
    "degradation:",
)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError(f"file is not valid UTF-8: {path}") from exc


def validate_adapter(path: Path) -> list[str]:
    errors: list[str] = []

    if not path.exists():
        return [f"adapter file does not exist: {path}"]
    if not path.is_file():
        return [f"adapter path is not a file: {path}"]

    try:
        text = read_text(path)
    except ValueError as exc:
        return [str(exc)]

    for section in REQUIRED_SECTIONS:
        if section not in text:
            errors.append(f"missing required section: {section}")

    for token in REQUIRED_TOKENS:
        if token not in text:
            errors.append(f"missing required token: {token}")

    return errors


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Validate a UE Harness Project Adapter markdown file."
    )
    parser.add_argument("adapter", type=Path, help="Adapter markdown file to validate.")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    errors = validate_adapter(args.adapter)
    if errors:
        for error in errors:
            print(error)
        return 1

    print(f"adapter validation passed: {args.adapter}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
