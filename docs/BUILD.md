# Build and packaging

## Controls Test 1 — local full-engine build, separate branch

Built on `test/vita-controls` without dispatching or running a build workflow.
The existing open-source tool exports supply pinned TeaVM 0.12.3, ECJ 3.37.0
and VitaSDK 2026.08 / GCC 15.2.0. The user-owned original APK/JAR and generated
C remain outside Git. Do not build the repository-root atlas bootstrap for
this test; it is not the playable engine.

```sh
python tools/aot/engine/generate.py --original-jar /private/original.jar \
  --ecj /private/tools/ecj-3.37.0.jar --lib-directory /private/tools/lib \
  --work-directory /private/controls-generation
python tools/aot/engine/vita/build.py --generated-c /private/controls-generation/c \
  --build-directory /private/controls-build --jobs 2 --test-controls
```

`--test-controls` sets `DBTBCT001`, APP_VER `01.02`, the test title and isolated
profile save filename. It retains the original full TeaVM amalgamation at
`-O1`; native code stays `-O2`, with existing cold paths and the launcher at
`-Os`. Launcher compaction preserves SCE import headroom for vita-elf-create
without changing the original Java engine or its optimization level.

Host validation: original Controller/KeyData input probe; native sparse views,
immutable cache and stream snapshot/fragmentation tests against effect DAC/CNV
from nine supplied APKs; native save preference/progress/isolation tests;
Python regression suite and final LiveArea/VPK integrity checks. Fixture
construction keeps private data outside Git; the native corpus uses actual
DAC/CNV payloads in normalized test containers, not nine rendered games.
Artifact: `DBTapBattle-Vita-01.02-Controls-Test-1.vpk`, 2749339 bytes.
SHA-256: `f0fb5355304671a69865c9e11b1ea513b682b50c670438210adc0bbcccc3b3f0`.
Source commit: `213b203c02798e60b821537263d4c7e7aa0228c6`.
Final RX-to-RW headroom: 9192 bytes; SCE conversion, SELF,
ZIP/SFO/seed/full-engine checks and approved LiveArea validation pass.
See [hardware protocol](TEST_VITA_CONTROLS.md) and
[build evidence](evidence/vita_controls_test_1.json).

## v1.0 public release — DBTB01178

Current public package identity:

- file: `Dragon-Ball-Tap-Battle-PS-Vita-v1.0.vpk`
- APP_VER: `01.00`
- TITLE_ID: `DBTB01178`
- VPK SHA-256:
  `15eb056274db6f3ad561c3befb670833c348f536c3073590b9768b04f74ee594`
- gameplay/runtime checkpoint:
  `0da8684805d1510caf93130a22eed523a854c1d6`

v1.0 promotes the hardware-confirmed 00.34 implementation to the first stable
public release. The package identity changed from the historical test ID
`DBTB00001` to `DBTB01178`; the gameplay executable/resources are otherwise
the 00.34 stable content. Historical build sections below retain the exact
APP_VER/TITLE_ID/hash values of the artifacts that were actually tested.

## 00.34 stable hardware-confirmed build — unified profiles + selector UX

Current complete user-test artifact:

- `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk`
- APP_VER: `00.34`
- TITLE_ID: `DBTB00001`
- VPK SHA-256:
  `24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03`
- runtime/selector checkpoint:
  `0da8684805d1510caf93130a22eed523a854c1d6`
- physical result: **HARDWARE CONFIRMED — stable and functional**

00.34 retains the 00.33 gameplay fixes and changes the external dataset contract
to `ux0:data/DBTapBattle/profiles/<Profile>/`. The VPK no longer distinguishes
`game/` from `mods/` at runtime and no longer synthesizes an Original row.

The current 00.34 rebuild changes only selector button/text geometry after the
no-orb fix: the 664 px-wide themed button is centered at x=148, the one-star
marker is shifted with it, and labels are fitted to a 530 px cyan text region
centered at x=480. Original game/core behavior is unchanged.

