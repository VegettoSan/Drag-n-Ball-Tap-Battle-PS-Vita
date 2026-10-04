# Architecture Decisions

## ADR-001 — Native VitaSDK reconstruction

**Status:** accepted — 2026-10-04

Use a native C/C++ VitaSDK application rather than attempting to run Android/Dalvik directly on Vita.

**Reason:** the supplied APK has no native game `.so` layer and its game/runtime code is accessible as Java/Dalvik. A native reconstruction gives deterministic control over rendering, input, audio and filesystem behavior.

## ADR-002 — vitaGL renderer

**Status:** accepted — 2026-10-04

Use vitaGL as the rendering compatibility layer and recreate the original OpenGL ES 1.x-oriented drawing behavior as faithfully as practical.

**Reason:** Tap Battle's renderer is conceptually close to fixed-function OpenGL ES, making vitaGL a natural bridge while avoiding a full renderer redesign.

## ADR-003 — Preserve original PAC files

**Status:** accepted — 2026-10-04

Read original `.pac` files directly at runtime.

**Reason:** the outer container has already been validated and is simple enough to parse natively. Preserving it improves fidelity, debugging and mod compatibility.

## ADR-004 — External original data

**Status:** accepted — 2026-10-04

Original copyrighted resources are not stored in the VPK/repository. They live under:

```text
ux0:data/DBTapBattle/game/
```

A PC-side extractor prepares this directory from a user-owned APK.

## ADR-005 — Non-destructive mod overlay

**Status:** accepted — 2026-10-04

Mods live under:

```text
ux0:data/DBTapBattle/mods/<mod>/
```

The virtual filesystem checks the active mod first and falls back to `game/` for missing resources.

**Reason:** mods remain small, original data stays pristine and users can switch mods without reinstalling the game.

## ADR-006 — Original always selectable

**Status:** accepted — 2026-10-04

The boot selector always exposes `Original` as entry 0 regardless of installed mods.

## ADR-007 — Experiment memory is mandatory

**Status:** accepted — 2026-10-04

Meaningful experiments must be recorded in `ATTEMPTS.md`; validated successes and failed approaches are promoted into their dedicated logs.

**Reason:** repeated dead ends cost more time than maintaining concise engineering records.

## ADR-008 — Independent writable namespaces and input service

**Status:** accepted — 2026-10-04.
Use config/, logs/ and saves/ alongside immutable game/ and mod overlays.
Input adapters produce neutral menu commands and stable pointer events; original
Controller/KeyData gameplay semantics will consume them, rather than reading
SceCtrl inside combat. Touch/physical boot selector is implemented; combat
mapping is a proposal until original command interpretation is reconstructed.
