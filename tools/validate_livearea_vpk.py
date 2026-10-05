#!/usr/bin/env python3
"""Validate the LiveArea payload inside a built VPK."""

from __future__ import annotations

import argparse
import hashlib
import zipfile
import xml.etree.ElementTree as ET
from pathlib import Path

from decode_livearea_assets import ASSETS, validate_png

EXPECTED = {
    "sce_sys/icon0.png": "26ea71975390073a76c69f83d526f039f7ffb96aaad34d0cc1e5203d9541b115",
    "sce_sys/livearea/contents/bg0.png": "df12c6104b0b877c9848c2385a3c8d8a4e093ddc3e215169d99811aac4963b0a",
    "sce_sys/livearea/contents/startup.png": "0f00cf71705d1a73de8f5a755505dd4d6f6b7d88b1966aa385c70dca6ba31cba",
    "sce_sys/pic0.png": "84d530e2def6332bddc817bbc5256192173b11928c21382bbeddcfa89bfae83a",
}
TEMPLATE_PATH = "sce_sys/livearea/contents/template.xml"


def validate_base_identity(vpk_path, base_path):
    """LiveArea repacks must retain every non-presentation base entry exactly."""
    presentation = set(EXPECTED) | {TEMPLATE_PATH}
    with zipfile.ZipFile(base_path) as base, zipfile.ZipFile(vpk_path) as output:
        if base.testzip() is not None:
            raise ValueError("base VPK CRC failure")
        if len(base.namelist()) != len(set(base.namelist())):
            raise ValueError("duplicate base VPK entries")
        for path in base.namelist():
            if path.endswith("/") or path in presentation:
                continue
            if path not in output.namelist() or output.read(path) != base.read(path):
                raise ValueError("repack changed/missing base entry: " + path)
        for path in ("eboot.bin", "sce_sys/param.sfo"):
            if path not in base.namelist():
                raise ValueError("base VPK missing " + path)
        unexpected = set(output.namelist()) - set(base.namelist()) - presentation
        if any(not name.endswith("/") for name in unexpected):
            raise ValueError("unexpected repack entries: " + str(sorted(unexpected)))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("vpk", type=Path)
    parser.add_argument("--base-vpk", type=Path, help="Check executable/SFO and all other original entries")
    args = parser.parse_args()

    if not args.vpk.is_file():
        raise FileNotFoundError(args.vpk)

    with zipfile.ZipFile(args.vpk, "r") as vpk:
        if len(vpk.namelist()) != len(set(vpk.namelist())):
            raise ValueError("VPK contains duplicate entries")
        bad_entry = vpk.testzip()
        if bad_entry:
            raise ValueError("VPK CRC error: " + bad_entry)
        names = set(vpk.namelist())
        required = set(EXPECTED) | {TEMPLATE_PATH}
        missing = sorted(required - names)
        if missing:
            raise ValueError("VPK is missing LiveArea files: " + ", ".join(missing))

        for path, expected_hash in EXPECTED.items():
            data = vpk.read(path)
            name = path.rsplit("/", 1)[-1]
            digest = validate_png(data, name, ASSETS[name])
            if digest != expected_hash:
                raise ValueError(f"{path}: SHA-256 mismatch: {digest}")
            print(f"{path}: OK sha256={digest}")

        template = vpk.read(TEMPLATE_PATH)
        root = ET.fromstring(template)
        if root.tag != "livearea" or root.attrib.get("style") != "a1":
            raise ValueError("template.xml: expected <livearea style=\"a1\">")
        if root.attrib.get("format-ver") != "01.00" or not root.attrib.get("content-rev", "").isdigit():
            raise ValueError("template.xml: invalid format/content revision")
        background = root.find("./livearea-background/image")
        startup = root.find("./gate/startup-image")
        if background is None or (background.text or "").strip() != "bg0.png":
            raise ValueError("template.xml: bg0.png background mapping is missing")
        if startup is None or (startup.text or "").strip() != "startup.png":
            raise ValueError("template.xml: startup.png gate mapping is missing")
        print(f"{TEMPLATE_PATH}: OK style=a1, bg0.png + startup.png gate")

    if args.base_vpk:
        validate_base_identity(args.vpk, args.base_vpk)
        print("Base VPK identity: PASS (all non-presentation entries unchanged)")
    print("LiveArea VPK validation: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
