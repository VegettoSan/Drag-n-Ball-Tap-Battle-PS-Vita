#!/usr/bin/env python3
"""Extract user-owned Dragon Ball Tap Battle runtime data from an APK.

The port intentionally keeps original copyrighted data outside Git and outside
its VPK. This tool copies the APK's res/raw files into a Vita-ready `game/`
directory without modifying PAC/OGG/etc. contents.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys
import zipfile

RAW_PREFIX = "res/raw/"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract Tap Battle res/raw data from an APK")
    parser.add_argument("apk", type=Path, help="Path to a legally obtained Tap Battle APK")
    parser.add_argument("output", type=Path, help="Output directory (copy its contents to ux0:data/DBTapBattle/game/)")
    parser.add_argument("--overwrite", action="store_true", help="Allow replacing files already present in output")
    args = parser.parse_args()

    if not args.apk.is_file():
        print(f"error: APK not found: {args.apk}", file=sys.stderr)
        return 2

    args.output.mkdir(parents=True, exist_ok=True)

    try:
        archive = zipfile.ZipFile(args.apk, "r")
    except (zipfile.BadZipFile, OSError) as exc:
        print(f"error: could not open APK as ZIP: {exc}", file=sys.stderr)
        return 3

    manifest_files = []
    extracted = 0

    with archive:
        names = sorted(
            name for name in archive.namelist()
            if name.startswith(RAW_PREFIX) and not name.endswith("/")
        )

        if not names:
            print("error: APK contains no res/raw files", file=sys.stderr)
            return 4

        for archive_name in names:
            relative = archive_name[len(RAW_PREFIX):]
            if not relative or "/" in relative or "\\" in relative or relative in {".", ".."}:
                print(f"warning: skipping unexpected raw path: {archive_name}")
                continue

            destination = args.output / relative
            if destination.exists() and not args.overwrite:
                print(f"error: destination exists (use --overwrite): {destination}", file=sys.stderr)
                return 5

            data = archive.read(archive_name)
            destination.write_bytes(data)
            extracted += 1
            manifest_files.append({
                "name": relative,
                "size": len(data),
                "sha256": sha256_bytes(data),
                "apk_path": archive_name,
            })
            print(f"extracted {relative} ({len(data)} bytes)")

    manifest = {
        "format": 1,
        "source_apk": args.apk.name,
        "source_apk_sha256": sha256_bytes(args.apk.read_bytes()),
        "file_count": extracted,
        "files": manifest_files,
    }
    (args.output / "dbtb_manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    print()
    print(f"Done: {extracted} files -> {args.output}")
    print("Copy this directory's contents to ux0:data/DBTapBattle/game/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
