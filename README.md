# Dragon Ball Tap Battle PS Vita

Native PS Vita port of Dragon Ball Tap Battle using VitaSDK, vitaGL and the
original Java game core generated privately to C with TeaVM. The port replaces
Android services while preserving the original task, drawing and combat logic.
APK-derived JAR/classes/C and original game assets are not committed to Git.

## Current state — 2026-10-05

Latest delivered full-engine test: **00.21**, compiled from `07222bb`.
Implementation: `2e71d51`. It restores the audio thread priority used by working
00.19 and adds exact setup-error diagnostics and cleanup. **00.21 hardware
startup/menu recovery is still pending.** Later documentation commits do not
change the delivered binary's source identity.

| Tested version | Confirmed result | Remaining limitation |
|---|---|---|
| 00.11 | Android14 profile reaches character selection and a real battle on Vita | Does not certify arbitrary mods or every mode |
| 00.16 | User confirms ability-card processing and long startup delay fixed | Character pauses, rough voices and then 35–45 FPS in battles |
| 00.18 | User and log confirm steady battle 60 FPS / 59.9 logged at 960×544 | Text disappeared; voices and selection pauses remained |
| 00.19 | User confirms text visible again | Voices still bad; character switching still stalls |
| 00.20 | Host PAC/cache/DSP tests pass | Hardware audio worker fails setup; Original exits before menu |
| 00.21 | Full-engine VPK and setup/DSP/resource tests pass | Physical recovery, voice quality and selection latency need testing |

Use [current status and evidence](docs/CURRENT_STATUS.md) for the authoritative
feature matrix, artifact hash and open issues. Older test reports describe their
own builds; a host or CI result does not establish physical Vita behavior.

## Install and data

Install the delivered `DBTapBattle-Vita-00.21-audio-startup-fix.vpk` over the
existing application with VitaShell, preserving `ux0:data/DBTapBattle/` and saves.
Follow [00.21 test instructions](docs/TEST_VITA_00_21.md). The full engine requires
the vitaGL shader compiler setup; see [build/setup](docs/BUILD.md).

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
| Test versions | [00.03](docs/TEST_FULL_ENGINE_00_03.md), [00.13](docs/TEST_VITA_00_13.md), [00.18](docs/TEST_VITA_00_18.md), [00.19](docs/TEST_VITA_00_19.md), [00.20](docs/TEST_VITA_00_20.md), [00.21](docs/TEST_VITA_00_21.md) |
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
