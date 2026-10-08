#!/usr/bin/env python3
"""Inventory control contracts without exporting APK bytecode or game content.

Requires androguard==4.1.4. Output contains only hashes, method identifiers,
instruction counts and references. It is not a mod compatibility certificate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

from loguru import logger
logger.remove()
from androguard.core.dex import DEX

PREFIX = 'Lcom/namcobandaigames/dragonballtap/apk/'


def sha_file(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def fingerprint(method):
    # Resolves referenced pool entries to names; does not compare raw DEX indices.
    body = '\n'.join(i.get_name() + ' ' + i.get_output()
                     for i in method.get_instructions())
    return {
        'name': method.get_name(), 'descriptor': method.get_descriptor(),
        'instruction_count': sum(1 for _ in method.get_instructions()),
        'resolved_instruction_sha256': hashlib.sha256(body.encode()).hexdigest(),
    }


def audit(path):
    with zipfile.ZipFile(path) as z:
        dex_bytes = z.read('classes.dex')
    dex = DEX(dex_bytes)
    classes = {c.get_name(): c for c in dex.get_classes()}
    named = PREFIX + 'Controller;' in classes
    controller = PREFIX + 'Controller;' if named else 'Lext/i;'
    keydata = PREFIX + 'KeyData;' if named else 'Lext/r;'
    addpad = '->AddPad(I I I I I I)I' if named else '->a(I I I I I I)I'
    result = {
        'filename': path.name, 'apk_sha256': sha_file(path),
        'classes_dex_sha256': hashlib.sha256(dex_bytes).hexdigest(),
        'family': 'named-original-core' if named else 'obfuscated-ext-family',
        'controller_class': controller, 'keydata_class': keydata,
        'status': 'STATIC INVENTORY ONLY; Vita executes the pinned original core',
        'input_methods': {}, 'virtual_pad_callers': [],
    }
    for name in [controller, keydata]:
        if name not in classes:
            raise ValueError('Unrecognized control class family: ' + path.name)
        result['input_methods'][name] = [fingerprint(m) for m in classes[name].get_methods()]
    for c in classes.values():
        for m in c.get_methods():
            instructions = list(m.get_instructions())
            if any(controller + addpad in i.get_output() for i in instructions):
                item = fingerprint(m)
                item['class'] = c.get_name()
                item['addpad_call_count'] = sum(controller + addpad in i.get_output()
                                              for i in instructions)
                result['virtual_pad_callers'].append(item)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path, nargs='+')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    payload = {
        'schema': 'dbtb-control-research-v1',
        'evidence_level': 'static APK inventory; not hardware validation',
        'fingerprint_definition': 'SHA256 of resolved instruction names/outputs, '
                                  'including registers and branch offsets; equality is '
                                  'strong evidence of identical inspected instruction streams; '
                                  'inequality does not establish changed semantics',
        'apks': [audit(p) for p in args.apk],
    }
    args.output.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + '\n')
    print('Audited', len(payload['apks']), 'APKs; metadata only:', args.output)


if __name__ == '__main__':
    main()
