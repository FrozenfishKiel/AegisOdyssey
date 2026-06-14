from __future__ import annotations

import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / "scripts"


class ValidateAdapterTests(unittest.TestCase):
    def run_adapter(self, adapter_path: Path) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPTS / "validate_adapter.py"), str(adapter_path)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )

    def test_accepts_aegis_adapter_example(self) -> None:
        result = self.run_adapter(ROOT / "adapters" / "examples" / "aegis-odyssey.md")

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("adapter validation passed", result.stdout)

    def test_rejects_adapter_missing_required_sections(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            bad_adapter = Path(tmp) / "bad-adapter.md"
            bad_adapter.write_text("# Bad Adapter\n\n## 1. Project Identity\n", encoding="utf-8")

            result = self.run_adapter(bad_adapter)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing required section", result.stdout + result.stderr)


class ValidateDeliveryTests(unittest.TestCase):
    def run_delivery(self, root: Path) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPTS / "validate_delivery.py"), "--root", str(root)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )

    def test_accepts_current_harness_assets(self) -> None:
        result = self.run_delivery(ROOT)

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("delivery assets validation passed", result.stdout)

    def test_rejects_missing_template_asset(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "templates").mkdir()
            (root / "scripts").mkdir()
            (root / "templates" / "rules-template.md").write_text(
                textwrap.dedent(
                    """\
                    # Rules Template

                    ## 1. Rule Identity
                    """
                ),
                encoding="utf-8",
            )

            result = self.run_delivery(root)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing required asset", result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
