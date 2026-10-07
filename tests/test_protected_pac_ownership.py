from pathlib import Path
import unittest

ROOT = Path(__file__).parents[1]


class ProtectedPacOwnershipTest(unittest.TestCase):
    def test_changed_pac_transfers_normalized_buffer_without_copy(self):
        source = (ROOT / "src/engine_resources.cpp").read_text()
        self.assertIn("if (changed) output.swap(out);", source)
        self.assertIn("else output = input;", source)
        self.assertNotIn("output = changed ? std::move(out) : input;", source)


if __name__ == "__main__":
    unittest.main()
