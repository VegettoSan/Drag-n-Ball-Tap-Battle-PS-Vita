# Porting plan — checkpoint 00.23, 2026-10-05

The original plan began with an atlas preview. The chosen implementation now
preserves the original Java engine through private TeaVM AOT and replaces its
Android platform services. Do not restart handwritten combat reconstruction.
[CURRENT_STATUS](CURRENT_STATUS.md) is the evidence matrix; this page is the work
order. Completion below applies only to the stated scope/build.

| Stage | Established work | Remaining exit condition |
|---|---|---|
| Bootstrap/data | Native selector, VFS, original/Community14 containers, byte-preserving import | Maintain path/error/fallback tests |
| Original core | APK-derived Init/Run/tasks/Controller/drawing/AI generated privately | Preserve method boundaries during future adaptations |
| Vita integration | GLES/FBO/PVF/touch/Vorbis/PCM/save/time services; real menu and battle on earlier builds | Exercise every required lifecycle/mode, not only a compile |
| Offline data | Local character/shared completeness, no dependency on dead catalog | Broader missing-data diagnostics and dataset coverage |
| Performance | 00.16 startup/cards fixed; 00.18 steady battle 60 FPS confirmed | Retain results on latest build; cold/repeated resource timing |
| Text | 00.19 image-rectangle fix restores visible text | Size/layout/script and lifecycle matrix |
| Audio | Decode/three channels/limiter/reconstruction; 00.21 setup repair | Clean audio/voices reported in the current hardware path; broaden character/phrase and long-session coverage |
| Saves/mods | Profile-local save plus file overlay/per-file codec | Android save round-trip and real asset/code-mod compatibility matrix |
| Product/distribution | 00.23 hardware-tested VPK/evidence, small main commits | Produce a normal reproducible release-quality 00.23+ package; public notice/relink review |

## Immediate physical checks

1. Repeat battle entry/exit and several consecutive fights to look for retained stream handles, cache churn or heap fragmentation.
2. Exercise multiple characters and both supported dataset/profile paths; compare first, repeated and evicted resource loads.
3. Reconfirm clean voices/text/FPS during those longer runs rather than assuming one successful battle proves all combinations.
4. Test return-to-menu, repeated launches, save round-trips and suspend/resume separately.
5. Build a release-quality package with the normal full-engine compilation recipe and compare its performance against the successful 00.23 test artifact.

## Subsequent work

- Keep the original frame/task order; improve loading only at verified adapter
  boundaries. Prefetch/async loading is a future design requiring ownership,
  scheduling and real-device measurements, not a current implemented feature.
- Validate all supported single-player flows, return-to-menu, repeated launches
  and suspend/resume before claiming complete gameplay fidelity.
- Confirm profile save isolation and backups with real fixtures; document any
  migration rather than silently sharing progress.
- Design physical controls around original gesture/command semantics. In-game
  button neutrality currently prevents accidental Android-Back exits.
- Extend asset mods first; Java/code-mod mechanics and new codecs require
  independent audits. File fallback does not merge PAC entries.
- Optional selector metadata/Unicode, remembered profile and log rotation remain
  secondary to reliable gameplay.
- Bluetooth/network synchronization requires packet/timing analysis. Billing,
  browser and obsolete catalog services are not single-player requirements.
- Before public full-engine release, review upstream notices, commercial-code
  distribution scope and whether a true relink bundle is available.

## Future port reuse

Use [PORTING_GUIDE](PORTING_GUIDE.md) for the workflow and failure lessons. Reuse
validated platform contracts, not this APK's constants, state labels, memory
budgets or formats without checking the next game's actual source. The GdGohan
archive is a comparison reference; the original supplied APK remains behavioral
source of truth. Historical stages/results live in [ATTEMPTS](ATTEMPTS.md).

<!-- DBTB_00_23_DETAIL:START -->
## Milestone update — 00.23 battle-start blocker closed for the reproduced path

The immediate blocker carried from 00.22 (managed OOM while starting a fight) is now
closed by the streaming PAC bridge and a successful physical-Vita retest. The next
phase is regression breadth rather than another rewrite: repeated battles, more
characters, both supported datasets/mod paths, longer sessions, control adaptation
and release-quality performance/build reproducibility.
<!-- DBTB_00_23_DETAIL:END -->

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
