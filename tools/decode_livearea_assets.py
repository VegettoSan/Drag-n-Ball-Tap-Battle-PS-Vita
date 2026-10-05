#!/usr/bin/env python3
"""Reconstruct and validate the approved PS Vita LiveArea PNGs."""

import argparse
import base64
import hashlib
import struct
import zlib
from pathlib import Path

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

ASSETS = {
    "icon0.png": {"size": (128, 128), "max_bytes": 128 * 1024, "sha256": "26ea71975390073a76c69f83d526f039f7ffb96aaad34d0cc1e5203d9541b115", "transparent": False},
    "bg0.png": {"size": (840, 500), "max_bytes": 128 * 1024, "sha256": "df12c6104b0b877c9848c2385a3c8d8a4e093ddc3e215169d99811aac4963b0a", "transparent": False},
    "startup.png": {"size": (280, 158), "max_bytes": 128 * 1024, "sha256": "0f00cf71705d1a73de8f5a755505dd4d6f6b7d88b1966aa385c70dca6ba31cba", "transparent": True},
    "pic0.png": {"size": (960, 544), "max_bytes": 1024 * 1024, "sha256": "84d530e2def6332bddc817bbc5256192173b11928c21382bbeddcfa89bfae83a", "transparent": False},
}

def png_chunks(data):
    """Parse real chunks and check CRCs; byte-substring searches are unsafe."""
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("not a valid PNG")
    pos = len(PNG_SIGNATURE)
    chunks = []
    while pos < len(data):
        if pos + 12 > len(data):
            raise ValueError("truncated PNG chunk")
        length = struct.unpack_from(">I", data, pos)[0]
        end = pos + length + 12
        if end > len(data):
            raise ValueError("truncated PNG chunk payload")
        kind = data[pos + 4:pos + 8]
        payload = data[pos + 8:end - 4]
        crc = struct.unpack_from(">I", data, end - 4)[0]
        if crc != zlib.crc32(kind + payload) & 0xffffffff:
            raise ValueError("invalid PNG CRC: " + kind.decode("ascii"))
        chunks.append((kind, payload))
        pos = end
        if kind == b"IEND":
            if payload or pos != len(data):
                raise ValueError("invalid PNG end/trailing bytes")
            break
    if not chunks or chunks[-1][0] != b"IEND":
        raise ValueError("missing PNG IEND")
    return chunks


def png_metadata(data):
    if not data.startswith(PNG_SIGNATURE) or len(data) < 33:
        raise ValueError("not a valid PNG")
    if struct.unpack(">I", data[8:12])[0] != 13 or data[12:16] != b"IHDR":
        raise ValueError("PNG does not begin with a valid IHDR")
    width, height, bit_depth, color_type, compression, filtering, interlace = struct.unpack(">IIBBBBB", data[16:29])
    if compression != 0 or filtering != 0 or interlace != 0:
        raise ValueError("unsupported PNG IHDR flags")
    chunks = png_chunks(data)
    kinds = [kind for kind, _ in chunks]
    if kinds.count(b"IHDR") != 1 or kinds.count(b"PLTE") != 1 or b"IDAT" not in kinds:
        raise ValueError("missing/duplicate indexed PNG structural chunks")
    if kinds.index(b"PLTE") > kinds.index(b"IDAT"):
        raise ValueError("PNG palette must precede image data")
    palette = dict(chunks)[b"PLTE"]
    if not palette or len(palette) % 3 or len(palette) > 768:
        raise ValueError("invalid PNG palette length")
    if kinds.count(b"tRNS") > 1:
        raise ValueError("duplicate PNG transparency chunk")
    if b"tRNS" in kinds:
        alpha = dict(chunks)[b"tRNS"]
        if not alpha or len(alpha) > len(palette) // 3 or not kinds.index(b"PLTE") < kinds.index(b"tRNS") < kinds.index(b"IDAT"):
            raise ValueError("invalid PNG palette transparency")
    return width, height, bit_depth, color_type, True, b"tRNS" in kinds


def validate_png(data, name, spec, check_hash=True):
    if len(data) > spec["max_bytes"]:
        raise ValueError(name + ": exceeds LiveArea byte limit")
    width, height, bit_depth, color_type, has_palette, has_transparency = png_metadata(data)
    if (width, height) != spec["size"]:
        raise ValueError(name + ": unexpected image dimensions")
    if bit_depth != 8 or color_type != 3 or not has_palette:
        raise ValueError(name + ": expected indexed 8-bit PNG")
    if has_transparency != spec["transparent"]:
        raise ValueError(name + ": transparency contract mismatch")
    chunks = png_chunks(data)
    if name == "pic0.png" and len(dict(chunks)[b"PLTE"]) != 256 * 3:
        raise ValueError(name + ": Vita splash requires exactly 256 palette entries")
    decoder = zlib.decompressobj()
    expected = height * (width + 1)
    pixels = decoder.decompress(b"".join(payload for kind, payload in chunks if kind == b"IDAT"), expected + 1)
    if len(pixels) != expected or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError(name + ": invalid PNG image stream")
    if any(pixels[row * (width + 1)] > 4 for row in range(height)):
        raise ValueError(name + ": invalid PNG scanline filter")
    digest = hashlib.sha256(data).hexdigest()
    if check_hash and digest != spec["sha256"]:
        raise ValueError(name + ": SHA-256 mismatch: " + digest)
    return digest

def decode_asset(source_dir, output_dir, name, spec):
    encoded_path = source_dir / "encoded" / (name + ".b64")
    if not encoded_path.is_file():
        raise FileNotFoundError("missing encoded LiveArea asset: " + str(encoded_path))
    encoded = "".join(encoded_path.read_text(encoding="ascii").split())
    data = base64.b64decode(encoded, validate=True)
    digest = validate_png(data, name, spec)
    width, height = spec["size"]
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
