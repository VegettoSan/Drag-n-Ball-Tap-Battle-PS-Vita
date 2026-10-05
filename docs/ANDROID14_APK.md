# Community Android 14 APK audit — 2026-10-04

Baseline: main `a04e264`. This document concerns the two user-supplied files;
"Android 14" is their supplied label, not proof of an Android 14 OS test.
This remains a pinned source audit; current Vita checkpoint and later hardware
results are in [CURRENT_STATUS](CURRENT_STATUS.md).

| Property | DBTapBattle.apk | tap battle android 14.apk |
|---|---|---|
| SHA-256 | `b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b` | `a210795bf7ded8636a91bea96df051557229149feb310cf07baf16b0731e79c4` |
| APK bytes | 16,050,451 | 72,391,449 |
| Version / code | 1.4 / 7 | 1.4 / 7 |
| Package | com.namcobandaigames.dragonballtap.apk | com.namcobandaigames.apk |
| Min / target SDK | 9 / unspecified | 8 / 29 |
| DEX classes | 91 | 39 (largely renamed to ext.*) |
| Launcher | dragonballtap | Primary extends ext.F |
| Resource location | res/raw/ (57 files) | assets/ (144 files) |
| PACs | 19 ordinary PACs | 106 encoded PACs |
| Native libraries | None | libabc.so in seven Android ABIs |
| Certificate subject | NAMCO BANDAI Games Inc. | Generic Android certificate |

## Confirmed differences

Logical-name comparison: **37 identical resources, 18 changed, 2 absent, 89 added**.
All 36 Ogg files and mk.bin are byte-identical. loading.png differs. The 17
mapped PACs have different bytes and often different entry counts; bobj00.pac
and font00.pac are not bundled in the community APK. D0BD is the font alias in
DEX, but that file is absent. Preserve original fallback for those resources.

The 89 added files are char00–12, chardemo00–12, charf0000–0012 (39 files) and
card001–050 (50 files). The original already has card000. This establishes 13
indexed character triplets and 51 indexed card files, not 13 universally playable
characters in every possible version/mod. The original character/menu/battle
logic is now retained via AOT, with earlier Vita gameplay confirmed; exhaustive
character/mode compatibility remains pending.

All 106 outer tables and six nested SPR tables fit their files. There are no
outer overlaps or trailing bytes. Outer records include 361 compressed RGBA
textures, 198 wav-tagged payloads, CNV/DAC/BIN/SPR and five ignored metadata
records. Nested SPR adds 29 RGBA textures: **390 textures decoded in the audit**.
The 198 top-level WAV payloads use a verified private wrapper and, when compressed,
PlayStation ADPCM-style 16-byte frames rather than ordinary PCM/Ogg bytes. Build
00.12 now decodes those streams natively before the unchanged original SoundEffect
path. Raw CNV/DAC schemas must not be conflated with converted tables.

**65 community textures corresponding to resources in the original APK match
original PNGs exactly after floor(RGB × alpha / 255) premultiplication**, including
nested SPR textures. Ordinary RGBA hashes differ because the original PNG is
straight-alpha and the community upload is premultiplied. This is a packaging
change for these textures, not evidence of changed artwork. Native rendering
must select GL_ONE rather than multiplying these RGB values by alpha again.
CNV and raw DAC payloads in common.pac are byte-identical to the original.
Converted gamedata/text tables use additional XOR fields and require separate
metadata support. Unknown/ignored records and removed BMP/DAT/PLT/DB entries
are recorded in the comparison; do not invent decoders for them.

DEX changes include replacing original class/method/field names with ext.*;
loading resources through AssetManager; native texture/file/sound helpers;
changed launcher/package; removal of the billing permission and the declared
Smap activity. Storage, Internet, Bluetooth/admin and vibration permissions
remain. Two original Smap device/news URLs remain in DEX. Android14 SAF/classes
seen in the separate GdGohan SWB are **not evidence they exist in this APK**.
This inventory is not a proof that battle rules are unchanged, that all network
code is removed, or that every modification to obfuscated methods is recovered.

## Name mapping (confirmed resource slots)

Derived from original TCBManajer.strDataFolder2 and community ext.o.a[17], with
common/select/demo/card metadata corroboration. The extractor normalizes paths
only; it must preserve every payload byte and retain the APK path/hash manifest.

