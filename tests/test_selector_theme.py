"""Regression tests for the Gen-derived Vita boot-selector theme."""
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "materialize_selector_theme.py"
spec = importlib.util.spec_from_file_location("selector_theme", MODULE_PATH)
selector_theme = importlib.util.module_from_spec(spec)
spec.loader.exec_module(selector_theme)


class SelectorThemeTests(unittest.TestCase):
    def test_split_payload_reconstructs_and_materializes_exact_assets(self):
        source = ROOT / "assets" / "selector"
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            selector_theme.materialize(source, output)
            self.assertEqual(
                {p.name for p in output.iterdir()},
                set(selector_theme.ASSETS),
            )
            for name, (expected_sha, width, height) in selector_theme.ASSETS.items():
                data = (output / name).read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), expected_sha)
                self.assertEqual(selector_theme.png_dimensions(data), (width, height))

    def test_build_and_release_paths_reference_all_selector_assets(self):
        cmake = (ROOT / "tools" / "aot" / "engine" / "vita" / "CMakeLists.txt").read_text()
        release = (ROOT / "tools" / "releases" / "publish_vita.py").read_text()
        for name in selector_theme.ASSETS:
            packaged = "selector/" + name
            self.assertIn(packaged, cmake)
            self.assertIn(packaged, release)
        self.assertIn('set(VITA_VERSION "00.34")', cmake)


if __name__ == "__main__":
    unittest.main()
