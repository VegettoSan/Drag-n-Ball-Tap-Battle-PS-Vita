# Validation and diagnostic reference — v1.2

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

> Current data/selector contract: [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).
> Historical fixtures keep their build identity; current VFS/runtime validation uses
> first-level `profiles/` directories only.

Commands run from the repository root. Use private game data and temporary output
outside tracked source. This page describes reproducible probes; it does not claim
all commands were freshly rerun for the documentation update. Recorded passes are
pinned in [CURRENT_STATUS](CURRENT_STATUS.md) and evidence JSONs.

## v1.2 package identity and build evidence

| Field | Current stable package |
|---|---|
| VPK | `Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk` |
| APP_VER / TITLE_ID | `01.02` / `DBTB01178` |
| Size | 2750706 bytes |
| SHA-256 | `343aee505f77fa743339111fa7cf29f1e9bda333e49bddb6be166933d7bac1fc` |
| Build source | `f6e9adaa792d38c4f3a7c7b27d112ae54b41ede5` |

Fresh local full engine: 468 classes / 4103 methods; ARM ELF/VELF/SELF/VPK,
source/version markers, 7080-byte import headroom, exact public-file allowlist,
ZIP CRC, approved seed/theme/LiveArea and packaged SELF equality pass.
The original patched JAR matches the same-input main compatibility pipeline.
This is a newly compiled executable, not a metadata-only v1.1 repack.
[Machine-readable evidence](evidence/vita_release_1.2.json).

The controls probes pass combat/character confirmation, Start resume, 37
audited CheckBack consumers, real touch priority, neutral dialogue X and
holds/transitions/loading/confirmation exclusions. Native preference tests pass;
Python tests: 41 pass / 20 fixture-gated skips. The nine-corpus pad visibility
evidence is inherited, not rerun or promoted to all-mod hardware acceptance.

```sh
python3 tools/aot/engine/tests/run_vita_controls_probe.py \
  --original-jar /private/original.jar --ecj /tools/ecj-3.37.0.jar \
  --adapter-classes /private/fresh-engine/classes
g++ -std=c++14 -Isrc tests/test_control_settings.cpp src/vfs.cpp \
  -o /private/probes/control-settings
/private/probes/control-settings
python3 tools/validate_livearea_vpk.py /private/Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk
```

Hardware approved the retained controls over test builds; dialogue X failed
and is retired. The user is testing stable v1.2; no result is recorded yet.
Check stable installation, profile progress, remembered control choices,
combat/character controls, Circle available Back and Start pause/resume, touch
dialogues and a previous mod regression. Capture version/hash/profile and logs
with the outcome. Earlier 00.xx SFOs/hashes remain correct for their own tests.

## Web Extractor validation

Static/browser extraction is validated separately from Vita runtime behavior.

Repository checks:

```sh
node --check web/extractor-core.mjs
node --check web/app.mjs
node tests/web_extractor_core.mjs
python3 tools/materialize_selector_theme.py assets/selector /tmp/dbtb-web-selector
```

The browser core was also exercised against real project APK inputs before
publication:

- original `DBTapBattle.apk`: raw layout, 57 data files, PASS;
- `gen.apk`: assets layout, 146 data files, 13-character roster, PASS;
- `tap battle android 14.apk`: audited `community14-a210795b`, 144 data files,
  13-character roster, PASS;
- Spanish Android14: audited `community14-es-d594affc`, 144 data files,
  13-character roster, PASS;
- Invasion Beta 3: audited `community14-invasion-05aa0c5e`, 177 data files,
  22-character roster `00..21`, PASS;
- Samu: assets layout, 383 data files, 92-character roster `00..91`, PASS;
  APK-bundled `save.bin` detected but intentionally omitted.

The generated Invasion, Spanish and Samu ZIPs were reopened and CRC-tested
independently. They reported the expected `profiles-v1` manifests and protected
codec identities; Samu's generated ZIP is 405,409,508 bytes and still passes ZIP
CRC validation. These checks establish extraction/package behavior, not every
browser/device memory ceiling or arbitrary mod compatibility.

The Pages workflow fails closed: JavaScript validation and selector-theme
materialization must pass before the static artifact can be deployed.

See [WEB_DATA_TOOL](WEB_DATA_TOOL.md).

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

## Historical 00.25 protected-profile regression gate

The public `Community mod profiles` workflow runs without APK/game bytes. It
checks Python extraction/alias safety plus synthetic C++ PAC/image, converted
GameData and full `engine_resources` normalization for the audited Android14,
Spanish and Invasion profiles. Run `37415176005` passed the complete gate.

Separately, the 00.25 Vita native smoke at run `37415326012` compiled and
packaged the Vita target after the profile integration/version bump. That is
BUILD evidence only; the generated smoke VPK contains the non-commercial link
probe and is **not playable**. The physical gameplay baseline remains 00.24.

The real user-supplied protected corpora were also checked locally without
committing bytes: Android14 106/106 PACs, Spanish 106/106 and Invasion 139/139
uniquely select the intended profile and have in-bounds outer tables.

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

# Private fixture/profiles/Original/ contains all 17 user-provided bgm_XX.ogg tracks.
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

Required current VFS fixture layout: `install/profiles/Original/` holds original
b84f98a3 resources, `install/profiles/Android14/` holds the encoded dataset and
`gen-root/profiles/Gen/` holds Gen assets. Each fixture must explicitly select
the intended profile; there is no `game/` fallback.
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
| `Selected profile` | First-level folder under `profiles/` | Folder label is not dataset hash |
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

