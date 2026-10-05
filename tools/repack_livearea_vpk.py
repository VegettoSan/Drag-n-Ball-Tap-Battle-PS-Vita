#!/usr/bin/env python3
"""Repack a hash-pinned playable VPK with approved LiveArea using VitaSDK."""

import argparse
import hashlib
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path, PurePosixPath

from decode_livearea_assets import ASSETS, decode_asset
from validate_livearea_vpk import EXPECTED, TEMPLATE_PATH, validate_base_identity


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-vpk", type=Path, required=True)
    parser.add_argument("--expected-base-sha256", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--packer", default="vita-pack-vpk")
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[1] / "assets/livearea")
    args = parser.parse_args()
    base = args.base_vpk.resolve()
    output = args.output.resolve()
    if base == output:
        raise ValueError("output must not overwrite the base VPK")
    if hashlib.sha256(base.read_bytes()).hexdigest() != args.expected_base_sha256:
        raise ValueError("base VPK identity mismatch; do not use a native smoke/link probe")
    with tempfile.TemporaryDirectory(prefix="dbtb-livearea-") as temp:
        staging = Path(temp) / "package"
        with zipfile.ZipFile(base) as original:
            if len(original.namelist()) != len(set(original.namelist())) or original.testzip():
                raise ValueError("invalid base VPK archive")
            for name in original.namelist():
                path = PurePosixPath(name)
                if path.is_absolute() or ".." in path.parts or "\\" in name:
                    raise ValueError("unsafe base VPK entry: " + name)
                if name.endswith("/"):
                    continue
                target = staging / name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(original.read(name))
        assets = Path(temp) / "assets"
        for name, spec in ASSETS.items():
            decode_asset(args.source, assets, name, spec)
        for path in EXPECTED:
            target = staging / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((assets / target.name).read_bytes())
        (staging / TEMPLATE_PATH).write_bytes((args.source / "template.xml").read_bytes())
        output.parent.mkdir(parents=True, exist_ok=True)
        cmd = [args.packer, "-s", str(staging / "sce_sys/param.sfo"), "-b", str(staging / "eboot.bin")]
        for path in sorted(staging.rglob("*")):
            if path.is_file() and path.relative_to(staging).as_posix() not in ("eboot.bin", "sce_sys/param.sfo"):
                cmd.extend(["-a", str(path) + "=" + path.relative_to(staging).as_posix()])
        cmd.append(str(output))
        subprocess.run(cmd, check=True)
    validate_base_identity(output, base)
    subprocess.run([sys.executable, str(Path(__file__).with_name("validate_livearea_vpk.py")), str(output)], check=True)
    print("Repack SHA-256: " + hashlib.sha256(output.read_bytes()).hexdigest())


if __name__ == "__main__":
    main()