The Gen-derived selector background now crops to the continuous 482×320
cyan/grid band at the top of the 512×512 source before stretching to 960×544,
so the embedded lower blue energy orb is never rendered. Confirming a profile presents **OPENING PROFILE /
LOADING GAME DATA...** before the original engine loads that selected directory.

The interactive full-engine build uses the documented split TeaVM compilation
technique only as a compilation strategy; original game behavior is not replaced.
The user reports this exact VPK stable and functional on physical Vita, with no
problem found so far in the exercised paths.

See [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md) and
[TEST_VITA_00_34](TEST_VITA_00_34.md).


## 00.33 physical-test build — Invasion Saitama -> Freezer allocation fix

00.33 is built from the exact 00.32 coredump diagnosis. The protected-PAC
normalizer no longer duplicates the fully rebuilt PAC at the final ownership
handoff.

Artifact:

- `DBTapBattle-Vita-00.33-Invasion-Saitama-Freezer-Fix.vpk`
- size: 2,650,664 bytes
- VPK SHA-256: `d241499a356ac11c523909a84b0c383910ef7a387efcfdc2c05d3581be86fd77`
- eboot SHA-256: `bc0a0d4293e5b416084d02050dd6b3c17529cc63fab00bfe7a48d0310303d43b`
- ELF SHA-256: `6c55a58f59277bee0d2632dbefca8c1a938d577457b867489d3605a11cee7dbe`
- APP_VER: `00.33`
- TITLE_ID: `DBTB00001`
- runtime source checkpoint: `71b95d54ad6eef0ebd2043eb96269f6e7a4e1370`
- LiveArea validation: PASS
- VPK save seed remains exact:
  `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.

The matching 00.32 coredump/ELF resolves the failing allocation through
`std::vector<unsigned char>::operator=` inside `normalise()`. For protected
Invasion `char15.pac`, the normalized buffer is about 4.64 MiB. 00.33 uses
`output.swap(out)` on the changed path, so that already-built allocation is
transferred rather than copied into a second contiguous vector.

The interactive full-engine package uses the same documented split TeaVM
compilation technique as recent hardware candidates. Runtime native smoke, Community profile tests, private tool export, complete private link/SELF/VPK and LiveArea validation pass. Physical Vita testing subsequently completed several Invasion fights without reproducing the Saitama -> Freezer crash, so this build is hardware-confirmed for that failure path.

See [TEST_VITA_00_33](TEST_VITA_00_33.md), [build evidence](evidence/vita_build_00.33.json) and [hardware result](evidence/vita_hardware_00.33.json).

## 00.32 physical-test build — Loading loop regression fix

00.32 retains the 00.31 roster/Shop/memory work but corrects the offline
Downloader polarity and replaces the deep startup PAC audit with a presence-only
profile scan.

Artifact:

- `DBTapBattle-Vita-00.32-Loading-Loop-Fix.vpk`
- size: 2,650,991 bytes
- VPK SHA-256: `07a8ab63923e4913cc1810a5658c84ef925d3eddf08ac04b81658c683956b4b9`
- eboot SHA-256: `4cc9b5cdc4e6409f6d59151b15dc38187fdb28908d86a7ac071f9ea541ff8863`
- ELF SHA-256: `d0cb6478a4e93af7a85f706a23b7cd1c81b35c931d7aa6255932889b924e14f4`
- APP_VER: `00.32`
- TITLE_ID: `DBTB00001`
- runtime checkpoint: `5d2c8ded88731da8340f0e029d4a666e4fbbec88`
- LiveArea validation: PASS
- exact VPK save seed retained.

The package is a complete private original-engine build. As with recent hardware
candidates, the interactive build uses split TeaVM compilation to fit the runner;
this does not change the runtime source behavior being tested.

See [TEST_VITA_00_32](TEST_VITA_00_32.md) and
[evidence](evidence/vita_build_00.32.json).

## 00.31 physical-test build — roster / Shop / memory / startup

Built privately from the pinned original APK after the 00.30 physical report.

Artifact:

- `DBTapBattle-Vita-00.31-Roster-Shop-Memory-Startup-Test.vpk`
- size: 2,653,793 bytes
- VPK SHA-256: `85b28d7a080c2bc5806ca3c269a7fe15b2be84565c60ddca243dd3fad0e6e699`
- eboot SHA-256: `be043461ae79f0ce789a7389f8d4ba315f8121172ae138f505a8ea33728134a6`
- ELF SHA-256: `d8ab0ca42d07228d100d225b8803f95275be055dbf12cf35ff8699c22ba20127`
- APP_VER: `00.31`
- TITLE_ID: `DBTB00001`
- source checkpoint: `bf283ae5c5e1d0e7c19b540dd89b0105edfa3a4b`
- LiveArea validation: PASS
- native smoke run `37569189645`: PASS
- private-tool export run `37569189747`: PASS

The exact VPK save seed remains 12,906 bytes with SHA-256
`64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.
00.31 synchronizes only installed-character visibility/download/open flags in the
selected profile's independent save.

