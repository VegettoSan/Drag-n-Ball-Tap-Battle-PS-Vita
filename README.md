# Dragon Ball Tap Battle PS Vita

Current hardware checkpoint: **DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk** fixes
native PCM allocation growth at Android14 battle start and preserves the now
hardware-confirmed LiveArea. Full build, host tests and the user’s Vita retest pass. See [00.24 result](docs/TEST_VITA_00_24.md).

Latest LiveArea test: **DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk**. It preserves
the hardware-tested 00.23 engine and fixes the splash palette. The previous
LiveArea-Final VPK contained a non-playable CI probe and must be discarded.
See [current status](docs/CURRENT_STATUS.md) and
[corrected device test](docs/TEST_VITA_00_23_LIVEAREA_FIXED.md).

<!-- DBTB_00_23_DETAIL:START -->
## Historical PS Vita hardware status — 00.23

The earlier validated checkpoint was **00.23**. A real-hardware test on 2026-10-05
reported the game working normally with no error observed in that session: the
previously repaired audio remained clean, character selection remained responsive,
and the game successfully entered and played a fight instead of crashing during
character PAC loading. The tested VPK SHA-256 is `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd` and its source
checkpoint is `0e17b0bac33c47698b414b67a839c839f0e555ce`.

The 00.22 battle-start crash remains documented as a historical failure. 00.23 fixes
that specific regression by preserving the original streaming `GameData.Init`
parser and replacing only Android resource opening with a Vita-backed native stream,
avoiding the multi-megabyte managed bridge allocation that exhausted TeaVM memory.
<!-- DBTB_00_23_DETAIL:END -->


Native PS Vita port of Dragon Ball Tap Battle using VitaSDK, vitaGL and the
original Java game core generated privately to C with TeaVM. The port replaces
Android services while preserving the original task, drawing and combat logic.
APK-derived JAR/classes/C and original game assets are not committed to Git.

## Current state — 2026-10-06

Earlier hardware gameplay checkpoint: **00.23** from source `0e17b0ba`. On a real
PS Vita the reported test path preserves visible text, clean audio/voices and
responsive character selection, then enters and plays a battle without the 00.22
managed-memory crash. The hardware-tested gameplay VPK SHA-256 is
`8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`.

A presentation-only repack,
`DBTapBattle-Vita-00.23-LiveArea.vpk` (SHA-256
`382927c8032fda1db5ec21078a006026daa78ef93f3bdfce7c8484f880fd4e50`),
adds the approved PS Vita bubble/LiveArea artwork while retaining every original
00.23 VPK entry byte-for-byte, including the already tested `eboot.bin`.
Its package structure and exact PNG hashes pass VitaSDK CI; physical installation
and LiveArea appearance are the remaining check for that repack.

| Tested version | Confirmed result | Remaining limitation |
|---|---|---|
| 00.11 | Android14 profile reaches character selection and a real battle on Vita | Does not certify arbitrary mods or every mode |
| 00.16 | User confirms ability-card processing and long startup delay fixed | Character pauses and rough voices remained |
| 00.18 | User and log confirm steady battle 60 FPS / 59.9 logged at 960×544 | Text disappeared; voices and selection pauses remained |
| 00.19 | User confirms text visible again | Voices still bad; character switching still stalls |
| 00.20 | Host PAC/cache/DSP tests pass | Hardware audio worker fails setup; Original exits before menu |
| 00.21 | Worker starts and user reaches menu on Vita | Character selection rejects char00 mask 187 |
| 00.22 | Original masks 187/251 accepted; clean audio/selection reported | Battle startup exposes whole-PAC TeaVM managed allocation failure |
| 00.23 | Physical Vita: text/audio/selection/battle path passes; no error observed in reported session | Broader regression matrix and release-quality normal build remain open |
| 00.23 LiveArea-Fixed | User confirms presentation on physical Vita; tested 00.23 eboot unchanged | Android14 battle-start native Ogg allocation crash reported |
| 00.24 | Full original-engine build; all 17 BGM PCM/low-allocation tests and ownership probes pass | Broader mode/profile and long-session coverage remains open |

Use [current status and evidence](docs/CURRENT_STATUS.md) for the authoritative
feature matrix, artifact hash and open issues. Older test reports describe their
own builds; a host or CI result does not establish physical Vita behavior.

## Install and data

For the current runtime retest, install
`DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` over the existing application with
VitaShell, preserving `ux0:data/DBTapBattle/` and saves. Follow the
[00.24 device test](docs/TEST_VITA_00_24.md). The package has the corrected full
engine and the exact LiveArea files confirmed by the user in 00.23. The full
engine requires the vitaGL shader compiler setup; see
[build/setup](docs/BUILD.md).

| Runtime path | Purpose |
|---|---|
| `ux0:data/DBTapBattle/game/` | Base dataset; the selector calls this Original |
| `ux0:data/DBTapBattle/mods/<Profile>/` | File overrides or an alternate dataset |
| `game/save.bin` or `mods/<Profile>/save.bin` | Independent active-profile save |
| `ux0:data/DBTapBattle/logs/runtime.log` | Appended boot, resource, text, audio and frame diagnostics |
| `config/`, `saves/` | Created for compatibility; not the current gameplay save location |

