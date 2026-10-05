# Original core integration (experimental)

This is the handwritten platform layer for the original APK's Java core. It is
not selected by the production CMake build yet. Native imports are declared in
`native/dbtb_bridge.h`; native implementation, linking and execution are separate
milestones. A generated core alone does not establish a playable VPK.

## Confirmed generation

TeaVM 0.12.3 now generates the complete reachable original Init/Run path without
Android framework, HTTP, billing or Bluetooth dependencies: 456 classes, 3989
methods. These counts include the standard library and adapters. There are no
compiler diagnostics in this experiment. The unadapted EngineProbe previously
reported 203 diagnostics, while the first adapter pass left three resource-I/O
diagnostics. Missing native implementations must fail linking; do not fill them
with successful no-ops.

Original TCBManajer/Game1..17, TCB, ObjReq, Controller, KeyData, GameTimer,
Graphics2D, GameData byte-array decoder and SpriteData remain APK-derived
bytecode. They are not copied into Git. `PatchResourceInit` requires the exact
original GameData.class SHA-256 from dex2jar 2.4, and replaces only its Android
String/stream loading overload with ResourceAdapter. The original byte-array
Init still receives the conversion and filter arguments. The GetString encoding token in TCBManajer is also adapted to the resolved
table codec (original Shift_JIS or community UTF-8). Its byte traversal, String
constructor and gameplay control flow are retained. The 104 other JAR
entry payloads remain byte-for-byte unchanged. This is not an adaptation of the
Android14 APK's changed Java behavior.

## Service contracts and limits

- ResourceAdapter tries raw lookup then `<name>.pac`, through the native VFS.
  Ordinary/community codecs are selected per file. No resource writes are allowed.
- Java GL Buffer active ranges are copied to native-owned client buffers before
  drawing, so GC cannot move memory still referenced by GL.
- Original touch scale/offset and stable IDs are passed to original KeyData.
  Native frame events can contain at most sixteen begin/move/end entries;
  KeyData still owns ten active slots. Overflow is an error.
- Original engine Init/Run/Dispose are invoked. The native frame loop must supply
  lifecycle/quit, touch events and back edges; actual controls remain unverified.
- Sound, textures, text surfaces, FBOs and saves have real native service imports.
  None is a fabricated successful backend. Text font metrics, premultiplication,
  scheduling and memory budgets need execution evidence.
- Bluetooth queries explicitly report disconnected. Sending and Android
  marketplace/browser launch are unsupported. HTTP/download queries report
  failure; the offline catalog reports an error and zero remote items. Installed
  assets use VFS, independently of the old catalog.
- Android Context/Intent are opaque boundary tokens. The original GlobalWork
  constructor's Bluetooth adapter null check is satisfied with a disabled token;
  VitaEngine explicitly clears bBluetoothEnebled before engine execution.
- Only save.bin is writable, through the native per-profile save service.
  Partial writes retain position/size arguments; full writes request truncation.
- TeaVM's stock charset implementation lacks Shift_JIS. The generated JDK
  decoder mapping supplies it; 65,792 single/two-byte cases match Java. UTF-8
  community strings retain all Unicode characters. Encoding is selected per
  resolved gamedata/text00 resource, including mixed overlays.
- TeaVM 0.12.3 direct-buffer GC faults on unreachable client buffers. Its
  allocateDirect boundary uses heap-backed buffers; the native GL layer already
  copies all active client arrays. Game bytecode and buffer ranges, native byte
  order, positions and views are retained.
- The input-probe-only runtime patch omits Date. Do not apply it unchanged to
  this core: original NewsData makes Date reachable and needs a real backend.

These are implementation contracts, not device confirmations. Native menu,
selection, battle, sound, saves and actual mod compatibility remain pending.

## Reproduce

Original APK SHA-256: b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b.
Prepare a private original.jar with dex2jar 2.4 and the TeaVM dependencies with
the parent pom.xml. Use ECJ 3.37.0 and Java 17, as in ../README.md.

```sh
python tools/aot/engine/generate.py --original-jar /private/original.jar \
  --ecj /tools/ecj.jar --lib-directory /private/lib \
  --work-directory /private/fresh-engine
```

The work directory must be new and outside the repository. Review logs there.
Generated C, original/adapted JARs, classes, APKs and commercial payloads remain
private. The patch refuses unknown GameData shapes and existing output files.
