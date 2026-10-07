# Dragon Ball Tap Battle PS Vita

A native PlayStation Vita port of **Dragon Ball Tap Battle** built with VitaSDK,
vitaGL, and the original game core compiled privately for Vita.

> **Important:** the repository and VPK do not distribute the original Android
> game data. You must provide data from your own APK and prepare it with the
> included extractor.

## Project status

The current hardware-confirmed gameplay checkpoint is **00.33**. On a real
PlayStation Vita, the tested paths include startup, menu flow, visible text,
clean audio, character selection, battles, standalone mod profiles, dynamic
character rosters, and the repaired Invasion repeated-fight crash.

The next presentation candidate is **00.34**, which redesigns only the initial
data-profile selector. It does not replace the original Tap Battle menu or combat
logic. See [Current Status](docs/CURRENT_STATUS.md) for the exact validation
scope and open test items.

## What you need

- A hacked PS Vita able to install homebrew VPKs.
- VitaShell or another way to copy files to `ux0:`.
- The Dragon Ball Tap Battle Vita VPK.
- One or more Dragon Ball Tap Battle APKs that you legally own.
- Windows 10/11 if you want to use the included drag-and-drop extractor.

The extractor does **not** require Python, Java, 7-Zip, administrator rights, or
an Internet connection.

## Quick installation

### 1. Install the VPK

Install the Dragon Ball Tap Battle Vita VPK with VitaShell.

The application uses title ID:

```text
DBTB00001
```

### 2. Extract your APK data on Windows

Download the Windows extractor package from the project artifacts/releases and
extract the entire ZIP into one folder.

Run:

```text
Extract_APK_for_Vita.bat
```

You can either:

- drag one or more APK files onto the BAT file; or
- double-click the BAT and choose the APK files in the file picker.

The extractor validates the APK, identifies its supported layout, preserves the
gameplay payloads, and creates a Vita-ready package.

Detailed instructions:
[tools/windows/README.txt](tools/windows/README.txt)

Technical documentation:
[Windows APK Data Extractor](docs/WINDOWS_DATA_TOOL.md)

### 3. Copy the data folder to the Vita

Open the newly generated package and copy its **data** folder to the **root of
`ux0:`** with VitaShell.

Correct result:

```text
ux0:data/DBTapBattle/
```

Do **not** end up with:

```text
ux0:data/data/DBTapBattle/
```

Do not copy the generated `Package_*` / `Paquete_*` folder itself into
`ux0:data`; copy the **data** folder inside it to `ux0:`.

### 4. Launch the game and select a profile

At startup, the Vita port scans:

```text
ux0:data/DBTapBattle/game/
ux0:data/DBTapBattle/mods/<Profile>/
```

Choose the profile that matches the APK you extracted.

## Data layout

| Vita path | Purpose |
|---|---|
| `ux0:data/DBTapBattle/game/` | Original/base profile |
| `ux0:data/DBTapBattle/mods/Gen/` | Known Gen APK |
| `ux0:data/DBTapBattle/mods/Android14/` | Known Android 14 community APK |
| `ux0:data/DBTapBattle/mods/Espanol/` | Known Spanish Android 14 APK |
| `ux0:data/DBTapBattle/mods/Invasion/` | Known Invasion profile |
| `ux0:data/DBTapBattle/mods/ZuperSamu/` | Known Zuper/Samu profile |
| `ux0:data/DBTapBattle/mods/<Name>/` | Other supported APK-derived profiles |
| `ux0:data/DBTapBattle/logs/runtime.log` | Runtime diagnostics |

Each selected profile is **standalone**. The current runtime does not borrow a
missing resource from `game/` or from another mod. If a selected APK is missing
a resource needed by its own code path, that is a compatibility issue to fix in
the port rather than a reason to mix datasets.

See [Data Layout](docs/DATA_LAYOUT.md).

## Save data

Every profile owns its own writable save:

```text
Original:
ux0:data/DBTapBattle/game/save.bin

Mod / alternate APK:
ux0:data/DBTapBattle/mods/<Profile>/save.bin
```

