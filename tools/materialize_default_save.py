#!/usr/bin/env python3
"""Materialize the audited shared save seed used by the Vita VPK.

The repository keeps the user-provided 12,906-byte save as a tiny zlib+base64
text payload so Git history remains text-friendly. Build packaging recreates
save.bin byte-for-byte and verifies its SHA-256 before placing it in app0:.
"""
from __future__ import annotations

import base64
import hashlib
from pathlib import Path
import sys
import zlib

EXPECTED_SIZE = 12906
EXPECTED_SHA256 = "64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb"


def materialize(source: Path, target: Path) -> None:
    encoded = source.read_text(encoding="ascii").strip()
    data = zlib.decompress(base64.b64decode(encoded, validate=True))
    if len(data) != EXPECTED_SIZE:
        raise SystemExit(f"default save size mismatch: {len(data)} != {EXPECTED_SIZE}")
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA256:
        raise SystemExit(f"default save SHA-256 mismatch: {digest}")
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: materialize_default_save.py SOURCE.zlib.b64 TARGET.bin", file=sys.stderr)
        return 2
    materialize(Path(sys.argv[1]), Path(sys.argv[2]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
