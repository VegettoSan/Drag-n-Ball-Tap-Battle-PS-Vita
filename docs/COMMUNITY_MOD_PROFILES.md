# Community mod codec profiles

This page records the protected resource profiles that have been audited for the
Vita port. It documents interoperability metadata only; no APK, DEX, native
library or commercial game payload is stored in this repository.

## Compatibility baseline

The physical-Vita baseline remains **00.24**, source `f5672d4d`. Its original
and legacy Android14 path is hardware-confirmed. The profile work below is layered
on that baseline and must not be described as hardware-confirmed until a new VPK
is tested on a real Vita.

All protected PACs are detected **per file**. A profile is accepted only when the
decoded directory is fully in bounds and exactly one audited profile matches.
Unknown or mixed private codecs fail instead of being guessed.

| Profile | Source APK SHA-256 | classes.dex SHA-256 | PACs checked | Status |
|---|---|---|---:|---|
| `community14-a210795b` | `a210795bf7ded8636a91bea96df051557229149feb310cf07baf16b0731e79c4` | `f4e52c47fac7f1f6288c4bf7e4d31bc819ebb6ec2d2a6afac2c0275602995184` | 106/106 | format + hardware path previously confirmed |
| `community14-es-d594affc` | `b38cc2c4ae3f20d1b1c6c1419a7b6b62ab57ea8954468874f6c5f8c40b39a098` | `d594affc14328decc5a9d898ab8fed52f83c54e2795cf06973454d9f5385a81c` | 106/106 | format confirmed; Vita gameplay pending |
| `community14-invasion-05aa0c5e` | `caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d` | `05aa0c5ec839161e59b93eccd8657925380c56b46f1a4a452c662b1f115212d1` | 139/139 | format confirmed; Vita gameplay pending |

The three supplied protected APKs use byte-identical `libabc.so` helper builds
for each corresponding ABI. That establishes common loader lineage, not identical
gameplay code.

## Codec constants

| Field | Android14 | Spanish | Invasion Beta 3 |
|---|---:|---:|---:|
| PAC count XOR | `0xA732` | `0xE6AA` | `0x842F` |
| PAC offset XOR | `0x3B681C6B` | `0x31874C24` | `0x71573ADB` |
| PAC size XOR | `0x02D6D26E` | `0x790E6BAF` | `0x33AC6051` |
| RGBA width XOR | `0xF34D` | `0x29CD` | `0xA42C` |
| RGBA height XOR | `0x93F9` | `0x42AC` | `0xED15` |
| GameData count XOR | `0x8722` | `0x2EA3` | `0x68A3` |
| GameData position XOR | `0x8F7FC2CA` | `0x07DB0921` | `0x122E64CB` |
| GameData width XOR | `0x5ADD` | `0x941F` | `0xB1D9` |
| GameData height XOR | `0xB9E8` | `0x126F` | `0x7C00` |
| WAV decoded-size XOR | `0xA732` | `0xE6AA` | `0x842F` |

Type keys are profile-specific and live in `src/community_profiles.hpp` and
`tools/community14.py`. Spanish and Invasion have no observed top-level
`act` entry in the supplied corpora; the implementation deliberately does not
invent a key for an unobserved type.

## Canonical alias families

The extractor renames only audited aliases while preserving file bytes exactly.
The runtime therefore continues to request canonical names.

| Logical resource | Android14 | Spanish | Invasion Beta 3 |
|---|---|---|---|
| common | `2752` | `4D7F` | `9036` |
| select0 | `1BC2` | `B4EB` | `7E8F` |
| effect | `9B28` | `B248` | `1E1C` |
| demo_00 | `59F2` | `AC3B` | `97E6` |
| demo_08 | `3C90` | `8827` | `6E24` |
| card_preview | `5D73` | `4919` | `0708` |
| gamedata | `D67E` | `EC5A` | `90EA` |
| text00 | `82B7` | `A602` | `D37C` |
| backXX | `0B49XX` | `0294XX` | `F813XX` |
| bobjXX | `BDC7XX` | `D794XX` | `17A5XX` |
| charXX | `E03BXX` | `F298XX` | `0953XX` |
| chardemoXX | `8AC1XX` | `AE52XX` | `364EXX` |
| charfXXXX | `FAFDXXXX` | `EB21XXXX` | `91F9XXXX` |
| cardXXX | `47DDXXX` | `6FA6XXX` | `1A4BXXX` |

The supplied Spanish and Invasion APKs do not bundle `font00.pac`; Invasion
also starts its bobj family at `bobj01.pac`. The Vita VFS keeps the original
base installation as fallback for missing shared files.

## Invasion extended-data audit

Invasion Beta 3 contains complete, contiguous character triplets
`char00..21`, `chardemo00..21`, and `charf0000..0021`: **22 characters**.
It also contains 51 card PACs, seven `back00..06` backgrounds and
`bobj01..07`.

The protected corpus contains 139 outer PACs. All 139 uniquely match the Invasion
profile. Their outer records include 731 RGBA entries, 344 WAV entries, 80 BIN
converted tables, 139 CNV, 139 DAC, nine SPR and five preserved unknown metadata
records, with no decoded directory extent outside its file.

The original AOT core is still the source of gameplay behavior. Static capacity
inspection of the original core shows room beyond this mod's 22 supplied
characters, and the native installed-data gate now accepts contiguous complete
triplets beyond the previously hardcoded 13. This is necessary resource support,
not proof that every Invasion-specific Dalvik gameplay modification is reproduced.
Any mechanics that exist only in the mod's `classes.dex` require an explicit
behavioral port.

## Audio compatibility

The hardware-confirmed 00.24 whole-clip Vorbis path is unchanged for BGM whose
decoded PCM is at most 32 MiB. Invasion `bgm_05.ogg` decodes to about 34.47 MiB,
so oversized BGM use a bounded sequential Vorbis stream instead of raising the
00.24 allocation ceiling. Invasion also contains 48 kHz BGM; the existing mixer
already uses each clip's source rate against the Vita 48 kHz output rate.

The streaming branch is build/host work until exercised on physical hardware.
Do not use its existence to claim stutter-free Invasion playback before testing.

## Validation rules for future profiles

A new protected mod is not compatible merely because its filenames look similar.
Record the APK and DEX hashes, alias table, PAC constants, semantic type keys,
image/table/WAV constants, complete character triplets and audio properties.
Validate every PAC before adding the profile. Keep private APK bytes outside Git
and add only synthetic regression fixtures.
