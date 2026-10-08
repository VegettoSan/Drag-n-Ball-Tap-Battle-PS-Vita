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


### Direct VitaSDK build and package audit

Compiled the **full original APK-derived TeaVM core** locally (not GitHub Actions and not the native dummy smoke core) with VitaSDK 2026.08 and the new file-backed AAC reader:

- Experimental package: `DBTapBattle-Vita-Universal-AAC-Stream-Fix-Experimental-01.00.vpk` (2,747,339 B).
- SHA-256: `1d4af497e86a53135c3fc20ece76a39cb5cb923bbc707b2197be241d98923833`.
- ELF → Sony VELF → SELF `eboot.bin` → VPK succeeded with the `compressed_bgm.cpp` `-Os` setting.
- ZIP CRC audit passed. The 15 member names match the previous MemoryFix experimental VPK, and **only `eboot.bin` differs**. LiveArea icon, background, startup, theme, selector graphics and seed files remain unchanged. `tools/validate_livearea_vpk.py` reports PASS.
- The compiled ELF contains `Compressed BGM indexed:`, the universal image marker `C14U` and the retained `Combat resource boundary:` diagnostic.
- Still experimental: Sony Vita hardware tests are pending. Check `dbz_mobile_v9` with the previous character pairing (char37 versus char20) and capture `runtime.log` if the crash persists. A successful build does **not** by itself prove playback or gameplay stability.

## Third PS Vita test — combat succeeds; transition to next fight stops responding

### New `runtime.log` evidence

- Full original-engine universal AAC-stream build, `dbz_mobile_v9`. The previous `bgm_03.ogg` allocation exception no longer occurs: `Compressed BGM indexed` confirms file-backed playback.
- Entering the tested battle generates a `[Perf]` window with **154 textures / 20,837.9 ms of texture work**, with a single **16,948 ms** maximum frame. Once in combat, `[Perf]` windows stabilize at ~59.4–59.9 FPS. This is a real loading bottleneck, **not a 4-FPS combat engine**.
- On the next fight transition, the last completed native stream is `chardemo37.pac`, after `char07.pac`, `back06.pac` and `demo_00.pac`. The user reported over three minutes stuck; however the log has no timestamp after that last stream, so it cannot distinguish a blocked TeaVM update from texture loading or a native lock.
- **No new psp2dmp was supplied** and there is no `std::bad_alloc` in this third log. Do not relabel a hang as another crash. Never assume the BGM path is the cause.

### Narrow experimental diagnostics + allocation reduction

- New `src/diagnostic_watchdog.hpp` tracks the main frame's progress and the current native texture phase with small atomic counters. The *existing* audio worker reports `[FrameWatchdog] blocked_seconds=... phase=... bytes=... operation=...` every 10 seconds if the main frame is stuck for at least 8 seconds while audio keeps running. Phases: 1 original-engine frame, 2 image decompression, 3 GL texture upload; stage 0 indicates outside original frame. If the watchdog also stops reporting, examine audio thread/GPU waits with a core dump and debugger; **do not spin or alter gameplay**.
- `src/image.cpp/.hpp` expose new pointer+length image decoding entry points and preserve all existing vector APIs. `dbtb_loadTexture` now passes raw `C14*` image bytes and original PNG bytes directly instead of duplicating each compressed image in temporary vectors. Decoder output, premultiplied alpha, GL upload and texture ownership are unchanged.
- Performance windows now show both `texture_decode_ms` and `texture_upload_ms`; any upload longer than 750 ms records `[TextureSlow]` with dimensions and byte count. Diagnoses the 20-second battle load before changing GPU behavior or texture cache lifetime.
- Host native regression passed: Original + Android14 real-data fixtures, **125 PACs, 137 containers, 470 textures, 68 protected BIN tables and 198 PCM WAV**; VPK full Vita original TeaVM core compiled directly and passed `validate_livearea_vpk.py`. ZIP member list and LiveArea contents match the previous AAC-stream VPK, with `eboot.bin` the only changed member.
- Test VPK SHA-256: `740647d4cd8fb74b9801df441fa90fd03ed6c882b30c40e4d4cc33dc8da145f5`. filename `DBTapBattle-Vita-Universal-NextFight-Diagnostic-01.00.vpk`.

### Next required PS Vita test

With the same `dbz_mobile_v9` profile and existing save, start battle 37 versus 20, finish it, and proceed to battle with character 07. If transition hangs, wait 20–30 s, then retrieve the full `runtime.log`. Specifically inspect `[FrameWatchdog]`, `[TextureSlow]`, `texture_decode_ms`, and `texture_upload_ms`. This determines where an actual behavioral optimization should target. Stable v1.0 release stays unchanged; nothing was modified in original AOT gameplay.

### Real APK resource atlas pressure (independent offline validation)

The supplied **dbz mobile v9.apk** was examined directly using its dynamically recovered PRIVATE codec, decoding the outer PAC entries and the per-image XOR dimensions. No game assets or gameplay logic were changed.

| Resource | Protected file bytes | RGBA images | Total RGBA pixel data |
|---|---:|---:|---:|
| char37.pac | 5,220,001 | 93 | **158.55 MiB** |
| char07.pac | 3,571,656 | 73 | **124.54 MiB** |
| char20.pac | 2,608,467 | 21 | **19.50 MiB** |
| effect.pac | 2,317,013 | 28 | **24.29 MiB** |
| chardemo37.pac | 1,017,260 | 5 | **5.78 MiB** |

Many actor pose textures are 712x712 pixels (2,027,776 RGBA bytes each). These numbers are **not** measurements of concurrent VRAM residency: a resource can load and release images or reuse textures. But they are compelling evidence for substantial texture throughput and allocation pressure on fight transitions.

Keep full image quality for now. The diagnostic VPK distinguishes texture_decode_ms versus texture_upload_ms and has a native frame watchdog, so hardware evidence can direct a narrow subsequent optimization. Avoid unilateral image downsampling across already-confirmed older profiles.
