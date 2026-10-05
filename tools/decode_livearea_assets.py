#!/usr/bin/env python3
"""Reconstruct and validate the approved PS Vita LiveArea PNGs.

The repository stores the PNG payloads as base64 text so they can be transported
without accidental image re-encoding. This tool writes the exact approved bytes
and refuses to continue if dimensions, PNG mode, transparency, size, or SHA-256
have changed.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import struct
from pathlib import Path

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

ASSETS = {
    "icon0.png": {
        "size": (128, 128),
        "max_bytes": 128 * 1024,
        "sha256": "26ea71975390073a76c69f83d526f039f7ffb96aaad34d0cc1e5203d9541b115",
        "transparent": false,
    },
    "bg0.png": {
        "size": (840, 500),
        "max_bytes": 128 * 1024,
        "sha256": "df12c6104b0b877c9848c2385a3c8d8a4e093ddc3e215169d99811aac4963b0a",
        "transparent": false,
    },
    "startup.png": {
        "size": (280, 158),
        "max_bytes": 128 * 1024,
        "sha256": "0f00cf71705d1a73de8f5a755505dd4d6f6b7d88b1966aa385c70dca6ba31cba",
        "transparent": true,
    },
    "pic0.png": {
        "size": (960, 544),
        "max_bytes": 1024 * 1024,
        "sha256": "17d875946a077684ae62720967941d9d3b82ba3b080d52a3eb7f7e93abfab3ce",
        "transparent": false,
    },
}


def png_metadata(data: bytes) -> tuple[int, int, int, int, bool, bool]:
    if not data.startswith(PNG_SIGNATURE) or len(data) < 33:
        raise ValueError("not a valid PNG")
    ihdr_len = struct.unpack(">I", data[8:12])[0]
    if ihdr_len != 13 or data[12:16] != b"IHDR":
        raise ValueError("PNG does not begin with a valid IHDR")
    width, height, bit_depth, color_type, compression, filtering, interlace = struct.unpack(
        ">IIBBBBB", data[16:29]
    )
    if compression != 0 or filtering != 0 or interlace not in (0, 1):
        raise ValueError("unsupported PNG IHDR flags")
    return width, height, bit_depth, color_type, b"PLTE" in data, b"tRNS" in data


def decode_asset(source_dir: Path, output_dir: Path, name: str, spec: dict) -> None:
    encoded_path = source_dir / "encoded" / f"{name}.b64"
    if not encoded_path.is_file():
        raise FileNotFoundError(f"missing encoded LiveArea asset: {encoded_path}")

    encoded = "".join(encoded_path.read_text(encoding="ascii").split())
    data = base64.b64decode(encoded, validate=True)

    digest = hashlib.sha256(data).hexdigest()
    if digest != spec["sha256"]:
        raise ValueError(f"{name}: SHA-256 mismatch: {digest}")

    if len(data) > spec["max_bytes"]:
        raise ValueError(
            f"{name}: {len(data)} bytes exceeds {spec['max_bytes']} byte limit"
        )

    width, height, bit_depth, color_type, has_palette, has_transparency = png_metadata(data)
    if (width, height) != spec["size"]:
        raise ValueError(
            f"{name}: got {width}x{height}, expected {spec['size'][0]}x{spec['size'][1]}"
        )
    if bit_depth != 8 or color_type != 3 or not has_palette:
        raise ValueError(
            f"{name}: expected indexed 8-bit PNG (color type 3 with PLTE), "
            f"got bit depth {bit_depth}, color type {color_type}, PLTE={has_palette}"
        )
    if has_transparency != spec["transparent"]:
        expected = "with transparency" if spec["transparent"] else "opaque"
        raise ValueError(f"{name}: expected {expected}, tRNS={has_transparency}")

    output_dir.mkdir(parents=True, exist_ok=True)
    out = output_dir / name
    out.write_bytes(data)
    print(
        f"{name}: OK {width}x{height}, indexed 8-bit, {len(data)} bytes, "
        f"sha256={digest}"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    for name, spec in ASSETS.items():
        decode_asset(args.source, args.output, name, spec)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
