# Porting plan — checkpoint 00.21, 2026-10-05

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
| Audio | Decode/three channels/limiter/reconstruction; 00.21 setup repair | Physical startup recovery and audible clean voices |
| Saves/mods | Profile-local save plus file overlay/per-file codec | Android save round-trip and real asset/code-mod compatibility matrix |
| Product/distribution | Verified private test VPK and evidence, small main commits | Public notice/relink review; reliable latest-build device test |

## Immediate physical checks

1. Test 00.21 Original startup. If setup still fails, use its exact
   open/create/start error; do not assume the old combined log proved a syscall
   or kernel priority range.
2. Compare first character visit with immediate and evicted revisits in the same
   profile. Correlate I/O/cache/decode/upload counters and elapsed frame timing.
3. Record bad voices by phrase/character in steady playback and switching.
   Separate source distortion, conversion images, mix clipping and delivery gaps.
4. Retest names/descriptions/cards and sustained battle FPS with effects.

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
