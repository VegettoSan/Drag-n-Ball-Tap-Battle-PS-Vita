"""Pinned metadata codec for the supplied community Android-14 APK.

Constants/names are verified against classes.dex ext.o/ext.u, not inferred from
hex-looking filenames. Payloads remain untouched. This is not a generic decoder
for other private/obfuscated mods with different constants.
"""
import re
import struct

PROFILE = 'community14-a210795b'
COUNT_XOR = 42802
OFFSET_XOR = 996678763
SIZE_XOR = 47633006
TYPE_XOR = (-982916625) & 0xffffffff
WIDTH_XOR, HEIGHT_XOR = 62285, 37881
TYPES = {1569944959: 'act', (-1893528307) & 0xffffffff: 'bin',
         (-2030065677) & 0xffffffff: 'cnv', (-2065696638) & 0xffffffff: 'dac',
         (-2027371382) & 0xffffffff: 'rgba', (-2081233254) & 0xffffffff: 'spr',
         1112671970: 'wav'}
# Logical resource slots in original TCBManajer.strDataFolder2 and ext.o.a[17].
FIXED_NAMES = {'2752': 'common', '1BC2': 'select0', '9B28': 'effect',
               '59F2': 'demo_00', '3C90': 'demo_08', 'D0BD': 'font00',
               '5D73': 'card_preview', 'D67E': 'gamedata', '82B7': 'text00'}
NUMBERED_NAMES = {'0B49': ('back', 2), 'BDC7': ('bobj', 2),
                  'E03B': ('char', 2), '8AC1': ('chardemo', 2),
                  'FAFD': ('charf', 4), '47DD': ('card', 3)}


def canonical_name(name):
    """Normalize only confirmed top-level PAC aliases; retain unknown paths."""
    if not name.endswith('.pac') or '/' in name:
        return name
    stem = name[:-4]
    if stem in FIXED_NAMES:
        return FIXED_NAMES[stem] + '.pac'
    for prefix, (logical, digits) in NUMBERED_NAMES.items():
        suffix = stem[len(prefix):]
        if stem.startswith(prefix) and re.fullmatch(r'[0-9]{' + str(digits) + '}', suffix):
            return logical + suffix + '.pac'
    return name


def looks_encoded(data):
    if len(data) < 18:
        return False
    count = struct.unpack_from('<H', data)[0] ^ COUNT_XOR
    if not count or 2 + count * 16 > len(data):
        return False
    return any((struct.unpack_from('>I', data, 10 + i * 16)[0] ^ TYPE_XOR ^ i) in TYPES
               for i in range(count))


def table(data):
    """Read ext.u's 16-byte records; unknown type IDs are retained as metadata."""
    if len(data) < 2:
        raise ValueError('truncated community PAC count')
    count = struct.unpack_from('<H', data)[0] ^ COUNT_XOR
    base = 2 + count * 16
    if base > len(data):
        raise ValueError('truncated community PAC table / unsupported codec')
    entries = []
    for i in range(count):
        offset, size, _, reserved = struct.unpack_from('<IIII', data, 2 + i * 16)
        offset ^= OFFSET_XOR ^ i
        size ^= SIZE_XOR ^ i
        tag = struct.unpack_from('>I', data, 10 + i * 16)[0] ^ TYPE_XOR ^ i
        if base + offset > len(data) or size > len(data) - base - offset:
            raise ValueError(f'community PAC entry {i} outside container / unsupported codec')
        entries.append(dict(index=i, offset=offset, size=size,
                            type=TYPES.get(tag, 'unknown'), encoded_type=f'0x{tag:08x}',
                            reserved=reserved))
    return base, entries


def image_dimensions(payload, index):
    if len(payload) < 4:
        raise ValueError('truncated community image dimensions')
    w, h = struct.unpack_from('>HH', payload)
    w ^= WIDTH_XOR ^ index
    h ^= HEIGHT_XOR ^ index
    if not w or not h or w > 4096 or h > 4096 or w * h * 4 > 16 * 1024 * 1024:
        raise ValueError('community image dimensions exceed texture budget')
    return w, h