| Community basename / prefix | Logical basename / prefix |
|---|---|
| 2752 | common |
| 1BC2 | select0 |
| 0B49 + 2 digits | back + same digits |
| BDC7 + 2 digits | bobj + same digits |
| E03B + 2 digits | char + same digits |
| 8AC1 + 2 digits | chardemo + same digits |
| FAFD + 4 digits | charf + same digits (charf0000–0012) |
| 9B28 | effect |
| 59F2 | demo_00 |
| 3C90 | demo_08 |
| D0BD | font00 (absent) |
| 5D73 | card_preview |
| D67E | gamedata |
| 82B7 | text00 |
| 47DD + 3 digits | card + same digits |

## Encoded profile contract

Profile name: `community14-a210795b`. Values are verified from ext.o initialization
and ext.u table reads in this exact DEX; they are not universal mod constants.

- Count: LE u16 XOR 42802; table/base still `2 + count * 16`.
- Entry i offset: LE u32 XOR 996678763 XOR i.
- Size: LE u32 XOR 47633006 XOR i.
- Type: BE u32 XOR unsigned(-982916625) XOR i; reserved word retained untouched.
- Texture dimensions: BE u16 XOR 62285 XOR i (width), XOR 37881 XOR i (height).
- Texture bytes after those four bytes: raw DEFLATE (`windowBits=-15`), exactly
  width × height × 4 bytes, uploaded GL_RGBA/GL_UNSIGNED_BYTE by libabc.so.
- Nested SPR uses the same profile, restarting the record index at zero.
- Converted tables: count XOR 34594, offset XOR unsigned(-1887452470) XOR i,
  X/Y sizes XOR 23261/47592 XOR i. Native normalization is implemented for
  verified top-level `bin` GameData entries and the converted DAC tables in gamedata/text00;
  nested SPR `bin` payloads are a separate schema and remain untouched; raw CNV and
  other DAC schemas are deliberately not passed through this decoder.
- WAV wrapper decoded length: LE u32 XOR 42802 XOR i, followed by an encoding
  flag. Flag 0 exposes wrapped PCM bytes; compressed streams use 16-byte
  PlayStation ADPCM frames (28 mono samples per frame) with predictor/shift in
  byte 0 and control/end flag in byte 1. Build 00.12 restores signed 16-bit PCM
  before SoundEffect. Do not treat these payloads as Ogg streams.

No Java/Dalvik or Android .so is run on Vita. These bounded metadata/pixel/audio
operations are reimplemented independently in native C++ from the verified format
contract. Other mods changing names/constants/code need a newly audited profile,
not silent trial decoding or an assumption of compatibility.

## Provenance: confirmed relationship, unconfirmed publisher

