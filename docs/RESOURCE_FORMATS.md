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
| OGG | Vorbis, 17 stereo BGM + 19 mono effects; all 44.1 kHz | Native Vorbis/PCM services; earlier Vita BGM/SE audible; 00.21 worker/menu recovery confirmed |
| WAV | Original GameData loader has WAV slot support (max 20 original, not 30) | 198 Gen RIFF mono PCM16/22050 streams and 198 community wrapped streams host-checked; audible quality pending |
| mk.bin | 392-byte raw resource read by Game9 | Present; complete command/schema meaning PENDING |
| loading.png | 4233-byte standalone raw resource | Present and loader reference confirmed |
| XML | Android manifest/layout/values resources, not a game XML scene system | Replace platform UI/lifecycle; do not add an invented scene XML parser |

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

The corpus expects original files in install/game and the audited community
files in install/mods/Android14. Use a fresh temporary test directory each run.

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
