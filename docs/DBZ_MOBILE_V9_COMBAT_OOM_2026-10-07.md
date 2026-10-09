# 2026-10-07: dbz_mobile_v9 — combat-loading `std::bad_alloc` after universal decoder

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Pinned source/research facts retain their corpus; dated runtime proposals are historical.
<!-- DBTB_DOC_STATUS:END -->

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

## Fourth hardware report: crash on first combat after NextFight diagnostics

**Observed:** user reported a crash at first fight when testing `DBTapBattle-Vita-Universal-NextFight-Diagnostic-01.00.vpk`, with `runtime.log` and `psp2core-1791435467-0x0000f328c7-eboot.bin.psp2dmp`.

**Source evidence:**
- `runtime.log` selects `dbz_mobile_v9`, detects 41 character triplets, and loads char37/char20/effect/cards/charf/bobj03 for combat. A `[TextureSlow]` line records a **4,003,310 microsecond** GPU texture upload of a 512x512 RGBA image. Last line: `std::bad_alloc`. There is **no** subsequent `Compressed BGM indexed: bgm_03.ogg` or `Combat resource boundary` on this run.
- Decode gzip-wrapped Sony ELF core (5,382,916 bytes). The MODULE_INFO note reports `dbtb_original_engine` load text base **0x81025000**. Rebase addresses by subtracting **0x25000** against the exact `NextFight` unstripped ELF (previous debug binaries have a different base/shape; do not mix them).
- The crashing main thread's stack contains `operator new(unsigned int)` -> `decodeCommunityImageProfile(const unsigned char*, ..., PacEncoding,...)` -> `dbtb_loadTexture` -> original `AndroidGLTexture.loadTexture` -> `GameData.Init`. This is a **large protected sprite decompression allocation**, *not* the earlier full AAC song buffer exception. It does not prove a particular individual image size caused exhaustion without malloc request tracing.
- The protected `dbz mobile v9.apk` char37 and char07 atlas directories contain numerous **712x712** RGBA frames, decoded to **2,027,776 bytes each**. Existing decoder materialized a complete image pixel vector for every texture, transiently increasing Newlib's memory pressure near gameplay transition. No original game-loop logic faults are proven.

### Fourth targeted mitigation: original-quality bounded texture streaming

Changed only the native image/texture bridge for **dynamic C14U protected profiles**. For these profiles:
1. Validate the exact original width, height, entry index, type and 16 MiB image bound *before* allocating a pixel buffer.
2. Create an original-resolution GPU texture with `glTexImage2D(...,nullptr)`.
3. Incrementally decode raw DEFLATE scanlines into at most **256 KiB** of temporary RGBA storage, uploading each complete strip with `glTexSubImage2D`. Validate zlib termination, full pixel count, and full input consumption.
4. Keep the texture cache ownership, reference counting, parameters, dimensions, premultiplied alpha and normalized UV semantics. On malformed image / GL allocation failure, log `[TextureStream] rejected` and fail that texture without deliberately changing gameplay or performing resolution downsampling.
5. Retain pointer/vector decoding for original, Android14, Spanish, Invasion and DBFZ formats; do not activate the new upload algorithm outside `PacEncoding::Community14Dynamic`.
6. Separate source-size optimization `-Os` for `image.cpp` and `native/resources.cpp` to meet Sony ELF stub requirements. Leave original TeaVM and AAC streaming unchanged.

**Direct no-workflow build:** `DBTapBattle-Vita-Universal-TextureStream-Experimental-01.00.vpk`; SHA-256 **`74e7b66e49a55b6c68cf963daa0862e48c463a5af04738a707cc99d9dfe1dd62`**, size **2,740,298 bytes**, complete TeaVM core compiled locally with VitaSDK 2026.08. ELF -> VELF -> SELF -> VPK passed, LiveArea validation passed, all 15 ZIP entries equal to `NextFight` VPK **except `eboot.bin`**.

**Host proof:** synthetic protected DEFLATE 1x1, 100x113, 512x512, 712x712 and 4096x1024 images decoded byte-for-byte identically to the full-buffer algorithm; truncated streams rejected. Extracted *actual* 712x712 protected images from `char37.pac` (entry 17) and `char07.pac` (entry 18) of the user-supplied APK also matched exactly, with **8 tiles / 262,016 maximum temporary pixel bytes**. Legacy community and engine-resource regression tests passed.

