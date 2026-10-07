# Dragon Ball Tap Battle PS Vita

> **Current 00.34 data/runtime contract:** see [Current runtime contract](docs/CURRENT_RUNTIME_CONTRACT.md). Historical documents may retain older paths only as build-specific evidence.

A native PlayStation Vita port of **Dragon Ball Tap Battle** built with VitaSDK,
vitaGL, and the original game core compiled privately for Vita.

> **Important:** the repository and VPK do not distribute the original Android
> gameplay data. Prepare data from an APK you own with the included extractor.

## Current data model

All playable datasets now use one directory:

\`\`\`text
ux0:data/DBTapBattle/profiles/
\`\`\`

Each first-level folder inside \`profiles/\` is one selectable game dataset.
There is no special \`game/\` folder and no separate \`mods/\` folder anymore.

The Vita selector shows only folders that actually exist inside \`profiles/\`.
If no profiles are installed, it shows **NO GAME DATA FOUND** and tells the user
to prepare a Tap Battle APK with the extractor.

## Profile names

The Windows extractor always derives the Vita profile folder from the APK filename.

Examples:

\`\`\`text
gen.apk
-> ux0:data/DBTapBattle/profiles/gen/

tap battle android 14.apk
-> ux0:data/DBTapBattle/profiles/tap_battle_android_14/

TAP BATTLE INVASION BETA 3.apk
-> ux0:data/DBTapBattle/profiles/TAP_BATTLE_INVASION_BETA_3/
\`\`\`

APK type detection is independent from profile naming. The extractor can still
identify ordinary Gen-style assets or audited Android14-family protected layouts
for correct extraction, but that detection never replaces the APK filename with
a hardcoded profile name.

If you want another name in the Vita selector, simply rename that profile folder.
No PAC files need to be edited and the APK does not need to be extracted again.

## Quick installation

1. Install the Dragon Ball Tap Battle Vita VPK with VitaShell.
2. Extract the Windows extractor ZIP.
3. Run \`Extract_APK_for_Vita.bat\` and select or drag one or more APK files.
4. Copy the generated package's **data** folder to the root of \`ux0:\`.
5. Confirm profiles end up under:
   \`ux0:data/DBTapBattle/profiles/<Profile>/\`
6. Launch the game and choose the installed profile.

Do not create:

\`\`\`text
ux0:data/data/DBTapBattle/
\`\`\`

Detailed extractor instructions:
[tools/windows/README.txt](tools/windows/README.txt)

## Save data

Each profile owns its own save:

\`\`\`text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
\`\`\`

The VPK contains one read-only initial save seed. When a profile is launched for
the first time, the port creates that profile's \`save.bin\` only if it does not
already exist.

The extractor intentionally does **not** install a \`save.bin\` found inside an
APK. Back up an existing profile's save before deleting or replacing its folder.

## APK extraction behavior

The extractor automatically determines where useful game data is stored.

For ordinary / Gen-style APKs it extracts canonical files from \`assets/\`.
For audited Android14-family APKs, protected PAC aliases are mapped back to their
canonical filenames while PAC payload bytes are preserved.

The extractor also detects contiguous character triplets in the supported
\`00..99\` namespace, so large datasets are not truncated to the original
13-character baseline.

Every extracted APK is standalone. The runtime does not borrow a missing resource
from another profile.

## Project status

The latest hardware-confirmed gameplay checkpoint before the unified-profile
layout work is **00.33**. The current source contains the next selector/data-layout
changes and requires a new real-hardware test before promotion.

See [Current Status](docs/CURRENT_STATUS.md) for validation scope and history.

## Technical documentation

- [Current Runtime Contract](docs/CURRENT_RUNTIME_CONTRACT.md)
- [Data Layout](docs/DATA_LAYOUT.md)
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
[THIRD_PARTY.md](docs/THIRD_PARTY.md) and \`licenses/\`.
