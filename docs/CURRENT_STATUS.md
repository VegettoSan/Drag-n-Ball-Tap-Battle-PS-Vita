# Current status — 2026-10-05 (America/Bogota), full engine 00.24

## Current hardware report — 00.24

Two manual full-engine publication entries are now implemented: Release and
Prerelease, with a shared builder, original-APK hash gate, audio regressions,
VPK/ELF/LiveArea validation, compiled-only symbols and draft-first publication.
Static/unit and real-VPK staging checks pass. GitHub-hosted validation run
[37395626518](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37395626518)
passed actionlint and all 13 publication/LiveArea tests. A live full build/publication
requires the private APK URL secret; see [setup](RELEASE_WORKFLOWS.md).

Before the 00.24 repair, the user confirmed that **00.23 LiveArea-Fixed presentation
worked on hardware**, but reported another battle-start crash with Android14 selected, characters 12/03
and `bobj03`. The earlier successful 00.23 test path remains historical evidence;
this extends coverage to a failing native-audio allocation path. The PAC streaming
repair remains active in the supplied log.

That 00.23 log ends in `std::bad_alloc`. Its supplied core's game-thread stack
returns to `decodeOgg` immediately after PCM vector growth, with a requested
9,506,304-byte allocation. The old decoder reproduces that exact request for
`bgm_03.ogg`: its C++ allocation peak is 14,260,324 bytes for a 5,454,332-byte
PCM track. This differs from 00.22's managed whole-PAC allocation failure.

00.24 allocates the exact Vorbis frame count once, decodes directly into that
buffer, checks channel/rate consistency and decoded length, and drops cache-only
PAC owners/idle imported textures before allocating. Active streams and textures
retain ownership. Newlib remains 96 MiB, TeaVM's maximum remains 48 MiB, and
original battle logic, music samples, voice DSP and approved LiveArea remain
unchanged. New Ogg diagnostics identify the track/frame count.

**Host confirmed:** all 17 supplied BGM tracks match the previous decoder sample
for sample; a 6 MiB single-allocation limit reproduces the old `bgm_03` failure
and permits all fixed loads. Fixed `bgm_03` C++ peak: 5,454,432 bytes, a reduction
of 8,805,892 bytes (excluding Vorbis C allocations and unrelated owners).
Audio setup/DSP and resource ownership probes pass with Vita APIs mocked.
**Hardware confirmed by the user on 2026-10-05 at 19:26 America/Bogota:**
the delivered 00.24 works and resolves the reported battle-start crash. The user
says it works very well; no new runtime log or exhaustive character/profile
matrix was supplied. The earlier failing log/core describe 00.23, not 00.24. See
[the result and broader regression procedure](TEST_VITA_00_24.md).

Hardware-confirmed package: `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk`, 2,663,883 bytes,
SHA-256 `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
Runtime source: `f5672d4d3fbf6b43cd699d7a5a2b80475e4db9f6`. Fresh generation
compiled 467 classes/4086 methods; the standard complete TeaVM amalgamation
compiled at -O1 and native services at -O2. Native smoke CI run `37392864767`
passed, separately from this local full-game build. All five presentation entries
match the hardware-confirmed 00.23 LiveArea-Fixed package byte-for-byte. Exact ELF
and private generated sources are retained with the candidate for crash analysis.
[Identity and measured allocation evidence](evidence/vita_battle_audio_00.24.json).

<!-- DBTB_00_23_DETAIL:START -->
## Historical hardware checkpoint — 00.23 (2026-10-05)

**Status:** hardware validated for the tested path; no error observed in the user's
00.23 session.

Verified together on a real PS Vita in the current progression:

- startup and menu flow continue to work;
- text remains visible;
- audio/voices remain clean (the prior rasp/worker issue did not regress);
- character selection remains responsive (the prior 1–2 second selection stalls did
  not return in the reported session);
- starting a fight now succeeds;
- the fight can be played without the 00.22 battle-start crash.

The specific 00.22 failure was a TeaVM managed-allocation abort while bridging the
entire `char00.pac` (~4.74 MiB) into one Java `byte[]` before the original parser's
disposal order could free the prior owner. 00.23 restores the original streaming
parser contract and exposes the PAC through a Vita native `InputStream` bridge, so
Java receives individual original payload arrays instead of one whole-PAC bridge
array.

This is a **checkpoint, not an exhaustive certification**: every character, mode,
mod dataset, repeated battle sequence and long-duration memory behavior still need
broader regression coverage before a final release claim.

Test artifact SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`  
Source checkpoint: `0e17b0bac33c47698b414b67a839c839f0e555ce`
<!-- DBTB_00_23_DETAIL:END -->


