# Current status — 2026-10-05, full engine 00.21

This is the current handoff. It describes implementation and evidence separately.
Historical audit/test pages remain useful for their pinned APK/builds; their old
pending statements do not override this page. No 00.21 physical test result has
been received at this checkpoint.

## Delivered build identity

| Field | Value |
|---|---|
| Full-engine test VPK | `DBTapBattle-Vita-00.21-audio-startup-fix.vpk` |
| Version / title ID | `00.21` / `DBTB00001` |
| Source embedded at configure/build time | `07222bb42f20ab2bac953531e42b8cf3796940ca` |
| Audio correction | `2e71d519939e9bf34b9f07b14ae58f0ecd14bf39` |
| Bytes | 2,235,382 |
| VPK SHA-256 | `ec551654abc68dfc5494c4f6b22ca620727c8fb898ff76b1682bbf141554bc47` |
| Toolchain | VitaSDK 2026.08, GCC 15.2.0, hard-float |
| Private generator | dex2jar 2.4, ECJ 3.37.0, TeaVM 0.12.3, Java 17 |
| Artifact verification | ZIP CRC, SFO version/title, eboot equality, original-engine/source markers pass |

[Build evidence](evidence/vita_audio_startup_build_00.21.json) records the exact
artifact. Documentation changes after the source commit do not modify that VPK.
The symbols ZIP contains ELF/VELF and evidence; it is diagnostic, not installable
and not a complete object-file relink kit. Full commercial engine artifacts are
private test deliverables; public native smoke artifacts have a different scope.

## Implementation versus observation

| Area | Current implementation | Verified scope / open limit |
|---|---|---|
| Core | Original APK Init/Run/task/combat code compiled privately to C; Android service adapters | Real menu/front touch/selection/combat on earlier Vita builds; not every mode/action certified |
| Profiles | Original/mod selector; file-level override/fallback; per-file codec | Earlier Vita profile selection and Android14 battle confirmed; arbitrary mods untested |
| Cards/startup | Cooperative EventQueue progresses one ready event after present; local-data checks | User reports fixed in 00.16 |
| Battle frame rate | Reused GLES client buffers, reduced adapter work; 960×544 | 00.18 steady battle windows 59.9 FPS, user reports stable 60; not every later build validated |
| Text | Memory-based PVF with fallback; glyph metrics/cache, image rectangles, visible-only rasterization, dirty uploads | Text recovery confirmed by user in 00.19; exhaustive script/font/layout fidelity untested |
| PAC I/O | Original exclusion filter applied before payload reads; in-memory normalization; bounded result cache | 26 character PAC host tests; filter 33 reads 87.146% fewer source payload bytes; real switching latency pending |
| Imported textures | Immutable byte/mode keyed cache; live-reference tracking and idle eviction | Native PNG/ownership host probes pass with GL mocked; hardware reuse/performance pending |
| Voice samples | Original PCM16 mono 22050 Hz or decoded Community14 wrapper; 3 voice channels | Format/host decoding verified; voices still bad in 00.19 |
| Voice output | 16-tap/256-phase Q14 reconstruction to 48000 Hz, peak limiter, PCM cache | Host spectral image reduced 32.3 dB; audible improvement/cost on Vita pending |
| Audio startup | Restored `0x10000100`; exact open/create/start diagnostics, failure cleanup/latch | 00.20 worker failed repeatedly and game exited; 00.21 simulated failure tests pass, physical recovery pending |
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

1. **Retest 00.21 startup.** 00.20 opened the audio port 20 times, failed worker
   setup 20 times and exited at frame 570 with `BGM load failed: bgm_16`.
   Its log cannot identify create versus start or the exact kernel error.
   The priority change is the leading regression, not a proven universal
   kernel priority-range rule. 00.21 restores the working value.
2. **Then measure selection latency.** Compare first visits, immediate revisits,
   evicted entries and each dataset. Host byte reduction is not measured Vita
   time reduction; asynchronous prefetch has not been added.
3. **Assess voices audibly.** Get character/phrase and a recording during steady
   playback and switching. Source rail samples, summed clipping, reconstruction
   images and delivery gaps are different phenomena; zero counters do not prove
   clean output.
4. **Recheck text and battles on 00.21.** Preserve confirmed 00.19 text behavior
   and the earlier 60 FPS work while validating the new native audio code.
5. Extend mode/save/mod/control coverage after the above; review distribution
   notices and relink deliverables before a public full-engine release.

## Evidence index

| Evidence | Scope |
|---|---|
| [00.11 battle](evidence/vita_hardware_battle_00.11.json) | Physical Android14 gameplay milestone |
| [00.18 performance](evidence/vita_hardware_performance_00.18.json) | User report, 15 steady battle windows, clocks and remaining regressions |
| [00.19 text/audio](evidence/vita_hardware_text_audio_00.19.json) | Text restored; voices/selection still fail; zero measured clipping in this session |
| [00.20 PAC/DSP build](evidence/vita_pac_voice_build_00.20.json) | Host format, I/O, cache and reconstruction tests |
| [00.20 startup failure](evidence/vita_hardware_audio_startup_00.20.json) | Physical worker setup failure and caught BGM exception |
| [00.21 build](evidence/vita_audio_startup_build_00.21.json) | Complete engine compilation and native setup/DSP/resource tests |

[Validation](VALIDATION.md) defines test scope and log interpretation.
[Porting guide](PORTING_GUIDE.md) explains reusable techniques and failures.
