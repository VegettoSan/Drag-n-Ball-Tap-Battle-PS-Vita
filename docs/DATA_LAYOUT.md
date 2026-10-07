# Runtime data layout — unified profiles

> Source of truth for the current executable/extractor contract: [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md). The migration section at the bottom intentionally names historical paths.

Current source contract after the unified-profile selector refactor.

## Base path

\`\`\`text
ux0:data/DBTapBattle/
\`\`\`

## Playable datasets

Every playable APK-derived dataset lives under one directory:

\`\`\`text
ux0:data/DBTapBattle/profiles/
\`\`\`

Each first-level folder is one selectable profile:

\`\`\`text
ux0:data/DBTapBattle/profiles/
├── gen/
├── tap_battle_android_14/
├── TAP_BATTLE_INVASION_BETA_3/
└── DragonBallZuperSamuGamerYT/
\`\`\`

There is no separate \`game/\` and \`mods/\` split in the current runtime.

The selector lists only the folders that actually exist under \`profiles/\`.

If no profile folders exist, the selector shows a no-data message and does not
offer a fake or disabled Original entry.

## Profile naming

The Web Extractor 1.0 and Windows Extractor 1.5 both derive the folder name from the APK filename.

Examples:

\`\`\`text
gen.apk
-> profiles/gen/

tap battle android 14.apk
-> profiles/tap_battle_android_14/

TAP BATTLE INVASION BETA 3.apk
-> profiles/TAP_BATTLE_INVASION_BETA_3/
\`\`\`

The folder name is also the name displayed by the Vita selector.

To change the displayed name, rename the folder. No PAC modification is required.

## Resource resolution

Once a profile is selected, every resource request resolves only inside that
profile:

\`\`\`text
ux0:data/DBTapBattle/profiles/<Profile>/<resource>
\`\`\`

Example:

\`\`\`text
ux0:data/DBTapBattle/profiles/Invasion/select0.pac
\`\`\`

If a requested file is missing, the runtime reports a selected-profile resource
error. It never borrows files from another profile.

## Save ownership

Each profile owns its own mutable save:

\`\`\`text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
\`\`\`

Rules:

- The VPK contains one read-only master seed at \`app0:/save.bin\`.
- On first launch of a profile, the runtime copies that seed only if the profile
  has no \`save.bin\`.
- Existing profile progress is not overwritten merely by launching the profile
  again or installing a newer VPK.
- APK-bundled \`save.bin\` files are recorded by the extractor for provenance but
  are not installed as runtime progress.
- Saves are never shared automatically between profiles.

## Extractor contract

The public Web and Windows extractors write:

\`\`\`text
data/DBTapBattle/profiles/<APK filename>/
\`\`\`

The extractor may internally detect a Gen-style, Android14, Spanish, Invasion,
or other supported layout/codec. That detection affects extraction and protected
PAC alias normalization only; it does not replace the APK filename with a
hardcoded profile name.

## Web-generated ZIP

The Web Extractor downloads a ZIP whose root contains the same `data/` tree
expected for Vita:

\`\`\`text
data/DBTapBattle/profiles/<Profile>/
LEEME_COPIAR_A_VITA.txt
RESULTADO.json
SHA256SUMS.txt
\`\`\`

Extract the ZIP first, then copy its `data` directory to the root of `ux0:`.
The web tool runs locally in the browser; APK bytes are not uploaded.

See [WEB_DATA_TOOL](WEB_DATA_TOOL.md).

## Dynamic roster

The runtime scans the selected profile's contiguous character triplets:

\`\`\`text
charXX.pac
chardemoXX.pac
charfXXXX.pac
\`\`\`

within the supported two-digit namespace \`00..99\`.

## Runtime diagnostics

\`\`\`text
ux0:data/DBTapBattle/logs/runtime.log
\`\`\`

## Migration from older test layouts

Older development builds used:

\`\`\`text
ux0:data/DBTapBattle/game/
ux0:data/DBTapBattle/mods/<Profile>/
\`\`\`

The current source no longer uses those directories as playable profile roots.
For a current build, move each complete dataset into its own folder under
\`profiles/\`.

Example:

\`\`\`text
old: ux0:data/DBTapBattle/mods/Invasion/
new: ux0:data/DBTapBattle/profiles/Invasion/
\`\`\`

The save should move with the rest of that profile if you want to preserve its
progress.
