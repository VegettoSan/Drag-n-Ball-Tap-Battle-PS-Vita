# Project Rules

These rules are part of the technical contract of the port.

1. **The original game is the behavioral source of truth.** Reconstruct or adapt its systems; do not replace them with arbitrary hardcoded approximations when the original behavior can be understood.
2. **Keep original data formats whenever practical.** `.pac` and the formats contained inside them should be read directly by the Vita port when possible.
3. **Every installed APK data set is an independent first-level profile.** Current runtime root is `ux0:data/DBTapBattle/profiles/<Profile>/`; there is no special `game/` root and no separate `mods/` root.
4. **Profiles never overwrite or borrow gameplay resources or mutable saves from one another.** Selecting a profile resolves PAC/audio/data and its writable `save.bin` only from that same `profiles/<Profile>/` directory. The VPK's `app0:/save.bin` is only a first-use seed.
5. **No silent cross-profile fallback.** A missing resource in a selected profile is a compatibility error to diagnose. Do not hide missing Vita behavior by borrowing a file from another profile; adapt the port to the selected APK contract with evidence.
6. **No silent destructive conversion.** If a format must be converted for Vita, document the limitation, source format, output format and reproducible conversion process.
7. **Small, reviewable commits.** Keep changes focused so working states can be recovered easily.
8. **Record experiments.** Every meaningful test belongs in `ATTEMPTS.md`; confirmed wins in `SUCCESSES.md`; dead ends in `FAILURES.md`.
9. **Do not repeat a recorded failure unchanged.** A retry must identify what changed and why the new attempt is meaningfully different.
10. **Separate fact from hypothesis.** Reverse-engineered claims must note whether they are confirmed by the original APK, confirmed on Vita, sourced from community research, or still speculative.
11. **Prefer original assets over recreated assets.** The objective is fidelity and mod compatibility.
12. **Keep legal game content out of Git.** This repository should not distribute copyrighted original game assets.

13. **Preserve the original core through the chosen private AOT pipeline.** Adapt Android service boundaries; do not replace original task/combat/draw behavior with guessed equivalents. Generated commercial C/JAR/classes stay outside Git.
14. **Maintain current and historical documentation separately.** Read CURRENT_STATUS.md before changing systems; update affected contracts and evidence. Historical test outcomes keep their build identity and are not rewritten as later successes.
15. **Validate ownership and failure paths.** Budgets are cache policies, not total-memory guarantees. Host mocks and native CI do not establish Vita scheduling, GPU fidelity or audible quality. Record exact platform errors and clean up only owned handles.

16. **Keep both battle-memory repairs.** Preserve native-backed streaming `GameData.Init`; never bridge a whole PAC into Java. Decode seekable Ogg using the exact bounded frame count, never incremental PCM vector doubling. Test PCM byte equality, transient allocation peaks and active-owner survival after cache reclamation.

17. **Manual publication keeps private inputs ephemeral.** Explicitly dispatched full-engine release/prerelease workflows may consume the pinned original APK and generate JAR/classes/C in a temporary runner directory outside Git. Never commit/cache/upload those inputs, generated sources or private logs. Publish only validated compiled VPK/ELF/VELF and provenance/checksum manifests. Automatic validation/native-smoke jobs remain public-source-only. Publishing a new binary is not a hardware-test result.
18. **Large transformed resources must finish with ownership transfer, not a hidden copy.** After building a multi-MiB normalized PAC, hand its buffer to the final owner with an explicit `swap`/verified move path. Do not use ambiguous conditional vector assignment. 00.32's Invasion coredump proved that an extra ~4.6 MiB copy can fail after battle-memory fragmentation even when cache budgets are respected.

## Target stack

- PS Vita / VitaSDK
- Original APK Java core generated privately to C with TeaVM 0.12.3
- Handwritten Java platform adapters and native C/C++ Vita services
- vitaGL for rendering
- Original Tap Battle resource files supplied by the user

## Current compatibility objective

The port should eventually accept ordinary community asset/data mods without requiring a Vita-specific repack whenever those mods only replace formats already understood by the original game.

Current public release: **v1.0** with Vita APP_VER `01.00` and TITLE_ID
`DBTB01178`. Its gameplay/runtime baseline is full engine **00.34**, which is
user-confirmed stable and functional on Vita for the exercised
selector/profile/gameplay paths. It retains the 00.33 protected-PAC fix and the
unified `profiles/` contract plus the final selector UX corrections. Loading recovery and dynamic installed
rosters are also hardware-confirmed in the recent test sequence. Original masks
187/251, PAC streaming, direct audio, independent saves and approved LiveArea are
retained. The 00.33 protected-PAC ownership fix must remain allocation-free.
See [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md) and [CURRENT_STATUS](CURRENT_STATUS.md) and [PORTING_GUIDE](PORTING_GUIDE.md).

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current release — v1.0 (2026-10-07):** APP_VER `01.00`, TITLE_ID
> `DBTB01178`. Gameplay/runtime is inherited from the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk`, which is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
