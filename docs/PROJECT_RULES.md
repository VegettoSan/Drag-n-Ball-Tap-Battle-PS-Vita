# Project Rules

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

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

17. **Public data extractors must stay contract-equivalent.** Web Extractor 1.0
    and Windows Extractor 1.5 both target `profiles-v1`, derive visible profile
    names from APK filenames, preserve payload bytes, exclude APK-local
    `save.bin`, and apply only audited protected aliases. The web extractor must
    remain local-only: no APK upload or network-dependent extraction.

18. **GitHub Pages must deploy only validated static assets.** The Pages workflow
    syntax-checks/tests the web core and reconstructs the approved selector PNGs
    from the repository's validated source payload before deployment.

19. **Manual publication keeps private inputs ephemeral.** Explicitly dispatched full-engine release/prerelease workflows may consume the pinned original APK and generate JAR/classes/C in a temporary runner directory outside Git. Never commit/cache/upload those inputs, generated sources or private logs. Publish only validated compiled VPK/ELF/VELF and provenance/checksum manifests. Automatic validation/native-smoke jobs remain public-source-only. Publishing a new binary is not a hardware-test result.
20. **Large transformed resources must finish with ownership transfer, not a hidden copy.** After building a multi-MiB normalized PAC, hand its buffer to the final owner with an explicit `swap`/verified move path. Do not use ambiguous conditional vector assignment. 00.32's Invasion coredump proved that an extra ~4.6 MiB copy can fail after battle-memory fragmentation even when cache budgets are respected.

## Target stack

- PS Vita / VitaSDK
- Original APK Java core generated privately to C with TeaVM 0.12.3
- Handwritten Java platform adapters and native C/C++ Vita services
- vitaGL for rendering
- Original Tap Battle resource files supplied by the user

## Current compatibility objective

The port should eventually accept ordinary community asset/data mods without requiring a Vita-specific repack whenever those mods only replace formats already understood by the original game.

Current prepared release: **v1.2**, Vita APP_VER `01.02`, TITLE_ID `DBTB01178`.
It retains v1.1's VisualQuality loader/memory policy and integrates the user's
approved Vita controls. Original task/combat/controller code stays unchanged.
Dialogue X failed on hardware and is retired; dialogues remain tactile.
Historical releases/test identities remain evidence, not the current build
contract. See [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md) and
[CURRENT_STATUS](CURRENT_STATUS.md).

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