This is the current handoff. It describes implementation and evidence separately.
Historical audit/test pages remain useful for their pinned APK/builds; their old
pending statements do not override this page. 00.21 and 00.22 failures are historical checkpoints.
The latest 00.24 user confirmation also resolves the distinct native Ogg memory crash reported after the 00.23 LiveArea repack.

## Historical 00.23 build identity

| Field | Value |
|---|---|
| Hardware-tested VPK | `DBTapBattle-Vita-00.23-battle-memory-test.vpk` |
| Version / title ID | `00.23` / `DBTB00001` |
| Source checkpoint | `0e17b0bac33c47698b414b67a839c839f0e555ce` |
| Battle-memory repair | original streaming `GameData.Init` + native-backed `InputStream` |
| Retained selection-mask repair | original masks 187/251 accepted |
| Retained audio repair | worker startup + clean voice output in reported hardware path |
| VPK SHA-256 | `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd` |
| Toolchain family | VitaSDK 2026.08, GCC 15.2.0, hard-float |
| Private generator | dex2jar 2.4, ECJ 3.37.0, TeaVM 0.12.3, Java 17 |
| Hardware result | startup/menu/text/audio/selection/battle path passed; no error observed in this session |

[00.23 hardware evidence](evidence/vita_hardware_full_game_00.23.json) records the tested artifact and scope.
The package used for this hardware checkpoint was an interactive test build; see [BUILD](BUILD.md) for the split-compilation caveat before treating it as a release-quality performance artifact.

The current presentation test is `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk`,
SHA-256 `19fae90627b1ddf4f42902ec228b0c50d992cb7c3cce6d3fbb6d8a8843d9edee`. It uses the exact hardware-tested 00.23
executable/SFO and retains every original package entry byte-for-byte, adding
only the five LiveArea files. `pic0.png` now has the required 256-entry palette
without changing any decoded pixels; the minimal MetalSyntax a1 gate uses XML
content revision 2. This package passes the toolkit and strengthened local
validators; the user subsequently confirmed its LiveArea works on real hardware.

The previous `DBTapBattle-Vita-00.23-LiveArea-Final.vpk` is **rejected**: its
`eboot.bin` matches the non-playable CI native link probe (run `37387861902`),
not the hardware-tested full engine. The user reported installation and
presentation failures. The supplied artwork also had a separate 192-entry
splash palette mismatch; its exact causal role in the installer failure is
unconfirmed because no VitaShell error code was supplied. The historical
LiveArea-only package/evidence remains a record of that earlier attempt, not
acceptance of the Final VPK.

See [corrected package evidence](evidence/vita_livearea_fixed_00.23.json) and
[the corrected device test](TEST_VITA_00_23_LIVEAREA_FIXED.md).

## Implementation versus observation

| Area | Current implementation | Verified scope / open limit |
|---|---|---|
| Core | Original APK Init/Run/task/combat code compiled privately to C; Android service adapters | Real menu/front touch/selection/combat on earlier Vita builds; not every mode/action certified |
| Profiles | Original/mod selector; file-level override/fallback; per-file codec | Earlier Vita profile selection and Android14 battle confirmed; arbitrary mods untested |
| Cards/startup | Cooperative EventQueue progresses one ready event after present; local-data checks | User reports fixed in 00.16 |
| Battle frame rate | Reused GLES client buffers, reduced adapter work; 960×544 | 00.18 steady battle windows 59.9 FPS, user reports stable 60; not every later build validated |
| Text | Memory-based PVF with fallback; glyph metrics/cache, image rectangles, visible-only rasterization, dirty uploads | Text recovery confirmed by user in 00.19; exhaustive script/font/layout fidelity untested |
| PAC I/O | Original exclusion filter plus 00.23 native-backed streaming InputStream; bounded native cache/stream handles | 187/251 masks and stream ownership pass host probes; physical selection and battle startup pass in 00.23 |
| Imported textures | Immutable byte/mode keyed cache; live-reference tracking and idle eviction | Native PNG/ownership host probes pass with GL mocked; hardware reuse/performance pending |
| Voice samples | Original PCM16 mono 22050 Hz or decoded Community14 wrapper; 3 voice channels | Format/host decoding verified; later physical tests report clean voices/audio |
| Voice output | 16-tap/256-phase Q14 reconstruction to 48000 Hz, peak limiter, PCM cache | Clean audible result reported on the physical 00.22/00.23 path; broader character/phrase matrix remains open |
| Audio startup | Restored `0x10000100`; exact open/create/start diagnostics, failure cleanup/latch | 00.21 worker/menu recovery confirmed and no audio regression reported in 00.23 |
| Saves | Active dataset's `save.bin`, max 12906 bytes; cached reads and temp/fsync/rename writes | Host ownership tests; full Android round-trip/mod progression matrix pending |
| Input | Stable slots mapped from Vita touch IDs; original coordinate transform and Controller | Touch gameplay confirmed; physical buttons serve selector, are neutral during game |
| Online / Bluetooth | Offline installed-data boundary; HTTP rejected; Bluetooth disconnected | Current local single-player path; multiplayer/billing/remote downloads unsupported |

