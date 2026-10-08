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

## Second hardware failure: definitive native call chain (2026-10-07 23:25)

Supplied `dbz mobile v9.apk`, SHA-256 DEX `b90abccce8952240760f50c4bad31a1da8fbd3621992391e77301bc89283118d`, was independently parsed: **210/210 protected PAC directories valid, 41 contiguous characters, no unrecognized PAC types, 642 embedded WAV streams**. This is a new protected profile and not the separately tested DBS Mobile v1.

User's second `runtime.log` records `Combat resource boundary: bobj03.pac idle_cache_released=1473726 newlib_used=93850288 newlib_free=6002000` followed by `std::bad_alloc`. Contrary to the initial working hypothesis, the latest compressed Sony core dump **does** narrow the allocation source precisely when parsed with the Vita-specific `THREAD_INFO` / `THREAD_REG_INFO` notes and rebased to the debug ELF.

- Core main module loaded its text at `0x81046000`, debug ELF linked at `0x81000000`. Subtract `0x46000` from runtime code addresses before symbolication.
- Main thread `DBTB01178` stopped at `0x8129e986` (terminate/kill). Stack contains `operator new(unsigned int)` at `0x811d6d59`, then `std::vector<unsigned char>::_M_default_append` at `0x811b5847`, and **`dbtb_openCompressedBgm` at `0x811c6b41`**.
- Other frames include `dbtb_bgmPlay`, original `SoundEffect.playBgm`, `TCBManajer_PlayBGM`, `TCBManajer_GdtBGM` and `TCBManajer_Game1`.
- The dumped stack string identifies the requested track **`bgm_03.ogg`**; in this mod it is an **AAC/M4A** (MP4 `ftyp`) weighing **2,159,645 bytes**, not Ogg Vorbis. The original compressed player loaded this entire file into a `std::vector<uint8_t>` after the fighters/effects were resident. Newlib had approximately 5.7 MiB free in total but no successful large contiguous allocation.
- The previous fix correctly reclaimed idle PAC cache ownership, but could not eliminate this **independent audio allocation**; do not keep modifying PAC/engine loading based on that earlier hypothesis.

### Targeted new remedy (source, hardware acceptance pending)

`tools/aot/engine/native/compressed_bgm.cpp` now creates a **small file-backed AAC/M4A index** from `ftyp` and `moov`, replaces `mdat` with an eight-byte placeholder in the parser's in-memory view, and reads original AAC sample bytes on demand using a 64 KiB read-ahead buffer. Retains the existing Vita Audiodec API, sample positions, looping, gains, resampling, native audio thread ownership and SceAudiodec decoder-lifetime behavior. The existing Ogg/Vorbis, MP3, PAC and original AOT gameplay code paths are untouched. Oversized/truncated MP4 atoms reject safely. Index limit 1 MiB, source limit 16 MiB. The change avoids the 2.16 MiB allocation observed in the failed battle.

Real APK offline validation: `bgm_03.ogg` metadata footprint **41,614 B** (instead of 2,159,645 B); original `stsz` describes **5,701 samples**, maximum compressed ES packet **599 bytes**. All nine M4A BGM files in this mod have index footprints under 102 KiB and each sample's max size is below Vita's supported AAC `SCE_AUDIODEC_AAC_MAX_ES_SIZE=1536`. These data-only checks do **not** prove realtime audio on PS Vita; a new full-engine hardware test must confirm both audio and combat.

Build note: native cold-path `compressed_bgm.cpp` is size-optimized separately with `-Os` to leave appropriate Sony ELF converter segment headroom. Do not change TeaVM heap policy or touch the original game logic to address this specific crash.