Public reference: [GdGohan/Dragon-Ball-Tap-Battle-Decompilation](https://github.com/GdGohan/Dragon-Ball-Tap-Battle-Decompilation),
commit `f4a275dec5a73bb8f88c0162b0fb056c3c5b46d8`. Its README describes a
Sketchware Pro project. SWB SHA-256:
`f9573eb8170650cba8381c14a7403745151da2b3761eb01ad7b1bcd02ebf56f7`.

**All seven libabc.so files in the supplied APK are byte-identical to the
SWB's data/files/native_libs counterparts.** Shared loader lineage is confirmed;
GdGohan being the original author or publisher of this particular APK is not.
The generic Android signing subject does not identify its packager. Public
search also found Android14 gameplay/repost listings, but no authoritative
release with this APK SHA-256 or a patch changelog. Origin/distributor remains
unconfirmed until an exact release URL/hash is provided. Do not misattribute
community authorship or claim this is the same APK as a similarly titled mod.

The SWB exposes PrivGameData and AndroidGLTexture descriptions of this loader;
these corroborate the actual DEX/native ABI and corpus observations. It also
contains unrelated later modifications. No archive Java or .so is committed,
linked into the port, or given an assumed open-source license.

## Reproduce

```sh
# Audit tools: Python + androguard 4.1.4; pixel comparison additionally Pillow.
python tools/audit_apk.py community.apk docs/evidence/android14_inventory.json --dex
python tools/compare_apks.py original.apk community.apk docs/evidence/android14_comparison.json \
  --dex --pixels --reference-swb /path/to/public-reference.swb
```

Reports contain paths, hashes, dimensions, API/class names and format facts;
no asset bytes or decompiled commercial source. Native import/preview tests
and limitations are recorded with their implementation in subsequent commits.

## Import and installed-mod contract

The extractor now supports `--layout auto|raw|assets|community14`. Auto rejects
archives with actual payloads on both raw/assets sides instead of choosing
arbitrarily; Gen-style empty raw stubs with real assets are a verified exception.
It recognizes the
verified aliases and validates the encoded tables before publication. `assets`
is also available for ordinary named asset mods. Unknown extensions are retained
and listed. All earlier path/symlink/CRC/conflict/budget protections remain.

```sh
# Preserve the original installation.
python tools/extract_apk_data.py original.apk ./install/game
# Place the complete community dataset in a separately selectable overlay.
python tools/extract_apk_data.py community.apk ./install --mod Android14
# The same import route handles mods retaining this APK's names/codec.
python tools/extract_apk_data.py my-community-mod.apk ./install --mod MyMod
```

Copy `install/game/` and `install/mods/` into `ux0:data/DBTapBattle/`. Keep the
original base available for missing bobj00/font00 resources. Importing the
community APK directly as the sole base is not an equivalent complete dataset;
required shared files need an explicitly audited compatible source.
Do not merge it over an existing original install. `--overwrite` operates only
on the selected destination. `--mod NAME` confines output to `OUTPUT/mods/NAME`.

Format-3 dbtb_manifest.json records source SHA, layout/profile, all 106 alias
renames, original APK paths, untouched content hashes and omitted Android files.
It is import provenance; the native reader detects PAC codecs per file so a
missing override can still resolve an ordinary original PAC. No Android .so,
DEX, resources.arsc or signing metadata is installed as game data.

Historical import checkpoint: nine extractor regressions passed, including alias collisions,
different-codec refusal, ambiguous archives, safe nested assets and CLI protection
of an existing game/common.pac. Both real APK extractions pass; every extracted
file matches its APK entry byte-for-byte and all 13 triplets have canonical names.

## Native support and verified limit

PacFile now reads original tables and this encoded profile, preserving unknown
record IDs. The preview decodes either original PNG or community raw-DEFLATE
RGBA directly into memory, records the codec in runtime.log and uses the correct
alpha blend. Existing VFS overlay/fallback needs no new global codec switch.
The actual new-source common preview is original 9 entries/community 6 entries,
each with a first 512×512 atlas. No Android binary is required at runtime.

Host ASan/UBSan tests read **125 outer PACs + 12 nested SPR containers** and
**470 textures (80 original including nested sprites, 390 community)**. Original
PNG corruption/budget regressions pass. Community tests cover wrong indexes,
truncated/trailing/incorrect-size DEFLATE, allocation limits, metadata-first PAC,
out-of-range encoded table entries, memory limits and clean state after failure.
The full resource regression additionally validates **68 top-level converted BIN
GameData tables and 198 WAV streams**, with decoded WAV lengths matching wrapper
metadata and nonzero PCM output. Nine extractor tests and real-byte-preservation
checks also pass.

Physical Vita build 00.11 is HARDWARE CONFIRMED through character selection and
an actual playable battle using the Android14 profile. Later checkpoints fixed the
observed stalls/voice/text regressions in stages: 00.18 restored stable battle
performance, 00.19 restored text, 00.21 recovered audio worker/menu startup, and
00.22 accepted the original 187/251 selection masks with clean audio/selection reported on hardware. 00.22 then exposed a separate whole-PAC TeaVM allocation failure at battle startup. 00.23 restores the original streaming PAC parser through a native-backed stream; the reported physical-Vita retest enters and plays a battle with no error observed in that session.

Current extractor suite has 12 tests (the nine-test counts above are historical
checkpoints). See [VALIDATION](VALIDATION.md) for fixtures/commands and scope.
Different private constants must fail explicitly or receive a new reviewed
codec profile. Code-dependent mod mechanics are not assumed compatible merely
because asset encoding matches. Do not reinterpret an audio setup failure as
proof of a PAC/ADPCM codec failure. Profile-local saves and source manifests:
[DATA_LAYOUT](DATA_LAYOUT.md), [MODS](MODS.md).

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
