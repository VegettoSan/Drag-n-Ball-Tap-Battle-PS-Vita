# Validation and diagnostic reference — 00.23

Commands run from the repository root. Use private game data and temporary output
outside tracked source. This page describes reproducible probes; it does not claim
all commands were freshly rerun for the documentation update. Recorded passes are
pinned in [CURRENT_STATUS](CURRENT_STATUS.md) and evidence JSONs.

## Evidence levels

| Label | Establishes | Does not establish |
|---|---|---|
| FORMAT CONFIRMED | Source contract plus pinned corpus bytes/bounds | Runtime gameplay behavior |
| HOST CONFIRMED | Tested adapter/DSP/parser behavior on PC | Vita kernel/driver scheduling or visual/audible fidelity |
| BUILD CONFIRMED | Complete target compile/link/VELF/SELF/VPK | Device startup or gameplay |
| VITA3K CONFIRMED | Observed emulator behavior for that build | Equivalent physical Vita behavior |
| HARDWARE CONFIRMED | User/device result for stated build, profile and feature | All modes, arbitrary mods or every later version |

No current full-engine Vita3K confirmation is recorded. API mocks must be named.
Keep build source, VPK SHA, dataset/profile and procedure with every promotion.

## Host probes

Requires host C/C++ compiler and development headers/libraries for png/zlib and
Vorbis/ogg where used. Do not add all VitaSDK headers to host include paths or
mix target libraries with host libraries. ASan/UBSan stay enabled; leak scanning
may be disabled in runners whose process/thread inspection is restricted.

```sh
mkdir -p /private/probes
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 tools/aot/engine/tests/run_gles_buffer_probe.py --ecj /tools/ecj-3.37.0.jar

g++ -std=c++14 -O2 -fsanitize=address,undefined -Itests/audio_stubs \
  -Itools/aot/engine/native -Isrc tests/test_vita_audio.cpp src/vfs.cpp \
  -lvorbisfile -lvorbis -logg -o /private/probes/audio
ASAN_OPTIONS=detect_leaks=0 /private/probes/audio

# Private fixture/game/ contains all 17 user-provided bgm_XX.ogg tracks.
# Tracks C++ allocations; reproduces legacy bgm_03 bad_alloc with a 6 MiB
# single-request ceiling and verifies exact PCM plus lower fixed-load peaks.
g++ -std=c++14 -O2 -Itests/audio_stubs -Itools/aot/engine/native -Isrc \
  tests/test_vita_ogg.cpp src/vfs.cpp -lvorbisfile -lvorbis -logg \
  -o /private/probes/ogg
/private/probes/ogg /private/fixture

g++ -std=c++14 -O2 -fsanitize=address,undefined -Itests/text_stubs \
  -Itools/aot/engine/native -Isrc tests/test_vita_text.cpp -o /private/probes/text
ASAN_OPTIONS=detect_leaks=0 /private/probes/text

g++ -std=c++14 -O2 -fsanitize=address,undefined -Isrc \
  tests/test_resource_stream.cpp src/engine_resources.cpp src/pac.cpp \
  src/game_data.cpp src/vfs.cpp -o /private/probes/resource-stream
ASAN_OPTIONS=detect_leaks=0 /private/probes/resource-stream /private/install /private/gen-root

g++ -std=c++14 -O2 -fsanitize=address,undefined -Itests/resource_stubs \
  -Itools/aot/engine/native -Isrc tests/test_vita_resources.cpp src/vfs.cpp \
  src/pac.cpp src/image.cpp src/engine_resources.cpp src/game_data.cpp \
  -lpng -lz -o /private/probes/native-resources
ASAN_OPTIONS=detect_leaks=0 /private/probes/native-resources /private/gen-root

g++ -std=c++14 -O2 -fsanitize=address,undefined -Isrc \
  tests/test_engine_resources.cpp src/engine_resources.cpp src/pac.cpp \
  src/game_data.cpp src/vfs.cpp src/image.cpp -lpng -lz \
  -o /private/probes/engine-resources
ASAN_OPTIONS=detect_leaks=0 /private/probes/engine-resources /private/install
```

Required fixture layout: `install/game/` holds original b84f98a3 resources,
`install/mods/Android14/` holds the encoded dataset; `gen-root/game/` holds Gen
assets. Extraction into `gen-root` without game/ is the wrong VFS fixture.
Host VFS rejects symlink components, so use real directories. No private fixture
is added to Git. Basic core/image/community/game-table/UTF16 probes remain in
`tests/`; format documents give their specialized commands.

| Probe | Real paths tested | Mocked / limit |
|---|---|---|
| Python suite | Import conflicts/aliases/layout and dataset save contract | Filesystem/source checks, not live Vita save round-trip |
| GLES JVM | Buffer position/limit/offset/reuse/bounds | Native imports mocked |
| Native audio | Real setup state machine, RIFF/PCM/cache/DSP/limiter | Kernel/audio calls mocked; no real worker scheduling |
| Native text | Real glyph cache/rectangle/raster/upload logic | PVF/GL mocked; not actual font repertoire |
| Stream/resource | Exact selected PAC bytes/order/filter/cache/invalidation | Host file timing, not Vita I/O latency |
| Native texture | Real PNG decoder and cache/reference/eviction behavior | GL upload/delete mocked; not GPU throughput |

Current evidence: 12 Python checks; 26 character PACs with filters 1/33/64/127/187/251 plus high/sign-bit edges;
Original+Community14 corpus 125 files / 137 containers / 470 textures / 68 BIN
converted tables / 198 WAVs. Gen's distinct corpus has 108 PACs / 405 PNG entries /
69 top-level BIN tables / 198 RIFF voices. Do not combine corpus counts as if
one APK contained all of them.

