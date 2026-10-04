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
└── config.ini
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

## Repository map

- `src/` — native Vita port code.
- `tools/` — PC-side tools, including APK data extraction.
- `docs/` — architecture, format notes and development history.
- `data/` — documentation/placeholders only; copyrighted game assets are not stored here.

## Development records

Before repeating an experiment, read:

- [`docs/ATTEMPTS.md`](docs/ATTEMPTS.md) — chronological experiment log.
- [`docs/SUCCESSES.md`](docs/SUCCESSES.md) — confirmed working discoveries/implementations.
- [`docs/FAILURES.md`](docs/FAILURES.md) — failed approaches and why they should not be repeated unchanged.
- [`docs/DECISIONS.md`](docs/DECISIONS.md) — architectural decisions and their rationale.

## Data philosophy

**Adapt the Vita port to Tap Battle, not Tap Battle's assets to Vita.**

Conversion is allowed only when a Vita limitation makes direct use impractical and the reason is documented. Original `.pac` and their internal resources remain the source of truth.

## Current status

Repository bootstrap in progress. The original APK has already been inspected and the base PAC container structure has been validated; see `docs/PAC_FORMAT.md`.
