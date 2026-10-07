# Internal resource map

Source: supplied APK 1.4 (hash in APK_AUDIT.md); original GameData,
GameData.SpriteData, TCBManajer._SetAct/_ActReqMain/DrawImage/DrawSprite,
checked against DEX and local jadx 1.5.6 output. The table below describes the
ordinary source contracts; the pinned community normalization is documented
separately. Full engine 00.22 preserves original payload interpreters privately;
see [CURRENT_STATUS](CURRENT_STATUS.md) for device/host scope.

| Format | Original role / layout established | Status and remaining work |
|---|---|---|
| PAC | u16 count + 16-byte LE records; offsets from data block | Ordinary/encoded native readers; source bounds and selective I/O tested; earlier Vita gameplay |
| PNG | 51 exterior entries; 29 more inside six SPR containers | 80 original images including nested sprites host-checked; actual gameplay rendering on earlier Vita builds, exhaustive pixel fidelity pending |
| SPR | Nested PAC-like container containing PNG and BIN; SpriteData owns image array and pData[0] | Six containers confirmed. Back00..03: 1 PNG+BIN; demo08: 12 PNG+BIN; select0: 13 PNG+BIN. Original SpriteData/DrawSprite interpreter retained through AOT; native normalization preserves schema |
| CNV | DrawImage reads nine-byte records: texture index byte, big-endian signed 16-bit x,y,w,h | Source path recovered; cannot treat all CNV as a count/table. Original DrawImage retained; exhaustive format coverage pending |
| DAC | Raw animation commands, action/frame duration, flags, movement, SFX, hitboxes and transforms | Source path recovered. Standard raw header's LE action count at +2, index offset +4, record offset +6; action index signed16, record start = base+index*4. Original action interpreter retained; exhaustive variable-record coverage pending |
| ACT | Original loader data[1] (or binCnv when cnvType=1); present in back00..03 | Original loader/interpreter retained; independent complete semantic map pending; do not confuse it with DAC frame commands |
| BIN | Original loader data[2] or selected binCnv table; SPR BIN drives composed quads in DrawSprite | Multiple BIN schemas. SPR draws positions/UVs and blend flags from metadata, not guessed rectangles. Original consumers retained; verified top-level community BIN normalization, nested SPR BIN untouched |
| GDT | Present in scenarios/card/gamedata/text resources | Not explicitly dispatched by observed GameData branches. Do not confuse tag 'gdt' with gameplay DAC converted to piGameData. Meaning/consumers UNCONFIRMED |
| BMP/DAT/PLT/DB | Found in common/select/card-preview/background-object PACs | Container/hash/type confirmed. No matching branch in the audited original GameData loader; possible authoring/legacy metadata remains UNCONFIRMED. Preserve bytes; do not claim needed runtime decoders |
| OGG / exterior audio | Original/Gen/Android14/Spanish: 17 stereo BGM + 19 mono SE, all real Vorbis 44.1 kHz. Invasion changes 7 `.ogg`-named BGM to 5 MP3 + 2 AAC/M4A; Samu has 12 MP3 + 3 AAC/M4A + 2 Vorbis | Runtime sniffs content: Vorbis uses libvorbisfile, MP3/AAC use Vita SceAudiodec without source conversion. 00.28 hardware confirmed Invasion audio in the tested path; 00.29 fixes one-stream compressed-BGM replacement exposed by Samu |
| WAV | Original GameData loader has WAV slot support (max 20 original, not 30) | 198 Gen RIFF mono PCM16/22050 streams and 198 community wrapped streams host-checked; audible quality pending |
| mk.bin | 392-byte raw resource read by Game9 | Present; complete command/schema meaning PENDING |
| loading.png | 4233-byte standalone raw resource | Present and loader reference confirmed |
| XML | Android manifest/layout/values resources, not a game XML scene system | Replace platform UI/lifecycle; do not add an invented scene XML parser |

## Audited protected-family additions — 2026-10-06

Android14, Spanish Android14 and Invasion Beta 3 use separate per-PAC protected
profiles. Their exact XOR constants, semantic type keys, aliases and corpus
counts are in [APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md) and
[COMMUNITY_MOD_PROFILES](COMMUNITY_MOD_PROFILES.md). Important bounds observed:

