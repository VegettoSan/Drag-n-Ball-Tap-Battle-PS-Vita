#!/usr/bin/env python3
"""Materialize the embedded Gen/select0.pac selector theme for Vita builds.

The repository stores one validated ZIP as split Base64 text so Git remains
friendly to review. This script reconstructs exactly four derived PNG assets.
No APK or PAC is written to the VPK.
"""
import argparse
import base64
import binascii
import hashlib
import io
from pathlib import Path
import struct
import zipfile

ZIP_SHA256 = "90418a27c6681ee644d5cc383e31fc73248a5c412527839d216b61bcc2516c12"
PART_NAMES = [f"gen_select0_theme.zip.b64.part{i:02d}" for i in range(5)]
ASSETS = {
    "select0_background.png": ("9af2f1674a5b4fc2e84031d24a13b7f89e2da4dad239e7e02f0f4e7f330a096c", 512, 512),
    "select0_header.png": ("809427746e13e1be27ea038ddc400de3e4452125fdab75fd9601dc7d44b95c2b", 467, 33),
    "select0_button.png": ("792a105d8bada00e77bcd483e32417d7f9589d9bbad0db9804188665cea49287", 260, 36),
    "select0_ball_1.png": ("a164d97d1883919e5e3fff0bbf31e3bc70afa920de55275c4f5c601c22ee2592", 52, 52),
}


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def png_dimensions(data):
    if len(data) < 24 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise ValueError("selector asset is not a valid PNG header")
    return struct.unpack(">II", data[16:24])


def reconstruct(source_dir):
    actual = sorted(p.name for p in source_dir.glob("gen_select0_theme.zip.b64.part*"))
    if actual != PART_NAMES:
        raise ValueError(f"selector theme parts mismatch: expected {PART_NAMES}, got {actual}")
    encoded = "".join((source_dir / name).read_text(encoding="ascii").strip() for name in PART_NAMES)
    try:
        raw = base64.b64decode(encoded, validate=True)
    except (binascii.Error, ValueError) as exc:
        raise ValueError("selector theme Base64 is invalid") from exc
    if sha256(raw) != ZIP_SHA256:
        raise ValueError("selector theme ZIP SHA-256 mismatch")
    return raw


def materialize(source_dir, output_dir):
    raw = reconstruct(source_dir)
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)) or set(names) != set(ASSETS) or archive.testzip():
            raise ValueError("selector theme ZIP layout/CRC mismatch")
        output_dir.mkdir(parents=True, exist_ok=True)
        for name, (expected_sha, width, height) in ASSETS.items():
            data = archive.read(name)
            if sha256(data) != expected_sha:
                raise ValueError(f"selector asset SHA-256 mismatch: {name}")
            if png_dimensions(data) != (width, height):
                raise ValueError(f"selector asset dimensions mismatch: {name}")
            target = output_dir / name
            temp = output_dir / (name + ".tmp")
            temp.write_bytes(data)
            temp.replace(target)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source_dir", type=Path)
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()
    materialize(args.source_dir, args.output_dir)
    print(f"Materialized {len(ASSETS)} selector assets in {args.output_dir}")


if __name__ == "__main__":
    main()
