# Current status — 2026-10-05, full engine 00.22

<!-- DBTB_00_23_DETAIL:START -->
## Authoritative hardware checkpoint — 00.23 (2026-10-05)

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
pending statements do not override this page. The 00.21 device test now confirms worker/menu recovery, but selection rejects
char00 and exits. 00.22 fixes that mask contract; its physical result is pending.

## Delivered build identity

| Field | Value |
|---|---|
| Full-engine test VPK | `DBTapBattle-Vita-00.22-selection-filter-fix.vpk` |
| Version / title ID | `00.22` / `DBTB00001` |
| Source embedded at configure/build time | `c40ce0a96a0effb129fe7dd2970a44ec6c3a7257` |
| Selection mask correction | `c40ce0a96a0effb129fe7dd2970a44ec6c3a7257` |
| Retained audio startup correction | `2e71d519939e9bf34b9f07b14ae58f0ecd14bf39` |
| Bytes | 2,235,731 |
| VPK SHA-256 | `96796359179cd99253e16d5ade6e4f33dd59ffcbd5b486d052c4325c3e99ec47` |
| Toolchain | VitaSDK 2026.08, GCC 15.2.0, hard-float |
| Private generator | dex2jar 2.4, ECJ 3.37.0, TeaVM 0.12.3, Java 17 |
| Artifact verification | ZIP CRC, SFO version/title, eboot equality, original-engine/source markers pass |

[Build evidence](evidence/vita_selection_filter_build_00.22.json) records the exact
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
| PAC I/O | Original exclusion filter applied before payload reads; in-memory normalization; bounded result cache | 26 character PACs with original 187/251 and signed/high-bit masks pass host tests; range rejection fixed; real switching latency pending |
| Imported textures | Immutable byte/mode keyed cache; live-reference tracking and idle eviction | Native PNG/ownership host probes pass with GL mocked; hardware reuse/performance pending |
| Voice samples | Original PCM16 mono 22050 Hz or decoded Community14 wrapper; 3 voice channels | Format/host decoding verified; voices still bad in 00.19 |
| Voice output | 16-tap/256-phase Q14 reconstruction to 48000 Hz, peak limiter, PCM cache | Host spectral image reduced 32.3 dB; audible improvement/cost on Vita pending |
| Audio startup | Restored `0x10000100`; exact open/create/start diagnostics, failure cleanup/latch | 00.20 setup fails; 00.21 worker/menu recovery physically confirmed; audible voice quality remains open |
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

1. **Retest 00.22 character selection.** The 00.21 worker starts and menu works.
   Original then rejects char00 with invalid GameData filter and exits at frame
   1273, md=1018, caught NullPointerException. Original Game3 uses 187, other
   paths 251; the native 0..127 enum-style guard is wrong. 00.22 removes it,
   retaining the exact mask/type tests and original parser/tasks. The new
   regression fails on the previous code and passes on both PAC formats.
2. **Then measure selection latency.** Compare first visits, immediate revisits,
   evicted entries and each dataset. Host byte reduction is not measured Vita
   time reduction; asynchronous prefetch has not been added.
3. **Assess voices audibly.** Get character/phrase and a recording during steady
   playback and switching. Source rail samples, summed clipping, reconstruction
   images and delivery gaps are different phenomena; zero counters do not prove
   clean output.
4. **Recheck text and battles on 00.22.** Preserve confirmed 00.19 text behavior
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
| [00.21 selection failure](evidence/vita_hardware_selection_00.21.json) | Physical worker/menu recovery, then rejected character mask and caught exception |
| [00.22 build](evidence/vita_selection_filter_build_00.22.json) | Before/after mask regression, full corpus and complete ARM artifact checks |

[Validation](VALIDATION.md) defines test scope and log interpretation.
[Porting guide](PORTING_GUIDE.md) explains reusable techniques and failures.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
