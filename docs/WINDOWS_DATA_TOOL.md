# Windows APK data extractor

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
VPK separately (00.23 or later); this tool does not build or bundle a VPK.

| Detected source | Destination within package | VPK selector |
|---|---|---|
| Non-empty `res/raw/`, no non-empty `assets/` | `data/DBTapBattle/game/` | Original |
| Confirmed encoded Community14 aliases | `data/DBTapBattle/mods/Android14/` | Android14 |
| Known supplied Gen SHA-256 | `data/DBTapBattle/mods/Gen/` | Gen |
| Other canonical `assets/` APK | `data/DBTapBattle/mods/<safe APK stem>/` | Its folder name |

Gen is recognized by source content hash, not filename. Modified Gen-derived
APKs receive their own sanitized stem. Multiple colliding profile names receive
suffixes rather than being merged; further raw APKs use `mods/Original_2`, etc.
Dragging the same input path twice imports it once. Non-empty resources on both
raw and assets sides are rejected as ambiguous. Gen's empty raw stubs are ignored.

Assets/mod profiles deliberately never overwrite `game/`. A complete Gen or
Android14 dataset can be selected independently; Original may show DATA MISSING
when no base is installed. For Gen as a deliberate base installation, copy the
contents of `mods/Gen/` into `game/` yourself. Ordinary partial data mods can need
compatible base resources for fallback; this full-APK tool requires `common.pac`.
The first supplied original APK lacks all 13 character triplets. Its extraction
is complete relative to that APK, but is not a complete battle installation.

## Preservation and validation

Selected `res/raw/` or `assets/` files retain their exact bytes, including unknown
extensions and an APK-provided `save.bin`. Community14 applies the pinned
`community14-a210795b` alias/outer-table contract from `tools/community14.py`;
106 confirmed filenames in the supplied APK are renamed without decoding or
rewriting their payloads. Other Community14 codecs are not assumed compatible.

The tool reads ZIP central metadata, verifies selected-file CRC and size while
streaming, checks Community14 table bounds/type signals, and writes SHA-256
source/file hashes. ZIP32 stored/deflated entries are supported; encryption,
ZIP64, split archives, non-regular selected entries, traversal, Windows device
names, duplicate/colliding paths and budgets above 64 MiB/file or 512 MiB/data set
are rejected. APK size is capped at 1 GiB. Output ancestors may not be reparse
points. A whole multi-APK operation is staged privately in the output directory;
only successful completion publishes the folder with a same-filesystem rename.
A failure removes its own staging directory and leaves previous output intact.

Each profile includes format-3 `dbtb_manifest.json`, with additional tool/profile,
character-completeness and bundled-save fields. The package contains
`LEEME_COPIAR_A_VITA.txt`, `RESULTADO.json` and `SHA256SUMS.txt`. The manifest covers
only imported data; SHA256SUMS also covers generated instructions/manifests,
excluding itself. DEX, classes, signatures and Android libraries outside the
selected data prefix are not extracted.

An APK-provided save is retained in its matching profile. **When copying an update
onto a Vita, back up and skip the package's `save.bin` if preserving existing
progress.** The desktop extractor does not connect to a Vita or read its save.
No progress is shared across profiles. Code-changing Android mods still require
appropriate engine support; data extraction does not incorporate Android code.

## Verification

- Real supplied APKs were processed locally by the actual PS1 under PowerShell
  7.4.6/Linux. An independent Python ZIP reader and the existing name/codec parser
  checked every output byte, filename and manifest hash against the APK.
- `tests/test_windows_extractor.py` executes the actual script on synthetic ZIPs:
  raw/assets/Community14, bundled save, unknown data, SHA sums, duplicate inputs
  and profile names, repeated output, late CRC corruption, batch rollback,
  traversal/device names, file/directory collisions, symlinks, oversized entries,
  ambiguous APKs and unsupported codecs.
- `.github/workflows/windows-data-extractor.yml` runs these tests using built-in
  Windows PowerShell 5.1, including the actual BAT with spaces, `&`, `!`, `%` and
  brackets in paths, then packages only the distributable tool.

Public CI uses synthetic fixtures only. Real APKs/data remain private. Host/Windows
extraction evidence does not establish new physical Vita gameplay coverage; the
existing 00.23 device checkpoint remains unchanged.

Windows CI run [37390881454](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37390881454) passed all 13 tests, including the actual BAT transport, under PowerShell 5.1.26100.33438 on Windows Server 2025. The first run passed the 12 extractor tests but failed to start the BAT because the Python harness used CRT quote escaping for cmd.exe. Commit `5ef4549` corrects the harness; the second run passes. The file picker and interactive Explorer opening are not automated checks.

See [machine-readable evidence](evidence/windows_extractor_2026-10-05.json).
