# Dragon Ball Tap Battle PS Vita

Development candidate **00.31** is the direct response to the 00.30
physical-Vita report. 00.30 proved Samu now boots, its direct compressed audio
works, and Invasion's earlier text corruption was not reproduced; it also exposed
four remaining issues: only the first 13 Samu characters were visible, Invasion
hit `std::bad_alloc` when starting a later fight, Shop threw the unsupported
Android-marketplace exception, and the obsolete online-check state waited around
10–25 seconds.

00.31 keeps standalone datasets, direct MP3/AAC/Vorbis, the Invasion text fix and
independent per-profile saves. It additionally derives character availability from
the audited profile roster (Samu = 92), extends only the verified character arrays
and loops needed for IDs 00..99, reduces multi-MiB PAC cache peaks between
battles, returns cleanly from the unavailable Android Shop service, and matches
the original Gen offline Downloader stub so the catalog/update state can finish
immediately.

Current hardware checkpoint: **DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk** fixes
native PCM allocation growth at Android14 battle start and preserves the now
hardware-confirmed LiveArea. Full build, host tests and the user’s Vita retest pass. See [00.24 result](docs/TEST_VITA_00_24.md).

Development candidate **00.33** keeps the standalone-profile and
independent-save model while targeting the last reproduced Invasion battle
crash. Physical 00.32 testing confirms the Loading loop is fixed and extended
mod rosters now show correctly, including Samu's full roster. The remaining
repro is Invasion Saitama -> second fight vs Freezer.

The supplied 00.32 Vita coredump, symbolicated against its exact ELF, resolves
the native `std::bad_alloc` through `std::vector<unsigned char>::operator=`
inside protected-PAC normalization. Invasion `char15.pac` expands from about
4.05 MiB to 4.64 MiB; GCC 15 turned the previous conditional move expression
into another vector copy. 00.33 explicitly transfers the changed buffer with
`output.swap(out)`, eliminating that duplicate PAC-sized allocation without
altering source PAC bytes, DEX or gameplay logic. Physical confirmation remains
pending; see [00.33 test](docs/TEST_VITA_00_33.md).

Development candidate **00.30** keeps the 00.28 standalone-data architecture and
adds the fixes discovered by the user's physical Vita test. `game/` remains only
the optional Original profile: selecting `mods/<Profile>/` resolves PAC/audio/data
exclusively from that directory, so Gen, Android14, Español, Invasion or
Zuper/Samu can be installed without Original.

The 00.28 hardware test established that **Invasion runs standalone on Vita with
its textures and direct Vorbis/MP3/AAC audio working**. Its remaining observed
issue was corrupted post-battle/result text. Samu also proved that the 92-character
dataset is detected standalone, but it exited after the title because the Vita
audio backend tried to create a second MP3 decoder before releasing the previous
one. 00.30 retains the 00.29 fix that serializes that decoder handoff and adds a data-driven UTF-8 character
charset fallback for Android14/Invasion SetString slot differences. These fixes
are build/CI validated and await the next physical retest.

00.30 refines save ownership: the VPK still contains the exact
user-provided 12,906-byte `app0:/save.bin` seed, but each selected dataset gets
its **own writable copy**. Original uses `game/save.bin`; a mod uses
`mods/<Profile>/save.bin`. The seed is copied only when that profile has no save
yet, so later launches never erase its progress. APK-local saves are still not
installed by the extractors; every profile starts from the same known VPK seed.