The interactive functional package uses 24 balanced TeaVM remainder units at
`-O1`, the large `TCBManajer.c` unit at `-O0`, and native adapters at
`-O2`. This is for functional hardware validation; release performance claims
still require the normal reproducible build.

See [TEST_VITA_00_31](TEST_VITA_00_31.md) and
[evidence](evidence/vita_build_00.31.json).

## 00.30 physical-test build — independent seeded profile saves

00.30 retains the Samu decoder-handoff and Invasion charset fixes from 00.29,
while changing only native save routing and package version. No original gameplay
method was changed.

Artifact:

- `DBTapBattle-Vita-00.30-Independent-Profile-Saves-Test.vpk`
- size: 2,654,057 bytes
- VPK SHA-256: `d823b9baddd667b09b1c407575a5698cd71276e1698e9d9845fb3086b3777379`
- eboot SHA-256: `c193a07463cbc7168bb1a5d5398f259dc4cdb0216b9d909b268df3b386faca2c`
- ELF SHA-256: `e5b38f5c4a5409d3ead2fc42929e3cf9e6a9fc1d931c0d54e0bf23262f59a236`
- APP_VER: `00.30`
- TITLE_ID: `DBTB00001`
- runtime source marker: `6d88bee`
- LiveArea validation: PASS

The root VPK `save.bin` remains the exact 12,906-byte approved seed with
SHA-256 `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.
Runtime copies it once to the selected profile's own writable save when absent;
existing profile progress is not overwritten.

See [00.30 test](TEST_VITA_00_30.md) and
[evidence](evidence/vita_build_00.30.json).

## 00.29 physical-test build — Samu / Invasion / shared save

A complete original-engine 00.29 VPK was generated from the pinned original APK
SHA-256 `b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`
using TeaVM 0.12.3 and VitaSDK 2026.08.

Artifact:

- `DBTapBattle-Vita-00.29-Samu-Invasion-SharedSave-Test.vpk`
- size: 2,653,396 bytes
- VPK SHA-256: `fc2a4ced375212eeb32609c199b95e0c58b55dfc6ccf07c0ce6da95f9a093e75`
- eboot SHA-256: `4f5f911adc27de89dea996670fdfd6d64b6a29b340f2f0826c1638eddfa6f06a`
- ELF SHA-256: `a3574f8777eb2d12a1db08af6a96bde509bff6787ee1d0af293d65b19e755f58`
- APP_VER: `00.29`
- TITLE_ID: `DBTB00001`
- LiveArea validation: PASS
- runtime source checkpoint: `1fe6e2dbff77b456846f7c4b6203c01383a5a549`

The VPK contains the exact approved 12,906-byte `save.bin` seed at its root,
SHA-256 `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.
Runtime copies it to `ux0:data/DBTapBattle/save.bin` only when the global save is
absent.

