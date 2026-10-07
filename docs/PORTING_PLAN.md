# Porting plan — v1.0 / 00.34 hardware baseline, 2026-10-07

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
| Performance | 00.18 steady battle 60 FPS; 00.33 repeated Invasion fight transitions survive the prior native allocation crash | Re-measure FPS/memory over longer latest-build sessions and more profiles |
| Text | 00.19 image-rectangle fix restores visible text | Size/layout/script and lifecycle matrix |
| Audio | Decode/three channels/limiter/reconstruction; 00.21 setup repair | Clean audio/voices reported in the current hardware path; broaden character/phrase and long-session coverage |
| Saves/mods | Independent per-profile seeded save, dynamic installed roster 00..99, standalone resource isolation | Broader save semantics and code-mod compatibility matrix |
| Product/distribution | v1.0 public package identity; 00.34 hardware-confirmed runtime, small main commits, manual release/prerelease automation | Broader compatibility, install/launch sanity on DBTB01178 and release-performance validation |

## Immediate physical checks

1. Keep the now-passing Invasion repeated-fight/Saitama->Freezer path in every future regression pass.
2. Exercise additional characters and profiles, especially large protected PACs, while watching cold/repeated/evicted resource loads.
3. Reconfirm clean voices/text/FPS during longer runs; the 00.33 success closes the reproduced crash but is not exhaustive coverage.
4. Explicitly test Shop return behavior, return-to-menu, repeated launches, save round-trips and suspend/resume.
5. Continue distinguishing every rebuilt VPK/ELF hash from the pinned 00.33 hardware-tested artifact.

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
> **Current public release — v1.0 / DBTB01178 (2026-10-07):** the 00.34
> gameplay/runtime baseline is hardware-confirmed stable for the tested paths.
> Earlier 00.33 hardware evidence remains valid for the protected-PAC repeated-fight
> repair, Loading recovery and dynamic rosters. Historical artifact identities are
> preserved; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
