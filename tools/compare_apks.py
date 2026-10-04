#!/usr/bin/env python3
"""Compare two user-owned Tap Battle APKs; export metadata/hashes only.

--dex requires androguard; --pixels requires Pillow and compares decoded RGBA
rather than declaring recompressed/private textures different by ZIP hash alone.
--reference-swb verifies shared library provenance against a local public SWB.
"""
import argparse
from collections import Counter
import io
import json
from pathlib import Path
import zipfile
import audit_apk
import community14


def pixel_records(data, pixels):
    inventory = audit_apk.pac_inventory(data)
    records = []
    def visit(blob, node, prefix=''):
        for entry in node['entries']:
            ident = prefix + str(entry['index'])
            start = node['data_base'] + entry['offset']
            payload = blob[start:start + entry['size']]
            record = {k: entry[k] for k in ('type', 'size', 'sha256')}
            record['entry'] = ident
            if 'rgba_sha256' in entry:
                record['pixels_sha256'] = entry['rgba_sha256']
                record['dimensions'] = entry['rgba_dimensions']
            elif pixels and 'png_dimensions' in entry:
                from PIL import Image
                with Image.open(io.BytesIO(payload)) as img:
                    if img.width * img.height * 4 > 16 * 1024 * 1024:
                        raise ValueError('pixel comparison exceeds image budget')
                    raw = img.convert('RGBA').tobytes()
                    record['pixels_sha256'] = audit_apk.sha(raw)
                    premultiplied = bytearray(raw)
                    for pos in range(0, len(raw), 4):
                        for channel in range(3):
                            premultiplied[pos + channel] = raw[pos + channel] * raw[pos + 3] // 255
                    record['premultiplied_sha256'] = audit_apk.sha(premultiplied)
                    record['dimensions'] = [img.width, img.height]
            records.append(record)
            if 'nested_pac' in entry:
                visit(payload, entry['nested_pac'], ident + '/')
    visit(data, inventory)
    return records


def compare(original, community, dex=False, pixels=False, reference_swb=None):
    old = audit_apk.audit(original, dex)
    new = audit_apk.audit(community, dex)
    originals = {Path(f['path']).name: f for f in old['files'] if f['path'].startswith('res/raw/')}
    targets = {community14.canonical_name(f['path'][7:]): f for f in new['files'] if f['path'].startswith('assets/')}
    resources = []
    with zipfile.ZipFile(original) as a, zipfile.ZipFile(community) as b:
        for name, before in sorted(originals.items()):
            after = targets.get(name)
            row = dict(logical_name=name, original_path=before['path'], original_sha256=before['sha256'],
                       community_path=after['path'] if after else None,
                       community_sha256=after['sha256'] if after else None,
                       status='absent' if not after else 'identical' if before['sha256'] == after['sha256'] else 'changed')
            if after and name.endswith('.pac'):
                p = pixel_records(a.read(before['path']), pixels)
                q = pixel_records(b.read(after['path']), pixels)
                row['original_entries'], row['community_entries'] = p, q
                payload_hashes = {r['sha256'] for r in p}
                image_hashes = {r['premultiplied_sha256'] for r in p if 'premultiplied_sha256' in r}
                row['exact_payload_matches'] = sum(r['sha256'] in payload_hashes for r in q)
                row['premultiplied_image_matches'] = sum(r.get('pixels_sha256') in image_hashes for r in q)
            resources.append(row)
        added = [dict(logical_name=n, **f) for n, f in sorted(targets.items()) if n not in originals]
        libraries = []
        if reference_swb:
            with zipfile.ZipFile(reference_swb) as ref:
                for path in new['native_libraries']:
                    refpath = path.replace('lib/', 'data/files/native_libs/', 1)
                    data = b.read(path)
                    libraries.append(dict(apk_path=path, sha256=audit_apk.sha(data), swb_path=refpath,
                                          identical=refpath in ref.namelist() and data == ref.read(refpath)))
    return dict(format=1, original_sha256=old['source_apk_sha256'], community_sha256=new['source_apk_sha256'],
                original_bytes=original.stat().st_size, community_bytes=community.stat().st_size,
                original_zip_files=old['zip_files'], community_zip_files=new['zip_files'],
                original_pac_count=len(old['pacs']), community_pac_count=len(new['pacs']),
                resource_status_counts=dict(Counter(r['status'] for r in resources)),
                resources=resources, added_resources=added,
                original_dex=old.get('dex'), community_dex=new.get('dex'),
                shared_library_reference=dict(swb_sha256=audit_apk.sha(reference_swb.read_bytes()), libraries=libraries) if reference_swb else None,
                limitation='Static comparison; does not prove Android14/Vita gameplay, author identity or identical mechanics.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original', type=Path)
    parser.add_argument('community', type=Path)
    parser.add_argument('report', type=Path)
    parser.add_argument('--dex', action='store_true')
    parser.add_argument('--pixels', action='store_true')
    parser.add_argument('--reference-swb', type=Path)
    args = parser.parse_args()
    result = compare(args.original, args.community, args.dex, args.pixels, args.reference_swb)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(result['resource_status_counts'], 'added:', len(result['added_resources']))


if __name__ == '__main__':
    main()