Because the interactive build container terminates long compiler invocations, this
functional hardware-test package uses the already documented split technique:
24 private TeaVM remainder units at `-O1`, the very large `TCBManajer.c` at
`-O0`, and native adapters at `-O2`. This affects build optimization layout,
not the original gameplay/data logic. Final performance/release claims still
require the standard reproducible build workflow.

See [00.29 device test](TEST_VITA_00_29.md) and
[evidence](evidence/vita_build_00.29.json).

For manual GitHub compilation/publication, use the separate
[release/prerelease workflows](RELEASE_WORKFLOWS.md). They consume the pinned
original APK privately in an ephemeral runner, generate the full core outside
Git and publish only compiled binaries and manifests. Configure the private
download secret before the first run. The native smoke remains non-playable.

## 00.27 Samu direct-audio physical-test build

00.27 supersedes the 00.26 import-time conversion candidate. Samu data is now
installed byte-for-byte from the audited APK; the Vita runtime selects Vorbis,
MP3 or AAC/M4A by content. Vorbis keeps libvorbisfile, while MP3/AAC use the
system `SceAudiodec` decoder. No BGM file is renamed, transcoded or repacked.

Exact physical-test artifact:

- `DBTapBattle-Vita-00.27-Samu-DirectAudio-Test.vpk`
- VPK SHA-256
  `bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`
- eboot SHA-256
  `447324fcd3c0c6400f7a3c3cea92bc3a105f64c240831e376f289a535341a5a3`
- ELF SHA-256
  `ecb70e9686b70a330dad4b85e1d0448791ff67a2d1b238c24d1610d1c46704f3`
- runtime marker `926eb6`, APP_VER `00.27`, TITLE_ID `DBTB00001`
- generation 467 classes / 4086 methods
- exact LiveArea validation PASS
- required full-engine symbols and `sceAudiodecDecode` present.

This interactive functional package splits the TeaVM remainder into ten private
compilation units at `-O1`, keeps `TCBManajer.c` at `-O0`, and leaves
native adapters at `-O2`. This is the same class of interactive-build exception
already documented for prior physical tests; it changes optimization/packaging,
not gameplay behavior. Final release performance should still be measured from
the standard reproducible build recipe.

Public native smoke run `37544588962` and Community mod profile run
`37544623252` both pass. See [TEST_VITA_00_27](TEST_VITA_00_27.md) and
[evidence](evidence/vita_samu_direct_audio_00.27.json).

## Historical 00.26 Samu conversion physical-test build

The 00.26 Samu candidate has a complete full-engine physical-test package built
from the same pinned original APK and exported private toolchain used by the
project. It is not the native smoke probe.

Exact identity:

- `DBTapBattle-Vita-00.26-Samu-Roster-Test.vpk`
- VPK SHA-256
  `749b9d32e6ed62a7b4593cb6f0b5af6dc2cabbc97cd9f25986757700879e18f5`
- eboot SHA-256
  `1de9962f19cf9c39a1534e9547de14e3a2569950a6b712ca49ab871443f90830`
- ELF SHA-256
  `db580cd100ac330d88908a9db2cd71f53a50b70c2295700ea0a17fbba68e7e9e`
- runtime marker `d7a4aa2`, APP_VER `00.26`, TITLE_ID `DBTB00001`
- generation 467 classes / 4086 methods
- exact LiveArea validation PASS.

Because the interactive runner has a per-command compilation limit, this test
package uses the same split technique previously accepted for the successful
00.23 device test: the TeaVM remainder is `-O1`,
`TCBManajer.c` is `-O0`, and native services remain `-O2`. This does not
alter gameplay source or adapter behavior, but it means this artifact is for
functional/hardware validation rather than final performance benchmarking.
A release-quality package should return to the standard monolithic `all.c -O1`
recipe after Samu functionality is confirmed.