Use [00.34](TEST_VITA_00_34.md) as the current hardware-confirmed gameplay/selector regression checkpoint. [00.33](TEST_VITA_00_33.md) remains historical evidence for the protected-PAC repeated-fight repair. Historical 00.23 instructions remain useful for the original baseline. For the current
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

## Historical hardware validation — 00.33

Physical Vita testing confirms the reproduced Invasion repeated-fight crash is
fixed. The user completed several fights on 00.33 without another crash after
the protected-PAC ownership handoff changed from an implicit vector assignment
to explicit `output.swap(out)`.

The same recent hardware sequence confirms the 00.31 Loading regression is gone
and dynamic mod rosters work, including Samu's 92 characters. Treat this as
feature/path evidence, not exhaustive certification of every mode, mod or
arbitrarily long session.

Current VPK SHA-256:
`d241499a356ac11c523909a84b0c383910ef7a387efcfdc2c05d3581be86fd77`.

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
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## Deep APK forensic evidence — 2026-10-06

The current APK reference was regenerated directly from the five supplied APK
archives, not inferred from filenames or prior notes. The audit records only
metadata/hashes/structure; commercial payload bytes are not committed.

Validated directly:

- ZIP path/layout counts and SHA-256 identities;
- DEX header/class/method structure and protected-family code-method comparisons;
- ordinary and protected PAC directories, nested SPR containers and bounds;
- decoded protected RGBA dimensions/hashes;
- converted GameData table metadata;
- wrapped voice counts/declared PCM contracts;
- exterior audio codec/rate/channel/duration using content probing;
- canonical logical-file comparisons after resolving aliases;
- protected-profile uniqueness across every supplied PAC.

The public evidence set is:

- `docs/APK_TECHNICAL_REFERENCE.md`
- `docs/APK_CANONICAL_DIFFERENCES.md`
- `docs/SPANISH_ANDROID14_APK.md`
- `docs/INVASION_BETA3_APK.md`
- `docs/evidence/APK_AUDIO_MATRIX_2026-10-06.md`
- `docs/evidence/apk_deep_structure_2026-10-06.json`

This evidence is sufficient to implement already-audited aliases/codecs/bounds
without possession of the APKs. It is **not** a substitute for the APK when
discovering previously undocumented commercial behavior, nor for a physical Vita
test when changing runtime behavior.


## 92-character Gen-derived mod evidence — 2026-10-06

The supplied `DragonBallZuperSamuGamerYT.apk` was audited directly after the
five-APK forensic pass. Its `classes.dex` and `AndroidManifest.xml` are
byte-identical to Gen, while its canonical assets expand the character triplets
from 13 to 92. Evidence is stored without APK payload bytes in:

- `docs/DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md`
- `docs/evidence/dragonball_zuper_samugameryt_2026-10-06.json`

Directly validated: 345/345 ordinary PAC directories, six nested SPR containers,
92/92 char BINs with 43 records, 51 cards, exact Gen asset comparison, nonstandard
`u`/ `.pn` entry tags, charf placeholder patterns, and content-based probing of
all 36 exterior audio files. This is APK/host evidence only; it does not certify
92-character Vita runtime behavior.

## Historical Samu 00.27 large-roster/direct-audio validation

Public synthetic checks:

```sh
python3 -m unittest discover -s tests -p test_prepare_samu_mod.py -v

g++ -std=c++14 -O2 -Wall -Wextra -Isrc \
  tests/test_installed_data.cpp src/installed_data.cpp src/pac.cpp src/vfs.cpp \
  -o /tmp/dbtb-installed-data
/tmp/dbtb-installed-data
```

Expected roster coverage: baseline 13, Invasion 22, Samu 92, complete two-digit
namespace 100 positions (`00..99`), incomplete-triplet rejection, gap
rejection and invalid >100-bound rejection.

The pinned engineering check below uses the historical 00.27 fixture layout;
it is not a current Vita installation recipe. For present-day installation use
Web/Windows profiles-v1 output. The historical import check is:

```sh
python3 tools/prepare_samu_mod.py \
  DragonBallZuperSamuGamerYT.apk \
  /private/install/mods/ZuperSamu
```

The helper must reject a different APK/DEX hash. Success requires 384 extracted
assets, 92 complete triplets and the documented 12 MP3 + 3 AAC/M4A + 2 Vorbis
matrix **without changing any payload hash or size**. The generated manifest must
keep `payloads_unchanged: true`.

Direct-audio validation additionally checks the real Samu containers:

- all 12 MP3 sources parse as 44.1 kHz stereo Layer III;
- `bgm_09/10/11` demux into 228 / 1578 / 228 AAC access units;
- their largest access unit is 1114 / 1143 / 1114 bytes, respectively;
- the Vita target compiles/links `SceAudiodec` together with the existing
  libvorbisfile path;
- the full candidate retains the original TeaVM symbols and approved LiveArea.

GitHub evidence: Community mod profiles run `37544623252` PASS and Vita engine
native smoke run `37544588962` PASS. Full candidate VPK SHA-256:
`bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`.
Audible playback/loop/transition behavior remains a physical gate; see
[TEST_VITA_00_27](TEST_VITA_00_27.md) and
[evidence](evidence/vita_samu_direct_audio_00.27.json).
