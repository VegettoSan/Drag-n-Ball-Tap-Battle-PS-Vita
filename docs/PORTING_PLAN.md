# Porting Plan

This plan intentionally grows from verifiable platform/data milestones into gameplay reconstruction. Do not skip ahead by hardcoding visual/gameplay results that the original engine already defines.

## Phase 0 — Bootstrap and data plumbing

- VitaSDK + vitaGL project builds a VPK.
- Runtime directories are created under `ux0:data/DBTapBattle/`.
- Original/mod boot selector works.
- Virtual filesystem resolves active-mod overrides then original fallback.
- Original `common.pac` is parsed without conversion.
- Runtime log is written for test evidence.

**Exit condition:** BUILD CONFIRMED and at least one untouched original PAC parsed on Vita/Vita3K.

## Phase 1 — Original resource decoding

Reverse engineer and implement only what the game needs, in dependency order:

1. PNG/BMP texture payloads from PAC.
2. `spr` sprite/layout data.
3. `act` animation/action data.
4. `cnv`, `dac`, `gdt`, `bin`, `dat`, `plt`, `db` as required by real code paths.
5. Original font data/UI assets.

Each format receives its own document and validation fixture/hash notes.

**Exit condition:** render an untouched original menu/background resource according to its original metadata.

## Phase 2 — Android platform abstraction replacement

Map original Android-facing systems to Vita equivalents:

- OpenGL ES 1.x style drawing -> vitaGL.
- Touch input -> Vita front touch, with physical-button mappings layered on top.
- Audio/SoundPool/MediaPlayer behavior -> Vita audio implementation.
- Android file/resource APIs -> `GameVfs` and native Vita I/O.
- Timers/frame pacing -> Vita timing APIs.
- Save/config handling -> dedicated writable save path.

**Exit condition:** platform services are available without changing game semantics.

## Phase 3 — Core engine/game-state reconstruction

Use the original APK/decompiled logic plus community reverse-engineering as references, with the APK as behavioral source of truth.

Priority systems:

- Main state machine / `TCBManajer` equivalents.
- `Game1...Game17` flows as applicable.
- `DrawSprite`, panel/UI creation and transitions.
- Character/resource loading.
- Input/touch command buffers.
- Battle initialization and active battle state.

**Exit condition:** original menu -> character select -> battle transition works with original data.

## Phase 4 — Battle fidelity

- Player movement/position/state.
- Attack/guard/special systems.
- CPU behavior and difficulty.
- Cards and player loadout data.
- HUD, effects and results.
- Training mode.

**Exit condition:** a complete original battle can be played and completed.

## Phase 5 — Audio, saves and secondary modes

- BGM and sound effects.
- Save/config compatibility strategy.
- Ranking/data/card screens.
- Versus/local behavior where practical.
- Original downloaded-content behavior replaced by local-data checks rather than dead servers.

## Phase 6 — Mod compatibility

Test real community mods in categories:

1. Asset-only replacements.
2. Character/card/data additions using standard Tap Battle formats.
3. Mods that alter executable/Dalvik logic.
4. Protected/private mods.

The first two categories are the primary compatibility target. Code-modified APKs require explicit compatibility work rather than attempting to execute their Android `classes.dex` on Vita.

## Phase 7 — Packaging and user experience

- Friendly missing-data diagnostics.
- Mod metadata (`mod.json`) and preview support.
- Remember last selected dataset optionally.
- Stable VPK/ZIP release packaging without copyrighted game data.
- Reproducible GitHub Actions build once the local toolchain is confirmed.

## External reverse-engineering reference

A useful community reference exists at `GdGohan/Dragon-Ball-Tap-Battle-Decompilation`, including documentation of `TCBManajer`, game states, resource loading, touch/button systems and several important arrays/variables. Treat it as a reference, not as a replacement for validation against the user's original APK.