See [00.26 device protocol](TEST_VITA_00_26.md) and
[evidence](evidence/vita_samu_build_00.26.json).

## 00.24 hardware-confirmed full build

The Android14 battle-start audio-memory candidate is built locally from a fresh
original APK dex2jar/TeaVM generation with pinned tools and the standard full
`tools/aot/engine/vita/build.py` recipe. The complete TeaVM `all.c` compiles at
`-O1`, including TCBManajer; native services compile at `-O2`. No split compilation
or unoptimized gameplay translation unit is used for this candidate. Private
APK/JAR/generated C remain outside Git. Approved LiveArea files are included
by the normal build. The user confirms this VPK works on Vita; see
[00.24 device test](TEST_VITA_00_24.md).

Current full-build/hardware evidence: [00.24](evidence/vita_battle_audio_00.24.json).
The 00.23 split-compilation caveat below applies to that historical package only.
Read [CURRENT_STATUS](CURRENT_STATUS.md) before interpreting build success as
hardware success. Commands below run from the repository root; keep private
inputs/outputs outside it.

## Choose the correct target

| Target | Inputs / output | Purpose |
|---|---|---|
| `tools/aot/engine/vita` | APK-derived JAR → current adapters → generated C → `DBTapBattle-Vita-00.24.vpk` | Full original game engine |
| Root `CMakeLists.txt` | Native atlas preview → `dbtb_vita.vpk` | Historical bootstrap; not the game |
| `.github/workflows/vita-engine-native-smoke.yml` | Tiny non-commercial all.c + real native services | Compile/link/package smoke; no game execution |
| `tools/aot` input probe | Original KeyData/Controller only | JVM/C feasibility comparison; not the full runtime |

The current hardware-tested package is `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk`,
SHA-256 `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`, built
from runtime source `f5672d4d`. Check `VITA_VERSION` and the embedded source marker for every fresh package.

## Dependencies and ABI

Validated private build: VitaSDK 2026.08 / GCC 15.2.0, ARM hard-float;
vitasdk-core 2026.08.1-1, vitaGL package r1488/2bdbe89, libpng 1.6.58,
zlib 1.3.2, libmathneon, vitaShaRK, SceShaccCgExt, taihen, Vorbis/ogg and
VitaSDK C/C++/pthread runtime. These describe the validated environment, not a
promise that every later package release is equivalent. Use matching target
ABI libraries throughout; do not combine softfp and hard-float archives.

