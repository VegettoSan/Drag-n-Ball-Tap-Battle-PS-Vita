"""Audited metadata codecs for known Community14-family Tap Battle APKs.

Names/constants are pinned to supplied classes.dex/resource corpora. Payload bytes
remain untouched. Unknown protected variants are rejected rather than guessed.
"""
import re
import struct


class Profile:
    def __init__(self, name, count_xor, offset_xor, size_xor, type_keys,
                 width_xor, height_xor, table_count_xor, table_position_xor,
                 table_width_xor, table_height_xor, fixed_names, numbered_names):
        self.name = name
        self.count_xor = count_xor
        self.offset_xor = offset_xor
        self.size_xor = size_xor
        self.type_keys = type_keys
        self.width_xor = width_xor
        self.height_xor = height_xor
        self.table_count_xor = table_count_xor
        self.table_position_xor = table_position_xor
        self.table_width_xor = table_width_xor
        self.table_height_xor = table_height_xor
        self.fixed_names = fixed_names
        self.numbered_names = numbered_names


LEGACY = Profile(
    'community14-a210795b', 0xA732, 0x3B681C6B, 0x02D6D26E,
    {0x98FA9490: 'act', 0x4A4AECE2: 'bin', 0x4396461C: 'cnv',
     0x41B6196D: 'dac', 0x42412565: 'rgba', 0x469B0775: 'spr', 0x873BE70D: 'wav'},
    0xF34D, 0x93F9, 0x8722, 0x8F7FC2CA, 0x5ADD, 0xB9E8,
    {'2752': 'common', '1BC2': 'select0', '9B28': 'effect',
     '59F2': 'demo_00', '3C90': 'demo_08', 'D0BD': 'font00',
     '5D73': 'card_preview', 'D67E': 'gamedata', '82B7': 'text00'},
    {'0B49': ('back', 2), 'BDC7': ('bobj', 2), 'E03B': ('char', 2),
     '8AC1': ('chardemo', 2), 'FAFD': ('charf', 4), '47DD': ('card', 3)})

SPANISH = Profile(
    'community14-es-d594affc', 0xE6AA, 0x31874C24, 0x790E6BAF,
    {0x00FC517E: 'bin', 0xEDB419C8: 'cnv', 0xD49FADE7: 'dac',
     0x5EE0F896: 'rgba', 0x03C296FD: 'spr', 0x5BAD42A6: 'wav'},
    0x29CD, 0x42AC, 0x2EA3, 0x07DB0921, 0x941F, 0x126F,
    {'4D7F': 'common', 'B4EB': 'select0', 'B248': 'effect',
     'AC3B': 'demo_00', '8827': 'demo_08', '4919': 'card_preview',
     'EC5A': 'gamedata', 'A602': 'text00'},
    {'0294': ('back', 2), 'D794': ('bobj', 2), 'F298': ('char', 2),
     'AE52': ('chardemo', 2), 'EB21': ('charf', 4), '6FA6': ('card', 3)})

INVASION = Profile(
    'community14-invasion-05aa0c5e', 0x842F, 0x71573ADB, 0x33AC6051,
    {0xAEBFC3A0: 'bin', 0x877E379F: 'cnv', 0x8125D853: 'dac',
     0xE7C20ECB: 'rgba', 0x403D58E7: 'spr', 0x153CCB39: 'wav'},
    0xA42C, 0xED15, 0x68A3, 0x122E64CB, 0xB1D9, 0x7C00,
    {'9036': 'common', '7E8F': 'select0', '1E1C': 'effect',
     '97E6': 'demo_00', '6E24': 'demo_08', '0708': 'card_preview',
     '90EA': 'gamedata', 'D37C': 'text00'},
    {'F813': ('back', 2), '17A5': ('bobj', 2), '0953': ('char', 2),
     '364E': ('chardemo', 2), '91F9': ('charf', 4), '1A4B': ('card', 3)})

PROFILES = (LEGACY, SPANISH, INVASION)
PROFILES_BY_NAME = {p.name: p for p in PROFILES}