**Status: experimental; hardware acceptance pending.** Do not mark the crash fixed until user confirms entering and completing first and second fights. If it fails, capture full `runtime.log` and Sony `psp2dmp`; `[TextureStream]` messages report per-image tile count and CPU/GPU timing. Particularly distinguish a further texture allocation failure from a GPU stall and the prior audio crash. Existing v1.0 release remains untouched.


## Fifth Vita hardware report: TextureStream crash at first battle (2026-10-08)

**Inputs**: user-provided `runtime.log` and `psp2core-1791436599-0x0001922053-eboot.bin.psp2dmp` while using experimental `TextureStream` VPK with dynamic `dbz_mobile_v9` profile.

### Evidence and correction of prior assumption

- The new `[TextureStream]` entries show 512x512 textures decoded and uploaded in four approximately 256 KiB chunks in 20-30 ms under light load. These logs independently confirm the bounded raw DEFLATE decoder was used.
- Near initial combat creation, the `[TextureStream]` entry for a 512x512 effect texture instead records **4023 ms total / 4005 ms GPU**; the final source resource reported before crash is `bobj03.pac` (`io_bytes=352833`). No completed combat boundary appears.
- The gzip-wrapped Sony core has a `dbtb_original_engine` text base of **0x81069000**. Important: this differs from the previous dump's base 0x81025000. The exact ELF symbols must be rebased with **0x69000**; otherwise call stacks produce misleading names. Rebased stack includes `glTexSubImage2D -> _malloc_r` along with `uploadRows -> decodeCommunityImageProfileRows -> dbtb_loadTexture -> AndroidGLTexture.loadTexture -> GameData.Init`. Root physical allocation behavior inside vitaGL is not yet proven beyond the visible stack.
- **Conclusion:** the CPU image decompression copy optimization alone does not address repeated GPU-side allocation/transfer pressure in this protected mod. Do not claim the AAC fix regressed; it remains unchanged.

### Experimental GPUCompact mitigation (for protected dynamic codec ONLY)

Because 712x712 protected character atlas images can individually require 2,027,776 bytes of RGBA8 plus driver staging, the new experimental native bridge avoids **all `glTexSubImage2D`** calls in `PacEncoding::Community14Dynamic`. The original/base/known protected profiles preserve their existing decoder and GL path.

1. DEFLATE decompression retains bounded 256 KiB scanline strips.
2. Dynamic images at least 384 px on either axis are sampled to half resolution; smaller ones retain their original resolution. Original source PACs, pixel coordinate definitions and original Java engine gameplay logic are untouched. **This changes physical image sharpness, intentionally and only in the new experimental path.**
3. Pixels are packed into 16-bit `GL_UNSIGNED_SHORT_4_4_4_4` (4-bit R/G/B/A, including quantized premultiplied alpha) and transferred using **one `glTexImage2D`** rather than a preceding allocation and multiple sub-image uploads.
4. VitaGL **requires `internalFormat=GL_RGBA`** to trigger its `fast_store` path for 16-bit packed input. Passing `GL_RGBA4` is insufficient because that path can convert to 32-bit. Channel bits follow vitaGL's `read_rgba4444`: R occupies bits 0-3, G bits 4-7, B bits 8-11, A bits 12-15.
5. The bridge reports the original *logical* width/height to the untouched engine, so original sprite layout and normalized UV math remain consistent. Physical GPU dimensions are smaller for large dynamic textures. A `[TextureCompact]` diagnostic records logical, GPU dimensions, packed KiB and decoding/upload times. On GL errors it deletes the new texture and returns -1.
6. CPU source RGBA budget remains bounded: 256 KiB strips plus one packed output allocation, at most 8 MiB. For actual 712x712 images from mod char37/char07, physical output is 356x356 pixels at **253472 B** each vs **2027776 B** original RGBA8 (**87.5% less**).

### Build and validation

- Built **directly, without GitHub Actions**, from the complete original DBTapBattle.apk TeaVM core using local VitaSDK 2026.08 (not the CI dummy core).
- `DBTapBattle-Vita-Universal-GPUCompact-Experimental-01.00.vpk`, **2,740,383 B**, SHA-256 `d76f33a82995e0386cdebeeda60b0e82b07dd4d53041b3117ce17f3602fdb18b`.
- Full ELF -> Sony VELF -> SELF -> VPK succeeded and LiveArea icon/background/startup/template validation passed; ZIP CRC verified. Relative to the previous TextureStream test VPK, all 15 file paths and content remain identical **except `eboot.bin`**.
- Previously supplied real compressed 712x712 source images from `char37` and `char07` passed byte-perfect bounded decoding tests; host resource normalization regression also passed for Android14/Spanish/Invasion/DBFZ. Repacked GPU pixels are predictably reduced to 4-bit precision and half-resolution (not byte-perfect source quality).
- **No hardware gameplay success claimed.** Before any stable release, test both a new protected mod and at least one previously working original/known mod. If GPUCompact still crashes, retain the new `runtime.log` and `psp2dmp`, and consider resource lifetime/driver allocation behavior instead of blindly reducing resolution further.