- Android14 / Spanish: maximum decoded protected RGBA 512×512.
- Invasion: maximum observed RGBA 736×500 (1,472,000 bytes), in `char15.pac`;
  a 512×512 hard limit would reject valid mod data.
- Android14 / Spanish: 13 contiguous character triplets; Invasion: 22.
- every audited protected `charXX` uses a 43-record converted BIN table.
- nested `spr` containers restart their protected entry index at zero.
- top-level protected WAV is a separate wrapper/ADPCM contract and is unrelated
  to the exterior music files named `*.ogg`.

The engine itself is not rewritten for these differences. Native/Vita resource
adapters restore only verified transport/metadata contracts before handing data
to the preserved original consumers.

## Important format distinctions

**Endianness is per format/field.** PAC fields are LE. DrawImage's CNV
rectangles are BE. DAC header/index and SPR BIN coordinate reads are LE.
Preserve signed byte/short arithmetic and fixed-point scales where used.

`GameData.binCnv` is not a universal decoder. It copies unsigned byte values
into a short array and, for the selected conversion type, reads u16 count plus
8-byte records (`u32 position`, `u16 xsize`, `u16 ysize`). In this APK,
`InitGameData` loads gamedata/text with cnvType=3, selecting DAC conversion:
271 table records in gamedata; one in text. Their positions are in payload bounds.
Most raw animation DACs start with u16 0x1100 and fail that generic table shape;
most CNV does too. The previous guess of a uniform table is rejected.

Raw DAC commands include variable fields selected by bit flags. _ActReqMain
updates duration/motion, hit regions (byte or short coordinates depending on the
high bit), links, alpha, rotation and zoom. A PNG atlas displayed by the bootstrap
proves none of these behaviors. The AOT engine retains the original command interpreter and rendering order;
only its platform data/render boundaries are adapted.

## Reproduce evidence

### Native initial game tables (2026-10-04)

`src/game_data.cpp` now implements the selected `binCnv` table used by original
`InitGameData`: resource 14 (`gamedata.pac`) and 15 (`text00.pac`), cnvType=3
selecting the converted DAC entry. It retains the entire byte payload and decodes
the directory into position/width/height records; cell values remain unsigned
bytes, as in the original Java short array. This does not interpret raw animation
DAC, GDT, strings, CNV draw rectangles or sprite commands.

Original directory: LE u16 count, then count records of LE u32 byte position,
u16 width and u16 height. For the audited Android14 profile, XOR count with
34594, position with uint32(-1887452470) and record index, width with 23261 and
record index, height with 47592 and record index. Indices are table-record
indices, independent of the outer PAC entry index. Payload cell bytes are not
XOR-decoded or replaced with original defaults.

Codec selection follows each resolved PAC. A converted gamedata override and
ordinary text fallback can therefore coexist. Truncated directories, header
overlap and record extents outside the payload fail with empty output. The
complete two-table load is atomic; a corrupt mod override reports an error.
These bounds are native safety constraints, verified on the supplied files.

Host ASan/UBSan validation: both APKs yield 271 game records and one text record.
262 of 39,783 comparable game cells differ (only equal-dimension records compared);
these differences are preserved. Remaining cells and dimension changes are not
included in that statistic. Tests also cover record-index XOR, unsigned 128/255
values, every truncated synthetic payload, coordinate bounds, large offsets and
products, stale-state clearing and mixed-codec VFS fallback. LeakSanitizer is
disabled because this container prevents its /proc thread inspection; address
and undefined-behavior instrumentation remain enabled. That initial host table test is historical; later Vita gameplay uses these
services. It is not exhaustive schema or fidelity proof.

```sh
g++ -std=c++14 -Wall -Wextra -Werror -fno-exceptions -fno-rtti \
  -fsanitize=address,undefined -g -Isrc tests/test_game_data.cpp \
  src/game_data.cpp src/pac.cpp src/vfs.cpp -o /tmp/test_game_data
mkdir /tmp/dbtb-table-test
ASAN_OPTIONS=detect_leaks=0 /tmp/test_game_data /path/to/install /tmp/dbtb-table-test
```

