import sys
from pathlib import Path
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import prepare_samu_mod as samu


class SamuPrepareTests(unittest.TestCase):
    def test_audio_magic(self):
        self.assertEqual(samu.audio_codec(b"OggS" + b"\0" * 20 + b"\x01vorbis"), "vorbis")
        self.assertEqual(samu.audio_codec(b"ID3\x04\0\0" + b"\0" * 20), "mp3")
        self.assertEqual(samu.audio_codec(bytes([0xFF, 0xFB]) + b"\0" * 30), "mp3")
        self.assertEqual(samu.audio_codec(b"\0\0\0\x18ftypM4A " + b"\0" * 20), "aac/m4a")
        self.assertEqual(samu.audio_codec(b"not audio"), "unknown")

    def test_audited_codec_matrix(self):
        self.assertEqual(len(samu.EXPECTED_CODECS), 17)
        self.assertEqual(sum(v == "vorbis" for v in samu.EXPECTED_CODECS.values()), 2)
        self.assertEqual(sum(v == "mp3" for v in samu.EXPECTED_CODECS.values()), 12)
        self.assertEqual(sum(v == "aac/m4a" for v in samu.EXPECTED_CODECS.values()), 3)

    def test_roster_00_through_91_and_reject_unexpected_extension(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for index in range(samu.SAMU_CHARACTER_COUNT):
                for name in (f"char{index:02d}.pac", f"chardemo{index:02d}.pac",
                             f"charf{index:04d}.pac"):
                    (root / name).write_bytes(b"\0\0")
            samu.validate_roster(root)
            (root / "char92.pac").write_bytes(b"\0\0")
            with self.assertRaisesRegex(ValueError, "beyond the audited Samu"):
                samu.validate_roster(root)


if __name__ == "__main__":
    unittest.main()
