# Original engine and Android boundaries

Source of behavior: original supplied DEX, APK hash b84f98a…; jadx 1.5.6 local
inspection and androguard 4.1.4 call metadata. No decompiled original code or
commercial payloads were committed. The GdGohan SWB at f4a275d is a comparison
reference, not a drop-in engine. Its licensing/completeness is not established.

| System | Observed original responsibility | Native direction / current state |
|---|---|---|
| dragonballtap / AndroidGLView | Activity/GLSurfaceView lifecycle, multitouch, browser/Bluetooth/Smap launch | VitaEngine/platform loop implemented; native selector before original Init |
| AndroidGLRender | Surface matrices; pause gating; TCBManajer.Init then Run per eligible draw callback | Original Init/Run invoked; GLES bridge supplies original drawing |
| GlobalWork | Shared services, screen/touch/timer/lifecycle fields | APK-derived state preserved; handwritten Vita platform objects replace Android references |
| TCBManajer / TCB / ObjReq | Ordered task lists, Game1..17 dispatch, repeat/skip/sleep, object execution, drawing | Original methods generated privately, not replaced with a new state machine |
| GameData / SpriteData | PAC dispatch, filter bits, images, raw action tables, nested SPR, sounds | Selective native PAC normalization feeds preserved original parser/commands |
| Graphics2D / AndroidGLTexture | Quad batching, matrix/blend state, texture decode/upload | Original Graphics2D plus native GLES/texture ownership/cache bridge |
| offscreen / StringTexture | FBO rendering and Android Canvas-generated text | Real FBOs and PVF text; boot diagnostic font remains a separate service |
| KeyData / Controller | Stable touch IDs, begin/move/end; virtual pad ranges, state/history and gesture timing | Stable touch slots and screen transform feed preserved original Controller |
| ResourceMiner / Utility | Android raw ID reflection, files, HTTP, save operations | VFS filename aliases/extensions, native file reads, profile-local save service |
| SoundEffect | MediaPlayer BGM, SoundPool SE and AudioTrack for supplied PCM | Whole-clip Vorbis decode and native PCM worker; 00.21 worker/menu recovered; audible quality pending |
| GameTimer | Millisecond intervals, suspended duration adjustment | Original timer preserved; Vita wall-millis/monotonic-nanos backend |
| Downloader / Smap | HTTP, catalog/device/news data and marketplace downloads/billing | Local installed-data path; HTTP rejected, offline catalog boundary |
| BluetoothManajer / BluetoothSearch | RFCOMM discovery/transport; game receives/sends battle data | Transport adapter, not a generic input remap; multiplayer PENDING |

## Current checkpoint — 00.22

The original core is now privately AOT-compiled with TeaVM, not manually
reconstructed. Earlier Vita builds run menus/front touch/selection/battle.
00.21 starts audio/reaches menu but rejects original selection mask 187.
00.22 preserves that mask; physical selection recovery is pending. See
[CURRENT_STATUS](CURRENT_STATUS.md) and [PORTING_GUIDE](PORTING_GUIDE.md).

## Main-loop ordering recovered

1. Renderer checks initialization, lifecycle/lock/pause flags.
2. TCBManajer.Run handles resume resources, processes queued sound and Controller.
3. Transfers stable KeyData touches into Tap/Touches arrays, clears begin flags.
4. Performs BTRev gating and priority-ordered TCB execution, preserving repeat,
   sleep/skip/hit-stop behavior; dispatches Game(gw, md).
5. Sends Bluetooth state where active; clears one-frame touch/back/resume flags.
6. Draws accumulated objects and flushes the original Graphics2D batches.

FPS=40 and WAIT_FRAME_MILLITS=25 are declared by AndroidGLRender, but the
inspected onDrawFrame does not use a 25-ms sleep to pace gameplay. Do **not**
conclude fixed 40-Hz behavior from an unused constant. GameTimer units and actual
hardware/reference behavior must settle update pacing. Selector UI delay is unrelated to original gameplay timing. The full loop
presents with vitaGL and progresses one cooperative EventQueue event afterward.

## Dispatch ranges in the actual APK

| Task md range | Dispatcher |
|---|---|
| 1–24 | Game17 |
| 25–42 | Game8 |
| 43–121 | Game9 |
| 122–183 | Game13 |
| 184–292 | Game12 |
| 293–426 | Game14 |
| 427–628 | Game10 |
| 629–674 | Game15 |
| 675–730 | Game1 |
| 731–795 | Game2 |
| 796–844 | Game4 |
| 845–861 | Game11 |
| 862–895 | Game5 |
| 896–942 | Game6 |
| 943–997 | Game7 |
| 998–1080 | Game3 |
| 1081–1122 | Game16 |

Recover selection, battle actions/AI, practice, cards, results and options from
these handlers and their resources. Merely listing all md cases is not a port.
For example, case 198 prepares LoadData and SetLoad; case 809 is within results;
952 is inside practice; 1014 inside selection/settings handlers. The initial source audit did not execute those modes. Later Vita menu/selection/
battle evidence is scoped in CURRENT_STATUS; all handlers/modes are not certified.

## Critical discrepancies with the community wiki/archive

- Wiki '390 = active fight': original Game14 case 390 selects a subsequent
  opponent from character data, checks availability and starts loading it.
  It is not the whole active-combat loop. Do not implement battle at that label.
- Wiki '193 = character selection': original case 193 kills task ranges after
  fade and redirects to md 249. Selection is not explained by that label alone.
- Original GameData SOUND_MAX is 20; the community source declares 30.
- Community AndroidData/PrivGameData/SAF, extra video/music/Wi-Fi classes and
  altered package names are not present in supplied original classes.dex.
- ResourceMiner is implemented in the original, commented out in the archive.
- DAD decompilation produced invalid float rendering and commented-out save
  calls. jadx plus DEX inspection confirms the original writes save.bin; never
  take broken decompiler output as intentional original behavior.

## Current execution and modification order

VitaEngine initializes platform/selector/VFS, creates original GlobalWork and
engine, sets screen/GL state, calls original Init, sets first-frame resume,
then loops native input → original Run → present → one EventQueue event.
Original Dispose runs when the loop exits. A caught original exception can stop
the loop without a native crash; 00.20's BGM setup failure is an example.

Inspect actual methods and native imports before changing a boundary. Keep
GameData filters, signed/endian arithmetic, sprite flush order, touch gestures,
channel IDs and save offsets. Validate new adapters on host, then affected device
paths. Do not infer a game implementation from a wiki state number or require
manual reimplementation of already-preserved original interpreters.

Source map: `tools/aot/engine/java/.../VitaEngine.java`, `PatchResourceInit.java`,
`native/dbtb_bridge.h`, `vita_platform.cpp`, `gles.cpp`, `resources.cpp`,
`vita_text.cpp`, `vita_audio.cpp`, and `src/engine_resources.cpp`. Full paths and
reusable boundaries are listed in PORTING_GUIDE/BUILD. Generated original classes
are intentionally absent from Git.

<!-- DBTB_00_23_DETAIL:START -->
## 00.23 GameData/PAC runtime path

Current battle resource flow:

`TCBManajer.SetLoad/Game1` → original `GameData.Init(...)` streaming parser →
`ResourceAdapter.open(...)` → `NativeResourceStream` → native resource cache/VFS.

The native stream pins its resource owner until close and supports ranged reads.
The Java side sees the original parser's per-entry allocations rather than a whole
PAC bridge array. This path is hardware-validated through successful battle startup
in the reported 00.23 session.
<!-- DBTB_00_23_DETAIL:END -->

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
