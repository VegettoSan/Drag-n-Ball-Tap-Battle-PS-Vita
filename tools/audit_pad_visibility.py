#!/usr/bin/env python3
"""Audit pad animation ownership in user-owned APKs; publish metadata only.

No APK/PAC is written or changed. The hypothetical DAC overlay is evaluated in
memory only. This is structural evidence, not a renderer/hardware test.
Requires Node for the repository's existing PRIVATE DEX metadata reader.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import zipfile

import community14

PAD_ACTIONS = set(range(200, 211)) | {330, 331, 333, 334, 336, 337, 339, 340}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def number(data, pos, kind='<H'):
    size = struct.calcsize(kind)
    if pos < 0 or pos + size > len(data):
        raise ValueError('animation field outside payload')
    return struct.unpack_from(kind, data, pos)[0]


def animations(data):
    if number(data, 0) != 0x1100:
        raise ValueError('unsupported animation DAC header')
    count, index, base = (number(data, p) for p in (2, 4, 6))
    if index < 8 or index + count * 2 > base or base > len(data):
        raise ValueError('invalid animation directory')
    result = {}
    for action in range(count):
        offset = number(data, index + action * 2, '<h')
        if offset == -1:
            continue
        if offset < 0:
            raise ValueError('negative animation record offset')
        p = base + offset * 4
        start = p
        flags, frames = number(data, p, '<B'), number(data, p + 1, '<B')
        p += 2
        if flags & 4:
            p += 1
        links = []
        if flags & 2:
            n = number(data, p, '<B')
            p += 1
            for _ in range(n):
                links.append(number(data, p + 4) & 4095)
                p += 6
        fields = []
        for frame in range(frames):
            f = p + frames * 2 + number(data, p + frame * 2)
            field = f + 1 + bool(number(data, f, '<B') & 32)
            fields.append((field, number(data, field, '<h')))
        result[action] = dict(start=start, links=links, fields=fields)
    return result


def inspect_effect(data, codec):
    if codec:
        base, entries = community14.table(data, codec)
    else:
        count = number(data, 0)
        base = 2 + count * 16
        if base > len(data):
            raise ValueError('truncated PAC directory')
        entries = [dict(offset=number(data, 2 + i * 16, '<I'),
                        size=number(data, 6 + i * 16, '<I'),
                        type=data[10 + i * 16:14 + i * 16].rstrip(b'\0').decode('ascii'))
                   for i in range(count)]
    payloads = {}
    for e in entries:
        start = base + e['offset']
        if start > len(data) or e['size'] > len(data) - start:
            raise ValueError('PAC payload outside bounds')
        if e['type'] in {'dac', 'cnv'}:
            if e['type'] in payloads:
                raise ValueError('ambiguous duplicate animation/CNV payload')
            payloads[e['type']] = data[start:start + e['size']]
    dac, cnv = payloads['dac'], payloads['cnv']
    if len(cnv) % 9:
        raise ValueError('unsupported CNV image-record width')
    records = animations(dac)
    if not PAD_ACTIONS <= records.keys():
        raise ValueError('expected original pad roles absent')
    closure = set(PAD_ACTIONS)
    while True:
        linked = {a for root in closure for a in records[root]['links']}
        if not linked <= records.keys():
            raise ValueError('linked pad action missing')
        expanded = closure | linked
        if expanded == closure:
            break
        closure = expanded
    target = {p for a in closure for p, image in records[a]['fields']}
    target_bytes = {p + k for p in target for k in (0, 1)}
    starts = sorted({r['start'] for r in records.values()} | {len(dac)})
    def record_end(r):
        return next(p for p in starts if p > r['start'])
    collisions = sorted({a for a, r in records.items() if a not in closure
                         and any(r['start'] <= p < record_end(r) for p in target_bytes)})
    cross_record = sorted(a for a in closure if any(
        not (records[a]['start'] <= p and p + 2 <= record_end(records[a]))
        for p, _ in records[a]['fields']))
    field_collisions = sorted({a for a, r in records.items() if a not in closure
                         and any({p, p + 1} & target_bytes
                                 for p, _ in r['fields'])})
    incoming_links = sorted({a for a, r in records.items() if a not in closure
                             and closure.intersection(r['links'])})
    # Only approve the narrow, observed panel schema. A future resource overlay
    # must additionally validate runtime role and cache mode.
    eligible = not (collisions or field_collisions or incoming_links or cross_record)
    overlay = bytearray(dac)
    if eligible:
        for p in target:
            struct.pack_into('<h', overlay, p, -1)
    changed = [i for i, (x, y) in enumerate(zip(dac, overlay)) if x != y]
    assert len(overlay) == len(dac)
    assert set(changed) <= target_bytes
    details = []
    for a in sorted(closure):
        images = sorted({image for _, image in records[a]['fields'] if image >= 0})
        if any(image >= len(cnv) // 9 for image in images):
            raise ValueError('pad image outside CNV')
        details.append(dict(action=a, root=a in PAD_ACTIONS, links=records[a]['links'],
                            frames=len(records[a]['fields']), images=images,
                            texture_slots=sorted({cnv[image * 9] for image in images}),
                            shared_images_with_other_actions=sorted({other for other, r in records.items()
                                if other not in closure and any(image in images for _, image in r['fields'])})))
    return dict(pac_sha256=digest(data), dac_sha256=digest(dac), cnv_sha256=digest(cnv),
                action_count=len(records), pad_actions=details, pad_image_fields=len(target),
                pad_action_closure=sorted(closure),
                fields_shared_with_other_actions=field_collisions,
                pad_fields_inside_other_action_records=collisions,
                pad_fields_outside_own_record=cross_record, incoming_links=incoming_links,
                narrow_overlay_structurally_eligible=eligible,
                overlay_changed_bytes=len(changed), overlay_dac_sha256=digest(overlay),
                unrelated_dac_bytes_unchanged=True)


def audit(apk):
    with zipfile.ZipFile(apk) as z:
        files = [i for i in z.infolist() if i.filename.endswith('.pac') and i.file_size]
        codec = community14.profile_from_names([i.filename.rsplit('/', 1)[-1] for i in files])
        effect_name = 'effect.pac'
        if not codec and not any(i.filename.rsplit('/', 1)[-1] == effect_name for i in files):
            with tempfile.TemporaryDirectory(prefix='dbtb-pad-audit-') as temp:
                dex = Path(temp) / 'classes.dex'
                dex.write_bytes(z.read('classes.dex'))
                module = Path(__file__).resolve().parents[1] / 'web/private-dex.mjs'
                script = "import fs from 'node:fs';import {discoverPrivateCodec} from " + json.dumps(module.as_uri()) + ";process.stdout.write(JSON.stringify(discoverPrivateCodec(fs.readFileSync(process.argv[1]))));"
                meta = json.loads(subprocess.check_output(['node', '--input-type=module', '-e', script, str(dex)], text=True))
            effect_name = meta['aliases']['effect'] + '.pac'
            codec = community14.Profile(meta['generator'], meta['count_xor'], meta['offset_xor'], meta['size_xor'],
                {v: k for k, v in meta['type_keys'].items()}, meta['image_width_xor'], meta['image_height_xor'],
                meta['table_count_xor'], meta['table_position_xor'], meta['table_width_xor'], meta['table_height_xor'], {}, {})
        sources = [i for i in files if (community14.canonical_name(i.filename.rsplit('/', 1)[-1], codec)
                                      if codec and codec.fixed_names else i.filename.rsplit('/', 1)[-1]) == effect_name]
        if not sources:
            raise ValueError('effect resource absent')
        return dict(apk=apk.name, apk_sha256=digest(apk.read_bytes()),
                    codec=codec.name if codec else 'original',
                    effects=[dict(source=i.filename, **inspect_effect(z.read(i), codec)) for i in sources])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apks', nargs='+', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    result = dict(scope='structural-only; no Vita or original renderer execution',
                  method='DAC image-field ownership; hypothetical same-length absent-image overlay',
                  apks=[audit(apk) for apk in args.apks])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(f'{len(result["apks"])} APKs audited; metadata only')


if __name__ == '__main__':
    main()
