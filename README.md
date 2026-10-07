# Dragon Ball Tap Battle PS Vita

## Current hardware checkpoint — 00.33 (2026-10-07)

**DBTapBattle-Vita-00.33-Invasion-Saitama-Freezer-Fix.vpk** is the current
hardware-confirmed development checkpoint.

Physical Vita testing confirms, in the tested paths, that the Loading regression
is fixed, mod rosters expand to the installed dataset (including Samu's 92
characters), Invasion text remains readable, Samu's direct compressed audio
survives the title/menu transition, and the reproduced Invasion Saitama ->
second fight vs Freezer crash no longer occurs after several consecutive fights.

The final Invasion failure was a native protected-PAC allocation bug in the Vita
adapter, not corrupt character data. 00.33 explicitly transfers the normalized
PAC buffer with `output.swap(out)`, eliminating a duplicate ~4.6 MiB allocation
identified by symbolizing the 00.32 coredump against its exact ELF.

Resource resolution remains profile-local with no fallback to `game/`. Each
profile keeps its own writable `save.bin`, seeded once from the VPK master save.
The runtime supports audited character IDs 00..99 and exposes only characters
actually present in the selected dataset.

Current artifact:

- VPK SHA-256: `d241499a356ac11c523909a84b0c383910ef7a387efcfdc2c05d3581be86fd77`
- eboot SHA-256: `bc0a0d4293e5b416084d02050dd6b3c17529cc63fab00bfe7a48d0310303d43b`
- ELF SHA-256: `6c55a58f59277bee0d2632dbefca8c1a938d577457b867489d3605a11cee7dbe`
- APP_VER `00.33`, TITLE_ID `DBTB00001`
- LiveArea: PASS

See [current status](docs/CURRENT_STATUS.md),
[00.33 device result](docs/TEST_VITA_00_33.md) and
[hardware evidence](docs/evidence/vita_hardware_00.33.json).

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
| 00.33 | Physical Vita: repeated Invasion fights pass after protected-PAC ownership fix; Loading and dynamic rosters also remain fixed | Broader modes/mods/very-long-session coverage still open |

Use [current status and evidence](docs/CURRENT_STATUS.md) for the authoritative
feature matrix, artifact hash and open issues. Older test reports describe their
own builds; a host or CI result does not establish physical Vita behavior.

## Install and data

For the current runtime checkpoint, install
`DBTapBattle-Vita-00.33-Invasion-Saitama-Freezer-Fix.vpk` over the existing
application with VitaShell while preserving `ux0:data/DBTapBattle/` and all
profile saves. The full engine still requires the VitaGL shader compiler setup;
see [build/setup](docs/BUILD.md).

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
384 canonical assets and 92 character triplets. 00.33 consumes its original Vorbis/MP3/AAC BGM bytes directly. Hardware testing confirms the full Samu roster is exposed; exhaustive audio/character combinations remain broader coverage rather than a blocker. Original in the selector means the
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
> **Current hardware checkpoint — 00.33 (2026-10-07):** physical Vita testing
> confirms the reproduced Invasion repeated-fight/Saitama→Freezer crash is fixed
> after the protected-PAC ownership-transfer repair. The recent hardware sequence
> also confirms Loading recovery and dynamic installed rosters, including Samu's
> 92 characters. Scope is limited to tested paths; see [CURRENT_STATUS](docs/CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