## Sixth hardware report: GPUCompact works, color/alpha wrong (2026-10-08)

User tested `DBTapBattle-Vita-Universal-GPUCompact-Experimental-01.00.vpk` on a real Vita with the previously crashing `dbz_mobile_v9` profile and supplied four display photographs plus `runtime.log`.

### Confirmed on hardware

- Menu, character selection, first fight, victory screen and next fight load **without crash or deadlock** during this test. The log shows sequential fights and periods close to **59.9 FPS** during gameplay.
- Native AAC/M4A `Compressed BGM indexed` loads succeed and original gameplay behavior is retained.
- The screen exhibits an obvious red/pink wash, distorted transparency/ki effects and excessively pixelated character/UI atlases. This is **not** an acceptable stable release.
- `[TextureCompact]` showed 512x512 -> 256x256 and 912x912 -> 456x456, explaining the soft presentation. The earlier GPU OOM is resolved *for the observed hardware test*, not every possible mod.

### Root cause of the red/pink tint and transparency

Old compact output wrote the 16-bit word as `AAAABBBBGGGGRRRR` (R in the least-significant nibble). But the **single direct** `glTexImage2D(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4)` upload and its `SCE_GXM_TEXTURE_FORMAT_U4U4U4U4_RGBA` hardware fast path interpret it as standard **`RRRRGGGGBBBBAAAA`** (R most significant, A least significant). The red channel was receiving the source's alpha and the output alpha was receiving source red. Consequences: intense red/pink appearance and invisible translucent visual effects. This was introduced **only by the GPUCompact experiment**.

Correct packing uses `((R&0xF0)<<8) | ((G&0xF0)<<4) | (B&0xF0) | (A>>4)`, retaining alpha in the correct nibble.

The repaired pack was evaluated against independently decoded real `char37.pac` and `char07.pac` images, both 712x712: output RGB(A) mean channels match the source to quantization precision (maximum per-channel difference <= 15 out of 255); the previous pack interchanged alpha and red. Every individual 4-bit alpha value can be represented. These are offline CPU checks; actual color correctness still requires the Vita visual test.

### Generic quality policy, not mod-specific hacks

- Keep **one GL upload, RGBA4444 packed storage and 256 KiB DEFLATE row decoding**. No `glTexSubImage2D`, no full-size RGBA temporary, no original Java logic changes.
- For any protected dynamic `C14U` image, choose a physical GPU resolution with longest side **up to 512 pixels**, preserving original aspect ratio. Small textures keep original resolution; examples: 512x512 now 512x512 (vs 256x256), 712x712 now 512x512 (vs 356x356), 912x912 now 512x512 (vs 456x456).
- Track current GPU-allocated bytes for the dynamic bridge, including cached textures until eviction; discount allocations on texture delete. After a **24 MiB soft quality budget**, new images automatically fall back to the previous conservative downscaling sizes. Do not attempt a risky allocate-then-retry fallback because a failing VitaGL allocation can terminate the process.
- All sizing and budget choices depend exclusively on image dimensions, live texture lifetime and available dynamic-bridge budget. **No mod name, saved-profile name, APK-specific override, or per-mod configuration** exists in the code.
- Original game, Android14, Spanish, Invasion and DBFZ legacy `C14R/C14S/C14I/C14D` and native PNG paths remain **byte-for-byte unchanged** in the renderer.
- Source PAC files, `dbtb_codec.json` and saved game files are neither modified nor regenerated.

### Direct VitaSDK experimental rebuild

Build `DBTapBattle-Vita-Universal-VisualQuality-Experimental-01.00.vpk` directly with VitaSDK 2026.08 and original TeaVM core. Offline original+Android14/Spanish/Invasion/DBFZ resource regression passed; real 712x712 protected DEFLATE pixel streams validated byte-for-byte; actual original pixels versus RGBA4444 output compared, max quantization error 15; LiveArea/ZIP validation passed. The only VPK member changed compared with the previous hardware-proven GPUCompact experiment is `eboot.bin`.

**Status: experimental.** Ask the user to verify restored color/transparency, first fight, next fight and at least one previously working original/legacy mod. Avoid claiming the enhanced quality tier is hardware-safe until measured.


### Artifact metadata and current acceptance status