## Verify the produced artifact

Follow [BUILD](BUILD.md), then check ZIP CRC, SFO APP_VER/TITLE_ID and eboot equality
against the current build output. Record bytes/SHA-256 of VPK and eboot; confirm
ELF contains expected full-engine symbols and version/commit, not the tiny CI main.
Keep ELF/VELF for diagnosis. A symbols ZIP is not automatically a relink bundle.
Native CI compiles services against a non-commercial dummy main; tool-export CI
only exports public tools. Neither runs the complete original game.

For every VPK that carries the current presentation assets, also run:

```sh
python3 tools/validate_livearea_vpk.py /path/to/DBTapBattle-Vita-00.23.vpk
```

This verifies that the package contains the exact approved `icon0.png`,
`pic0.png`, `bg0.png`, `startup.png` SHA-256 values and a style-`a1`
`template.xml` that maps the expected background and startup gate. Native smoke
run `37384814624` passed this check after VitaSDK built the ELF, SELF and VPK.

For the presentation-only playable test package, compare it to the hardware-tested
00.23 base as well: every pre-existing ZIP member must remain byte-identical and
the member-set difference must contain only the five LiveArea paths. That exact
comparison is recorded in
[evidence/vita_livearea_00.23.json](evidence/vita_livearea_00.23.json).
It establishes packaging identity, not physical shell rendering.

## Interpret runtime.log

The log appends sessions. Analyze from the relevant `full original engine Vita`
boot marker to that session's end; do not add older clipping/failure counts.
Source commit is captured by CMake configuration, not inferred from current main.

| Field / marker | Meaning / units | Limit |
|---|---|---|
| `Selected profile` | Original base or mod folder | Folder label is not dataset hash |
| `ORIGINAL ENGINE INIT PASS` | Original Init returned successfully | Does not mean menu/battle reached |
| `ORIGINAL ENGINE RUN FRAMES` | Main loop ended, reports frame count | May be caught exception/normal exit without native crash |
| `Run: md=... ERROR=...` | Original caught Java exception/state | Use stack/last resources before assuming corrupt data |
| `Audio: ... failed: 0x...` | 00.21 identifies open/create/start result | 00.20 combined failure lacked this distinction |
| `PVF glyph` | Image rectangle and nonzero coverage for initial diagnostics | Limited sample, not exhaustive font validation |
| `Resource: filter/cache/io_bytes/bridge_bytes/us` | Exclusion filter, cache reuse, source bytes and elapsed load | Excluded payload reads/cached hits do not equal whole-process memory reduction |
| `fps` | Frames divided by elapsed 120-frame window | Average hides individual stalls |
| `run_ms`, `run_max_ms` | Average/max elapsed main-thread work before swap | Includes Java/GL/loads; not CPU utilization |
| `swap_ms` | Average present elapsed time | Includes pacing/GPU waits; not proof of GPU saturation |
| `interval_max_ms` | Largest between-frame interval | Includes work after present, including cooperative events |
| `load_ms`, `texture_ms`, `text_ms`, `audio_decode_ms` | Accumulated elapsed work in window | Timings may overlap; decode includes associated diagnostics |
| `resource_io_KiB`, cache-hit counts | Source bytes / retained-resource reuse | Compare same filters/profile/cold or repeated conditions |
| `audio_clip_samples` | Actual post-limiter saturation in 00.19+ | Older 00.18 field counted hard-clamped mixing |
| `audio_overload_samples` | Pre-limiter sum exceeded PCM range | Does not imply post-limiter clipping |
| `audio_late_mix`, `audio_mix_max_us` | Mix computations beyond one block / largest duration | Does not cover all delivery/driver gaps |
| `audio_submit_gaps`, `audio_submit_max_us` | Intervals between output calls over two blocks / largest interval | Gap alone does not prove output underrun |

Output blocks are 1024 frames at 48000 Hz (~21.33 ms); gap threshold is two
blocks (~42.67 ms). Worker performs no per-block file logging/allocation. Vita
nano printf did not support `%zu` in the new 00.20 resource line, shifting its
values; 00.21 uses explicit supported-width formatting there. Other historic
unsupported diagnostics must not be treated as reliable numbers.

## Physical test protocol

Use [00.23 instructions](TEST_VITA_00_23.md) as the gameplay baseline. For the current
LiveArea-only derivative, follow
[TEST_VITA_00_23_LIVEAREA](TEST_VITA_00_23_LIVEAREA.md): verify VitaShell
installation, bubble icon, LiveArea background and startup gate first, then perform
a short menu/selection/battle regression check without changing game data.

After presentation is confirmed, extend gameplay coverage with repeated battles,
multiple characters, cold/repeated/evicted resource loads, both supported dataset
paths, return-to-menu, repeated launches and longer sessions.

Report exact profile/data provenance, version/hash and whether a clean exception exit
or native crash occurs. Copy `runtime.log`; include `psp2core` only if produced.
Screenshots/photos establish physical LiveArea rendering; recordings remain the
right evidence for audible artifacts that counters cannot establish.

<!-- DBTB_00_23_DETAIL:START -->
## Latest hardware validation — 00.23

See [TEST_VITA_00_23](TEST_VITA_00_23.md). The acceptance path that failed in 00.22
now passes on a physical Vita: audio remains clean, character switching remains
responsive, battle startup succeeds and gameplay proceeds without an error observed
in the reported session.

This does not close exhaustive regression. Keep exact VPK/source hashes and extend
testing to repeated battles, additional characters, datasets/mods and long sessions.
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
