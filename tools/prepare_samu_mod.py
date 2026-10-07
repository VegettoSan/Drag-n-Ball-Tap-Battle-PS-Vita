#!/usr/bin/env python3
"""Prepare the audited SamuGamerYT/Gen-derived APK as a Vita mod dataset.

All game payloads are preserved byte-for-byte, including BGM files whose `.ogg`
name hides MP3 or AAC/M4A content. The Vita 00.28 runtime detects the real codec from
content and decodes it directly; this helper only validates the known profile,
extracts it safely, and records metadata.
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

SAMU_APK_SHA256 = "1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d"
SAMU_DEX_SHA256 = "cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729"
SAMU_CHARACTER_COUNT = 92
MAX_TWO_DIGIT_CHARACTER_COUNT = 100
EXPECTED_CODECS = {
    **{f"bgm_{i:02d}.ogg": "mp3" for i in list(range(0, 9)) + [14, 15, 16]},
    **{f"bgm_{i:02d}.ogg": "aac/m4a" for i in [9, 10, 11]},
    "bgm_12.ogg": "vorbis",
    "bgm_13.ogg": "vorbis",
}


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


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def verify_source(apk: Path) -> None:
    observed = file_hash(apk)
    if observed != SAMU_APK_SHA256:
        raise ValueError(
            "This helper is pinned to the audited DragonBallZuperSamuGamerYT.apk; "
            f"expected {SAMU_APK_SHA256}, got {observed}"
        )
    with zipfile.ZipFile(apk) as archive:
        try:
            dex = archive.read("classes.dex")
        except KeyError as exc:
            raise ValueError("classes.dex is missing") from exc
    dex_sha = sha256_bytes(dex)
    if dex_sha != SAMU_DEX_SHA256:
        raise ValueError(f"unexpected Samu classes.dex hash: {dex_sha}")


def validate_roster(root: Path) -> None:
    for index in range(SAMU_CHARACTER_COUNT):
        expected = (
            f"char{index:02d}.pac",
            f"chardemo{index:02d}.pac",
            f"charf{index:04d}.pac",
        )
        for name in expected:
            if not (root / name).is_file():
                raise ValueError(f"incomplete Samu character triplet at {index:02d}: {name}")
    if any((root / f"char{index:02d}.pac").exists()
           for index in range(SAMU_CHARACTER_COUNT, MAX_TWO_DIGIT_CHARACTER_COUNT)):
        raise ValueError("unexpected character data beyond the audited Samu 00..91 roster")


def validate_audio(root: Path) -> list[dict]:
    result = []
    for name, expected in EXPECTED_CODECS.items():
        path = root / name
        if not path.is_file():
            raise ValueError(f"missing Samu BGM: {name}")
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
    by_name = {entry["name"]: entry for entry in manifest["files"]}
    for item in audio:
        source = by_name.get(item["name"])
        if source is None or source["size"] != item["size"] or source["sha256"] != item["sha256"]:
            raise ValueError(f"manifest/source mismatch for {item['name']}")
    if manifest.get("payloads_unchanged") is not True:
        raise ValueError("Samu preparation must not transform source payloads")
    manifest["prepared_profile"] = {
        "name": "zuper-samu-1771d71d",
        "apk_sha256": SAMU_APK_SHA256,
        "classes_dex_sha256": SAMU_DEX_SHA256,
        "character_indices": "00..91",
        "character_count": SAMU_CHARACTER_COUNT,
        "vita_two_digit_namespace": "00..99",
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
    with tempfile.TemporaryDirectory(prefix=".dbtb-samu-", dir=output.parent) as temporary:
        stage = Path(temporary) / "Samu"
        manifest = extract(apk, stage, overwrite=False, layout="auto")
        if manifest["source_layout"] != "assets" or manifest["file_count"] != 384:
            raise ValueError("unexpected Samu APK asset layout")
        validate_roster(stage)
        audio = validate_audio(stage)
        update_manifest(stage, audio)
        stage.replace(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path)
    parser.add_argument("output", type=Path,
                        help="fresh output directory, e.g. install/mods/ZuperSamu")
    args = parser.parse_args()
    try:
        prepare(args.apk.resolve(), args.output.resolve())
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(f"Prepared Samu Vita mod: {args.output}")
    print("Roster: 92 characters (00..91); runtime namespace supported: 00..99.")
    print("Audio: source bytes preserved (12 MP3, 3 AAC/M4A, 2 Vorbis); Vita decodes by content.")
    print("Copy this directory under ux0:data/DBTapBattle/mods/ and select it at boot; game/ may remain empty.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
