# Original-style APK with bundled characters

Validated source: user-supplied `gen.apk`, SHA-256
`d52cbd7ef248d995ad17ba6ec8ec6fa08590a344ac2a9786e5ac839bf7715f28`.

This APK is a third data profile distinct from both the original Play Store APK
and the encoded Community14 APK. It keeps ordinary/canonical resource names and
ordinary PAC headers, but bundles the character/card data that the supplied
original APK lacks.

## Layout

The 57 `res/raw/` entries are zero-byte stubs. The actual runtime data lives in
`assets/`: 147 files total, including 108 PACs, 36 OGGs, `loading.png`, `mk.bin`
and an APK-bundled `save.bin`. The current port reads/writes the selected
profile's save: game/save.bin for Original, mods/<Profile>/save.bin for a mod.
Extraction preserves that bundled file; installing it can provide the profile's
initial/current save. Back up existing progress before replacing it. Old saves/
paths are not read automatically; no migration or cross-profile fallback occurs.

`tools/extract_apk_data.py` auto mode now recognizes this pattern: when
`res/raw/` contains only empty stubs and `assets/` contains real payloads, it
selects the ordinary `assets` layout automatically. APKs with non-empty payloads
on both sides remain ambiguous and still require `--layout` explicitly.

```sh
python tools/extract_apk_data.py gen.apk ./install/game
```

Copy the resulting files to `ux0:data/DBTapBattle/game/`, preserving any existing
save deliberately. To retain another base, import it separately instead:

```sh
python tools/extract_apk_data.py gen.apk ./install --mod Gen
```

Copy install/mods/Gen to ux0:data/DBTapBattle/mods/Gen. The dual-APK ZIP helper
pins original raw + Community14 layouts; it is not the Gen import route.
See [DATA_LAYOUT](DATA_LAYOUT.md).

## Runtime compatibility

Host validation against the actual APK confirms:

- all 108 PAC outer tables are ordinary and in bounds;
- all 69 top-level BIN payloads are valid Original-format GameData tables;
- `char00.pac` through `char12.pac` each contain a valid 43-record character table;
- all 13 `char/chardemo/charf` triplets required by `dbtb_installedData()` exist;
- `gamedata.pac` contains the expected 271-record converted table;
- `text00.pac` contains one valid converted text table;
- 405 PNG entries, including nested SPR images, decode successfully and are at
  most 512x512;
- all 36 OGG files are Vorbis at 44.1 kHz (17 stereo BGM, 19 mono SE);
- all 198 PAC `wav` entries are ordinary RIFF/WAVE PCM, mono, signed 16-bit,
  22050 Hz, which the Vita voice backend accepts directly.

## Hybrid charset detail

Container encoding is not sufficient to select the string charset for this APK.
`gamedata.pac` and character GameData remain Shift_JIS, but `text00.pac` is UTF-8
even though its PAC header is ordinary. Its converted text-table bytes match the
normalized Community14 `text00` table.

Older Vita builds selected UTF-8 only when the PAC itself used the Community14
encoded container, so build 00.12 can misdecode this profile's `text00` strings.
Current `main` uses `detectEngineTextEncoding()` on normalized top-level BIN/DAC
payloads and falls back to the container profile only when there is no strong
strict-UTF-8 signal. This preserves:

- original APK -> Shift_JIS;
- Community14 -> UTF-8;
- this APK -> Shift_JIS for game/character data and UTF-8 for `text00`.

Evidence: `docs/evidence/original_plus_characters_apk_2026-10-05.json`.

Status: **FORMAT CONFIRMED + HOST COMPATIBILITY CONFIRMED**. The full engine now
builds as 00.22, but selection/voices and every mode with this exact source hash
still need a physical run identified by its manifest. An Original selector label
alone does not establish which dataset was tested. Do not promote this exact
profile to complete HARDWARE CONFIRMED from generic earlier game observations.

The mixed charset is handled at normalized payload boundaries; ordinary PAC
headers do not imply Shift_JIS text00. 00.20 introduces selective character reads,
texture/PCM reuse and voice sinc reconstruction; its startup regression prevents
a device quality/latency conclusion. Current test: [TEST_VITA_00_22](TEST_VITA_00_22.md).
Current saves have bounded/atomic host coverage, but bundled-save Android
round-trip, all progress fields and a complete profile-isolation matrix remain
pending. See [CURRENT_STATUS](CURRENT_STATUS.md) and [VALIDATION](VALIDATION.md).