The VPK contains one clean read-only save seed. When a profile is launched for
the first time, the Vita port creates that profile's `save.bin` only if it does
not already exist.

The extractor intentionally does **not** install a `save.bin` found inside an
APK. This prevents an APK or mod package from silently replacing existing Vita
progress.

Before replacing or refreshing profile data, back up that profile's
`save.bin`.

## Windows APK data extractor

The public-facing launcher is:

```text
tools/windows/Extract_APK_for_Vita.bat
```

The older Spanish-named launcher is kept only for compatibility:

```text
tools/windows/Extraer_APK_para_Vita.bat
```

Both launch the same PowerShell extractor.

The extractor currently:

- supports `res/raw/`, ordinary `assets/`, and audited protected
  Android14-family layouts;
- recognizes the known Original, Gen, Android14, Spanish, Invasion, and
  Zuper/Samu APKs by SHA-256 when applicable;
- stores non-Original APKs as independent profiles under `mods/`;
- preserves gameplay resource bytes;
- canonicalizes only audited protected PAC aliases;
- detects character triplets dynamically in the Vita-supported `00..99`
  namespace;
- validates ZIP CRCs, sizes, unsafe paths, duplicate paths, and output limits;
- records source/file SHA-256 hashes in `dbtb_manifest.json`;
- excludes APK-local `save.bin` from installation;
- does not copy Android DEX/classes/native libraries into the Vita data folder.

A data extractor cannot automatically reproduce code changes made by a mod's
Android Java/Dalvik implementation. Such mods may require separate Vita-side
compatibility work.

### Important Original APK limitation

The first audited Original APK available during development does not contain all
downloadable character packs. The extractor copies exactly what exists in the
APK; it does not fabricate missing character data.

## Supported mod/data work

The port has dedicated documentation for the APK families audited during
development:

- [APK Technical Reference](docs/APK_TECHNICAL_REFERENCE.md)
- [Canonical APK Differences](docs/APK_CANONICAL_DIFFERENCES.md)
- [Android14 APK](docs/ANDROID14_APK.md)
- [Spanish Android14 APK](docs/SPANISH_ANDROID14_APK.md)
- [Invasion Beta 3 APK](docs/INVASION_BETA3_APK.md)
- [Zuper / SamuGamerYT APK](docs/DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md)
- [Mod Compatibility](docs/MODS.md)

## Controls and current scope

The original touch gameplay path is preserved. The native Vita profile selector
supports Vita controls, while in-game physical-control adaptation remains a
separate work item.

Online downloads, Android billing, and Bluetooth multiplayer are not part of the
current offline Vita port.

## Building from source

The full game target lives under:

```text
tools/aot/engine/vita/
```

It requires VitaSDK plus the private original-game AOT generation step. The
repository intentionally does not commit proprietary generated game code or APK
payloads.

Build and validation documentation:

- [Build](docs/BUILD.md)
- [Validation](docs/VALIDATION.md)
- [Release Workflows](docs/RELEASE_WORKFLOWS.md)
- [Current Status](docs/CURRENT_STATUS.md)

The repository also contains manual GitHub Actions workflows for full **Release**
and **Prerelease** VPK publication. Those workflows require the project's private
original-APK build input and validate the produced VPK before publication.

## Development rules

The port aims to preserve the original game engine and gameplay logic. Vita-side
changes should stay at platform, rendering, audio, input, filesystem, lifecycle,
or proven compatibility boundaries unless direct evidence requires an engine
change.

Hardware observations, successful fixes, failed attempts, APK structure, and
reusable porting findings are documented under `docs/`.

## Legal / redistribution note

This project is a compatibility/porting effort. The repository and public tooling
are designed so users prepare data from APK files they already possess. Original
game APKs, proprietary gameplay datasets, Android DEX/classes, and extracted
commercial assets should not be redistributed through this repository.

Third-party attribution and license notes are available in
[THIRD_PARTY.md](docs/THIRD_PARTY.md) and `licenses/`.
