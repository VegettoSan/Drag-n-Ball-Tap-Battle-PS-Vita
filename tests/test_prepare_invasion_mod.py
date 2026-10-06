import sys
from pathlib import Path
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import prepare_invasion_mod as invasion


class InvasionPrepareTests(unittest.TestCase):
    def test_audio_magic(self):
        self.assertEqual(invasion.audio_codec(b"OggS" + b"\0" * 20 + b"\x01vorbis"), "vorbis")
        self.assertEqual(invasion.audio_codec(b"ID3\x04\0\0" + b"\0" * 20), "mp3")
        self.assertEqual(invasion.audio_codec(bytes([0xFF, 0xFB]) + b"\0" * 30), "mp3")
        self.assertEqual(invasion.audio_codec(b"\0\0\0\x18ftypM4A " + b"\0" * 20), "aac/m4a")
        self.assertEqual(invasion.audio_codec(b"not audio"), "unknown")

    def test_audited_codec_matrix(self):
        self.assertEqual(len(invasion.EXPECTED_CODECS), 17)
        self.assertEqual(sum(v == "vorbis" for v in invasion.EXPECTED_CODECS.values()), 10)
        self.assertEqual(sum(v == "mp3" for v in invasion.EXPECTED_CODECS.values()), 5)
        self.assertEqual(sum(v == "aac/m4a" for v in invasion.EXPECTED_CODECS.values()), 2)

    def test_roster_00_through_21(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for index in range(invasion.INVASION_CHARACTER_COUNT):
                for name in (f"char{index:02d}.pac", f"chardemo{index:02d}.pac",
                             f"charf{index:04d}.pac"):
                    (root / name).write_bytes(b"\0\0")
            invasion.validate_roster(root)
            (root / "char22.pac").write_bytes(b"\0\0")
            with self.assertRaisesRegex(ValueError, "beyond the audited Invasion"):
                invasion.validate_roster(root)


if __name__ == "__main__":
    unittest.main()
