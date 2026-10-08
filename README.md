# Dragon Ball Tap Battle PS Vita

> **Ready to publish: v1.1 — Universal Mod Support** — Vita `APP_VER 01.01`, `TITLE_ID DBTB01178`.
> **Hardware-confirmed:** the updated VisualQuality engine plays successive battles in the heavy `dbz_mobile_v9` mod, with no crash or freeze in the user's accepted test. Previous public **v1.0 / 00.34** remains the historical fallback.
> The release VPK contains the *exact* hardware-tested `eboot.bin`; only `sce_sys/param.sfo` differs from the approved experimental VPK to set APP_VER 01.01.

> **Current data/runtime contract:** see [Current runtime contract](docs/CURRENT_RUNTIME_CONTRACT.md). Historical documents retain earlier 00.34/v1.0 observations as build-specific evidence.

A native PlayStation Vita port of **Dragon Ball Tap Battle** built with VitaSDK,
vitaGL, and the original game core compiled privately for Vita.

> **Important:** the repository and VPK do not distribute the original Android
> gameplay data. Prepare data from an APK you own with the included extractor.

## Requirements (PS Vita)

Before installing the port, make sure you have:

- **A homebrew-enabled PlayStation Vita** (PS Vita 1000 or 2000, with HENkaku/taiHEN, Ensō, or a compatible homebrew setup). The v1.1 release has been tested on real PS Vita hardware; PS TV and Vita3K compatibility have **not** been confirmed.
- **VitaShell** (or another compatible VPK installer) to install the game and transfer extracted data using USB, FTP, or your preferred method.
- **`libshacccg.suprx` installed and working on the console.** The port uses **vitaGL**, which requires this shader compiler module. **It is not included in the VPK.** Follow the [vitaGL prerequisites](https://github.com/Rinnegatamante/vitaGL#prerequisites) and the [libshacccg extraction/installation guide](https://samilops2.gitbook.io/vita-troubleshooting-guide/shader-compiler/extract-libshacccg.suprx) to prepare it from your own console.
- **Writable `ux0:` storage with enough free space** for the VPK and at least one extracted game profile. The amount of space needed depends on the APK/mod; large mods can use considerably more storage.
- **A compatible Dragon Ball Tap Battle APK that you legally possess.** The VPK contains **no playable game data**, so you must prepare a profile using the [Windows extractor](docs/WINDOWS_DATA_TOOL.md) (Windows 10/11) or the [Web extractor](https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/) (modern desktop/mobile browser). A PC is **not required** if you use the Web extractor on a phone or tablet.

**Required data location:** `ux0:data/DBTapBattle/profiles/<Profile>/`. If an extractor generates `dbtb_codec.json`, keep it with that profile's PAC files.

**No VitaSDK or separate vitaGL installation is needed on the console**; those are build-time dependencies. The required `libshacccg.suprx` module is a separate runtime prerequisite.

> **Compatibility note:** Not every community mod is guaranteed to work. Some high-resolution mods need longer loading times and may display softer/blurry textures because the port reduces GPU memory usage to prevent crashes. See [installation and extraction](docs/INSTALLATION_AND_EXTRACTION.md) and [mod compatibility](docs/MODS.md).

## Current data model

All playable datasets now use one directory:

```text
ux0:data/DBTapBattle/profiles/
```

Each first-level folder inside `profiles/` is one selectable game dataset.
There is no special `game/` folder and no separate `mods/` folder anymore.

The Vita selector shows only folders that actually exist inside `profiles/`.
If no profiles are installed, it shows **NO GAME DATA FOUND** and tells the user
to prepare a Tap Battle APK with the extractor.

## Important: high-resolution mod assets and PS Vita memory

**Some community mods contain unusually heavy or high-resolution sprite atlases, backgrounds and effects.** Loading all of those textures at full size can exhaust the PS Vita's limited system/graphics memory and cause a crash, freeze or severe loading delays.

For stability, v1.1 applies a **generic memory-aware texture quality policy** to newly discovered protected formats. It can store textures in a compact GPU format and **reduce the physical resolution of some images**. Consequently **some textures may look blurry while others remain sharp**. This is a deliberate protection against memory exhaustion, *not* a damaged APK, extraction error or a per-mod exception. Extraction retains the source PAC bytes. Older audited profiles keep their previous rendering paths.

Compatibility is **not guaranteed for every mod**, especially ones that modify Android game logic. Heavy mods may take longer to load. See [Installation & extraction](docs/INSTALLATION_AND_EXTRACTION.md), [Mod compatibility](docs/MODS.md) and [GPU/memory diagnostic](docs/DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md).

## Profile names

Both the Web Extractor 1.0 and Windows Extractor 1.5 derive the Vita profile folder from the APK filename.

Examples:

```text
gen.apk
-> ux0:data/DBTapBattle/profiles/gen/

tap battle android 14.apk
-> ux0:data/DBTapBattle/profiles/tap_battle_android_14/

TAP BATTLE INVASION BETA 3.apk
-> ux0:data/DBTapBattle/profiles/TAP_BATTLE_INVASION_BETA_3/
```

APK type detection is independent from profile naming. The extractor can still
identify ordinary Gen-style assets or audited Android14-family protected layouts
for correct extraction, but that detection never replaces the APK filename with
a hardcoded profile name.

If you want another name in the Vita selector, simply rename that profile folder.
No PAC files need to be edited and the APK does not need to be extracted again.

## Quick installation

1. Install the Dragon Ball Tap Battle Vita VPK with VitaShell.
2. Prepare your APK data with either:
   - **Web Extractor 1.0:** https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/
   - **Windows Extractor 1.5:** `tools/windows/Extract_APK_for_Vita.bat`
3. With the web version, select the APK and download the generated ZIP; with
   Windows, run/drag the APK onto the BAT.
4. Extract the generated ZIP/package when necessary, then copy its **data**
   folder to the root of `ux0:`.
5. Confirm profiles end up under:
   `ux0:data/DBTapBattle/profiles/<Profile>/`
6. Launch the game and choose the installed profile.

The Web Extractor runs entirely in the browser: the selected APK is **not
uploaded**. It is intended especially for Android/phone/tablet users who do not
have a PC.

Do not create:

```text
ux0:data/data/DBTapBattle/
```

**[Complete installation & extraction guide (Web + Windows)](docs/INSTALLATION_AND_EXTRACTION.md)**

Detailed extractor instructions:

- [Web APK Data Extractor](docs/WEB_DATA_TOOL.md)
- [Windows extractor](tools/windows/README.txt)

## Save data

Each profile owns its own save:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

The VPK contains one read-only initial save seed. When a profile is launched for
the first time, the port creates that profile's `save.bin` only if it does not
already exist.

The extractor intentionally does **not** install a `save.bin` found inside an
APK. Back up an existing profile's save before deleting or replacing its folder.

## APK extraction behavior

The extractor automatically determines where useful game data is stored.

For ordinary / Gen-style APKs it extracts canonical files from `assets/`.
For audited Android14-family APKs, protected PAC aliases are mapped back to their
canonical filenames while PAC payload bytes are preserved.

The extractor also detects contiguous character triplets in the supported
`00..99` namespace, so large datasets are not truncated to the original
13-character baseline.

Every extracted APK is standalone. The runtime does not borrow a missing resource
from another profile.

## Project status

**v1.1 Universal Mod Support — hardware-approved release candidate (2026-10-08)**

- Release VPK: `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk` — Vita APP_VER `01.01`, TITLE_ID `DBTB01178`
- SHA-256: `9953e8c99ce59a5b4b55dab3ae2caec788c39ffe1edb19a2d4e5833c958ee6bd`
- Runtime: identical `eboot.bin` to the user-approved VisualQuality experimental VPK; **only APP_VER SFO metadata changed** for the release.
- New: Web/Windows DEX-based PRIVATE mod detection, optional per-profile `dbtb_codec.json`, native dynamic decoding; file-backed AAC/M4A BGM; safer compressed texture handling with adaptive memory-aware quality.
- Real Vita result: `dbz_mobile_v9` entered and completed consecutive fights without the previous crashes/freezes. Selected textures can look blurry because high-resolution content is downscaled for GPU stability.
- Compatibility is best effort, not verified for every community mod. Older audited formats keep their original rendering path. Preserve profile folders and saves during upgrades.
- [Full user installation & extraction guide](docs/INSTALLATION_AND_EXTRACTION.md) · [v1.1 release notes](docs/RELEASE_v1.1.md)

**Historical public release: v1.0**

The original v1.0 published package was:

- file: `Dragon-Ball-Tap-Battle-PS-Vita-v1.0.vpk`
- Vita APP_VER: `01.00`
- TITLE_ID: `DBTB01178`
- SHA-256: `15eb056274db6f3ad561c3befb670833c348f536c3073590b9768b04f74ee594`

v1.0 is a release-identity promotion of the **00.34 hardware-confirmed gameplay
checkpoint**. The executable/game resources are unchanged by the Title ID
migration; only the package metadata identity changed. The exact 00.34 artifact
that was physically tested remains documented with its original
`TITLE_ID DBTB00001` and hash as historical evidence.

See [Current Status](docs/CURRENT_STATUS.md) for validation scope and history.

## Technical documentation

- [Vita physical controls research and selector-mode plan](docs/VITA_CONTROLS_RESEARCH_2026-10-08.md) — original virtual-pad host probes pass; gameplay bindings are not yet implemented.
- [Install & extract data for v1.1](docs/INSTALLATION_AND_EXTRACTION.md)
- [v1.1 Release Notes](docs/RELEASE_v1.1.md)
- [Current Runtime Contract](docs/CURRENT_RUNTIME_CONTRACT.md)
- [Data Layout](docs/DATA_LAYOUT.md)
- [Web APK Data Extractor](docs/WEB_DATA_TOOL.md)
- [Windows APK Data Extractor](docs/WINDOWS_DATA_TOOL.md)
- [APK Technical Reference](docs/APK_TECHNICAL_REFERENCE.md)
- [Mod Compatibility](docs/MODS.md)
- [Build](docs/BUILD.md)
- [Validation](docs/VALIDATION.md)
- [Release Workflows](docs/RELEASE_WORKFLOWS.md)

## Legal / redistribution note

This project is a compatibility/porting effort. Public tooling is designed so
users prepare game data from APK files they already possess. Original APKs,
proprietary gameplay datasets, Android DEX/classes, and extracted commercial
assets should not be redistributed through this repository.

Third-party attribution is documented in
[THIRD_PARTY.md](docs/THIRD_PARTY.md) and `licenses/`.