- Direct compiled VPK file: `DBTapBattle-Vita-Universal-VisualQuality-Experimental-01.00.vpk`; **2,741,413 bytes**, SHA-256 **`479fb4e4b2fc50d7dc7dec08c08101bb83ecb6b414328cf467d85e9e9b09d71a`**.
- Verified 15 ZIP members; `eboot.bin` is the only member changed relative to the hardware-tested GPUCompact VPK. Icon, startup image, LiveArea background/template, installed app metadata and bundled resources remain unchanged.
- Direct VitaSDK ELF -> VELF -> SELF -> VPK build and `validate_livearea_vpk.py` **PASS**.
- Native host image/PAC regressions **PASS** for Android14, Spanish, Invasion and DBFZ. Two actual 712x712 PRIVATE protected image streams **PASS** exact RGBA decompression. New CPU-side nibble ordering agrees with real source images within 4-bit quantization (per-channel max error <= 15).
- **Experimental on Vita** until visual color and alpha, quality, first battle, next battle and previously working profiles are confirmed by hardware testing. Do not label it as stable 1.0 yet.


## Final hardware acceptance and release freeze — 2026-10-08, v1.1

**User-confirmed outcome:** On an actual PS Vita, the VisualQuality experimental VPK successfully ran the `dbz mobile v9` mod, including first fight, victory and transitioning to later fights. No repeat crash or three-minute freeze occurred in the accepted test session. Heavy mods still load more slowly than light/original datasets, which is accepted for this release.

**Final uploaded `runtime.log` evidence** (user tested, 2026-10-08 00:50 local): `Selected profile: dbz_mobile_v9`, 7 visible installed profile directories, 78 `[TextureCompact]` diagnostics, 7 `Compressed BGM indexed` music events, 3 `Combat resource boundary` events, and 340 `[Perf]` reporting windows (317 at >=58 FPS). No `std::bad_alloc` or crash message appears in the log; independent user observation confirms successive playable fights. Do not extrapolate this to every mod or any duration beyond the test.

**Accepted known issue:** Some sprites/UI backgrounds/effects remain very blurry while other textures are fairly sharp. Cause is the **intentionally lower physical resolution for some protected high-resolution sprite atlases** chosen by the generic dynamic `C14U` GPU memory-aware path, using the RGBA4444 texture format. This is the protective tradeoff that stopped the prior system/GPU-memory crashes. Source images and protected PACs remain intact on disk. Sprite-atlas grouping can make several visual elements share a lower-resolution texture; we have not proved this explains every soft asset individually. Fine-grained selective image fidelity is a **future non-blocking enhancement**.

**No mod-specific hacks:** The accepted quality policy depends only on resource dimensions and dynamic texture memory pressure. All previously audited/known original and community profile decoders retain their prior path. Existing v1.0 was not modified, and the v1.1 baseline is frozen for publication.

### Publication package identity

- Hardware-proven experimental VPK: `DBTapBattle-Vita-Universal-VisualQuality-Experimental-01.00.vpk`, SHA-256 `479fb4e4b2fc50d7dc7dec08c08101bb83ecb6b414328cf467d85e9e9b09d71a`.
- Release candidate: `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk`, SHA-256 `9953e8c99ce59a5b4b55dab3ae2caec788c39ffe1edb19a2d4e5833c958ee6bd`, `TITLE_ID DBTB01178`, `APP_VER 01.01`.
- Game `eboot.bin` SHA-256: `bea473a4f1287702eafb2fe79e1b529d192b862bcd1ef63d681bf13f6a6d3af2`, **byte-identical in both VPKs**.
- The ONLY changed VPK member between experimental and release archives is `sce_sys/param.sfo`, APP_VER `01.00` -> `01.01`, preserving title, icons, LiveArea, selector theme and save seed. CRC/ZIP and LiveArea validation PASS. Repacked metadata has not received a separate physical install test; gameplay executable is the exact accepted one.
- Universal Windows extractor source scripts kept byte-identical to earlier 20-test-pass artifact; release ZIP updates only text documentation, SHA-256 `148480f1f5447086796eaa66ad3f97a45af7a15b5c32f03eadea929b5c4d44a2`.
- Source/release documentation [v1.1 notes](RELEASE_v1.1.md), [Web/Windows installation](INSTALLATION_AND_EXTRACTION.md), [README](../README.md) warn users explicitly about high-resolution mod crashes and protective texture downscaling.

**Release-ready, manually published by the maintainer. Do not claim all mods tested, nor publish the next visual optimization as part of this v1.1 freeze.**