Use the official [VitaSDK setup](https://vitasdk.org/) and package channel.
The linker archive is `vitashark` even when the package is named vitaShaRK.
Full native dependency order is maintained in the full-engine CMake file.
A device also needs the shader compiler module required by vitaGL
(`libshacccg.suprx`); it is not included in this VPK.

Private Java tools: JDK 17, dex2jar 2.4, ECJ 3.37.0 and TeaVM 0.12.3 dependencies
from `tools/aot/pom.xml`. `Export private build tools` CI can provide public tool
bundles; it does not contain the original JAR or generated commercial C.

## Generate the original core privately

The engine input is the **original** b84f98a3 APK. Alternate APKs supply datasets,
not substitute game bytecode for this recipe. See [APK_AUDIT](APK_AUDIT.md).

```sh
# /private denotes your own directory outside tracked source.
bash /tools/dex-tools-v2.4/d2j-dex2jar.sh -f /private/DBTapBattle.apk -o /private/original.jar
mvn -f tools/aot/pom.xml \
  org.apache.maven.plugins:maven-dependency-plugin:3.8.1:copy-dependencies \
  -DincludeScope=runtime -DoutputDirectory=/private/lib
python3 tools/aot/engine/generate.py \
  --original-jar /private/original.jar --ecj /tools/ecj-3.37.0.jar \
  --lib-directory /private/lib --work-directory /private/engine-fresh
python3 tools/aot/engine/vita/patch_runtime.py /private/engine-fresh/c
```

Generation checks the expected loader/string/static-field shapes and fails on
unexpected inputs. 00.23 preserves the original streaming `GameData.Init` parser,
patches only Android resource-opening expressions, and adds the native-backed
`NativeResourceStream` import path. The fresh 00.23 generation produced 467 classes /
4086 methods. CMake 4.4.4 and the VitaSDK 2026.08 toolchain family remain the validated build environment.

Use a fresh generation directory. The runtime patch is deliberately single-use;
it rejects already-patched or incompatible shapes. Do not apply the input-only
patch under `tools/aot/vita` to the full engine. Do not add the generated C root
to global include paths: its string.h/time.h would shadow native libc headers.
Re-generate after Java/import adapter changes; changing C++ alone does not
require Java regeneration when import contracts are unchanged.

## Build the complete engine

```sh
export VITASDK=/your/vitasdk
export PATH="$VITASDK/bin:$PATH"
cmake -S tools/aot/engine/vita -B /private/build-vita \
  -DCMAKE_BUILD_TYPE=Release -DTEAVM_C_DIR=/private/engine-fresh/c
cmake --build /private/build-vita -j2
```

Compile generated all.c once; do not also compile its included constituents.
TeaVM all.c uses `-O1` to bound compiler memory; native services use `-O2`, C++14,
no exceptions/RTTI. The full process reserves a 96 MiB Newlib heap and generated
managed heap min/max 8/48 MiB plus GC metadata. These are distinct budgets.
Do not link the input-only or root-bootstrap heap definitions into this target.

Link uses `-Wl,-q,-z,max-page-size=0x10000` to leave room for SCE relocation
metadata. ELF→VELF→unsafe homebrew SELF→VPK packaging must all succeed. A
successful ELF link alone does not prove a valid SELF/package. Output remains
960×544; the 16 MiB vglInitExtended argument is an allocation threshold, not a
total GPU-memory cap. Its return describes resolution fallback, not success.

## LiveArea packaging

Both Vita CMake targets now use the same fail-closed LiveArea pipeline. Approved
artwork is stored as base64 transport text under `assets/livearea/encoded/`;
`cmake/LiveArea.cmake` runs `tools/decode_livearea_assets.py` during configure,
reconstructs the exact PNG bytes into the build directory and rejects a SHA-256,
dimension, indexed-PNG, transparency or size mismatch before packaging.

The full-engine VPK maps the validated files to `sce_sys/icon0.png`,
`sce_sys/pic0.png`, `sce_sys/livearea/contents/bg0.png`,
`startup.png` and the verified style-`a1` `template.xml`.
After building, run:

```sh
python3 tools/validate_livearea_vpk.py /private/build-vita/DBTapBattle-Vita-00.23.vpk
```

The 2026-10-05 native smoke run `37384814624` passes complete
ELF→VELF→SELF→VPK packaging plus exact LiveArea validation. Its VPK uses a
non-commercial dummy TeaVM main, so it proves the packaging path rather than game
execution.

For the already hardware-tested gameplay binary, a separate LiveArea-only repack is
available as `DBTapBattle-Vita-00.23-LiveArea.vpk`, SHA-256
`382927c8032fda1db5ec21078a006026daa78ef93f3bdfce7c8484f880fd4e50`.
Every original entry from the tested 00.23 VPK is byte-identical; only the five
presentation files above were added. See
[evidence](evidence/vita_livearea_00.23.json) and
[device test instructions](TEST_VITA_00_23_LIVEAREA.md). Physical installation and
LiveArea appearance remain pending.

## Data preparation and installation

```sh
# First original APK alone: 57 base resources, no character triplets.
python3 tools/extract_apk_data.py /private/DBTapBattle.apk /private/install/game
# Or use the ordinary populated supplied Gen dataset as your base.
python3 tools/extract_apk_data.py /private/gen.apk /private/install-gen/game
# Audited Community14, isolated from base data.
python3 tools/extract_apk_data.py /private/community.apk /private/install --mod Android14
# Optional private two-source ZIP; preserves dataset payloads, excludes Android binaries.
python3 tools/prepare_vita_data.py /private/DBTapBattle.apk \
  /private/community.apk /private/DBTapBattle-data.zip
```

Both routes preserve a save when the selected APK dataset supplies one. The
validated original/Community14 ZIP has no bundled save because those two source
datasets supply none; this is not a save-exclusion policy. Choose the intended
initial save and back up existing profile progress before copying. Do not use --overwrite over your only working copy. Extraction
preflights/stages/CRC-checks but publication into an existing directory is not
an all-or-nothing transaction. See [DATA_LAYOUT](DATA_LAYOUT.md).

Install the VPK over the existing application and copy your dataset under
`ux0:data/DBTapBattle/`; APKs and Android .so/DEX are not runtime files. Do not
replace data or saves merely to update a port VPK. Symbols ZIP is diagnostic
and is not installed. Front touch is the tested in-game control.

## Verify and test

Follow [VALIDATION](VALIDATION.md) for host probes and artifact checks. The
gameplay checkpoint remains [TEST_VITA_00_23](TEST_VITA_00_23.md); the immediate
presentation check is [TEST_VITA_00_23_LIVEAREA](TEST_VITA_00_23_LIVEAREA.md).
Keep exact VPK SHA, SFO, build source, profile provenance, `runtime.log` and any
`psp2core` together. The base hardware-tested 00.23 VPK has eboot, param.sfo and
three notice entries; the LiveArea derivative adds only five `sce_sys`
presentation entries and still contains no asset dataset. The generated original
code is still commercial engine code; absence of game data inside the VPK is not an
assertion of an all-open-source executable.

Old bootstrap build evidence at 2c3fecd remains historical in
[build_validation.json](evidence/build_validation.json). It is not the current
full-engine artifact. [THIRD_PARTY](THIRD_PARTY.md) documents notices and the
limits of the current symbols bundle.

<!-- DBTB_00_23_DETAIL:START -->
## 00.23 hardware-tested package note

The source checkpoint `0e17b0bac33c47698b414b67a839c839f0e555ce` introduces the streaming PAC bridge used by the
successful 00.23 hardware test. The tested package is
`DBTapBattle-Vita-00.23-battle-memory-test.vpk`, SHA-256 `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`.

That package validated the runtime repair on hardware. It should be distinguished
from a future release-quality reproducible package: the interactive build session
used split compilation of TeaVM generated C to fit the build runner, with the large
`TCBManajer.c` translation unit compiled at `-O0` while the rest retained the normal
build settings. Do not infer final performance characteristics from that packaging
exception. The source/runtime fix itself is the 00.23 checkpoint.

For installation, update the VPK without deleting `ux0:data/DBTapBattle/` or saves.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## Corrected LiveArea repack — 2026-10-05

Use the real, hardware-tested full engine as the base. A CI native link probe is
not a playable engine even when its filename/version match. The checked repack
requires the expected base archive hash and preserves the original executable,
SFO and notices. For this 00.23 checkpoint:

```sh
python3 tools/repack_livearea_vpk.py \
  --base-vpk /path/to/DBTapBattle-Vita-00.23-battle-memory-test.vpk \
  --expected-base-sha256 8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd \
  --output /path/to/DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk
```

`vita-pack-vpk` must be on PATH, or pass `--packer /absolute/path/vita-pack-vpk`.
The wrapper reconstructs approved assets, calls the SDK packer and checks every
non-presentation base entry. Exact archive hashes can vary with packer timestamps;
entry identity and validation are the reproducible contract. See
[LiveArea asset contract](../assets/livearea/README.md) and
[corrected package evidence](evidence/vita_livearea_fixed_00.23.json).
