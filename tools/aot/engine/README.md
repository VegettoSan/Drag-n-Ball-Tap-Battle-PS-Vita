# Original core integration — full engine 00.22

This directory contains the handwritten platform layer for the original APK's
Java core. The full private engine can now be generated with TeaVM and built for
PS Vita through `vita/CMakeLists.txt`. Native imports are declared in
`native/dbtb_bridge.h`. Generated game C and APK-derived JAR/classes remain
outside Git; a successful build is not by itself hardware-playability evidence.

## Current checkpoint

Latest full-engine artifact is 00.22 at `c40ce0a`. 00.21 starts audio and reaches
the menu on Vita, then rejects char00 mask 187 and exits selection. 00.22 removes
that native range guard; physical selection recovery is pending. Earlier menu/touch/selection/combat
and 00.19 text recovery are hardware confirmed at their own builds. Read
[CURRENT_STATUS](../../../docs/CURRENT_STATUS.md),
[BUILD](../../../docs/BUILD.md) and [VALIDATION](../../../docs/VALIDATION.md).
The original core is preserved; 00.21 generation has 465 classes / 4059 methods.

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
hard-float: TeaVM C + native services -> ARM ELF -> VELF -> Sony SELF -> VPK 00.22. The
TeaVM amalgamation is compiled at `-O1` because optimizing its ~24 MiB single C
translation unit at `-O2` exceeded a modest builder's memory budget. Native
render/audio/platform code remains at `-O2`.

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
  front-touch events; physical gameplay Back/pause edges are deliberately neutral.
  First-frame resume initializes text; one EventQueue event progresses after
  present for original background card tasks. Physical gameplay mappings are open.
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
- Only `save.bin` is writable, isolated by selected profile and published with a
  temporary-file/rename path.
- The full Vita TeaVM runtime patch retains `java.util.Date`; NewsData makes it
  reachable. Vita provides the missing UTC calendar conversion instead of
  dropping Date as the old input-only experiment did.

The current native bridge retains bounded PAC/texture/voice caches, PVF glyph
rectangles and bandlimited character-voice output. Audio worker priority is the
restored 0x10000100, with explicit failure cleanup/diagnostics. Implementations
and host probes do not substitute for the pending 00.22 physical selection test.
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

python tools/aot/engine/vita/patch_runtime.py /private/fresh-engine/c

export VITASDK=/path/to/vitasdk
cmake -S tools/aot/engine/vita -B /private/build-vita \
  -DCMAKE_BUILD_TYPE=Release \
  -DTEAVM_C_DIR=/private/fresh-engine/c
cmake --build /private/build-vita -j2
```

The private output is `DBTapBattle-Vita-00.22.vpk`. The work directories must be
outside the repository. Generated C, original/adapted JARs, classes, APKs and
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
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
