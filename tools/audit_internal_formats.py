#!/usr/bin/env python3
"""Record internal format metadata, never game payloads.

Important: binCnv's 8-byte table is selected by cnvType, not universal to CNV/DAC.
Raw animation DAC has a different header/index. This tool validates only bounds,
not complete record semantics or gameplay behavior.
"""
import argparse
import json
from pathlib import Path
import struct
import zipfile
from audit_apk import pac_inventory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path)
    parser.add_argument('report', type=Path)
    args = parser.parse_args()
    result = []
    with zipfile.ZipFile(args.apk) as archive:
        for name in sorted(archive.namelist()):
            if not name.endswith('.pac'):
                continue
            data = archive.read(name)
            pac = pac_inventory(data)
            for entry in pac['entries']:
                if entry['type'] not in {'cnv', 'dac', 'act', 'gdt', 'bin', 'db', 'dat', 'plt', 'bmp'}:
                    continue
                start = pac['data_base'] + entry['offset']
                payload = data[start:start + entry['size']]
                item = dict(pac=name, entry=entry['index'], type=entry['type'], size=len(payload))
                if len(payload) >= 2 and entry['type'] in {'cnv','dac','act','gdt'}:
                    count = struct.unpack_from('<H', payload)[0]
                    item['first_u16'] = count
                    if 2 + count * 8 <= len(payload):
                        fields = [struct.unpack_from('<IHH', payload, 2+i*8) for i in range(count)]
                        item['candidate_8byte_table'] = dict(count=count, table_end=2+count*8,
                            offset_min=min((f[0] for f in fields), default=0),
                            offset_max=max((f[0] for f in fields), default=0),
                            offsets_inside_payload=all(f[0] < len(payload) for f in fields))
                    if entry['type'] == 'dac' and count == 0x1100 and len(payload) >= 8:
                        actions, indices, records = struct.unpack_from('<HHH', payload, 2)
                        table_ok = indices + actions * 2 <= len(payload)
                        offsets = struct.unpack_from('<'+'h'*actions, payload, indices) if table_ok else []
                        item['animation_dac_index'] = dict(action_count=actions, index_offset=indices,
                            record_offset=records, index_bounds_valid=table_ok,
                            record_starts_in_bounds=table_ok and all(x == -1 or (x >= 0 and records+x*4 < len(payload)) for x in offsets))
                result.append(item)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2)+'\n')
    print(f'{len(result)} internal entries inspected')


if __name__ == '__main__':
    main()
