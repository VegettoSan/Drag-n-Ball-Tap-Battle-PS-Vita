# Internal resource map

Source: supplied APK 1.4 (hash in APK_AUDIT.md); original GameData,
GameData.SpriteData, TCBManajer._SetAct/_ActReqMain/DrawImage/DrawSprite,
checked against DEX and local jadx 1.5.6 output. Community changes are excluded.

| Format | Original role / layout established | Status and remaining work |
|---|---|---|
| PAC | u16 count + 16-byte LE records; offsets from data block | FORMAT CONFIRMED on all 19; native reader tested on host and compiled for Vita |
| PNG | 51 exterior entries; 29 more inside six SPR containers | 51 exterior PNG decoded by native host decoder; GPU/premultiplication PENDING |
| SPR | Nested PAC-like container containing PNG and BIN; SpriteData owns image array and pData[0] | Six containers confirmed. Back00..03: 1 PNG+BIN; demo08: 12 PNG+BIN; select0: 13 PNG+BIN. Native sprite command decoding PENDING |
| CNV | DrawImage reads nine-byte records: texture index byte, big-endian signed 16-bit x,y,w,h | Source path recovered; cannot treat all CNV as a count/table. Full bounds/record coverage PENDING |
| DAC | Raw animation commands, action/frame duration, flags, movement, SFX, hitboxes and transforms | Source path recovered. Standard raw header's LE action count at +2, index offset +4, record offset +6; action index signed16, record start = base+index*4. Full variable flag records PENDING |
| ACT | Original loader data[1] (or binCnv when cnvType=1); present in back00..03 | Role and loader confirmed; complete semantics PENDING; do not confuse it with DAC frame commands |
| BIN | Original loader data[2] or selected binCnv table; SPR BIN drives composed quads in DrawSprite | Multiple BIN schemas. SPR draws positions/UVs and blend flags from metadata, not guessed rectangles. Full decoders PENDING |
| GDT | Present in scenarios/card/gamedata/text resources | Not explicitly dispatched by observed GameData branches. Do not confuse tag 'gdt' with gameplay DAC converted to piGameData. Meaning/consumers UNCONFIRMED |
| BMP/DAT/PLT/DB | Found in common/select/card-preview/background-object PACs | Container/hash/type confirmed. No matching branch in the audited original GameData loader; possible authoring/legacy metadata remains UNCONFIRMED. Preserve bytes; do not claim needed runtime decoders |
| OGG | Vorbis, 17 stereo BGM + 19 mono effects; all 44.1 kHz | Format probed. No native audio playback yet |
| WAV | Original GameData loader has WAV slot support (max 20 original, not 30) | Not found in bundled outer PACs; may exist in absent downloaded character packages. PCM semantics require external examples |
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
proves none of these behaviors. Native equivalents must retain the original
command interpreter and rendering order.

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
and undefined-behavior instrumentation remain enabled. No Vita runtime proof.

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
