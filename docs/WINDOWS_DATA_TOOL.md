# Windows APK data extractor 1.5 — v1.2-compatible profiles-v1 output

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

The public launcher is:

```text
tools/windows/Extract_APK_for_Vita.bat
```

The older Spanish-named BAT remains for compatibility and launches the same
PowerShell extractor.

## No PC / phone or tablet

Users without Windows can use **Web Extractor 1.0**:

https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

It implements the same `profiles-v1` output contract directly in a modern
browser and does not upload the selected APK. See
[WEB_DATA_TOOL](WEB_DATA_TOOL.md).

## Requirements

- Windows 10 or Windows 11
- built-in Windows PowerShell 5.1
- no Python, Java, 7-Zip, administrator rights, or network access required

## Universal protected mods retained from v1.1

The release ZIP includes `PrivateModDex.ps1` beside
`Extraer_APK_para_Vita.ps1`. Keep both files together. Unknown compatible
PRIVATE PAC variants are identified by reading the APK's DEX constants without
executing Android code. The result includes `dbtb_codec.json` where needed;
copy it with the rest of the profile. Older profiles may have no codec sidecar.
Heavy mod images can be blurred/downscaled on Vita for memory stability.
See [full installation guide](INSTALLATION_AND_EXTRACTION.md).


Extractor 1.5 is compatible with v1.2 (`APP_VER 01.02`, `TITLE_ID DBTB01178`)
and its `profiles-v1` contract documented in
[CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md). The VPK scans only
`ux0:data/DBTapBattle/profiles/`; it does not use `game/` or `mods/` as
current profile roots and it does not synthesize an Original row.

The existing public download is `DBTapBattle-Extractor-Windows-v1.1.zip` from
release `1.1`; its filename is not the internal extractor version (1.5).
No extractor code/data-layout change is needed for v1.2 controls; valid profiles
do not need re-extraction.

`DBTapBattle-Extractor-Windows-v1.2.zip` is also prepared for the authorized
release `1.2`. It contains the same Extractor 1.5 scripts plus v1.2-aligned
English instructions; publication is pending. The ZIP includes all six files
listed by the Windows packaging workflow, with no APK or game data.

## Output contract

Every APK becomes one independent Vita profile under:

```text
data/DBTapBattle/profiles/<sanitized APK filename>/
```

The profile name is derived from the APK filename for **all** APK types.

Examples:

| APK filename | Output profile |
|---|---|
| `gen.apk` | `profiles/gen/` |
| `tap battle android 14.apk` | `profiles/tap_battle_android_14/` |
| `TAP BATTLE INVASION BETA 3.apk` | `profiles/TAP_BATTLE_INVASION_BETA_3/` |
| `DragonBallZuperSamuGamerYT.apk` | `profiles/DragonBallZuperSamuGamerYT/` |

The extractor no longer assigns special folder names such as `Gen`,
`Android14`, `Invasion`, or `ZuperSamu` merely because a known APK hash or
codec was detected.

Detection and naming are separate:

- detection decides how the APK should be extracted;
- the APK filename decides what the Vita profile folder is called.

If the user wants another selector name, they can rename the resulting folder
inside `ux0:data/DBTapBattle/profiles/`.

## Supported layouts

The extractor detects:

- `res/raw/` data layouts;
- ordinary Gen-style `assets/` layouts;
- audited protected Android14-family layouts.

Protected aliases are canonicalized using a known audited codec or validated
constants recovered by the bounded PRIVATE DEX reader. Unknown/ambiguous
loaders fail closed. Payload bytes are not decrypted/recompressed by the
desktop extractor.

## Vita copy procedure

Each run creates a new package under `Listo_para_Vita/`.

Copy the package's **data** folder to the root of `ux0:`.

The final layout must be:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

Do not create:

```text
ux0:data/data/DBTapBattle/
```

## Save behavior

Each profile owns:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

APK-bundled saves are detected for provenance but are not installed. The VPK
creates a profile save from its read-only seed only when that profile has no
existing save.

Back up a profile's `save.bin` before deleting or replacing its folder.

## Validation

The extractor verifies:

- selected ZIP entry CRC and size;
- safe paths and duplicate/colliding names;
- per-file and total extraction budgets;
- protected PAC structure for audited Android14-family profiles;
- SHA-256 source/file hashes.

Each profile includes `dbtb_manifest.json`. The package also includes
`RESULTADO.json`, `SHA256SUMS.txt`, and an English copy-to-Vita guide.

## Runtime relationship

The current Vita selector scans only:

```text
ux0:data/DBTapBattle/profiles/
```

It displays only installed profile folders. If none exist, the selector shows a
no-game-data message and instructs the user to prepare a Tap Battle APK.

There is no cross-profile resource fallback.

A mod that changes Android Java/Dalvik code may still require separate Vita-side
compatibility work; extracting its files alone cannot reproduce code changes.


## CI evidence

Windows extractor 1.5 with the explicit `profiles-v1` assertions passes GitHub
Actions run `37689580096` (latest cleanup/test run; contract regression run `37688246446` also passed). The suite exercises PowerShell 5.1/BAT transport,
unsafe ZIP paths, CRC failure rollback, protected aliases/codecs, filename-based
profile naming, collisions, independent saves and the absence of current
`game/` / `mods/` output roots.
