# Dragon Ball Tap Battle PS Vita

Native PS Vita port/reconstruction of **Dragon Ball Tap Battle**, targeting VitaSDK + vitaGL while preserving the original game data formats whenever practical.

> This repository contains port code and documentation only. Original copyrighted game data is **not** committed. Users provide their own legally obtained APK/data.

## Core goals

- Reconstruct the original game behavior on PS Vita instead of redesigning it.
- Use **VitaSDK + vitaGL** for the native Vita build.
- Read the original Tap Battle resource formats directly, especially `.pac`.
- Keep original game data outside the VPK under `ux0:data/DBTapBattle/game/`.
- Keep community mods separately under `ux0:data/DBTapBattle/mods/`.
- Present a boot selector that lets the player choose **Original** or an installed mod.
- Mods use file-level override/fallback: a mod only needs to contain files it changes; missing files fall back to the original game data.
- Preserve a technical history of attempts, successes and failures so dead ends are not repeated.

## Runtime data layout

```text
ux0:data/DBTapBattle/
├── game/                    # Original data extracted from the user's APK
│   ├── common.pac
│   ├── effect.pac
│   ├── font00.pac
│   ├── gamedata.pac
│   ├── select0.pac
│   ├── text00.pac
│   ├── back00.pac
│   └── ...
├── mods/
│   ├── ExampleMod/
│   │   ├── mod.json         # Optional metadata for the Vita selector
│   │   ├── charXX.pac
│   │   └── ...
│   └── AnotherMod/
├── config/
├── logs/
└── saves/
```

File lookup when a mod is active:

```text
1. ux0:data/DBTapBattle/mods/<active-mod>/<requested-file>
2. ux0:data/DBTapBattle/game/<requested-file>
3. Report a missing required resource
```

The original installation is therefore never overwritten by a mod.

## First technical milestone

1. Build and launch a VitaSDK/vitaGL VPK.
2. Create/check the runtime data directories.
3. Detect `game/` and installed mod folders.
4. Let the user choose Original or a mod.
5. Resolve `common.pac` through the virtual filesystem.
6. Parse its original PAC table successfully.
7. Continue toward loading an original texture and rendering it with vitaGL.

Bootstrap 00.02 is **BUILD CONFIRMED** with a real VitaSDK build. It resolves and parses common.pac, decodes its first PNG and includes a diagnostic texture preview. Device execution remains **PENDING**. This is not yet the original menu or gameplay.

## Repository map

- `src/` — native Vita port code.
- `tools/` — PC-side tools, including APK data extraction.
- `docs/` — architecture, format notes and development history.
- `data/` — documentation/placeholders only; copyrighted game assets are not stored here.

## Documentation

- [`docs/ANDROID14_APK.md`](docs/ANDROID14_APK.md) — original/community APK comparison, codec, provenance and mod import.
- [`docs/BUILD.md`](docs/BUILD.md) — build, data preparation and first-test procedure.
- [`docs/PORTING_PLAN.md`](docs/PORTING_PLAN.md) — staged roadmap from bootstrap to gameplay/mod compatibility.
- [`docs/MODS.md`](docs/MODS.md) — mod overlay model and compatibility tiers.
- [`docs/DATA_LAYOUT.md`](docs/DATA_LAYOUT.md) — original/mod runtime directory contract.
- [`docs/PAC_FORMAT.md`](docs/PAC_FORMAT.md) — PAC container format validated against the supplied original APK.
- [`docs/PROJECT_RULES.md`](docs/PROJECT_RULES.md) — non-negotiable project/porting rules.

## Development records

Before repeating an experiment, read:

- [`docs/ATTEMPTS.md`](docs/ATTEMPTS.md) — chronological experiment log.
- [`docs/SUCCESSES.md`](docs/SUCCESSES.md) — confirmed working discoveries/implementations.
- [`docs/FAILURES.md`](docs/FAILURES.md) — failed approaches and why they should not be repeated unchanged.
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — architectural decisions and their rationale.

## Data philosophy

**Adapt the Vita port to Tap Battle, not Tap Battle's assets to Vita.**

Conversion is allowed only when a Vita limitation makes direct use impractical and the reason is documented. Original `.pac` and their internal resources remain the source of truth.

## APK data already validated

The supplied original APK currently yields 57 direct `res/raw/` runtime files: 19 PAC files, 36 OGG files and auxiliary resources. `tools/extract_apk_data.py` prepares these files without converting them and creates a SHA-256 manifest.

## Current status

- APK architecture: all 77 files inventoried; 91 DEX classes inspected.
- PAC outer container: FORMAT CONFIRMED.
- Original raw-data extraction: FORMAT CONFIRMED.
- VitaSDK/vitaGL bootstrap implementation: committed.
- Original/mod VFS overlay: host regressions pass; Vita build confirmed.
- Boot selector: physical/touch implementation compiled; device verification pending.
- C++ PAC reader: all 19 original PACs and every entry read on host; Vita build confirmed.
- PNG: all 51 exterior PAC textures decoded on host; GPU preview pending runtime verification.
- BUILD CONFIRMED: **yes**, bootstrap 00.02.
- VITA3K / HARDWARE CONFIRMED: **not yet**.
- Original APK lacks charNN/chardemoNN/charf00NN resources needed for complete gameplay.

## Independent audit and device test

- [Audit results and next milestone](docs/AUDIT_STATUS.md)
- [APK inventory and reference qualification](docs/APK_AUDIT.md)
- [Internal formats](docs/RESOURCE_FORMATS.md)
- [Original engine map](docs/ENGINE_MAP.md)
- [Exact renderer API mapping](docs/RENDER_MAPPING.md)
- [Audio/input/save/external-data/Bluetooth](docs/PLATFORM_SERVICES.md)
- [Third-party library notices](docs/THIRD_PARTY.md)

Original/mod lookup and container parsing are implemented; gameplay compatibility
with actual community mods has not been established. mod.json remains optional
and ignored. No original assets or decompiled game sources are distributed.

## Community Android 14 data support

The user-supplied Android14 APK is now supported at the **resource/import level**:
144 assets, encoded PAC tables and raw-DEFLATE premultiplied RGBA textures. Import
into an isolated mod folder, preserving the original installation:

```sh
python tools/extract_apk_data.py /path/to/community.apk ./install --mod Android14
```

Copy install/mods/Android14/ to ux0:data/DBTapBattle/mods/Android14/. Original
res/raw and ordinary assets layouts remain supported. Bytecode/native Android
helpers are not imported. Mods retaining the audited names/codec use the same
route; new code behavior or encoding constants need a separately recovered port.

Host validation passed for both APKs: 125 outer PACs + 12 nested SPRs, 470 decoded
textures and nine extractor regressions. New native preview source handles
ordinary PNG and community RGBA with the corresponding alpha blend. This change
has **not** been built/tested on Vita here: historical bootstrap 00.02 VPK evidence
applies to its recorded source commit, not automatically to these new changes.
Gameplay, converted community tables and WAV playback remain pending.
See ANDROID14_APK.md for exact differences, confirmed shared-library provenance,
unknown distributor and full commands/evidence.