Cache budgets are 8 MiB retained PAC-vector capacity, 4 MiB source+RGBA texture
cost and 2 MiB voice-input+PCM cost. These limits are **not** total process-memory
caps: live shared owners, Java copies, allocator/driver overhead and uncached
resources also use memory. First loads and eviction reloads still do work.

Clock requests are CPU 444, bus 166, GPU 222, crossbar 166 MHz. The 00.18 device
log reports effective bus 222; log API results/effective values instead of
assuming the requested value is the applied value. No higher clock profile is
established here.

## Latest observations and next work

1. **Confirm the LiveArea repack on hardware.** Install the corrected `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk` and verify VitaShell promotion, bubble icon, Shenlong background, launch gate/logo and a short launch/battle regression pass.
2. **Broaden 00.23 regression coverage.** Repeat battles, switch across more characters and revisit evicted resources to confirm the streaming fix under churn rather than only one successful progression.
3. **Retest both supported dataset paths/mod overlays.** Keep original fallback rules and record exact dataset hashes when comparing behavior.
4. **Measure release-quality performance.** The 00.23 hardware test package used split TeaVM compilation with `TCBManajer.c` at `-O0` because of the interactive build runner; create a normal reproducible full-engine package before making final FPS/performance claims for 00.23.
5. **Extend lifecycle/control coverage.** Return-to-menu, repeated launches, suspend/resume, save round-trips and physical Vita control adaptation remain separate work. Multiplayer/Bluetooth synchronization, billing/remote services and arbitrary code mods remain unsupported/unvalidated.

No currently reproduced crash is open in the 00.23 tested path. New failures should be recorded with exact VPK hash, dataset/profile, `runtime.log` and `psp2core` when produced.

## Evidence index

| Evidence | Scope |
|---|---|
| [00.11 battle](evidence/vita_hardware_battle_00.11.json) | Physical Android14 gameplay milestone |
| [00.18 performance](evidence/vita_hardware_performance_00.18.json) | User report, 15 steady battle windows, clocks and remaining regressions |
| [00.19 text/audio](evidence/vita_hardware_text_audio_00.19.json) | Text restored; voices/selection still fail; zero measured clipping in this session |
| [00.20 PAC/DSP build](evidence/vita_pac_voice_build_00.20.json) | Host format, I/O, cache and reconstruction tests |
| [00.20 startup failure](evidence/vita_hardware_audio_startup_00.20.json) | Physical worker setup failure and caught BGM exception |
| [00.21 build](evidence/vita_audio_startup_build_00.21.json) | Complete engine compilation and native setup/DSP/resource tests |
| [00.21 selection failure](evidence/vita_hardware_selection_00.21.json) | Physical worker/menu recovery, then rejected character mask and caught exception |
| [00.22 build](evidence/vita_selection_filter_build_00.22.json) | Before/after mask regression, full corpus and complete ARM artifact checks |
| [00.23 hardware](evidence/vita_hardware_full_game_00.23.json) | Physical Vita: clean audio, responsive selection, battle startup/gameplay pass; no error observed in reported session |
| [00.23 LiveArea package](evidence/vita_livearea_00.23.json) | Exact presentation-only repack: original tested VPK entries unchanged; LiveArea hashes/layout pass CI; physical shell test pending |

[Validation](VALIDATION.md) defines test scope and log interpretation.
[Porting guide](PORTING_GUIDE.md) explains reusable techniques and failures.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.24 (2026-10-05, America/Bogota):** the user
> confirms `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` works on the physical Vita
> after the Android14 battle-start crash. Runtime source `f5672d4d`, VPK SHA-256
> `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
> The original PAC streaming repair remains; Ogg PCM now uses one exact allocation
> instead of transient vector doubling, with cache-only resource reclamation.
> The approved LiveArea is retained. This is a user-confirmed test checkpoint,
> not exhaustive character/profile/mode or long-session certification. Historical
> records keep their original artifact and evidence scope.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
