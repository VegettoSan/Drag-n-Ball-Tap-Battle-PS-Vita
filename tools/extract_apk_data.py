#!/usr/bin/env python3
"""Extract untouched raw/assets data from a user-owned APK, with hashes.

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
import struct
import sys
import tempfile
import zipfile
import community14

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


def zip_member_hash(archive, info):
    digest = hashlib.sha256()
    with archive.open(info) as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def safe_name(name):
    parts = name.split('/')
    return (bool(name) and not any(ord(c) < 32 or ord(c) == 127 for c in name)
            and not any(c in name for c in ':\\')
            and all(p not in {'', '.', '..'} and len(p.encode('utf-8')) <= 255 for p in parts)
            and len(name.encode('utf-8')) <= 900)


def local_header_name(apk_path, info):
    """Read the raw ZIP local-header filename.

    Python's zipfile may normalize backslashes on Windows before exposing
    ZipInfo.filename/orig_filename. The local header preserves the archive's
    actual separator bytes, so validate those directly before path handling.
    """
    with apk_path.open('rb') as stream:
        stream.seek(info.header_offset)
        header = stream.read(30)
        if len(header) != 30 or struct.unpack_from('<I', header, 0)[0] != 0x04034B50:
            raise ValueError(f'invalid ZIP local header for {info.filename!r}')
        name_len, extra_len = struct.unpack_from('<HH', header, 26)
        raw = stream.read(name_len)
        if len(raw) != name_len:
            raise ValueError(f'truncated ZIP local name for {info.filename!r}')
        encoding = 'utf-8' if (info.flag_bits & 0x800) else 'cp437'
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError as exc:
            raise ValueError(f'invalid ZIP filename encoding for {info.filename!r}') from exc


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


def choose_layout(archive, layout):
    infos = [i for i in archive.infolist() if not i.is_dir()]
    raw_infos = [i for i in infos if i.filename.startswith(RAW_PREFIX)]
    asset_infos = [i for i in infos if i.filename.startswith('assets/')]
    raw = bool(raw_infos)
    assets = bool(asset_infos)
    if layout == 'auto':
        if raw and assets:
            # Some later/community-derived builds keep the original res/raw names
            # as zero-byte resource stubs while moving the real game data into
            # assets/. Treat those as asset-backed APKs instead of forcing a
            # manual --layout choice. If both sides contain payload bytes the
            # archive is genuinely ambiguous and still requires an explicit mode.
            raw_payload = any(i.file_size for i in raw_infos)
            asset_payload = any(i.file_size for i in asset_infos)
            if raw_payload and asset_payload:
                raise ValueError('ambiguous res/raw + assets APK: choose --layout explicitly')
            if asset_payload and not raw_payload:
                raw = False
            elif raw_payload and not asset_payload:
                assets = False
            else:
                raise ValueError('APK contains only empty res/raw/assets entries')
        if raw:
            return 'raw'
        if assets:
            files = [i.filename[7:] for i in asset_infos]
            return 'community14' if community14.profile_from_names(files) else 'assets'
        raise ValueError('APK contains no res/raw or assets files')
    return layout


def extract(apk, output, overwrite=False, layout='auto'):
    if layout not in {'auto', 'raw', 'assets', 'community14'}:
        raise ValueError('unknown APK layout')
    with zipfile.ZipFile(apk) as archive:
        layout = choose_layout(archive, layout)
        prefix = RAW_PREFIX if layout == 'raw' else 'assets/'
        profile = None
        if layout == 'community14':
            profile = community14.profile_from_names(
                [i.filename[len(prefix):] for i in archive.infolist()
                 if i.filename.startswith(prefix) and not i.is_dir()])
            if profile is None:
                # Explicit --layout community14 can still identify a canonical-name
                # protected APK from its first PAC instead of guessing constants.
                for info in archive.infolist():
                    if info.filename.startswith(prefix) and info.filename.endswith('.pac') and not info.is_dir():
                        with archive.open(info) as source:
                            profile = community14.detect_profile(source.read())
                        if profile:
                            break
            if profile is None:
                raise ValueError('unsupported or ambiguous Community14 profile')
        entries = []
        names = set()
        ignored_profile_save = None
        total = 0
        for info in sorted(archive.infolist(), key=lambda x: x.filename):
            # zipfile normalizes backslashes to '/' on Windows when constructing
            # ZipInfo.filename. Validate the original central-directory name too,
            # otherwise an APK entry such as res/raw/bad\\x can masquerade as a
            # nested safe path only on Windows.
            original_name = local_header_name(apk, info)
            if '\\' in original_name and (original_name.startswith(prefix) or info.filename.startswith(prefix)):
                raise ValueError(f'unsafe ZIP path separator: {original_name!r}')
            exposed_original = getattr(info, 'orig_filename', info.filename)
            if exposed_original != info.filename and (exposed_original.startswith(prefix) or info.filename.startswith(prefix)):
                raise ValueError(f'unsafe ZIP path separator: {exposed_original!r}')
            if not info.filename.startswith(prefix):
                continue
            name = info.filename[len(prefix):]
            if info.is_dir():
                if name and not safe_name(name.rstrip('/')):
                    raise ValueError(f'unsafe raw directory: {info.filename!r}')
                continue
            if not safe_name(name):
                raise ValueError(f'unsafe raw path: {info.filename!r}')
            if layout == 'community14':
                name = community14.canonical_name(name, profile)
            key = name.casefold()
            if key.split('/')[0] == MANIFEST or key in names:
                raise ValueError(f'duplicate or reserved raw path: {name}')
            mode = (info.external_attr >> 16) & 0o170000
            if mode not in (0, stat.S_IFREG):
                raise ValueError(f'non-regular ZIP entry: {name}')
            if info.flag_bits & 1:
                raise ValueError(f'encrypted raw file: {name}')
            if key == 'save.bin':
                if ignored_profile_save is not None:
                    raise ValueError('multiple save.bin entries in selected APK data layout')
                ignored_profile_save = {
                    'apk_path': info.filename,
                    'size': info.file_size,
                    'sha256': zip_member_hash(archive, info),
                    'runtime_policy': 'ignored-apk-save-use-vpk-profile-seed',
                }
                continue
            total += info.file_size
            if info.file_size > MAX_FILE or total > MAX_TOTAL:
                raise ValueError('raw data exceeds extraction budget (64 MiB/file, 512 MiB total)')
            names.add(key)
            entries.append((info, name))
        if not entries:
            raise ValueError(f'APK contains no {prefix} files')
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
                if layout == 'community14' and name.endswith('.pac'):
                    # Refuse a different codec before publishing any output.
                    data = target.read_bytes()
                    if community14.detect_profile(data) is not profile:
                        raise ValueError(f'unsupported/mixed community PAC codec: {info.filename}')
                    community14.table(data, profile)
                files.append({'name': name, 'size': info.file_size, 'sha256': file_hash(target),
                              'apk_path': info.filename})
            unknown = [f['name'] for f in files if Path(f['name']).suffix.lower() not in KNOWN]
            manifest = {'format': 4, 'source_layout': layout,
                        'pac_codec': profile.name if profile else 'original-or-unknown',
                        'payloads_unchanged': True,
                        'save_policy': 'per-profile-vpk-seed',
                        'ignored_profile_save': ignored_profile_save,
                        'renamed_files': [{'apk_path': f['apk_path'], 'name': f['name']} for f in files
                                          if f['apk_path'][len(prefix):] != f['name']], 'source_apk': apk.name, 'source_apk_sha256': file_hash(apk),
                        'file_count': len(files), 'files': files, 'unknown_files': unknown,
                        'unknown_raw_files': unknown if layout == 'raw' else [],
                        'not_extracted': [i.filename for i in archive.infolist()
                                          if not i.filename.startswith(prefix) and not i.is_dir()]}
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
    parser.add_argument('--layout', choices=['auto', 'raw', 'assets', 'community14'], default='auto')
    parser.add_argument('--mod', help='extract into OUTPUT/mods/NAME, protecting OUTPUT/game')
    parser.add_argument('--overwrite', action='store_true')
    args = parser.parse_args()
    try:
        if args.mod:
            if not safe_name(args.mod) or '/' in args.mod or args.mod.casefold() == MANIFEST:
                raise ValueError('mod name must be a single safe directory component')
            args.output = args.output / 'mods' / args.mod
        manifest = extract(args.apk, args.output, args.overwrite, args.layout)
    except (OSError, ValueError, zipfile.BadZipFile, RuntimeError, NotImplementedError) as exc:
        print(f'error: {exc}', file=sys.stderr)
        return 2
    for name in manifest['unknown_files']:
        print(f'warning: unknown resource format preserved: {name}')
    print(f"Done: {manifest['file_count']} untouched files -> {args.output}")
    print('Layout: ' + manifest['source_layout'] + '; PAC codec: ' + manifest['pac_codec'])
    if args.mod:
        print(f'Copy contents to ux0:data/DBTapBattle/mods/{args.mod}/; select that folder at boot.')
    else:
        print('Copy contents to game/ for a base installation, or mods/NAME/ for a community dataset.')
    print('Resource import does not establish gameplay or code-mod compatibility.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
