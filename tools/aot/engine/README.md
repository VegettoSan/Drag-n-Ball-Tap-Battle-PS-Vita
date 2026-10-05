# Original core integration (experimental)

This directory contains the handwritten platform layer for the original APK's
Java core. The full private engine can now be generated with TeaVM and built for
PS Vita through `vita/CMakeLists.txt`. Native imports are declared in
`native/dbtb_bridge.h`. Generated game C and APK-derived JAR/classes remain
outside Git; a successful build is not by itself hardware-playability evidence.

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
hard-float: TeaVM C + native services -> ARM ELF -> Sony SELF -> VPK 00.03. The
TeaVM amalgamation is compiled at `-O1` because optimizing its ~24 MiB single C
translation unit at `-O2` exceeded a modest builder's memory budget. Native
render/audio/platform code remains at `-O2`.

## Service contracts and limits

- ResourceAdapter tries raw lookup then `<name>.pac` through the native VFS.
  Ordinary/community codecs are selected per resolved file. Resource writes are
  not allowed.
- Java GL Buffer active ranges are copied to native-owned client buffers before
  drawing, so GC cannot move memory still referenced by GL.
- Original touch scale/offset and stable IDs are passed to original KeyData.
  Native frame events are bounded and preserve Begin/Move/End phases.
- Original engine Init/Run/Dispose are invoked. The Vita frame loop supplies
  front-touch events and Android-style Back edges; physical gameplay mappings
  beyond that are still unverified.
- Texture upload, FBO, save, system-PVF text, Vorbis BGM/SE and PCM/RIFF voice
  services have real Vita implementations. They are not successful no-ops.
- Android14 text uses its confirmed UTF-8 table codec; original text uses the
  generated Shift_JIS mapping. 65,792 single/two-byte Shift_JIS cases were
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

These are build/implementation confirmations. The full 00.03 VPK still requires
Vita3K or real-hardware execution before menu, character selection, battle,
audio and saves can be promoted to VITA3K/HARDWARE CONFIRMED.

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

The private output is `DBTapBattle-Vita-00.03.vpk`. The work directories must be
outside the repository. Generated C, original/adapted JARs, classes, APKs and
commercial payloads must not be committed. The source repository contains only
the adapters, reproducible generation/build tooling and non-commercial evidence.
