# Original core integration — v1.2

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](../../../docs/CURRENT_RUNTIME_CONTRACT.md) · [Status](../../../docs/CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

This directory contains the handwritten platform layer for the original APK's
Java core. The full private engine can now be generated with TeaVM and built for
PS Vita through `vita/CMakeLists.txt`. Native imports are declared in
`native/dbtb_bridge.h`. Generated game C and APK-derived JAR/classes remain
outside Git; a successful build is not by itself hardware-playability evidence.

## Current checkpoint

Prepared v1.2: APP_VER `01.02`, TITLE_ID `DBTB01178`, stable per-profile
`save.bin`. The v1.1 VisualQuality loader/memory baseline is retained. Approved
physical controls, hidden pads and English launcher are integrated; text remains
tactile. The exact rebuilt stable package is awaiting physical retest.

Fresh local TeaVM generation reaches 468 classes / 4103 methods. Every patched
original JAR entry matches the same-input pre-controls main pipeline; the
Controller/KeyData bytes are original. No original task/combat implementation
is replaced. See [runtime contract](../../../docs/CURRENT_RUNTIME_CONTRACT.md),
[build](../../../docs/BUILD.md), [validation](../../../docs/VALIDATION.md) and
[v1.2 evidence](../../../docs/evidence/vita_release_1.2.json).

## Confirmed generation and Vita build

TeaVM 0.12.3 generates the complete reachable original Init/Run path without
Android framework, HTTP, billing or Bluetooth runtime dependencies. Original
TCBManajer/Game1..17, TCB, ObjReq, Controller, KeyData, GameTimer, Graphics2D,
GameData byte-array decoder and SpriteData remain APK-derived bytecode and are
not copied into Git.

`PatchResourceInit` replaces only the Android resource-loading overload of
GameData with ResourceAdapter and keeps the original byte-array parser. dex2jar
2.4 is not byte-for-byte deterministic for large classes, so the patch validates
the exact class/method/field and Android loader boundaries it depends on rather
than relying on unstable whole-class hashes. The GetString/SetString Shift_JIS
boundaries are also counted and adapted explicitly. Unexpected shapes fail the
private generation step.

A complete private build has been demonstrated with VitaSDK GCC 15.2.0
hard-float: TeaVM C + native services -> ARM ELF -> VELF -> Sony SELF -> VPK. The
TeaVM amalgamation is compiled at `-O1` because optimizing its ~24 MiB single C
translation unit at `-O2` exceeded a modest builder's memory budget. Native
render/audio/platform code remains at `-O2`, with audited cold resource/image/
compressed-BGM paths and launcher at `-Os`. Keep 64 KiB linker pages and the
required SCE import headroom; v1.2 has 7080 bytes available (minimum 3804).

## Service contracts and limits

- ResourceAdapter tries raw lookup then `<name>.pac` through the native VFS.
  The original GameData exclusion filter reaches disk reads before normalization.
  Ordinary/community codecs are selected per resolved file. General resource
  writes are not allowed; the dedicated save.bin service is writable.
- Java GL Buffer active ranges are copied to native-owned client buffers before
  drawing, so GC cannot move memory still referenced by GL.
- Original touch scale/offset and stable IDs are passed to original KeyData.
  Native frame events are bounded and preserve Begin/Move/End phases. Raw Vita
  touch IDs map to stable slots 0–4 before entering the original Controller.
- Original engine Init/Run/Dispose are invoked. The Vita frame loop supplies
  front-touch and held-button events to VitaControls. It emits contacts only in
  audited combat/character/menu contexts; global Android Back stays neutral.
  First-frame resume initializes text; one EventQueue event progresses after
  present for original background card tasks. Dialogue X was retired; real dialogue touches remain intact.
- Texture upload, FBO, save, system-PVF text, Vorbis BGM/SE and PCM/RIFF voice
  services have real Vita implementations. They are not successful no-ops.
- Android14 text uses its confirmed UTF-8 table codec; original text uses the
  generated Shift_JIS mapping. Gen uses content-detected UTF-8 for text00
  while retaining Shift_JIS game/character tables; PAC headers alone do not
  determine charset. 65,792 single/two-byte Shift_JIS cases were
  previously compared against Java.
- Bluetooth remains disconnected and Android marketplace/browser services are
  unsupported. Remote HTTP/downloads fail honestly. The obsolete catalog reports
  success only when the active VFS resolves the complete locally installed
  character/shared dataset required by the current offline path; remote items
  remain empty.
- Only the logical game `save.bin` service is writable, isolated by selected profile and published with a
  temporary-file/rename path.
- The full Vita TeaVM runtime patch retains `java.util.Date`; NewsData makes it
  reachable. Vita provides the missing UTC calendar conversion instead of
  dropping Date as the old input-only experiment did.

The current native bridge retains bounded PAC/texture/voice caches, PVF glyph
rectangles and bandlimited character-voice output. Audio worker priority is the
restored 0x10000100, with explicit failure cleanup/diagnostics. Implementations and host probes still do not replace device testing. The retained resource baseline was approved in v1.1 and controls in separate
tests. The exact rebuilt stable v1.2 package awaits physical retest; 00.33/00.34
remain historical evidence for the earlier repairs.
See [PLATFORM_SERVICES](../../../docs/PLATFORM_SERVICES.md) for exact contracts.

## Reproduce privately

Original APK SHA-256:
`b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`.
Prepare `original.jar` with dex2jar 2.4. Use TeaVM 0.12.3 dependencies, ECJ
3.37.0 and Java 17+.

```sh
python tools/aot/engine/generate.py --original-jar /private/original.jar \
  --ecj /tools/ecj.jar --lib-directory /private/lib \
  --work-directory /private/fresh-engine

export VITASDK=/path/to/vitasdk
export PATH="$VITASDK/bin:$PATH"
python tools/aot/engine/vita/build.py --generated-c /private/fresh-engine/c \
  --build-directory /private/fresh-vita --jobs 2
```

The wrapper copies and patches raw generated C into a fresh private directory,
then configures and builds the complete target. A direct manual CMake recipe is
available in [BUILD](../../../docs/BUILD.md).

The full-engine CMake defaults are `01.02` / `DBTB01178` / `save.bin`.
The local wrapper above selects the stable defaults unless explicitly overridden. Use fresh generation after Java adapter changes.
Do not use `--test-controls` for the stable release: it selects the separate
`DBTBCT001` test bubble, `01.06` and `save-controls-test.bin`. Work directories
must be outside the repository. Generated C, original/adapted JARs, classes, APKs and
commercial payloads must not be committed. The source repository contains only
the adapters, reproducible generation/build tooling and non-commercial evidence.

The root CMake target and CI native smoke are not this complete engine. Cache
budgets and heap/compiler/link options are documented in BUILD/CURRENT_STATUS;
future reuse is described in [PORTING_GUIDE](../../../docs/PORTING_GUIDE.md).

<!-- DBTB_00_23_DETAIL:START -->
## 00.23 engine-generation checkpoint

`PatchResourceInit` must keep the original streaming loader structure and replace
only Android-specific resource-opening expressions. `ResourceAdapter.open` returns a
`NativeResourceStream` backed by native open/size/read/close imports. Do not restore
the former whole-PAC `byte[]` shortcut: it caused the reproduced 00.22 battle-start
managed-memory abort and 00.23 hardware testing validates the streaming repair.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](../../../docs/CURRENT_STATUS.md) and
> [runtime contract](../../../docs/CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
