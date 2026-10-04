# Build and first test — bootstrap 00.02

## Confirmed compilation

A real VitaSDK GCC 15.2.0 **hard-float** build has compiled, linked and packaged
this bootstrap. Installed package set: vitasdk-core 2026.08.1-1; vitaGL
0.0.0.r1488.g2bdbe89-1; libpng 1.6.58-1; zlib 1.3.2-2; libmathneon
0.0.0.r11.g0faab81-1; vitaShaRK 1.7-1; SceShaccCgExt 1.0.1-1; taihen 0.11-1.
Final source commit/artifact hashes are recorded in evidence/build_validation.json.
Do not mix soft-float libraries or a different renderer ABI into this build.

Use official VitaSDK installation/package guidance at https://vitasdk.org/.
A complete release channel is preferable to a partially rebuilt nightly package
set. Install the matching core before renderer dependencies. Package names and
link archive names differ (e.g. vitaShaRK installs libvitashark.a).

```sh
export VITASDK=/your/vitasdk
export PATH="$VITASDK/bin:$PATH"
# Install through the supported channel's package manager as documented there:
vdpm install vitaGL libpng zlib libmathneon vitaShaRK SceShaccCgExt taihen
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

CMake also accepts -DVITASDK or an explicit toolchain file. Dependencies must
come from the same target ABI. CMake uses C++14, no exceptions/RTTI, warning flags,
64-MiB application heap, standard 960×544 output, and no legacy vitaGL pool.
The vglInitExtended fourth argument is a RAM allocation threshold, **not** a
hard total GPU-memory cap. Native SELF is created with homebrew unsafe permission
(-s equivalent) required by this graphics/runtime stack; it is not signed retail.

Artifact: build/dbtb_vita.vpk. Title ID: DBTB00001. Version: 00.02.
No commercial data is packaged. Native code source and relinking files are
available alongside the test deliverable; see THIRD_PARTY.md.

## Data preparation

```sh
python tools/extract_apk_data.py /path/to/DBTapBattle.apk ./original-data
```

Copy the complete output directory contents to ux0:data/DBTapBattle/game/.
The manifest includes per-file exact hashes and unknown raw formats. Extraction
preflights conflicts (including manifest), rejects traversal/duplicates/symlinks,
stages and CRC-checks data before publication. --overwrite permits replacement
of regular files. Disk/concurrent publication failure into an existing output
is not an all-or-nothing transaction; preserve a backup when using overwrite.
For community assets, the extractor now supports explicit/audited layout selection;
see ANDROID14_APK.md. External Android folders are not silently imported.

This supplied APK alone is sufficient for common.pac preview, **not battle**:
character triplets are absent. Read PLATFORM_SERVICES.md before calling it a
complete game installation.

Optional mod: place replacement files inside
ux0:data/DBTapBattle/mods/MyMod/. Missing files fall back to game/. A mod need
not duplicate all 57 files. mod.json is optional and currently ignored.

## Expected test sequence (not yet observed on device)

Current source additionally initializes gamedata.pac/text00.pac before common
preview; both are now required. Their converted table directories accept ordinary
and audited community profiles per resolved file. Failure displays GAME DATA and
logs the error (exit 8); success logs both paths and record counts. This change
is host-tested and has not received a new Vita build or device run here.

1. vitaGL initializes; selector displays Original and discovered folders.
2. D-pad/left stick moves; Cross or a front-screen tap confirms a visible row.
   Circle/Triangle cancels. More than eight rows scroll using physical controls.
3. Selected overlay resolves common.pac; parser validates its table.
4. First PNG is read unchanged and decoded. For supplied original this is a
   512×512 texture atlas, shown scaled as a diagnostic resource preview.
   **It is not a reconstructed original menu or a playable game.**
5. Green PAC OK + atlas after successful upload; errors show a red diagnostic.
   Release/repress Cross, Start or Circle/Triangle to exit result.
6. ux0:data/DBTapBattle/logs/runtime.log records timestamp/version/ref, selected
   data source/path, entry count, decoded dimensions and render/input errors.

vitaGL requires libshacccg.suprx on the system according to its official setup
instructions. This module is not supplied in the VPK. No audio is expected.
Touch/UI rendering, driver/shader behavior and lifecycle are PENDING runtime
verification; a successfully packaged binary does not prove startup.

## Evidence to return

Exact VPK/build, logs/runtime.log, photo of selector and atlas, Original/mod
choice, whether front touch/D-pad/stick worked. On a failure include any Vita
crash dump; symbols/relink bundle contains the native ELF. Do not supply raw
commercial data to Git.

## Host regression commands

```sh
python -m unittest discover -s tests -p 'test_*.py' -v
mkdir -p build-host
g++ -std=c++14 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc \
  src/pac.cpp src/vfs.cpp tests/test_core.cpp -o build-host/test_core
ASAN_OPTIONS=detect_leaks=0 build-host/test_core /path/to/original-data/*.pac
# Requires host libpng development headers/library:
g++ -std=c++14 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc \
  src/image.cpp src/pac.cpp tests/test_image.cpp -lpng -lz -o build-host/test_image
ASAN_OPTIONS=detect_leaks=0 build-host/test_image /path/to/original-data/*.pac
```

Only leak scanning was disabled in this restricted runner; ASan/UBSan remained
active. The core test enumerates 258 folders, including a 255-byte name. PNG
tests reject truncated data and valid-CRC IHDRs exceeding decoded-memory budget.

## Android14 resource support after the historical 00.02 build

The new PAC/RGBA reader and alpha-aware diagnostic preview are host-tested source
changes. A VitaSDK/CMake installation is unavailable in the current session, so
no updated VPK or ARM-build claim accompanies this change. Existing
build_validation.json remains historical and must not be reused as proof of it.
zlib was already linked by CMake; no Android helper library is added.

```sh
python tools/extract_apk_data.py community.apk ./install --mod Android14
# Host libpng/zlib development headers/libraries required:
g++ -std=c++14 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc \
  src/image.cpp src/pac.cpp tests/test_community.cpp -lpng -lz -o build-host/test_community
ASAN_OPTIONS=detect_leaks=0 build-host/test_community ./original-data/*.pac ./install/mods/Android14/*.pac
```

Expected new-source device preview: original common.pac has 9 entries; community
common.pac (imported from 2752.pac) has 6 entries. Both first textures are 512×512;
logs identify the actual PAC codec and resolved folder. The community decoded
pixel data is premultiplied, so its preview uses GL_ONE/ONE_MINUS_SRC_ALPHA.
Do not expect original menus, characters, sound or battle from this bootstrap.
