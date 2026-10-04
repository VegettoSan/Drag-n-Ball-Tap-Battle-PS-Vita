# APK audit — 2026-10-04

Baseline: main `1e3699b`. Source: supplied `DBTapBattle.apk` version 1.4,
version code 7, package `com.namcobandaigames.dragonballtap.apk`, minimum SDK 9.
SHA-256: `b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`.

Reproduction:

```sh
python tools/audit_apk.py /path/to/DBTapBattle.apk docs/evidence/apk_inventory.json --dex
```

`--dex` requires androguard (audited with 4.1.4). Without it ZIP/PAC inspection
uses only the Python standard library. The report records all 77 APK files,
sizes, compression methods, SHA-256 hashes, every outer PAC record, nested SPR
containers, PNG dimensions, DEX classes/method names and actual GL/Android API
calls. It contains no game payloads or decompiled source.

## Confirmed data packaging

- 57 raw resources: 19 PAC, 36 Ogg, `loading.png`, `mk.bin`.
- 77 ZIP files overall: also Android manifest/resources, drawable PNGs, XML and
  signing metadata. No `assets/` dataset, no native `lib/*.so`.
- 91 DEX classes, including original game, Android adapters and Smap billing.
- Every outer PAC table is little-endian, 16-byte records, data-relative
  offsets. All outer entries fit; reserved fields are zero; no overlaps or
  trailing bytes. Payloads are contiguous without an alignment requirement.
- No `charXX.pac` is bundled. `card000.pac`, `card_preview.pac`, `demo_00.pac`
  and `demo_08.pac` are present. **Bundled raw data is not a complete playable
  character installation.** External data must be inventoried before battle.
- SPR is itself a PAC-like container with PNGs and BIN metadata. Opening the
  outer PAC alone does not decode original sprites or animations.

## Baseline code review

All six native source/header files, CMake, extractor and existing documents
were reviewed. The bootstrap selects a folder and parses `common.pac`; it
does not yet read an entry, decode a texture, play audio or execute game logic.

The original PAC arithmetic is correct. Remaining weaknesses include an
unbounded entry allocation, state after parse failure, VFS NUL/control-path
acceptance, inaccurate original-data presence, unreported directory failures,
extractor partial writes and unguarded manifest replacement, and input held
across screens dismissing the result. These need regression validation.

## Reference qualification

GdGohan repository commit `f4a275dec5a73bb8f88c0162b0fb056c3c5b46d8`
contains a README and a Sketchware `.swb` ZIP, not a ready native decompilation.
The archive includes Java source, modified/private loaders, Android 14 storage
code, additional activities and game data. These are **not** all present in
the supplied original DEX. For example `AndroidData`, `PrivGameData`, Wi-Fi and
music/video activities are community additions; `ResourceMiner` is commented
out there but implemented in the supplied APK. Do not import the whole archive
or treat its mods/changed game-state wiki as original behavior.

Native Vita build and execution are PENDING until a real toolchain and runtime
test establish them. Host format tests cannot establish Vita behavior.

## Supplied Android14 variant follow-up

The above audit is the original APK baseline. The second supplied APK is now
compared in ANDROID14_APK.md, with hashes, alias/profile contracts and independent
metadata evidence. It has a genuine Android native *helper* layer, not a complete
native game engine; the original Java-game reconstruction decision still applies.
It supplies indexed character/card data missing from the baseline. Support is
resource-level and host-tested; actual native menus/battle/audio remain pending.
