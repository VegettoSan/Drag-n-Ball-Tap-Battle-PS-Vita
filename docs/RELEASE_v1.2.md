# Dragon Ball Tap Battle PS Vita v1.2 — PS Vita Controls

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

## What's new

- Physical PS Vita combat controls with hidden touch pads.
- Before every profile launch, choose **PS VITA CONTROLS** first or **TOUCH ONLY** second. The English launcher remembers the profile's highlight.
- D-pad or left stick moves in combat; D-pad left/right changes character, and X confirms a ready character.
- X attacks; Square, Triangle, Circle and R use special shortcuts 1–4. L activates rage when available.
- Start pauses and resumes from the main pause menu. Circle uses the original available Back action in audited menus.
- Dialogue advancement and other menu choices remain touch-operated. Dialogue X was removed after failing the hardware test.
- English launcher messages and an [English control diagram](VITA_CONTROLS_REFERENCE.md).

The original task, combat and controller implementation is retained. Buttons
feed the original touch-input boundary; hidden pad sprites use a sparse runtime
DAC overlay. Original PAC files and shared caches are not edited. The v1.1
VisualQuality loader and memory repairs remain in place, including the accepted
adaptive texture quality limitation on heavy protected mods.

## Installation

Install `Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk` with VitaShell over the stable
application. APP_VER is `01.02`; TITLE_ID remains `DBTB01178`. Keep existing
`ux0:data/DBTapBattle/profiles/<Profile>/` data and `save.bin` files. Data
extraction is unchanged; existing prepared profiles do not need re-extraction.

The experimental `DBTBCT001` bubble and `save-controls-test.bin` remain separate.
There is no automatic transfer of experimental progress into the stable save.
`vita-controls.cfg` contains only the input preference; legacy values retain
their meaning. The two launcher rows do not change the original/mod language.

The VPK includes no original APK or proprietary playable game data. See the
[installation and extraction guide](INSTALLATION_AND_EXTRACTION.md).

## Evidence and scope

The user approved the retained controls over several test builds, most recently
Circle in Test 5. Dialogue X still failed and was retired at the user's request.
[Final hardware report](evidence/vita_controls_hardware_report_test_5.json).
That report did not include a profile identity or runtime log. It is not blanket
certification of every original menu or community mod.

v1.2 is a fresh local full-engine build, not a metadata-only repack and not a
workflow/native-smoke stub. The exact rebuilt stable package has not yet had
its own physical install/launch check. [Build provenance and checksums](evidence/vita_release_1.2.json).

## GitHub release preparation

Use release tag **1.2**, following the existing published **1.1** convention.
Target the build source commit recorded in the evidence, attach the exact VPK
and include its SHA-256 below. This document can be used as the English release
body. No release workflow was dispatched. Public upload is left to the
maintainer; this documentation does not claim release 1.2 is already published.

## Exact package identity

| Field | Value |
|---|---|
| File | `Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk` |
| Size | 2750706 bytes |
| APP_VER / TITLE_ID | `01.02` / `DBTB01178` |
| Build source | `f6e9adaa792d38c4f3a7c7b27d112ae54b41ede5` |
| VPK SHA-256 | `343aee505f77fa743339111fa7cf29f1e9bda333e49bddb6be166933d7bac1fc` |

Full local compilation and packaging pass: 468 TeaVM classes / 4103 methods,
ARM ELF + VELF + SELF, source/version markers, 7080-byte import headroom, exact
public file allowlist, ZIP CRC, approved LiveArea/theme/seed and packaged SELF
equality. Input and preference probes pass; Python regressions pass (41 run,
20 fixture-gated skips). All patched original JAR entries are identical to the
existing main patch pipeline regenerated from the same private input.
