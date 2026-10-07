# Windows APK data extractor

<!-- DBTB_DOC_STATUS:START -->
> **Project checkpoint:** 00.33 is the current hardware-confirmed development
> checkpoint for the tested paths. This file may document an earlier component
> or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


`tools/windows/Extraer_APK_para_Vita.bat` prepares a Vita installation from one
or several user-owned Tap Battle APKs. Extract the distributed tool ZIP first,
keep its BAT and PS1 together, then drag APKs onto the BAT. Double-clicking the
BAT opens a multi-file APK picker. Windows 10/11 built-in PowerShell 5.1 and .NET
are the only runtime requirements; no Python, Java, 7-Zip, administrator access
or network request is needed on the user's PC.

The launcher passes file paths through indexed environment variables, with
CMD delayed expansion disabled. APK paths are never inserted into PowerShell
source code. The BAT applies process-local `-ExecutionPolicy Bypass`; it does
not change the permanent machine/user execution policy.

## Output and copy procedure

Each launch creates a new `Listo_para_Vita/Paquete_<time>_<id>/` beside the tool.
Copy that package's **data** directory to the Vita's **ux0:** root with VitaShell.
The resulting runtime root is `ux0:data/DBTapBattle/`. Install the full engine
VPK separately (00.33 current checkpoint; 00.30+ share the profile-save model); this tool does not build or bundle a VPK.

| Detected source | Destination within package | VPK selector |
|---|---|---|
| Exact audited Original APK hash | `data/DBTapBattle/game/` | Original |
| Exact audited Gen APK hash | `data/DBTapBattle/mods/Gen/` | Gen |
| Exact audited Zuper/Samu APK hash | `data/DBTapBattle/mods/ZuperSamu/` | ZuperSamu |
| Exact audited Android14 APK hash | `data/DBTapBattle/mods/Android14/` | Android14 |
| Exact audited Spanish APK hash | `data/DBTapBattle/mods/Espanol/` | Espanol |
| Exact audited Invasion APK hash | `data/DBTapBattle/mods/Invasion/` | Invasion |
| Other canonical/Gen-style `assets/` APK | `data/DBTapBattle/mods/<safe APK stem>/` | Its folder name |
| Other non-Original `res/raw/` APK | `data/DBTapBattle/mods/<safe APK stem>/` | Its folder name |
| Protected Android14-style APK matching an audited codec profile | `data/DBTapBattle/mods/<safe APK stem>/` | Its folder name |

The exact known APKs are recognized by source SHA-256, not filename. Modified
Gen-derived APKs and non-Original raw APKs receive their own sanitized profile
name. Multiple colliding names receive suffixes rather than being merged.

`DragonBallZuperSamuGamerYT.apk` is the audited large Gen-derived reference: its
DEX/manifest are identical to Gen, it contains 384 canonical assets and 92
character triplets, and the exact source hash is labeled `ZuperSamu`. Unknown
Gen-derived mods are still accepted under their own sanitized profile name. The
extractor counts contiguous triplets dynamically through the two-digit namespace
`00..99`; it does not truncate a 70/92-character mod to Gen's 13 characters.
Dragging the same input path twice imports it once. Non-empty resources on both
raw and assets sides are rejected as ambiguous. Gen's empty raw stubs are ignored.

Every non-Original APK is emitted as a standalone profile under `mods/`.
00.33 retains the no-fallback contract introduced earlier: `game/` may remain empty when a standalone mod profile is selected. A missing
resource in the selected profile is a compatibility issue to investigate, not a
reason to borrow bytes from Original. The full-APK tool requires `common.pac`
so arbitrary partial patch ZIPs are not misrepresented as complete profiles.
The first supplied original APK lacks all 13 character triplets. Its extraction
is complete relative to that APK, but is not a complete battle installation.

## Preservation and validation

Selected `res/raw/` or `assets/` gameplay files retain their exact bytes, including unknown extensions. If the APK contains `save.bin`, its presence/hash is recorded but that file is intentionally excluded from the installed profile; 00.33 continues this policy. The current tool has three separately
audited protected profiles in `tools/community14.py`: Android14
`community14-a210795b` (106 PAC), Spanish `community14-es-d594affc`
(106 PAC) and Invasion `community14-invasion-05aa0c5e` (139 PAC). Each profile
has its own aliases/XOR constants. The extractor canonicalizes filenames only;
it does not decode or rewrite PAC payload bytes. A protected APK that does not uniquely satisfy one audited profile must be rejected rather than guessed. Therefore “Android14-compatible” currently means it reuses the audited Android14, Spanish or Invasion alias/XOR family; a new protection family requires one audit before the Windows tool can safely extract it.

