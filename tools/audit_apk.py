#!/usr/bin/env python3
"""Inventory a user-owned APK without exporting commercial payloads.

Standard-library PAC/ZIP audit; optional --dex requires androguard.
Reports contain hashes, dimensions and API/format metadata, never asset bytes.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import zipfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def pac_inventory(data, depth=0):
    if len(data) < 2:
        raise ValueError('truncated PAC count')
    count = struct.unpack_from('<H', data)[0]
    base = 2 + count * 16
    if base > len(data):
        raise ValueError('truncated PAC table')
    entries = []
    for i in range(count):
        offset, size, tag, reserved = struct.unpack_from('<II4sI', data, 2 + i * 16)
        start = base + offset
        if start > len(data) or size > len(data) - start:
            raise ValueError(f'entry {i} outside PAC')
        payload = data[start:start + size]
        kind = tag.rstrip(b'\0').decode('ascii', errors='backslashreplace')
        entry = dict(index=i, offset=offset, size=size, type=kind, reserved=reserved,
                     sha256=sha(payload))
        if payload.startswith(b'\x89PNG\r\n\x1a\n') and len(payload) >= 24:
            entry['png_dimensions'] = list(struct.unpack_from('>II', payload, 16))
        if kind == 'spr' and depth < 2:
            try:
                entry['nested_pac'] = pac_inventory(payload, depth + 1)
            except ValueError as exc:
                entry['nested_error'] = str(exc)
        entries.append(entry)
    ranges = sorted((e['offset'], e['offset'] + e['size']) for e in entries if e['size'])
    return dict(count=count, data_base=base, entries=entries,
                overlapping=any(b[0] < a[1] for a, b in zip(ranges, ranges[1:])),
                trailing_bytes=len(data) - base - max((e['offset'] + e['size'] for e in entries), default=0))


def dex_inventory(apk_path):
    from loguru import logger
    logger.remove()
    from androguard.core.apk import APK
    from androguard.core.dex import DEX
    apk = APK(str(apk_path))
    dex = DEX(apk.get_dex())
    classes = []
    gl_calls, android_calls = Counter(), Counter()
    for cls in dex.get_classes():
        classes.append(dict(name=cls.get_name(), methods=[m.get_name() for m in cls.get_methods()]))
        for method in cls.get_methods():
            if not method.get_code():
                continue
            for ins in method.get_instructions():
                if not ins.get_name().startswith('invoke'):
                    continue
                output = ins.get_output()
                target = output.split(', ')[-1]
                if target.startswith('Ljavax/microedition/khronos/opengles/'):
                    gl_calls[target] += 1
                elif target.startswith('Landroid/'):
                    android_calls[target] += 1
    strings = dex.get_strings()
    return dict(package=apk.get_package(), version=apk.get_androidversion_name(),
                version_code=apk.get_androidversion_code(), min_sdk=apk.get_min_sdk_version(),
                permissions=apk.get_permissions(), activities=apk.get_activities(),
                class_count=len(classes), classes=classes,
                gl_calls=dict(sorted(gl_calls.items())), android_calls=dict(sorted(android_calls.items())),
                resource_names=sorted(s for s in strings if s.endswith(('.pac', '.bin', '.ogg', '.dat', '.zip'))),
                urls=sorted(s for s in strings if s.startswith(('http://', 'https://'))))


def audit(path, include_dex=False):
    with zipfile.ZipFile(path) as archive:
        files, pacs = [], {}
        for info in sorted(archive.infolist(), key=lambda x: x.filename):
            if info.is_dir():
                continue
            data = archive.read(info)
            files.append(dict(path=info.filename, size=len(data), compressed_size=info.compress_size,
                              zip_method=info.compress_type, sha256=sha(data)))
            if info.filename.endswith('.pac'):
                pacs[info.filename] = pac_inventory(data)
        result = dict(format=1, source_apk_sha256=sha(path.read_bytes()), zip_files=len(files),
                      extension_counts=dict(Counter(Path(f['path']).suffix for f in files)),
                      native_libraries=[f['path'] for f in files if f['path'].startswith('lib/')],
                      files=files, pacs=pacs)
    if include_dex:
        result['dex'] = dex_inventory(path)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path)
    parser.add_argument('report', type=Path)
    parser.add_argument('--dex', action='store_true')
    args = parser.parse_args()
    result = audit(args.apk, args.dex)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(f"{result['zip_files']} ZIP files; {len(result['pacs'])} PACs; report: {args.report}")


if __name__ == '__main__':
    main()
