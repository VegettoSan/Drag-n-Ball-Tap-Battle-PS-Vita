#!/usr/bin/env python3
"""Prepare the audited TAP BATTLE INVASION BETA 3 APK for Vita.

Protected PAC filenames are canonicalized by the existing Community14 extractor,
but payload bytes are never transcoded or rewritten. BGM files keep their
original bytes even when the .ogg name contains MP3 or AAC/M4A; the 00.27 Vita
runtime detects the actual codec from content and decodes it directly.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import zipfile

from extract_apk_data import MANIFEST, extract, file_hash

INVASION_APK_SHA256 = "caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d"
INVASION_DEX_SHA256 = "05aa0c5ec839161e59b93eccd8657925380c56b46f1a4a452c662b1f115212d1"
INVASION_PROFILE = "community14-invasion-05aa0c5e"
INVASION_CHARACTER_COUNT = 22
EXPECTED_CODECS = {
    **{f"bgm_{i:02d}.ogg": "vorbis" for i in [0, 1, 2, 8, 9, 10, 11, 12, 13, 16]},
    **{f"bgm_{i:02d}.ogg": "mp3" for i in [3, 6, 7, 14, 15]},
    **{f"bgm_{i:02d}.ogg": "aac/m4a" for i in [4, 5]},
}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def audio_codec(data: bytes) -> str:
    head = data[:64]
    if head.startswith(b"OggS") and b"vorbis" in head:
        return "vorbis"
    if head.startswith(b"ID3") or (
        len(head) >= 2 and head[0] == 0xFF and (head[1] & 0xE0) == 0xE0
    ):
        return "mp3"
    if len(head) >= 12 and head[4:8] == b"ftyp":
        return "aac/m4a"
    return "unknown"


def verify_source(apk: Path) -> None:
    observed = file_hash(apk)
    if observed != INVASION_APK_SHA256:
        raise ValueError(
            "This helper is pinned to the audited TAP BATTLE INVASION BETA 3.apk; "
            f"expected {INVASION_APK_SHA256}, got {observed}"
        )
    with zipfile.ZipFile(apk) as archive:
        try:
            dex = archive.read("classes.dex")
        except KeyError as exc:
            raise ValueError("classes.dex is missing") from exc
    dex_sha = sha256_bytes(dex)
    if dex_sha != INVASION_DEX_SHA256:
        raise ValueError(f"unexpected Invasion classes.dex hash: {dex_sha}")


def validate_roster(root: Path) -> None:
    for index in range(INVASION_CHARACTER_COUNT):
        for name in (
            f"char{index:02d}.pac",
            f"chardemo{index:02d}.pac",
            f"charf{index:04d}.pac",
        ):
            if not (root / name).is_file():
                raise ValueError(f"incomplete Invasion character triplet at {index:02d}: {name}")
    if (root / "char22.pac").exists() or (root / "chardemo22.pac").exists() or (root / "charf0022.pac").exists():
        raise ValueError("unexpected character data beyond the audited Invasion 00..21 roster")


def validate_audio(root: Path) -> list[dict]:
    result = []
    for name, expected in EXPECTED_CODECS.items():
        path = root / name
        if not path.is_file():
            raise ValueError(f"missing Invasion BGM: {name}")
        observed = audio_codec(path.read_bytes()[:64])
        if observed != expected:
            raise ValueError(f"unexpected codec for {name}: expected {expected}, got {observed}")
        result.append({
            "name": name,
            "codec": observed,
            "size": path.stat().st_size,
            "sha256": file_hash(path),
            "runtime": "direct-content-detection",
        })
    return result


def update_manifest(root: Path, audio: list[dict]) -> None:
    path = root / MANIFEST
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if manifest.get("source_layout") != "community14":
        raise ValueError("Invasion extraction did not select the protected Community14 layout")
    if manifest.get("pac_codec") != INVASION_PROFILE:
        raise ValueError(f"unexpected Invasion PAC profile: {manifest.get('pac_codec')}")
    if manifest.get("payloads_unchanged") is not True:
        raise ValueError("Invasion preparation must not transform source payload bytes")

    by_name = {entry["name"]: entry for entry in manifest["files"]}
    for item in audio:
        source = by_name.get(item["name"])
        if source is None or source["size"] != item["size"] or source["sha256"] != item["sha256"]:
            raise ValueError(f"manifest/source mismatch for {item['name']}")

    manifest["prepared_profile"] = {
        "name": INVASION_PROFILE,
        "apk_sha256": INVASION_APK_SHA256,
        "classes_dex_sha256": INVASION_DEX_SHA256,
        "character_indices": "00..21",
        "character_count": INVASION_CHARACTER_COUNT,
        "valid_omissions": ["bobj00.pac", "font00.pac"],
    }
    manifest["audio_runtime"] = {
        "policy": "preserve-source-bytes-and-detect-by-content",
        "bgm_count": len(audio),
        "vorbis": sum(x["codec"] == "vorbis" for x in audio),
        "mp3": sum(x["codec"] == "mp3" for x in audio),
        "aac_m4a": sum(x["codec"] == "aac/m4a" for x in audio),
        "files": audio,
    }
    path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def prepare(apk: Path, output: Path) -> None:
    if output.exists():
        raise ValueError(f"output already exists: {output}")
    verify_source(apk)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".dbtb-invasion-", dir=output.parent) as temporary:
        stage = Path(temporary) / "Invasion"
        manifest = extract(apk, stage, overwrite=False, layout="auto")
        if manifest["source_layout"] != "community14" or manifest["file_count"] != 177:
            raise ValueError("unexpected Invasion APK asset layout")
        validate_roster(stage)
        audio = validate_audio(stage)
        update_manifest(stage, audio)
        stage.replace(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path)
    parser.add_argument("output", type=Path,
                        help="fresh output directory, e.g. install/mods/Invasion")
    args = parser.parse_args()
    try:
        prepare(args.apk.resolve(), args.output.resolve())
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(f"Prepared Invasion Vita mod: {args.output}")
    print("Roster: 22 characters (00..21), protected PAC profile validated.")
    print("Audio: source bytes preserved (5 MP3, 2 AAC/M4A, 10 Vorbis); Vita decodes by content.")
    print("Invasion is valid without bobj00.pac/font00.pac; keep game/ only for the normal optional VFS fallback contract.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
