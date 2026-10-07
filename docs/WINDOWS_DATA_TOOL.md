# Windows APK data extractor 1.5

The public launcher is:

\`\`\`text
tools/windows/Extract_APK_for_Vita.bat
\`\`\`

The older Spanish-named BAT remains for compatibility and launches the same
PowerShell extractor.

## Requirements

- Windows 10 or Windows 11
- built-in Windows PowerShell 5.1
- no Python, Java, 7-Zip, administrator rights, or network access required

## Runtime contract

Extractor 1.5 targets the VPK's `profiles-v1` contract documented in
[CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md). The VPK scans only
`ux0:data/DBTapBattle/profiles/`; it does not use `game/` or `mods/` as
current profile roots and it does not synthesize an Original row.

## Output contract

Every APK becomes one independent Vita profile under:

\`\`\`text
data/DBTapBattle/profiles/<sanitized APK filename>/
\`\`\`

The profile name is derived from the APK filename for **all** APK types.

Examples:

| APK filename | Output profile |
|---|---|
| \`gen.apk\` | \`profiles/gen/\` |
| \`tap battle android 14.apk\` | \`profiles/tap_battle_android_14/\` |
| \`TAP BATTLE INVASION BETA 3.apk\` | \`profiles/TAP_BATTLE_INVASION_BETA_3/\` |
| \`DragonBallZuperSamuGamerYT.apk\` | \`profiles/DragonBallZuperSamuGamerYT/\` |

The extractor no longer assigns special folder names such as \`Gen\`,
\`Android14\`, \`Invasion\`, or \`ZuperSamu\` merely because a known APK hash or
codec was detected.

Detection and naming are separate:

- detection decides how the APK should be extracted;
- the APK filename decides what the Vita profile folder is called.

If the user wants another selector name, they can rename the resulting folder
inside \`ux0:data/DBTapBattle/profiles/\`.

## Supported layouts

The extractor detects:

- \`res/raw/\` data layouts;
- ordinary Gen-style \`assets/\` layouts;
- audited protected Android14-family layouts.

Protected Android14-family aliases are canonicalized only when a known audited
codec profile is identified. Payload bytes are not decrypted/recompressed by the
desktop extractor.

## Vita copy procedure

Each run creates a new package under \`Listo_para_Vita/\`.

Copy the package's **data** folder to the root of \`ux0:\`.

The final layout must be:

\`\`\`text
ux0:data/DBTapBattle/profiles/<Profile>/
\`\`\`

Do not create:

\`\`\`text
ux0:data/data/DBTapBattle/
\`\`\`

## Save behavior

Each profile owns:

\`\`\`text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
\`\`\`

APK-bundled saves are detected for provenance but are not installed. The VPK
creates a profile save from its read-only seed only when that profile has no
existing save.

Back up a profile's \`save.bin\` before deleting or replacing its folder.

## Validation

The extractor verifies:

- selected ZIP entry CRC and size;
- safe paths and duplicate/colliding names;
- per-file and total extraction budgets;
- protected PAC structure for audited Android14-family profiles;
- SHA-256 source/file hashes.

Each profile includes \`dbtb_manifest.json\`. The package also includes
\`RESULTADO.json\`, \`SHA256SUMS.txt\`, and an English copy-to-Vita guide.

## Runtime relationship

The current Vita selector scans only:

\`\`\`text
ux0:data/DBTapBattle/profiles/
\`\`\`

It displays only installed profile folders. If none exist, the selector shows a
no-game-data message and instructs the user to prepare a Tap Battle APK.

There is no cross-profile resource fallback.

A mod that changes Android Java/Dalvik code may still require separate Vita-side
compatibility work; extracting its files alone cannot reproduce code changes.
