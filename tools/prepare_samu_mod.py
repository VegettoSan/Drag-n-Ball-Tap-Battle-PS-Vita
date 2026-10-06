#!/usr/bin/env python3
"""Prepare the audited SamuGamerYT/Gen-derived APK as a Vita mod dataset.

The source APK stays untouched. Only the 15 BGM files whose content is MP3/AAC
despite their .ogg names are transcoded to real Ogg Vorbis. The output keeps the
logical filenames requested by the original engine and records every transform
in dbtb_manifest.json.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
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
    # The known APK ends at 91. A future mod may use the remaining two-digit
    # namespace, but that must be audited as its own source profile.
    if any((root / f"char{index:02d}.pac").exists()
           for index in range(SAMU_CHARACTER_COUNT, MAX_TWO_DIGIT_CHARACTER_COUNT)):
        raise ValueError("unexpected character data beyond the audited Samu 00..91 roster")


def ffmpeg_identity(executable: str) -> str:
    result = subprocess.run(
        [executable, "-version"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, check=True
    )
    first = result.stdout.splitlines()
    if not first:
        raise ValueError("ffmpeg produced no version information")
    return first[0].strip()


def validate_vorbis(executable: str, path: Path) -> None:
    if audio_codec(path.read_bytes()[:64]) != "vorbis":
        raise ValueError(f"normalized file is not Ogg Vorbis: {path.name}")
    subprocess.run(
        [executable, "-hide_banner", "-loglevel", "error", "-nostdin",
         "-i", str(path), "-map", "0:a:0", "-f", "null", "-"],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=True
    )


def transcode_to_vorbis(executable: str, path: Path) -> dict:
    source_size = path.stat().st_size
    source_sha = file_hash(path)
    source_codec = audio_codec(path.read_bytes()[:64])
    if source_codec not in {"mp3", "aac/m4a"}:
        raise ValueError(f"refusing unexpected source codec for {path.name}: {source_codec}")
    temporary = path.with_name("." + path.name + ".normalized.ogg")
    try:
        subprocess.run(
            [executable, "-hide_banner", "-loglevel", "error", "-nostdin", "-y",
             "-i", str(path), "-map", "0:a:0", "-vn", "-sn", "-dn",
             "-ac", "2", "-ar", "44100", "-c:a", "libvorbis", "-q:a", "5",
             str(temporary)],
            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=True
        )
        validate_vorbis(executable, temporary)
        output_size = temporary.stat().st_size
        output_sha = file_hash(temporary)
        if output_size <= 0 or output_sha == source_sha:
            raise ValueError(f"audio normalization did not produce a distinct Vorbis file: {path.name}")
        temporary.replace(path)
    finally:
        if temporary.exists():
            temporary.unlink()
    return {
        "name": path.name,
        "source_codec": source_codec,
        "source_size": source_size,
        "source_sha256": source_sha,
        "output_codec": "vorbis",
        "sample_rate": 44100,
        "channels": 2,
        "output_size": output_size,
        "output_sha256": output_sha,
    }


def normalize_audio(root: Path, executable: str) -> list[dict]:
    results = []
    for name, expected in EXPECTED_CODECS.items():
        path = root / name
        if not path.is_file():
            raise ValueError(f"missing Samu BGM: {name}")
        observed = audio_codec(path.read_bytes()[:64])
        if observed != expected:
            raise ValueError(f"unexpected codec for {name}: expected {expected}, got {observed}")
        if observed == "vorbis":
            validate_vorbis(executable, path)
            continue
        results.append(transcode_to_vorbis(executable, path))
    if len(results) != 15:
        raise ValueError(f"expected exactly 15 normalized Samu BGM files, got {len(results)}")
    return results


def update_manifest(root: Path, transforms: list[dict], ffmpeg_version: str) -> None:
    path = root / MANIFEST
    manifest = json.loads(path.read_text(encoding="utf-8"))
    files = {entry["name"]: entry for entry in manifest["files"]}
    for transform in transforms:
        entry = files.get(transform["name"])
        if entry is None:
            raise ValueError(f"manifest is missing normalized audio: {transform['name']}")
        entry["source_size"] = entry["size"]
        entry["source_sha256"] = entry["sha256"]
        if entry["source_size"] != transform["source_size"] or entry["source_sha256"] != transform["source_sha256"]:
            raise ValueError(f"manifest/source mismatch before normalization: {transform['name']}")
        entry["size"] = transform["output_size"]
        entry["sha256"] = transform["output_sha256"]
        entry["normalized_from"] = transform["source_codec"]
        entry["normalized_to"] = "ogg/vorbis"
    manifest["payloads_unchanged"] = False
    manifest["prepared_profile"] = {
        "name": "zuper-samu-1771d71d",
        "apk_sha256": SAMU_APK_SHA256,
        "classes_dex_sha256": SAMU_DEX_SHA256,
        "character_indices": "00..91",
        "character_count": SAMU_CHARACTER_COUNT,
        "vita_two_digit_namespace": "00..99",
    }
    manifest["audio_normalization"] = {
        "reason": "15 source BGM use MP3/AAC content behind .ogg names; Vita runtime consumes Vorbis",
        "normalized_count": len(transforms),
        "unchanged_vorbis_count": 2,
        "target": "Ogg Vorbis, 44100 Hz, stereo, libvorbis quality 5",
        "ffmpeg": ffmpeg_version,
        "files": transforms,
    }
    path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def prepare(apk: Path, output: Path, ffmpeg: str) -> None:
    if output.exists():
        raise ValueError(f"output already exists: {output}")
    verify_source(apk)
    output.parent.mkdir(parents=True, exist_ok=True)
    version = ffmpeg_identity(ffmpeg)
    with tempfile.TemporaryDirectory(prefix=".dbtb-samu-", dir=output.parent) as temporary:
        stage = Path(temporary) / "Samu"
        manifest = extract(apk, stage, overwrite=False, layout="auto")
        if manifest["source_layout"] != "assets" or manifest["file_count"] != 384:
            raise ValueError("unexpected Samu APK asset layout")
        validate_roster(stage)
        transforms = normalize_audio(stage, ffmpeg)
        update_manifest(stage, transforms, version)
        stage.replace(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path)
    parser.add_argument("output", type=Path,
                        help="fresh output directory, e.g. install/mods/ZuperSamu")
    parser.add_argument("--ffmpeg", default=shutil.which("ffmpeg") or "",
                        help="ffmpeg/ffmpeg.exe path; required for the 15 MP3/AAC BGM")
    args = parser.parse_args()
    try:
        if not args.ffmpeg:
            raise ValueError("ffmpeg was not found; pass --ffmpeg /path/to/ffmpeg")
        prepare(args.apk.resolve(), args.output.resolve(), args.ffmpeg)
    except (OSError, ValueError, zipfile.BadZipFile, subprocess.CalledProcessError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(f"Prepared Samu Vita mod: {args.output}")
    print("Roster: 92 characters (00..91); runtime namespace supported: 00..99.")
    print("Audio: 15 MP3/AAC-backed .ogg files normalized to real Vorbis; 2 Vorbis kept unchanged.")
    print("Copy this directory under ux0:data/DBTapBattle/mods/ and select it at boot.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