# Backwards-compatible constants used by existing tests/tooling for a210795b.
PROFILE = LEGACY.name
COUNT_XOR = LEGACY.count_xor
OFFSET_XOR = LEGACY.offset_xor
SIZE_XOR = LEGACY.size_xor
TYPE_XOR = (-982916625) & 0xffffffff
WIDTH_XOR, HEIGHT_XOR = LEGACY.width_xor, LEGACY.height_xor
TYPES = {1569944959: 'act', (-1893528307) & 0xffffffff: 'bin',
         (-2030065677) & 0xffffffff: 'cnv', (-2065696638) & 0xffffffff: 'dac',
         (-2027371382) & 0xffffffff: 'rgba', (-2081233254) & 0xffffffff: 'spr',
         1112671970: 'wav'}
FIXED_NAMES = LEGACY.fixed_names
NUMBERED_NAMES = LEGACY.numbered_names


def _profile(value):
    if value is None:
        return None
    if isinstance(value, Profile):
        return value
    try:
        return PROFILES_BY_NAME[value]
    except KeyError:
        raise ValueError('unknown Community14 profile: ' + str(value))


def canonical_name(name, profile=None):
    """Normalize confirmed top-level aliases; retain unknown/nested paths."""
    if not name.endswith('.pac') or '/' in name:
        return name
    selected = _profile(profile)
    candidates = (selected,) if selected else PROFILES
    matches = []
    stem = name[:-4]
    for current in candidates:
        logical = current.fixed_names.get(stem)
        if logical:
            matches.append(logical + '.pac')
            continue
        for prefix, (base, digits) in current.numbered_names.items():
            suffix = stem[len(prefix):]
            if stem.startswith(prefix) and re.fullmatch(r'[0-9]{' + str(digits) + r'}', suffix):
                matches.append(base + suffix + '.pac')
                break
    return matches[0] if len(set(matches)) == 1 else name


def profile_from_names(names):
    """Return the unique audited profile identified by protected aliases."""
    scores = []
    for profile in PROFILES:
        score = sum(canonical_name(name, profile) != name for name in names)
        if score:
            scores.append((score, profile))
    if not scores:
        return None
    scores.sort(key=lambda item: item[0], reverse=True)
    if len(scores) > 1 and scores[0][0] == scores[1][0]:
        return None
    return scores[0][1]


def _decoded_table(data, profile):
    if len(data) < 2:
        return None
    count = struct.unpack_from('<H', data)[0] ^ profile.count_xor
    base = 2 + count * 16
    if not count or base > len(data):
        return None
    entries = []
    known = False
    for i in range(count):
        offset, size = struct.unpack_from('<II', data, 2 + i * 16)
        offset ^= profile.offset_xor ^ i
        size ^= profile.size_xor ^ i
        raw_type = struct.unpack_from('>I', data, 10 + i * 16)[0] ^ i
        if base + offset > len(data) or size > len(data) - base - offset:
            return None
        kind = profile.type_keys.get(raw_type, 'unknown')
        known |= kind != 'unknown'
        entries.append(dict(index=i, offset=offset, size=size, type=kind,
                            encoded_type=f'0x{raw_type:08x}',
                            reserved=struct.unpack_from('<I', data, 14 + i * 16)[0]))
    return (base, entries) if known else None


def detect_profile(data):
    matches = [p for p in PROFILES if _decoded_table(data, p) is not None]
    return matches[0] if len(matches) == 1 else None


def looks_encoded(data, profile=None):
    selected = _profile(profile)
    if selected:
        return _decoded_table(data, selected) is not None
    return detect_profile(data) is not None


def table(data, profile=None):
    """Read a protected 16-byte directory using an audited profile."""
    selected = _profile(profile) or detect_profile(data)
    if not selected:
        raise ValueError('unsupported or ambiguous Community14 PAC codec')
    parsed = _decoded_table(data, selected)
    if parsed is None:
        raise ValueError('truncated community PAC table / unsupported codec')
    return parsed


def image_dimensions(payload, index, profile=None):
    selected = _profile(profile) or LEGACY
    if len(payload) < 4:
        raise ValueError('truncated community image dimensions')
    w, h = struct.unpack_from('>HH', payload)
    w ^= selected.width_xor ^ index
    h ^= selected.height_xor ^ index
    if not w or not h or w > 4096 or h > 4096 or w * h * 4 > 16 * 1024 * 1024:
        raise ValueError('community image dimensions exceed texture budget')
    return w, h
