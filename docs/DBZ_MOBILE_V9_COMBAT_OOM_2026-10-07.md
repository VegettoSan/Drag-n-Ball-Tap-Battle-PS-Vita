# 2026-10-07: dbz_mobile_v9 — combat-loading `std::bad_alloc` after universal decoder

## Hardware evidence

User-tested full-engine universal experimental VPK, TITLE_ID `DBTB01178`, with 7 profiles installed. The selected profile from `runtime.log` is **`dbz_mobile_v9`**. This profile is not the independently inspected `dbs mobile tap battle v1.apk`, so do **not** attribute the failure to that APK.

The log shows:
- Original TeaVM engine initialized, data tables, sound, menu and 41 complete character triplets loaded.
- The selected combatants have resource files `char37.pac` (raw **5,220,001 B**, normalized **5,648,438 B**) and `char20.pac` (raw **2,608,467 B**, normalized **2,889,990 B**).
- `effect.pac` is 2,317,013 B raw; multiple `card*.pac` and `charf0037.pac`, `charf0020.pac`, then `bobj03.pac` (352,833 B raw) completed their PAC resource stream requests.
- Immediately after `bobj03.pac` closed, C++ terminated with `std::bad_alloc`.
- Dump `psp2core-1791432311-0x00004f37cd-eboot.bin.psp2dmp` is a gzip-wrapped ARM ELF core. It supports the crash report; exact malloc request and source callsite are **not** yet symbolicated. The log is the source for the resource sequence.
- There is no "invalid PAC table", "unsupported type", or "missing combat PAC" error before termination. A memory allocation peak is the supported working hypothesis; the cause is **not yet proven to be a specific buffer**.

## Bounded, reversible native improvements

1. `src/engine_resources.cpp`: plan normalized PAC output capacity **from decoded WAV lengths** rather than source PAC length. ADPCM wrappers expand on decode, triggering geometric vector growth when materialized. Pre-sizing reduces avoidable multi-MiB old+new overlap.
2. The `rgba -> C14* -> png` image bridge is appended **directly into the output** rather than copying raw image to `payload`, copying again to `marked`, and then into `out`. Reserved directory/type byte layout and image index are unchanged.
3. `tools/aot/engine/native/resources.cpp`: when the combat object PAC stream (e.g. `bobj03.pac`) closes, release only **idle** resource/texture cache ownership, retaining active streams and in-use textures. Diagnostic Newlib used/free bytes are logged at that boundary.
4. Do **not** increase the Newlib 96 MiB heap or TeaVM 48 MiB max blindly; both are shared within Vita's finite allocation budget. No original Android logic, class behavior, combat rules, or asset conversion was changed.

## Full local test build (same session)

- Built the *complete pinned original TeaVM/AOT engine* locally using user-provided `DBTapBattle.apk`, exported pinned Java tools and VitaSDK 2026.08. No dummy TeaVM core and no GitHub Action used for this binary.
- Output: `DBTapBattle-Vita-Universal-MemoryFix-Experimental-01.00.vpk`; SHA-256 `3562e444ddd72e8552bac107c80377d0927aed97c57bbe48bafcf5945af7a97c`.
- Existing universal experimental VPK ZIP and replacement contain the **same 15 entry names**, and the **only changed payload is `eboot.bin`**. Selector art, LiveArea files, save seed and third-party notices are unchanged.
- Verified ZIP member CRCs, `validate_livearea_vpk.py` pass, full 32-bit ARM ELF (unstripped) and new native `Combat resource boundary` diagnostic string.
- Host original + Android14 real resource regression: **125 PACs, 137 containers, 470 images, 68 converted BIN, 198 PCM WAV**; all pass.
- Do not claim hardware crash resolution until a new `dbz_mobile_v9` battle is tested. Preserve 1.0 release and user save/profile folders.

## Validation / limitations

- Host synthetic protected PAC normalizer tests (Android14, Spanish, Invasion, DBFZ and a dynamic profile) pass with the updated capacity/bridge path.
- VitaSDK compile/link and LiveArea checks are required before handing a test VPK to hardware. Once built, test all existing stable profiles to rule out regressions.
- **Hardware gameplay not yet confirmed fixed**. If it still terminates, collect the new `Combat resource boundary: ... newlib_used=... newlib_free=...` line, full `runtime.log`, `psp2dmp` and the mod APK itself (if shareable) to isolate the exact allocation.

## Important stability policy

Preserve the known-good 1.0 VPK / 00.34 runtime. This repair is an experimental native memory optimization only; do not publish as stable until user confirmation.
