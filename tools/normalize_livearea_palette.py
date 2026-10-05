#!/usr/bin/env python3
"""Pad an indexed Vita splash palette to 256 slots without changing pixels."""

import argparse
import struct
import zlib
from pathlib import Path

from decode_livearea_assets import PNG_SIGNATURE, png_chunks, png_metadata


def normalize(data):
    width, height, depth, color, _, alpha = png_metadata(data)
    if (width, height, depth, color, alpha) != (960, 544, 8, 3, False):
        raise ValueError("expected opaque indexed 8-bit 960x544 pic0.png")
    result = bytearray(PNG_SIGNATURE)
    for kind, payload in png_chunks(data):
        if kind == b"PLTE":
            payload = payload.ljust(256 * 3, b"\0")
        result.extend(struct.pack(">I", len(payload)))
        result.extend(kind + payload)
        result.extend(struct.pack(">I", zlib.crc32(kind + payload) & 0xffffffff))
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(normalize(args.input.read_bytes()))


if __name__ == "__main__":
    main()
