# Architecture Decisions

## ADR-001 — Native VitaSDK execution (implementation refined by ADR-011)

**Status:** accepted — 2026-10-04

Use a native C/C++ VitaSDK application rather than attempting to run Android/Dalvik directly on Vita.

**Reason:** the supplied APK has no native game `.so` layer and its game/runtime code is accessible as Java/Dalvik. Native execution gives control over rendering, input, audio and filesystem behavior. The accepted implementation preserves the original Java core through AOT; it does not manually rebuild combat.

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

**Reason:** resource overlays avoid changing base assets. Existing broken overrides report errors instead of falling back. Saves are profile-local writable exceptions (ADR-008); whole PAC files, not individual records, are overridden.

## ADR-006 — Original always selectable

**Status:** accepted — 2026-10-04

The boot selector always exposes `Original` as entry 0 regardless of installed mods.

## ADR-007 — Experiment memory is mandatory

**Status:** accepted — 2026-10-04

Meaningful experiments must be recorded in `ATTEMPTS.md`; validated successes and failed approaches are promoted into their dedicated logs.

**Reason:** repeated dead ends cost more time than maintaining concise engineering records.

## ADR-008 — Profile-local saves and original input service

**Status:** updated to implemented contract — 2026-10-05.
The early separate saves/ proposal is superseded: Original uses game/save.bin;
a selected mod uses mods/<Profile>/save.bin. Only save.bin is writable through
the resource adapter. No save fallback/migration crosses profiles. Exclusive
temp creation, complete write/fsync/close/rename and session cache prevent partial
publication; preserve backups before importing an APK-bundled save.

Front touch feeds stable pointer IDs to original KeyData/Controller. Physical
selector controls remain separate and physical gameplay buttons are neutral;
there is no implemented physical combat mapping. See [PLATFORM_SERVICES](PLATFORM_SERVICES.md).
Android save round-trip and the full progression/profile matrix remain pending.
## ADR-009 — Source facts before inferred internal schemas

**Status:** accepted — 2026-10-04.
Keep raw DAC animation and converted gamedata/text tables as distinct formats.
Outer PAC LE is not a universal endian rule: original CNV rectangle reads use
BE signed 16-bit values. Preserve unknown GDT/BMP/DAT/PLT/DB payloads without
claiming a decoder. Do not impose a 40-FPS game tick from constants unused in
the observed render callback. Original state dispatch overrides community wiki
labels. Save compatibility requires a real original save fixture before approval.

## ADR-010 — Pinned community codec and canonical import names

**Status:** accepted — 2026-10-04. Support the supplied Android14 APK alongside
the original with a verified metadata/image profile, not an Android .so loader.
Normalize confirmed resource aliases during PC import and preserve every data
byte/hash. Decode per file at runtime; keep the existing mod/original fallback.
Carry premultiplied-alpha state explicitly rather than double-multiplying RGB.
Do not import arbitrary SWB changes or infer changed game mechanics from shared
helpers. New constants/code need a fresh audit. Full native gameplay has since been demonstrated on earlier Vita builds; resource encoding alone still does not establish compatibility with code-modified APK rules.


## ADR-011 — Preserve the original core through private AOT

**Status:** accepted and implemented — 2026-10-05.
Use original APK bytecode, dex2jar, handwritten platform adapters and TeaVM C
rather than a new combat/state interpreter. Init/Run/Dispose, Game1–17, tasks,
GameData, Controller and Graphics2D stay original. Commit adapters/tools/evidence,
not APK-derived Java/JAR/C/assets. The full target is tools/aot/engine/vita;
root CMake and dummy-import CI are separate bootstrap/smoke targets.

**Consequence:** changes to Java/native imports require regeneration, and private
inputs/tool versions must be pinned. [BUILD](BUILD.md) provides the recipe.

## ADR-012 — Preserve lifecycle and cooperative progress

**Status:** implemented; exhaustive device lifecycle matrix pending.
Set the first active bResume edge and pump one ready TeaVM EventQueue event after
presentation. This restores original text initialization and queued card work.
Do not simulate completed jobs or add a second concurrent core Run loop.

## ADR-013 — Optimize resource ownership, not game semantics

**Status:** implemented in 00.19/00.20; latest selection latency pending.
Keep original exclusion-mask polarity, directory indices, selected payload bytes
and action order. Cache normalized PAC results (8 MiB), immutable textures
(4 MiB) and decoded voice sources (2 MiB) with identity/ownership checks.
These are retained-cache budgets, not process memory caps. Mutable text/FBO
surfaces stay outside immutable texture reuse. Restore PVF image rectangles for
visible glyphs; metric caching alone cannot supply their raster coverage.

## ADR-014 — Audio failure must retain its exact cause

**Status:** 00.21 implemented and host-tested; device recovery pending.
Restore previously working encoded worker priority 0x10000100; log output-port,
thread-create and thread-start errors separately with native return codes.
Dispose only owned handles, once; latch failed setup until disposal resets it.
00.20's log establishes a worker setup failure, not which syscall failed.
Keep character-voice sinc reconstruction separate from BGM/SE conversion and
from output scheduling. Synthetic spectral/limiter tests do not prove clean
voices or uninterrupted playback on Vita.

## ADR-015 — Versioned evidence and reusable documentation

**Status:** accepted — 2026-10-05.
[CURRENT_STATUS](CURRENT_STATUS.md) states the current source/artifact and open
hardware checks. Test sheets and journals retain the build they describe;
observed user results, host mocks and hypotheses are labeled separately.
[PORTING_GUIDE](PORTING_GUIDE.md) generalizes the method and failure lessons.
Do not turn a historical pending item into a present blocker, or a later fix
into retroactive proof for an older artifact. Docs-only commits do not rebuild
or change the delivered 00.21 executable.