The tool reads ZIP central metadata, verifies selected-file CRC and size while
streaming, checks Community14 table bounds/type signals, and writes SHA-256
source/file hashes. ZIP32 stored/deflated entries are supported; encryption,
ZIP64, split archives, non-regular selected entries, traversal, Windows device
names, duplicate/colliding paths and budgets above 64 MiB/file or 512 MiB/data set
are rejected. APK size is capped at 1 GiB. Output ancestors may not be reparse
points. A whole multi-APK operation is staged privately in the output directory;
only successful completion publishes the folder with a same-filesystem rename.
A failure removes its own staging directory and leaves previous output intact.

Each profile includes format-4 `dbtb_manifest.json`, with standalone-profile, dynamic-roster, tool/profile and source-save metadata. APK-local `save.bin` is recorded but not installed; the runtime creates an independent profile save from the VPK seed on first use. The package contains
`LEEME_COPIAR_A_VITA.txt`, `RESULTADO.json` and `SHA256SUMS.txt`. The manifest covers
only imported data; SHA256SUMS also covers generated instructions/manifests,
excluding itself. DEX, classes, signatures and Android libraries outside the
selected data prefix are not extracted.

An APK-provided save is **not installed into its profile**. The desktop extractor does not connect to a Vita or read/replace the active save. 00.33 does not share mutable progress. Original uses `game/save.bin` and each mod uses `mods/<Profile>/save.bin`; each is seeded from the same VPK copy only when that profile save is absent. Code-changing Android mods still require
appropriate engine support; data extraction does not incorporate Android code.


### Media caveat: names are not codecs

The extractor intentionally preserves media bytes. Deep audit found that Invasion
keeps `.ogg` filenames for seven BGM that are actually MP3 or AAC/M4A.
Zuper/SamuGamerYT broadens this case to **15/17 BGM** under `.ogg` names that
are MP3/AAC; only bgm_12/13 remain baseline Vorbis. The
extractor therefore must not validate or rename a media file merely from its
extension. Runtime/import codec adaptation is a separate concern; see
[INVASION_BETA3_APK](INVASION_BETA3_APK.md). Preserving source bytes and hashes
is required so any future conversion remains traceable and non-destructive.

## Verification

- Real supplied APKs were processed locally by the actual PS1 under PowerShell
  7.4.6/Linux. An independent Python ZIP reader and the existing name/codec parser
  checked every output byte, filename and manifest hash against the APK.
- `tests/test_windows_extractor.py` executes the actual script on synthetic ZIPs:
  standalone raw/assets/Community14, a 70-character Gen-style roster, bundled save, unknown data, SHA sums, duplicate inputs
  and profile names, repeated output, late CRC corruption, batch rollback,
  traversal/device names, file/directory collisions, symlinks, oversized entries,
  ambiguous APKs and unsupported codecs.
- `.github/workflows/windows-data-extractor.yml` runs these tests using built-in
  Windows PowerShell 5.1, including the actual BAT with spaces, `&`, `!`, `%` and
  brackets in paths, then packages only the distributable tool.

Public CI uses synthetic fixtures only. Real APKs/data remain private. Host/Windows
extraction evidence does not establish new physical Vita gameplay coverage; the
the extractor itself does not alter runtime evidence; the current device checkpoint is 00.33.

Windows CI run [37390881454](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37390881454) passed all 13 tests, including the actual BAT transport, under PowerShell 5.1.26100.33438 on Windows Server 2025. The first run passed the 12 extractor tests but failed to start the BAT because the Python harness used CRT quote escaping for cmd.exe. Commit `5ef4549` corrects the harness; the second run passes. The file picker and interactive Explorer opening are not automated checks.

See [machine-readable evidence](evidence/windows_extractor_2026-10-05.json).

For the complete five-APK structure, profile constants and cross-APK differences,
see [APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md) and
[evidence/apk_deep_structure_2026-10-06.json](evidence/apk_deep_structure_2026-10-06.json).

Audited large canonical-mod reference: [DRAGONBALL_ZUPER_SAMUGAMERYT_APK](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md).