Current VFS regression fixtures use independent first-level profiles, for example
`install/profiles/Original/` and `install/profiles/Android14/`. Use a fresh
temporary test directory each run. Historical corpus results generated before
00.34 may retain their old fixture path names, but the current runtime never
uses `game/` or `mods/` as active profile roots.

```sh
python tools/audit_internal_formats.py /path/to/DBTapBattle.apk docs/evidence/internal_tables.json
python tools/audit_audio.py /path/to/DBTapBattle.apk docs/evidence/audio_inventory.json
```

Candidate tables in the report are deliberately labeled **candidate**. A
plausible count alone is not FORMAT CONFIRMED. Zero-count interpretations of
raw CNV do not establish an empty sprite set. Raw animation DAC index validation
checks only table and record starts, not full variable record bounds.

## Current adapter boundary and corpus — 00.22

`normaliseEnginePac` adapts confirmed Community14 directories, image wrappers,
WAV wrappers and converted **top-level BIN** GameData entries; gamedata/text00
use their verified converted DAC schema. Nested SPR BIN is different and stays
untouched. Native PNG/private RGBA decoders retain straight/premultiplied state.
The unchanged original GameData byte-array parser, SpriteData, DrawImage and
animation/action methods consume the normalized in-memory bytes.

Original+Community14 full regression: 125 outer files, 137 total containers,
470 images, 68 converted BIN tables and 198 WAV wrappers. Gen is a separate
corpus: 108 PACs, 405 PNG entries, 69 top-level BIN tables, 198 ordinary RIFF voices.
These counts do not imply complete semantic recovery for GDT/BMP/DAT/PLT/DB.
Preserve unknown data and do not add guessed runtime decoders.

Original source/character strings are Shift_JIS; Community14 tables are UTF-8;
Gen uses Shift_JIS game/character data with ordinary-header UTF-8 text00.
Content detection at the string boundary prevents lossy conversion. Container
encoding alone is not a charset. Selective PAC filters preserve directory
indices even when excluded payloads are not read; see [PAC_FORMAT](PAC_FORMAT.md).

Private game bytes never enter tests committed to Git. Host probes use supplied
external APK extractions; [VALIDATION](VALIDATION.md) gives commands, fixtures and
which APIs are mocked. Current audible voice quality and 00.22 selection recovery need Vita
confirmation even though decoding/normalization pass on host.

<!-- DBTB_00_23_DETAIL:START -->
## 00.23 resource-loading note

Resource **formats** did not change in the battle-memory fix. The corrected layer is
transport/lifetime: character PACs are exposed to the original Java parser as a
stream backed by native Vita storage/cache ownership. Keep this distinction in
future ports: format conversion and bridge allocation strategy are separate concerns.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

Deep machine-readable evidence: [apk_deep_structure_2026-10-06.json](evidence/apk_deep_structure_2026-10-06.json).

### 00.29 character-string boundary

Protected Android14-family character BIN tables can contain UTF-8 even when the
preserved Gen core's SetString slot arithmetic would otherwise fall back to
Shift_JIS. Invasion char20 provides concrete evidence. The adapter records the
charset of each loaded GameData object and uses the content-detected active
character charset only when the exact slot lookup misses. This changes no PAC
payload or string contents.

### 00.33 protected-PAC ownership handoff

The protected PAC format itself did not change. The hardware issue was the native
buffer handoff after successful normalization. Invasion `char15.pac` normalizes
to roughly 4.64 MiB; 00.32's conditional assignment caused an additional vector
copy before publishing the resource, and the matching Vita coredump ended in
native `std::bad_alloc` on a later fight transition.

00.33 publishes changed PACs with explicit `output.swap(out)`, transferring the
already-normalized vector without a second PAC-sized allocation. The unchanged
ordinary path still copies the original input intentionally. Several consecutive
Invasion fights then passed on physical Vita, closing the reproduced crash path.
This is a transport/ownership rule, not a new PAC codec or data conversion.
