#!/usr/bin/env python3
"""Extract untouched res/raw data from a user-owned APK, with hashes.

All archive paths, destination conflicts and CRCs are checked before publishing
files. Unknown raw extensions are preserved and reported, never discarded.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import sys
import tempfile
import zipfile

RAW_PREFIX = 'res/raw/'
MANIFEST = 'dbtb_manifest.json'
KNOWN = {'.pac', '.ogg', '.png', '.bmp', '.bin', '.dat', '.db', '.dac', '.gdt', '.cnv', '.spr', '.act', '.plt', '.xml'}
MAX_FILE = 64 * 1024 * 1024
MAX_TOTAL = 512 * 1024 * 1024


def file_hash(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def safe_name(name):
    parts = name.split('/')
    return (bool(name) and not any(ord(c) < 32 or ord(c) == 127 for c in name)
            and not any(c in name for c in ':\\')
            and all(p not in {'', '.', '..'} and len(p.encode('utf-8')) <= 255 for p in parts)
            and len(name.encode('utf-8')) <= 900)


def check_destination(path, overwrite, directory=False):
    # Do not follow a user-created or leftover symlink outside the output root.
    for parent in [path, *path.parents]:
        if parent.is_symlink():
            raise ValueError(f'symlink destination is not allowed: {parent}')
        if parent != path and parent.exists() and not parent.is_dir():
            raise ValueError(f'destination parent is not a directory: {parent}')
    if path.exists():
        if directory and path.is_dir():
            return
        if not path.is_file() or not overwrite:
            raise ValueError(f'destination exists (use --overwrite for regular files): {path}')


def extract(apk, output, overwrite=False):
    with zipfile.ZipFile(apk) as archive:
        entries = []
        names = set()
        total = 0
        for info in sorted(archive.infolist(), key=lambda x: x.filename):
            if not info.filename.startswith(RAW_PREFIX):
                continue
            name = info.filename[len(RAW_PREFIX):]
            if info.is_dir():
                if name and not safe_name(name.rstrip('/')):
                    raise ValueError(f'unsafe raw directory: {info.filename!r}')
                continue
            if not safe_name(name):
                raise ValueError(f'unsafe raw path: {info.filename!r}')
            key = name.casefold()
            if key == MANIFEST or key in names:
                raise ValueError(f'duplicate or reserved raw path: {name}')
            mode = (info.external_attr >> 16) & 0o170000
            if mode not in (0, stat.S_IFREG):
                raise ValueError(f'non-regular ZIP entry: {name}')
            if info.flag_bits & 1:
                raise ValueError(f'encrypted raw file: {name}')
            total += info.file_size
            if info.file_size > MAX_FILE or total > MAX_TOTAL:
                raise ValueError('raw data exceeds extraction budget (64 MiB/file, 512 MiB total)')
            names.add(key)
            entries.append((info, name))
        if not entries:
            raise ValueError('APK contains no res/raw files')
        for _, name in entries:
            parts = name.casefold().split('/')
            if any('/'.join(parts[:i]) in names for i in range(1, len(parts))):
                raise ValueError(f'file/directory collision: {name}')
        check_destination(output, overwrite, directory=True)
        for _, name in entries:
            check_destination(output / name, overwrite)
        check_destination(output / MANIFEST, overwrite)
        output.parent.mkdir(parents=True, exist_ok=True)
        # Stage outside the final directory: corrupt late entries leave it untouched.
        with tempfile.TemporaryDirectory(prefix='.dbtb-extract-', dir=output.parent) as temporary:
            stage = Path(temporary)
            files = []
            for info, name in entries:
                target = stage / name
                target.parent.mkdir(parents=True, exist_ok=True)
                with archive.open(info) as source, target.open('wb') as destination:
                    shutil.copyfileobj(source, destination, 1024 * 1024)
                if target.stat().st_size != info.file_size:
                    raise ValueError(f'wrong extracted size: {name}')
                files.append({'name': name, 'size': info.file_size, 'sha256': file_hash(target),
                              'apk_path': info.filename})
            unknown = [f['name'] for f in files if Path(f['name']).suffix.lower() not in KNOWN]
            manifest = {'format': 2, 'source_apk': apk.name, 'source_apk_sha256': file_hash(apk),
                        'file_count': len(files), 'files': files, 'unknown_raw_files': unknown,
                        'not_extracted': [i.filename for i in archive.infolist()
                                          if not i.filename.startswith(RAW_PREFIX) and not i.is_dir()]}
            (stage / MANIFEST).write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
            output.mkdir(parents=True, exist_ok=True)
            for name in [f['name'] for f in files] + [MANIFEST]:
                destination = output / name
                check_destination(destination, overwrite)
                destination.parent.mkdir(parents=True, exist_ok=True)
                if overwrite:
                    os.replace(stage / name, destination)
                else:
                    # Exclusive creation prevents a concurrently-created file being replaced.
                    with (stage / name).open('rb') as source, destination.open('xb') as dest:
                        shutil.copyfileobj(source, dest, 1024 * 1024)
            return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--overwrite', action='store_true')
    args = parser.parse_args()
    try:
        manifest = extract(args.apk, args.output, args.overwrite)
    except (OSError, ValueError, zipfile.BadZipFile, RuntimeError, NotImplementedError) as exc:
        print(f'error: {exc}', file=sys.stderr)
        return 2
    for name in manifest['unknown_raw_files']:
        print(f'warning: unknown raw format preserved: {name}')
    print(f"Done: {manifest['file_count']} untouched files -> {args.output}")
    print('Copy contents to ux0:data/DBTapBattle/game/; this APK may require additional external character data.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
