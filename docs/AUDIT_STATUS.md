# Audit status — v1.2, 2026-10-09

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

Current source integrates approved Vita controls into main and retains the v1.1
VisualQuality baseline. The prepared VPK is 01.02 / DBTB01178; the user is testing
it and has not reported a stable-v1.2 result yet. The existing published GitHub
release is 1.1. [Current status](CURRENT_STATUS.md) distinguishes each artifact.

| Area | Current conclusion | Evidence / boundary |
|---|---|---|
| APK data | Pinned original/community audits remain unchanged | APK hashes/inventories describe their exact source, not all future mods |
| Core | Original methods privately generated; same-input patched JAR matches pre-controls main | Controller/KeyData bytes match original; no task/combat rewrite |
| Loader/graphics | Audited codecs + PRIVATE metadata; v1.1 VisualQuality memory path retained | Heavy dbz_mobile_v9 successive fights approved; accepted blur remains |
| Input | VitaControls through original KeyData; combat/hidden pads/character X/Start/Circle accepted over tests | Dialogue X failed and is retired; exhaustive mod/menu coverage open |
| Selector | English, Vita first / Touch only second, remembered per-profile choice | Original/mod text and user folder names keep their language |
| Audio | Native PCM/Vorbis and content-sniffed MP3/AAC, indexed file-backed AAC/M4A | Clean audio reported in tested paths, all character/phrase coverage open |
| Saves | Stable profile save.bin retained; input preference separate | Test save-controls-test.bin is isolated; no automatic migration |
| Build/package | Local complete full-engine VPK 1.2; exact allowlist/CRC/SELF/SFO/assets/import-headroom checks pass | Fresh stable binary still awaits its own physical retest |
| Regression | JVM controls/native preferences PASS; Python 41 pass / 20 gated skips | These local checks do not certify every Vita scene/mod |
| Extractors | Existing Web/Windows profiles-v1 data remains compatible | No re-extraction needed for valid existing profiles; no new extractor version claimed |
| Multiplayer | Disconnected/offline boundaries retained | No synchronized Vita multiplayer |
| Markdown | Repository-wide current/historical status review | [Documentation index and audit](DOCUMENTATION_INDEX.md) |

The chronological bootstrap/00.xx tests retain their original identities and
observations; later successes do not retroactively certify earlier failures.

## Completed corrections from the bootstrap audit

Extraction validates aliases/conflicts/CRC and preserves raw bytes. The 00.28 VFS
rejects unsafe/non-regular resources and isolates the selected APK dataset;
missing files do not fall back across profiles. PAC reads
validate extents and allocation budgets. Subsequent integration fixed missing
resume initialization, direct-buffer GC ownership, charset boundaries, stable
touch IDs, Community14 BIN/WAV normalization, card-task scheduling and PVF
rectangle usage. Current audio setup logs distinguish actual failed syscalls.

## Remaining risks and verification gaps

- Full downloadable character data is absent in the first original APK.
- A file named Original in the selector does not prove installed provenance.
- Imported Community14 assets do not reproduce altered Java mechanics.
- PAC LE fields do not imply CNV/text/save fields are LE.
- Format and build success do not establish audible fidelity or stable gameplay.
- Historical blocker chain: 00.20 thread setup -> 00.21 filter mask -> 00.22 whole-PAC managed allocation -> 00.23 streaming repair -> 00.31 Loading polarity regression -> 00.32 protected-PAC repeated-fight bad_alloc. 00.33 closes the latest reproduced allocation failure on hardware.
- Save interoperability, return/suspend lifecycle, arbitrary mods, all secondary
  modes, complete font coverage and physical controls lack an exhaustive device matrix despite the approved tested paths.
- mod.json/selector Unicode labels, remembered profile selection and log rotation
  are open; remembered per-profile control choice is implemented.
- Verify full-engine notices/attribution and relink materials before public
  distribution; [THIRD_PARTY](THIRD_PARTY.md) records actual delivered scope.

Next: record the exact v1.2 hardware result and broaden regression coverage to Shop return, return-to-menu,
suspend/resume, saves across more profiles, secondary modes, additional mods and
longer sessions. [PORTING_PLAN](PORTING_PLAN.md) tracks that work; resolved
loading/allocation blockers should not be reopened without new evidence.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
