# Full original-engine Vita test — 00.03

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Historical build sheet: its identities/results apply to the named build only.
<!-- DBTB_DOC_STATUS:END -->

**Archived test sheet.** 00.03 is the first full-core build, not the current
installation recommendation. Its device startup rejected vitaGL's normal false
return; that interpretation was corrected in subsequent builds. The original
publication's pending checks below are retained as history. Current hardware baseline: [TEST_VITA_00_34](TEST_VITA_00_34.md); public release status: [CURRENT_STATUS](CURRENT_STATUS.md).
Evidence: [00.03 startup finding](evidence/vita_hardware_vgl_init_00.03.json).

This is the first PS Vita package built from the complete reachable original
`TCBManajer` Init/Run core instead of the diagnostic bootstrap. Build success is
confirmed; Vita3K and real-hardware behavior are still pending.

## Install

1. Install `DBTapBattle-Vita-00.03.vpk`.
2. Extract the private data package produced from the two user-owned APKs.
3. Copy its `data/DBTapBattle` directory to `ux0:data/DBTapBattle`.
4. Keep the vitaGL shader compiler dependency required by the device setup
   (`libshacccg.suprx`) available as for other vitaGL homebrew.

Expected layout includes at least:

```text
ux0:data/DBTapBattle/game/common.pac
ux0:data/DBTapBattle/game/bobj00.pac
ux0:data/DBTapBattle/mods/Android14/common.pac
ux0:data/DBTapBattle/mods/Android14/char00.pac
ux0:data/DBTapBattle/mods/Android14/chardemo00.pac
ux0:data/DBTapBattle/mods/Android14/charf0000.pac
...
ux0:data/DBTapBattle/mods/Android14/char12.pac
ux0:data/DBTapBattle/mods/Android14/chardemo12.pac
ux0:data/DBTapBattle/mods/Android14/charf0012.pac
```

`Android14` intentionally falls back to `game/` for files it does not override;
for example `bobj00.pac` resolves from the original profile.

## Test order

Use **Android14 first**. It is the supplied profile that contains all 13 locally
verified character triplets. The original APK itself did not bundle those
previously downloaded character packages, so Original is not yet evidence for a
complete battle path.

1. Launch the VPK and select `Android14` in the native profile selector.
2. Confirm that the original animated title appears.
3. Use front touch to enter/navigate the original menu. The gameplay path still
   preserves the original touch controller; physical-button gameplay mapping is
   not claimed yet.
4. Enter character selection and select fighters.
5. Start one battle and test movement/attacks with the original touch controls.
6. Confirm BGM, sound effects, Japanese text and at least one return/menu flow.
7. If possible, start a second launch to exercise save loading.

## What to return after the test

Always copy:

```text
ux0:data/DBTapBattle/logs/runtime.log
```

If the app crashes, also copy the generated `psp2core-*.psp2dmp` file. A photo or
short video is useful for visual/render/input issues but is not required when the
log already identifies an initialization failure.

Useful success markers include the selected profile, successful VFS startup and
TeaVM's `ORIGINAL ENGINE INIT PASS` line. Do not classify 00.03 as HARDWARE
CONFIRMED until the device test actually reaches the corresponding milestone.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