Original names are resolved through a file overlay: selected mod first, then
base `game/`. An existing corrupt override reports an error. Saves never fall
back across profiles. `mod.json` is optional and currently ignored.

The first supplied original APK has **57 raw resources and no character
triplets**. It is not a complete battle installation. Supplied Android14 has
144 encoded assets and 13 indexed triplets; supplied `gen.apk` has 147 ordinary
assets including those triplets and an optional bundled save. Original in the
selector means the base folder, not proof of which APK supplied its contents.

For Windows 10/11, use the [portable drag-and-drop tool](tools/windows/LEEME.txt):
extract its ZIP, keep the BAT and PS1 together, and drag one or several APKs onto
`Extraer_APK_para_Vita.bat`. It creates a new `Listo_para_Vita/Paquete_*/data/`
ready to copy to the `ux0:` root. It keeps Original, Android14 and Gen separate,
normalizes confirmed Community14 names without changing bytes, and records
source/file hashes. No Python or 7-Zip is required. Preserve your existing
profile-local `save.bin` when copying an update. See [Windows tool details and
verification](docs/WINDOWS_DATA_TOOL.md).

```sh
# Run from this repository, using private output outside tracked source.
python3 tools/extract_apk_data.py /private/gen.apk /private/install/game
python3 tools/extract_apk_data.py /private/community.apk /private/install --mod Android14
```

Copy `install/game/` and `install/mods/` under `ux0:data/DBTapBattle/`. Keep existing
saves/backups before importing a dataset. The extractor preserves supplied
payload bytes; runtime codecs normalize only the confirmed formats in memory.
See [data layout](docs/DATA_LAYOUT.md) and [mod compatibility](docs/MODS.md).

## Development and documentation

The **full game target** is `tools/aot/engine/vita/CMakeLists.txt` with freshly
generated private TeaVM C. Root `CMakeLists.txt` is the older atlas bootstrap;
GitHub's native smoke target uses a tiny replacement main. Neither produces the
full gameplay engine. [BUILD.md](docs/BUILD.md) gives the current recipe.

| Topic | Documents |
|---|---|
| Status, next work, rules | [CURRENT_STATUS](docs/CURRENT_STATUS.md), [AUDIT_STATUS](docs/AUDIT_STATUS.md), [PORTING_PLAN](docs/PORTING_PLAN.md), [PROJECT_RULES](docs/PROJECT_RULES.md) |
| Reproduce and validate | [BUILD](docs/BUILD.md), [VALIDATION](docs/VALIDATION.md), [full-engine AOT](tools/aot/engine/README.md) |
| Architecture and future ports | [ENGINE_MAP](docs/ENGINE_MAP.md), [PLATFORM_SERVICES](docs/PLATFORM_SERVICES.md), [RENDER_MAPPING](docs/RENDER_MAPPING.md), [PORTING_GUIDE](docs/PORTING_GUIDE.md), [DECISIONS](docs/DECISIONS.md) |
| Data, formats and provenance | [APK_AUDIT](docs/APK_AUDIT.md), [ANDROID14_APK](docs/ANDROID14_APK.md), [Original+Characters](docs/ORIGINAL_PLUS_CHARACTERS_APK.md), [PAC_FORMAT](docs/PAC_FORMAT.md), [RESOURCE_FORMATS](docs/RESOURCE_FORMATS.md), [DATA_LAYOUT](docs/DATA_LAYOUT.md), [MODS](docs/MODS.md) |
| Engineering history | [ATTEMPTS](docs/ATTEMPTS.md), [SUCCESSES](docs/SUCCESSES.md), [FAILURES](docs/FAILURES.md), [input AOT experiment](tools/aot/README.md) |
| Test versions | [00.03](docs/TEST_FULL_ENGINE_00_03.md), [00.13](docs/TEST_VITA_00_13.md), [00.18](docs/TEST_VITA_00_18.md), [00.19](docs/TEST_VITA_00_19.md), [00.20](docs/TEST_VITA_00_20.md), [00.21](docs/TEST_VITA_00_21.md), [00.22](docs/TEST_VITA_00_22.md), [00.23](docs/TEST_VITA_00_23.md), [00.23 LiveArea](docs/TEST_VITA_00_23_LIVEAREA.md) |
| Attribution | [THIRD_PARTY](docs/THIRD_PARTY.md) and upstream license files in `licenses/` |

`src/` contains native data/input/UI utilities; `tools/aot/engine/java/` contains
handwritten Android replacements; `tools/aot/engine/native/` contains the C ABI
and Vita services. `tests/` and `tools/aot/engine/tests/` contain host probes.
`docs/evidence/` contains hashes and non-commercial result summaries.

Work directly on `main` in small functional commits. Record failed approaches
and actual verification scope. Preserve original gameplay; do not replace its
methods with guessed menus/combat. Front touch is the tested gameplay input;
physical gameplay mappings, multiplayer and comprehensive save interoperability
remain open. No stable-public-release claim is made for this test build.

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
