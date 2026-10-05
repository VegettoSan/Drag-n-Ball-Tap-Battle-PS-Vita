#!/usr/bin/env python3
"""Reconstruct and validate the approved PS Vita LiveArea PNGs."""

import argparse
import base64
import hashlib
import struct
from pathlib import Path

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

ASSETS = {
    "icon0.png": {"size": (128, 128), "max_bytes": 128 * 1024, "sha256": "26ea71975390073a76c69f83d526f039f7ffb96aaad34d0cc1e5203d9541b115", "transparent": False},
    "bg0.png": {"size": (840, 500), "max_bytes": 128 * 1024, "sha256": "df12c6104b0b877c9848c2385a3c8d8a4e093ddc3e215169d99811aac4963b0a", "transparent": False},
    "startup.png": {"size": (280, 158), "max_bytes": 128 * 1024, "sha256": "0f00cf71705d1a73de8f5a755505dd4d6f6b7d88b1966aa385c70dca6ba31cba", "transparent": True},
    "pic0.png": {"size": (960, 544), "max_bytes": 1024 * 1024, "sha256": "17d875946a077684ae62720967941d9d3b82ba3b080d52a3eb7f7e93abfab3ce", "transparent": False},
}

def png_metadata(data):
    if not data.startswith(PNG_SIGNATURE) or len(data) < 33:
        raise ValueError("not a valid PNG")
    if struct.unpack(">I", data[8:12])[0] != 13 or data[12:16] != b"IHDR":
        raise ValueError("PNG does not begin with a valid IHDR")
    width, height, bit_depth, color_type, compression, filtering, interlace = struct.unpack(">IIBBBBB", data[16:29])
    if compression != 0 or filtering != 0 or interlace not in (0, 1):
        raise ValueError("unsupported PNG IHDR flags")
    return width, height, bit_depth, color_type, b"PLTE" in data, b"tRNS" in data

def decode_asset(source_dir, output_dir, name, spec):
    encoded_path = source_dir / "encoded" / (name + ".b64")
    if not encoded_path.is_file():
        raise FileNotFoundError("missing encoded LiveArea asset: " + str(encoded_path))
    encoded = "".join(encoded_path.read_text(encoding="ascii").split())
    data = base64.b64decode(encoded, validate=True)
    digest = hashlib.sha256(data).hexdigest()
    if digest != spec["sha256"]:
        raise ValueError(name + ": SHA-256 mismatch: " + digest)
    if len(data) > spec["max_bytes"]:
        raise ValueError(name + ": exceeds LiveArea byte limit")
    width, height, bit_depth, color_type, has_palette, has_transparency = png_metadata(data)
    if (width, height) != spec["size"]:
        raise ValueError("%s: got %dx%d, expected %dx%d" % (name, width, height, spec["size"][0], spec["size"][1]))
    if bit_depth != 8 or color_type != 3 or not has_palette:
        raise ValueError(name + ": expected indexed 8-bit PNG (color type 3 with PLTE)")
    if has_transparency != spec["transparent"]:
        raise ValueError(name + ": transparency contract mismatch")
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / name).write_bytes(data)
    print("%s: OK %dx%d, indexed 8-bit, %d bytes, sha256=%s" % (name, width, height, len(data), digest))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    for name, spec in ASSETS.items():
        decode_asset(args.source, args.output, name, spec)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
