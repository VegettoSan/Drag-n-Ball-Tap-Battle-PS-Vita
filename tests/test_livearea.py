"""Regression checks for the palette bug missed by the old VPK validator."""

import struct
import sys
import tempfile
import unittest
import zipfile
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from decode_livearea_assets import ASSETS, PNG_SIGNATURE, validate_png
from normalize_livearea_palette import normalize
from validate_livearea_vpk import validate_base_identity


def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)


def splash():
    return (PNG_SIGNATURE
            + chunk(b"IHDR", struct.pack(">IIBBBBB", 960, 544, 8, 3, 0, 0, 0))
            + chunk(b"PLTE", bytes(range(192)) * 3)
            + chunk(b"IDAT", zlib.compress(b"\0" * (544 * 961)))
            + chunk(b"IEND", b""))


class LiveAreaRegression(unittest.TestCase):
    def test_192_entry_splash_rejected_and_lossless_padding_accepted(self):
        original = splash()
        with self.assertRaisesRegex(ValueError, "exactly 256"):
            validate_png(original, "pic0.png", ASSETS["pic0.png"], check_hash=False)
        fixed = normalize(original)
        validate_png(fixed, "pic0.png", ASSETS["pic0.png"], check_hash=False)
        self.assertEqual(normalize(fixed), fixed)
        # Only the PLTE length, padded slots and its CRC change; IDAT is retained.
        self.assertEqual(original[original.index(b"IDAT") - 4:], fixed[fixed.index(b"IDAT") - 4:])

    def test_palette_crc_corruption_rejected(self):
        damaged = bytearray(normalize(splash()))
        damaged[damaged.index(b"PLTE") + 4] ^= 1
        with self.assertRaisesRegex(ValueError, "CRC"):
            validate_png(bytes(damaged), "pic0.png", ASSETS["pic0.png"], check_hash=False)

    def test_repack_rejects_substituted_executable(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory) / "base.vpk"
            output = Path(directory) / "output.vpk"
            for path, executable in [(base, b"playable engine"), (output, b"native link probe")]:
                with zipfile.ZipFile(path, "w") as archive:
                    archive.writestr("eboot.bin", executable)
                    archive.writestr("sce_sys/param.sfo", b"same metadata")
            with self.assertRaisesRegex(ValueError, "eboot.bin"):
                validate_base_identity(output, base)
            validate_base_identity(base, base)


if __name__ == "__main__":
    unittest.main()