Manual full-game publication: [Release](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/workflows/vita-release.yml)
or [Prerelease](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/workflows/vita-prerelease.yml).
Both compile the original engine and publish validated VPK/symbols/hashes to
Releases. Configure the private `DBTB_ORIGINAL_APK_URL` secret once; see
[setup and usage](docs/RELEASE_WORKFLOWS.md).

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
| 00.24 | Full original-engine build; all 17 BGM PCM/low-allocation tests and ownership probes pass; user confirms hardware fix | Broader mode/profile and long-session coverage remains open |
| 00.28 | Physical Vita: Invasion standalone gameplay/textures/direct audio work; Samu standalone detects 92 characters but exits during MP3 BGM transition | Invasion result text corrupt; Samu MP3 decoder handoff fails |
| 00.29 | CI/native build: Samu decoder handoff + Invasion charset fallback + experimental global save | Superseded save model |
| 00.30 | Same Samu/Invasion fixes; VPK seed copied independently to each selected profile on first use | Superseded by later hardware candidates |
| 00.31 | Dynamic roster, Shop return and memory/startup experiments | Introduced infinite Loading regression |
| 00.32 | Physical Vita: Loading fixed and complete mod rosters visible | Invasion Saitama second fight vs Freezer crashes with native `std::bad_alloc` |
| 00.33 | Coredump-driven protected-PAC ownership fix removes duplicate ~4.6 MiB copy | Physical Saitama -> Freezer retest pending |

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
| `ux0:data/DBTapBattle/mods/<Profile>/` | Independent selectable APK-derived dataset |
| `game/save.bin` or `mods/<Profile>/save.bin` | Independent writable save, seeded once from the VPK for that profile |
| `ux0:data/DBTapBattle/logs/runtime.log` | Appended boot, resource, text, audio and frame diagnostics |
| `config/`, `saves/` | Compatibility/legacy directories; not the active gameplay save location |

Resource resolution is profile-local. With a profile selected, only
`mods/<Profile>/` is read; missing files do **not** fall back to `game/`. With
Original selected, only `game/` is read. Gameplay resources and save progress are profile-local in 00.33. Every profile starts from the same VPK seed but then modifies only its own `save.bin`. `mod.json` is optional and currently ignored.

The first supplied original APK has **57 raw resources and no character
triplets**. It is not a complete battle installation. Supplied Android14 has
144 encoded assets and 13 indexed triplets; supplied `gen.apk` has 147 ordinary
assets including those triplets and an optional bundled save. The audited
`DragonBallZuperSamuGamerYT.apk` is Gen-derived with the same DEX/manifest but
384 canonical assets and 92 character triplets. 00.33 can consume its original
Vorbis/MP3/AAC BGM bytes directly, but the complete 92-character/audio matrix is
not yet a hardware claim. Original in the selector means the
base folder, not proof of which APK supplied its contents.

For Windows 10/11, use the [portable drag-and-drop tool](tools/windows/LEEME.txt):
extract its ZIP, keep the BAT and PS1 together, and drag one or several APKs onto
`Extraer_APK_para_Vita.bat`. It creates a new `Listo_para_Vita/Paquete_*/data/`
ready to copy to the `ux0:` root. It keeps Original, Gen, Android14, Español and Invasion profiles separate,
normalizes only audited protected aliases without changing source bytes, and
records source/file hashes. No Python or 7-Zip is required. APK-local saves are reported but not installed; preserve each profile's existing `save.bin` when copying an update. See [Windows tool details and
verification](docs/WINDOWS_DATA_TOOL.md).

```sh
# Run from this repository, using private output outside tracked source.
python3 tools/extract_apk_data.py /private/gen.apk /private/install --mod Gen
python3 tools/extract_apk_data.py /private/community.apk /private/install --mod Android14
# Samu: validates the audited identity and preserves every asset byte-for-byte.
python3 tools/prepare_samu_mod.py /private/DragonBallZuperSamuGamerYT.apk /private/install/mods/ZuperSamu
```

Copy the profile directories under `ux0:data/DBTapBattle/mods/`. Install `game/` only if you also want the optional Original profile. Keep backups of the profile-local `game/save.bin` and `mods/<Profile>/save.bin` files before destructive maintenance. The extractor preserves supplied
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
| Data, formats and provenance | [APK technical reference](docs/APK_TECHNICAL_REFERENCE.md), [canonical APK differences](docs/APK_CANONICAL_DIFFERENCES.md), [APK_AUDIT](docs/APK_AUDIT.md), [ANDROID14_APK](docs/ANDROID14_APK.md), [Español](docs/SPANISH_ANDROID14_APK.md), [Invasion Beta 3](docs/INVASION_BETA3_APK.md), [Zuper/SamuGamerYT](docs/DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md), [Original+Characters](docs/ORIGINAL_PLUS_CHARACTERS_APK.md), [audio matrix](docs/evidence/APK_AUDIO_MATRIX_2026-10-06.md), [PAC_FORMAT](docs/PAC_FORMAT.md), [RESOURCE_FORMATS](docs/RESOURCE_FORMATS.md), [DATA_LAYOUT](docs/DATA_LAYOUT.md), [MODS](docs/MODS.md) |
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
