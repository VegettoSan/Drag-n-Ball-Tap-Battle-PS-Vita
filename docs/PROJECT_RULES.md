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

## Target stack

- PS Vita / VitaSDK
- C/C++
- vitaGL for rendering
- Original Tap Battle resource files supplied by the user

## Current compatibility objective

The port should eventually accept ordinary community asset/data mods without requiring a Vita-specific repack whenever those mods only replace formats already understood by the original game.
