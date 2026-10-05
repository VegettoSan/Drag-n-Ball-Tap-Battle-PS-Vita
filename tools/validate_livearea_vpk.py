#!/usr/bin/env python3
"""Validate the LiveArea payload inside a built VPK."""

from __future__ import annotations

import argparse
import hashlib
import zipfile
import xml.etree.ElementTree as ET
from pathlib import Path

EXPECTED = {
    "sce_sys/icon0.png": "26ea71975390073a76c69f83d526f039f7ffb96aaad34d0cc1e5203d9541b115",
    "sce_sys/livearea/contents/bg0.png": "df12c6104b0b877c9848c2385a3c8d8a4e093ddc3e215169d99811aac4963b0a",
    "sce_sys/livearea/contents/startup.png": "0f00cf71705d1a73de8f5a755505dd4d6f6b7d88b1966aa385c70dca6ba31cba",
    "sce_sys/pic0.png": "17d875946a077684ae62720967941d9d3b82ba3b080d52a3eb7f7e93abfab3ce",
}
TEMPLATE_PATH = "sce_sys/livearea/contents/template.xml"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("vpk", type=Path)
    args = parser.parse_args()

    if not args.vpk.is_file():
        raise FileNotFoundError(args.vpk)

    with zipfile.ZipFile(args.vpk, "r") as vpk:
        names = set(vpk.namelist())
        required = set(EXPECTED) | {TEMPLATE_PATH}
        missing = sorted(required - names)
        if missing:
            raise ValueError("VPK is missing LiveArea files: " + ", ".join(missing))

        for path, expected_hash in EXPECTED.items():
            data = vpk.read(path)
            digest = hashlib.sha256(data).hexdigest()
            if digest != expected_hash:
                raise ValueError(f"{path}: SHA-256 mismatch: {digest}")
            print(f"{path}: OK sha256={digest}")

        template = vpk.read(TEMPLATE_PATH)
        root = ET.fromstring(template)
        if root.tag != "livearea" or root.attrib.get("style") != "a1":
            raise ValueError("template.xml: expected <livearea style=\"a1\">")
        background = root.find("./livearea-background/image")
        startup = root.find("./gate/startup-image")
        if background is None or (background.text or "").strip() != "bg0.png":
            raise ValueError("template.xml: bg0.png background mapping is missing")
        if startup is None or (startup.text or "").strip() != "startup.png":
            raise ValueError("template.xml: startup.png gate mapping is missing")
        print(f"{TEMPLATE_PATH}: OK style=a1, bg0.png + startup.png gate")

    print("LiveArea VPK validation: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
