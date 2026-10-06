# Project Rules

These rules are part of the technical contract of the port.

1. **The original game is the behavioral source of truth.** Reconstruct or adapt its systems; do not replace them with arbitrary hardcoded approximations when the original behavior can be understood.
2. **Keep original data formats whenever practical.** `.pac` and the formats contained inside them should be read directly by the Vita port when possible.
3. **Original data lives outside the VPK.** Runtime game data belongs under `ux0:data/DBTapBattle/game/` and is supplied by the user.
4. **Mods never overwrite original data.** They live under `ux0:data/DBTapBattle/mods/<mod>/` and override files virtually at runtime.
5. **Fallback is mandatory.** When a selected mod does not provide a requested resource, load the original resource from `game/`.
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

## Target stack

- PS Vita / VitaSDK
- Original APK Java core generated privately to C with TeaVM 0.12.3
- Handwritten Java platform adapters and native C/C++ Vita services
- vitaGL for rendering
- Original Tap Battle resource files supplied by the user

## Current compatibility objective

The port should eventually accept ordinary community asset/data mods without requiring a Vita-specific repack whenever those mods only replace formats already understood by the original game.

Current checkpoint: full engine 00.24 is user-confirmed on Vita. Original masks
187/251, PAC streaming, audio-worker fixes and approved LiveArea are retained.
Both managed PAC and native Ogg battle-start memory failures have recorded fixes.
See [CURRENT_STATUS](CURRENT_STATUS.md) and [PORTING_GUIDE](PORTING_GUIDE.md).

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
