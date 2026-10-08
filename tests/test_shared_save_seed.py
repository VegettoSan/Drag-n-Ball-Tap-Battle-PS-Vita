import hashlib
import subprocess
import sys
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).parents[1]
EXPECTED_SIZE = 12906
EXPECTED_SHA256 = "64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb"


class SharedSaveSeedTest(unittest.TestCase):
    def test_seed_materializes_exact_user_save(self):
        with tempfile.TemporaryDirectory() as td:
            output = Path(td) / "save.bin"
            subprocess.run([
                sys.executable,
                str(ROOT / "tools/materialize_default_save.py"),
                str(ROOT / "assets/default_save.bin.zlib.b64"),
                str(output),
            ], check=True)
            data = output.read_bytes()
            self.assertEqual(len(data), EXPECTED_SIZE)
            self.assertEqual(hashlib.sha256(data).hexdigest(), EXPECTED_SHA256)

    def test_vpk_packages_seed_and_runtime_uses_independent_profile_paths(self):
        cmake = (ROOT / "tools/aot/engine/vita/CMakeLists.txt").read_text()
        resources = (ROOT / "tools/aot/engine/native/resources.cpp").read_text()
        self.assertIn('FILE "${DBTB_DEFAULT_SAVE}" save.bin', cmake)
        self.assertIn('save_path = base + "/profiles/" + profile + "/" DBTB_SAVE_BASENAME;', resources)
        self.assertIn('#define DBTB_SAVE_BASENAME "save.bin"', resources)
        self.assertIn('readFile("app0:/save.bin", seed)', resources)
        self.assertNotIn('save_path = base + "/save.bin";', resources)
        self.assertIn('Never overwrite', resources)


if __name__ == "__main__":
    unittest.main()
