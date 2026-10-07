# APK audit — 2026-10-04

Historical audit baseline: main `1e3699b`. Current engine status is maintained
in [CURRENT_STATUS](CURRENT_STATUS.md); baseline findings below describe that
source and early bootstrap, not the present implementation. Source: supplied `DBTapBattle.apk` version 1.4,
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

## Historical baseline code review

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

At this audit baseline native Vita build/execution had not yet been established.
Subsequent private AOT builds run original menu/selection/combat on Vita. By 00.23,
physical testing reports clean audio, responsive selection and successful battle startup/gameplay for the tested path. Host format tests alone still cannot establish hardware behavior. Baseline weaknesses were investigated in later attempts; see
[ATTEMPTS](ATTEMPTS.md), [AUDIT_STATUS](AUDIT_STATUS.md) and [VALIDATION](VALIDATION.md).

## Supplied Android14 variant follow-up

The above audit is the original APK baseline. The second supplied APK is now
compared in ANDROID14_APK.md, with hashes, alias/profile contracts and independent
metadata evidence. It has a genuine Android native *helper* layer, not a complete
native game engine; the original core is preserved through private Java-to-C AOT (ADR-011).
It supplies indexed character/card data missing from the baseline. Earlier Vita
00.11 verifies its selection/battle and BGM/SE. Packed voices later decode; subsequent 00.22/00.23 hardware testing reports clean audio/voices in the tested path. The ordinary-name bundled-character Gen
profile is separately audited in [ORIGINAL_PLUS_CHARACTERS_APK](ORIGINAL_PLUS_CHARACTERS_APK.md);
an Original selector label identifies a folder, not a particular APK hash.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.33 (2026-10-07):** physical Vita testing
> confirms the reproduced Invasion repeated-fight/Saitama→Freezer crash is fixed
> after the protected-PAC ownership-transfer repair. The recent hardware sequence
> also confirms Loading recovery and dynamic installed rosters, including Samu's
> 92 characters. Scope is limited to tested paths; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## Deep offline reference — 2026-10-06

A newer forensic pass compares all five supplied APKs at ZIP/manifest/DEX/data/PAC/audio/native-library level so future work does not require the binaries. See [APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md) and [machine-readable evidence](evidence/apk_deep_structure_2026-10-06.json). Profile-specific details are in [SPANISH_ANDROID14_APK](SPANISH_ANDROID14_APK.md) and [INVASION_BETA3_APK](INVASION_BETA3_APK.md). The deep audit also establishes that seven Invasion BGM files retain `.ogg` names while containing MP3/AAC media; do not generalize the baseline Vorbis contract to that mod.

## Runtime hardware update — 00.33

The deep APK audit remains format/provenance evidence, but later runtime work now
has corresponding hardware confirmation. 00.32/00.33 physical testing confirms
dynamic installed rosters (including Samu's 92-character dataset) and resolves
the reproducible Invasion repeated-fight crash. The latter was not a malformed
APK/PAC: the matching coredump identified a duplicate native vector allocation
at the final protected-PAC normalization handoff. 00.33 transfers that buffer by
ownership and several subsequent Invasion fights complete without a crash.

Keep this distinction explicit: APK structure validation proves the data shape;
hardware tests prove only the runtime paths actually exercised.
