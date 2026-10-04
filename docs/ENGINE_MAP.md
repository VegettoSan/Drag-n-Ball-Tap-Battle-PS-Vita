# Original engine and Android boundaries

Source of behavior: original supplied DEX, APK hash b84f98a…; jadx 1.5.6 local
inspection and androguard 4.1.4 call metadata. No decompiled original code or
commercial payloads were committed. The GdGohan SWB at f4a275d is a comparison
reference, not a drop-in engine. Its licensing/completeness is not established.

| System | Observed original responsibility | Native direction / current state |
|---|---|---|
| dragonballtap / AndroidGLView | Activity/GLSurfaceView lifecycle, multitouch, browser/Bluetooth/Smap launch | Replace with Vita app/input/lifecycle; original game loop still PENDING |
| AndroidGLRender | Surface matrices; pause gating; TCBManajer.Init then Run per eligible draw callback | vitaGL init implemented; real Run/game states not ported |
| GlobalWork | Shared services, screen/touch/timer/lifecycle fields | Preserve core state; replace Android references with service interfaces |
| TCBManajer / TCB / ObjReq | Ordered task lists, Game1..17 dispatch, repeat/skip/sleep, object execution, drawing | Port semantics in stages; do not invent a single new state machine |
| GameData / SpriteData | PAC dispatch, filter bits, images, raw action tables, nested SPR, sounds | Outer PAC/PNG implemented; original conversion/commands pending |
| Graphics2D / AndroidGLTexture | Quad batching, matrix/blend state, texture decode/upload | Complete API map in RENDER_MAPPING.md; atlas preview only implemented |
| offscreen / StringTexture | FBO rendering and Android Canvas-generated text | FBO adapter + text rasterizer PENDING; boot font is diagnostic only |
| KeyData / Controller | Stable touch IDs, begin/move/end; virtual pad ranges, state/history and gesture timing | Neutral input layer implemented; original gesture/command logic PENDING |
| ResourceMiner / Utility | Android raw ID reflection, files, HTTP, save operations | VFS for file lookup; filename→raw resource adapter must preserve extension rules |
| SoundEffect | MediaPlayer BGM, SoundPool SE and AudioTrack for supplied PCM | Vorbis streaming + PCM mixer design; playback PENDING |
| GameTimer | Millisecond intervals, suspended duration adjustment | Monotonic Vita timer with exact time units; PENDING |
| Downloader / Smap | HTTP, catalog/device/news data and marketplace downloads/billing | Local dataset completeness replaces startup dependency; no online calls in current bootstrap |
| BluetoothManajer / BluetoothSearch | RFCOMM discovery/transport; game receives/sends battle data | Transport adapter, not a generic input remap; multiplayer PENDING |

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
hardware/reference behavior must settle update pacing. Bootstrap UI delay is
unrelated to the future gameplay timing.

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
952 is inside practice; 1014 inside selection/settings handlers. The audit has
not executed those modes on Android/Vita.

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

## Reconstruction order

Get first hardware PAC/PNG evidence. Then implement a bounded GameData loader
with exact filter bits, CNV rectangles and DAC actions, original DrawImage and
DrawSprite. Bring in the original menu's task/panel logic and text service.
Only after those agree should character selection, data-driven battle, AI,
practice/cards/results and original saves be reconstructed.
