# Vita Controls Test 4 — English launcher

Branch: `test/vita-controls`. Stable `main` remains unchanged. Test 2 is user
accepted; Test 3's new pause/back/text shortcuts and this English launcher
still need physical Vita testing.

## Install

Install **DBTapBattle-Vita-01.05-Controls-Test-4-English.vpk** over the existing controls-test
bubble with VitaShell. TITLE_ID `DBTBCT001`, APP_VER `01.05`, title
**DB Tap Battle Controls Test**. Existing isolated profile progress stays in
`profiles/<Profile>/save-controls-test.bin`; stable `save.bin` is untouched.
Existing profile data does not need reinstalling.

## English selector

The launcher asks on every profile launch, remembering the highlighted row:

| Label | Behavior |
|---|---|
| TOUCH ONLY | Original touch input/settings |
| PS VITA CONTROLS | Vita button input with original touch-pad sprites hidden |

All launcher-owned text is English, including profiles, controls, hints,
loading, empty-data messages and fallback labels. Profile names remain the
user's folder names. This change does not translate original game/mod text.
The help now shows hidden pads, touch/Circle back, X text and character select,
and Start resume. Layout, original theme textures and preference values are
unchanged. Input behavior is exactly Test 3; this build changes native copy
and package version only.

## Test on Vita

1. Open the launcher, confirm English profile/controls/help/loading text and
   centered button labels. Verify **TOUCH ONLY** and **PS VITA CONTROLS**.
2. Open a profile in Vita mode: hidden pads, previous progress and remembered
   choice must remain. Reopen a profile and verify it asks again.
3. Follow the [Test 3 shortcut checks](TEST_VITA_CONTROLS_3.md): Start pauses and
   resumes the main pause menu; Circle returns when the original back button
   is shown; X advances/reveals interactive text. Held controls do not repeat.
4. Compare buttons with the [English control schematic](VITA_CONTROLS_REFERENCE.md).

## Validation scope

Python: 61 total, 41 passes, 20 existing gated skips. Native English labels
fit original text widths and are present in ELF; old Spanish selector literals
are absent. Local complete ARM/VELF/SELF/VPK build and ZIP/SFO/SELF/seed/theme/
LiveArea checks pass, no workflow. Core/Java/input code unchanged; Test 3 JVM
and existing native preference/visibility tests are inherited, not rerun.
The patched original JAR matches the main pipeline regenerated on the same
private dex2jar input in every entry; Controller/KeyData also match original.
No new engine patches or private source uploads.

VPK SHA-256: `352ddfd66e78758b49e24819b6b5cee31c8e64010581d83e63f2e1c0e454d924`.
[Exact evidence](evidence/vita_controls_test_4.json).
