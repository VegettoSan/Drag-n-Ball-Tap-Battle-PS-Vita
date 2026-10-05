# Build and install — full engine 00.21

Current build evidence: [00.21](evidence/vita_audio_startup_build_00.21.json).
Read [CURRENT_STATUS](CURRENT_STATUS.md) before interpreting build success as
hardware success. Commands below run from the repository root; keep private
inputs/outputs outside it.

## Choose the correct target

| Target | Inputs / output | Purpose |
|---|---|---|
| `tools/aot/engine/vita` | APK-derived JAR → current adapters → generated all.c → `DBTapBattle-Vita-00.21.vpk` | Full original game engine |
| Root `CMakeLists.txt` | Native atlas preview → `dbtb_vita.vpk` | Historical bootstrap; not the game |
| `.github/workflows/vita-engine-native-smoke.yml` | Tiny non-commercial all.c + real native services | Compile/link/package smoke; no game execution |
| `tools/aot` input probe | Original KeyData/Controller only | JVM/C feasibility comparison; not the full runtime |

The delivered VPK is renamed `DBTapBattle-Vita-00.21-audio-startup-fix.vpk` after
verification. Its embedded source is `07222bb`; a fresh build at a later main
commit embeds that later commit. Check the VITA_VERSION in the full target.

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
mvn -f tools/aot/pom.xml org.apache.maven.plugins:maven-dependency-plugin:3.8.1:copy-dependencies -DincludeScope=runtime -DoutputDirectory=/private/lib
python3 tools/aot/engine/generate.py   --original-jar /private/original.jar   --ecj /tools/ecj-3.37.0.jar   --lib-directory /private/lib   --work-directory /private/engine-fresh
python3 tools/aot/engine/vita/patch_runtime.py /private/engine-fresh/c
```

Generation checks the expected loader/string/static-field shapes and fails on
unexpected inputs. It preserves original GameData byte-array parsing and core
methods. 00.21 generated 465 classes / 4059 methods; this is evidence for the
pinned recipe, not a required count for an intentionally changed engine.

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
cmake -S tools/aot/engine/vita -B /private/build-vita   -DCMAKE_BUILD_TYPE=Release -DTEAVM_C_DIR=/private/engine-fresh/c
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

## Data preparation and installation

```sh
# First original APK alone: 57 base resources, no character triplets.
python3 tools/extract_apk_data.py /private/DBTapBattle.apk /private/install/game
# Or use the ordinary complete supplied Gen dataset as your base.
python3 tools/extract_apk_data.py /private/gen.apk /private/install-gen/game
# Audited Community14, isolated from base data.
python3 tools/extract_apk_data.py /private/community.apk /private/install --mod Android14
# Optional private two-source ZIP; preserves dataset payloads, excludes Android binaries.
python3 tools/prepare_vita_data.py /private/DBTapBattle.apk   /private/community.apk /private/DBTapBattle-data.zip
```

Both routes preserve a save when the selected APK dataset supplies one. The
validated original/Community14 ZIP has no bundled save because those two source
datasets supply none; this is not a save-exclusion policy. Choose the intended
initial save and back up existing profile progress
before copying. Do not use --overwrite over your only working copy. Extraction
preflights/stages/CRC-checks but publication into an existing directory is not
an all-or-nothing transaction. See [DATA_LAYOUT](DATA_LAYOUT.md).

Install the VPK over the existing application and copy your dataset under
`ux0:data/DBTapBattle/`; APKs and Android .so/DEX are not runtime files. Do not
replace data or saves merely to update a port VPK. Symbols ZIP is diagnostic
and is not installed. Front touch is the tested in-game control.

## Verify and test

Follow [VALIDATION](VALIDATION.md) for host probes and artifact checks, then
[TEST_VITA_00_21](TEST_VITA_00_21.md) on the device. Keep exact VPK SHA, SFO,
build source, profile provenance, runtime.log and any psp2core together.
Current VPK has eboot, param.sfo and three notice entries, no asset dataset.
The generated original code is still commercial engine code; absence of data
inside the VPK is not an assertion of an all-open-source executable.

Old bootstrap build evidence at 2c3fecd remains historical in
[build_validation.json](evidence/build_validation.json). It is not the current
full-engine artifact. [THIRD_PARTY](THIRD_PARTY.md) documents notices and the
limits of the current symbols bundle.
