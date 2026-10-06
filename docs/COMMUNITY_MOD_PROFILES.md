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

Original, Gen, Android14 and Spanish carry the same 36 real Vorbis files
byte-for-byte: 17 stereo BGM + 19 mono SE at 44.1 kHz.

Invasion preserves 29/36 of those files, but changes exactly
`bgm_03/04/05/06/07/14/15`. Despite retaining the `.ogg` filenames,
`03/06/07/14/15` are MP3 and `04/05` are AAC-LC inside M4A/ISO-BMFF.

00.27 no converts or renames them. The BGM adapter first retains the already
hardware-proven Vorbis path and, when that fails, sniffs the selected VFS file
by content. MP3 is sent to the Vita hardware MP3 decoder and AAC/M4A is demuxed
to its original AAC access units and sent to the Vita AAC decoder through
`SceAudiodec`.

The supplied Invasion AAC tracks were checked against that contract: both are
44.1 kHz stereo and their largest access units are 455 and 548 bytes,
respectively, below Vita's 1536-byte AAC ES limit. Its MP3 tracks are valid
MPEG Layer III at 44.1 or 48 kHz and also fit the native decoder limits.

Therefore Invasion's seven non-Vorbis BGM are now **implemented as a 00.27
candidate path with source bytes unchanged**. Hardware confirmation remains
pending. Do not modify the original engine or pre-convert the BGM for the 00.27
test; a successful device log should show `Compressed BGM direct:` for the
MP3/AAC cases.

## Validation rules for future profiles

A new protected mod is not compatible merely because its filenames look similar.
Record the APK and DEX hashes, alias table, PAC constants, semantic type keys,
image/table/WAV constants, complete character triplets and audio properties.
Validate every PAC before adding the profile. Keep private APK bytes outside Git
and add only synthetic regression fixtures.

## Detailed offline references

- [Full five-APK technical reference](APK_TECHNICAL_REFERENCE.md)
- [Spanish Android14 audit](SPANISH_ANDROID14_APK.md)
- [Invasion Beta 3 audit](INVASION_BETA3_APK.md)
- [Machine-readable audit evidence](evidence/apk_deep_structure_2026-10-06.json)
