# Porting plan — v1.2 integration, 2026-10-09

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

Preserve the original Java core through private TeaVM AOT and adapt Vita
platform boundaries. v1.2 keeps the v1.1 VisualQuality resource/memory baseline.
[Current status](CURRENT_STATUS.md) records evidence; this page sets work order.

| Area | Established work | Remaining validation |
|---|---|---|
| Engine/platform | Private original Init/Run/tasks/combat; GLES/PVF/audio/file/time adapters | Broader mode/lifecycle coverage, preserve task ordering |
| Resource memory | Native PAC streaming, exact Ogg allocation, protected-PAC ownership swap; indexed file-backed AAC/M4A and adaptive C14U texture policy | Re-measure long sessions/profile churn; accepted blur remains |
| Profiles/saves | Unified profiles-v1, no resource borrowing, profile-local stable save.bin and seed | Broader save round-trips; no automatic experimental progress migration |
| Mods/extractors | Audited codecs plus bounded PRIVATE DEX recognition, optional dbtb_codec.json | Additional data/code mods require independent evidence |
| Controls | Combat/hidden pads, swapped L/R, character X, Start resume and Circle accepted over tests | Exact rebuilt stable v1.2 retest and more original menu contexts |
| Selector | English labels; Vita first / Touch only second; remembered profile input choice | Long/Unicode folder names and accessibility remain separate |
| Packaging | Local full-engine v1.2, 01.02 / DBTB01178, complete build/package checks | User physical retest; public release upload by maintainer |

## Immediate work

1. Record the user's stable v1.2 result against its exact VPK hash. The user is
   testing it; no result has been reported yet.
2. Recheck stable profile progress, control-choice persistence, combat and
   character confirmation, available Circle Back and Start pause/resume.
   Dialogues and tutorial remain tactile; dialogue X is retired, not a blocker
   to investigate again without a new explicit request and changed evidence.
3. Preserve the previous heavy dbz_mobile_v9 successive-fight and Invasion
   Saitama -> Freezer regression paths; broaden character/profile/long-session
   coverage without assuming every mod is certified.
4. Check Shop return, repeated launches, save round-trips and suspend/resume.
5. Prepare publication using the exact compiled file and English
   [v1.2 release notes](RELEASE_v1.2.md); compiled does not mean newly
   hardware-tested or already published.

## Subsequent work

- Improve loading or heavy-texture quality only at evidenced adapter boundaries;
  retain approved ownership/memory repairs and source PAC bytes. No current
  generic async prefetch/loading feature is claimed.
- Audit new code-mod mechanics/task/pad layouts independently; compatible
  extraction is not execution of modified Android DEX.
- Continue profile/save isolation. Do not silently move experimental progress
  over the stable save or borrow another profile's resources.
- Optional remembered profile selection, Unicode metadata and log rotation are
  distinct from the implemented remembered **control-mode** choice.
- Network/Bluetooth synchronization remains unsupported and requires its own
  packet/timing study. Billing and obsolete remote catalogs are offline boundaries.

## Future port reuse

[PORTING_GUIDE](PORTING_GUIDE.md) explains the reusable method. Reuse verified
platform contracts, not this game's constants, memory budgets or task IDs
without checking the next game's source. Journals retain dated experiments.

<!-- DBTB_00_23_DETAIL:START -->
## Milestone update — 00.23 battle-start blocker closed for the reproduced path

The immediate blocker carried from 00.22 (managed OOM while starting a fight) is now
closed by the streaming PAC bridge and a successful physical-Vita retest. The next
phase is regression breadth rather than another rewrite: repeated battles, more
characters, both supported datasets/mod paths, longer sessions, control adaptation
and release-quality performance/build reproducibility.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
