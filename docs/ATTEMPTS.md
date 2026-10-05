# Attempts Log

Chronological record of meaningful experiments. Add an entry before/after each test that can teach us something.

## Entry template

```markdown
## YYYY-MM-DD — Attempt NNN — Short title

**Goal**
What are we trying to prove or fix?

**Baseline**
Commit/build used and relevant known-good state.

**Changes**
Exact files/systems changed.

**Test procedure**
Steps performed on PC/Vita/Vita3K.

**Expected**
What should happen if the hypothesis is correct?

**Observed**
What actually happened?

**Evidence**
Logs, screenshots, crash dumps, hashes, PAC names, FPS, etc.

**Result**
SUCCESS / PARTIAL / FAILURE / INCONCLUSIVE

**Next action**
What should be done next, and what must not simply be repeated?
```

---

## 2026-10-04 — Attempt 001 — Inspect original APK architecture

**Goal**
Determine whether Tap Battle is a realistic native Vita reconstruction candidate.

**Baseline**
User-supplied `DBTapBattle.apk`.

**Changes**
None; analysis only.

**Test procedure**
Inspected APK contents, native-library presence and original raw resources.

**Expected**
Identify engine/runtime dependencies and asset packaging.

**Observed**
The APK contains Java/Dalvik game code and no `lib/*.so` native game libraries. Original resources are present under `res/raw/`, including PAC and OGG files.

**Evidence**
APK content inspection.

**Result**
SUCCESS

**Next action**
Reconstruct platform layers natively with VitaSDK instead of trying to execute Android native binaries.

---

## 2026-10-04 — Attempt 002 — Validate PAC outer container

**Goal**
Determine whether original `.pac` files can be read directly on Vita.

**Baseline**
`res/raw/back00.pac` from the supplied APK.

**Changes**
None; binary format analysis only.

**Test procedure**
Parsed a 16-bit entry count followed by 16-byte entries and verified each entry against its pointed data.

**Expected**
Offsets/sizes should land on valid embedded resources.

**Observed**
`back00.pac` reports 8 entries; `data_base = 130`. The first entry resolves to a valid PNG signature and all eight entries remain within file bounds.

**Evidence**
See `PAC_FORMAT.md` for the validated table.

**Result**
SUCCESS

**Next action**
Implement the same parser in C++ and validate it on Vita using an untouched original PAC.

---

## 2026-10-04 — Attempt 003 — Validate raw APK data extraction

**Goal**
Confirm that the user-supplied APK can be turned into the external `game/` directory without converting original resources.

**Baseline**
User-supplied `DBTapBattle.apk`.

**Changes**
Added `tools/extract_apk_data.py` using ZIP extraction of `res/raw/` with SHA-256 manifest generation.

**Test procedure**
Enumerated and extracted every regular file directly under `res/raw/` using the same copy-without-conversion strategy implemented by the repository tool.

**Expected**
All original raw runtime files should be independently extractable with their original bytes preserved.

**Observed**
57 files were extracted successfully: 19 `.pac`, 36 `.ogg`, plus auxiliary resources including `loading.png` and `mk.bin`.

**Evidence**
Per-file SHA-256 values were generated during validation. Example: `back00.pac` SHA-256 `a19c425b0496aadc780d3a12b47fad91363423c9b5944407bdd5f74d64f18012`.

**Result**
SUCCESS

**Next action**
Use these untouched files as the first real Vita dataset and verify the C++ VFS/PAC reader in an actual VitaSDK build.

---

## 2026-10-04 — Attempt 004 — Bootstrap Vita build implementation

**Goal**
Create the first VitaSDK/vitaGL executable path with mod selection, VFS fallback and PAC validation.

**Baseline**
Fresh repository plus validated APK/PAC research.

**Changes**
Added CMake VitaSDK/vitaGL project, `GameVfs`, `PacFile`, vitaGL selector UI and runtime logging.

**Test procedure**
Source and build configuration were reviewed against current VitaSDK/vitaGL sample conventions. A real VitaSDK toolchain is not available in the current execution environment, so no compile or hardware claim is made yet.

**Expected**
The next environment with VitaSDK should compile the bootstrap and allow Original/mod selection before validating `common.pac`.

**Observed**
Implementation is committed; build execution remains pending.

**Evidence**
Repository source and CMake configuration.

**Result**
INCONCLUSIVE

**Next action**
Compile with VitaSDK, fix any SDK/link issues, then run on Vita/Vita3K and promote only confirmed results to `SUCCESSES.md`.

## 2026-10-04 — Attempt 005 — Independent full APK inventory

**Goal / hypothesis:** validate packaging across all PAC families, not just back00.
**Baseline:** main 1e3699b, original APK SHA-256 recorded in APK_AUDIT.md.
**Changes:** reproducible tools/audit_apk.py and metadata-only evidence.
**Procedure:** read all 77 ZIP files, validate 19 outer PACs and nested SPRs,
inspect DEX with androguard; compare GdGohan SWB at f4a275d.
**Expected:** original offsets agree across families; distinguish original from mods.
**Observed:** all outer bounds valid, reserved=0, no overlap/tail; 91 DEX classes;
no bundled charXX; community archive has added/modified platform classes.
**Evidence:** docs/evidence/apk_inventory.json; docs/APK_AUDIT.md.
**Result:** SUCCESS (FORMAT CONFIRMED only).
**Next:** harden native readers/extraction and inspect external data/game services.

## 2026-10-04 — Attempt 006 — Safe extraction and exact-byte regression

**Goal / hypothesis:** preflight paths/conflicts and stage CRC-checked data to avoid
silently replacing manifests or publishing half an invalid APK.
**Baseline:** 9e770eb; old extractor skips nested data and writes incrementally.
**Changes:** hardened extractor; tests/test_extractor.py.
**Procedure:** five unittest groups: nested UTF-8/unknown extension + hash,
traversal/duplicate/case/file-directory collisions, manifest conflict/overwrite,
symlink escape, corrupt late ZIP entry. Extract supplied APK; compare all hashes
with independent audit evidence.
**Expected / observed:** five groups pass; 57 files extracted; all SHA-256 values match.
**Evidence:** unittest output and independent manifest/inventory comparison.
**Result:** SUCCESS (host extraction only).
**Next:** test native PAC/VFS. Publication into an existing directory is not an
all-or-nothing filesystem transaction if disk failure/concurrent writes happen;
ZIP validation and ordinary preflight failures leave existing output intact.

## 2026-10-04 — Attempt 007 — Native PAC/VFS host regressions

**Goal / hypothesis:** correct safety/state weaknesses without changing original
PAC semantics or requiring a new asset container.
**Baseline:** 7213bb2.
**Changes:** component-based VFS validation, regular-file checks, explicit errors,
portable host I/O, dedicated config/logs/saves; PAC transactional entry table,
read budget, reopen/changed-length validation.
**Procedure:** compile test_core.cpp with g++ C++14, warnings-as-errors,
AddressSanitizer/UndefinedBehaviorSanitizer. Run synthetic corruption/NUL/path/
symlink/UTF-8/fallback tests, then read every entry of all 19 original PACs.
**Expected / observed:** CORE PASS; all 19 originals parsed/read successfully.
**Evidence:** tests/test_core.cpp; independent APK inventory entry counts.
**Result:** SUCCESS (host core only).
**Limit:** LeakSanitizer cannot scan /proc under this runner; disabled leak scan
only with ASAN_OPTIONS=detect_leaks=0. Address/undefined-behavior checks remain on.
**Next:** real Vita compile; GPU display remains unconfirmed.

## 2026-10-04 — Attempt 008 — First real Vita VPK and original PNG preview

**Goal / hypothesis:** close the missing PAC→entry→texture chain without recreating
an original game menu.
**Baseline:** 99df49b.
**Changes:** bounded libpng RGBA decoder; diagnostic first-PNG atlas preview;
checked vitaGL initialization/upload; timestamped logs/runtime.log; configured
heap; CMake target-scoped flags, explicit version/ref, corrected static link order.
**Procedure:** install official VitaSDK core and matching hard-float packages via
2026.08 channel; cmake configure/build. Host test_image reads all 51 exterior PNG
entries and rejects invalid/truncated PNGs under ASan/UBSan. Decode budget 16 MiB.
**Expected / observed:** native ARM ELF→VELF→SELF→VPK succeeds, no source warnings;
51 PNG PASS on host. GPU display has not been executed in this environment.
**Evidence:** actual CMake output; tests/test_image.cpp. Build provenance and final
artifact hashes will be recorded after the source commit is published/rebuilt.
**Result:** PARTIAL — BUILD CONFIRMED; GPU/hardware INCONCLUSIVE.
**Next:** verify selector and original atlas in Vita, then implement CNV/DAC metadata.

## 2026-10-04 — Attempt 009 — Platform-neutral input events and touch selector

**Goal / hypothesis:** keep Vita controls outside original gameplay logic and
support front touch without inventing attack behavior.
**Baseline:** 2c7fde7.
**Changes:** input.hpp neutral menu commands/stable pointer phases; input.cpp Vita
polling adapter; selector consumes this layer, supports front tap and left stick.
**Procedure:** real VitaSDK compile/link/package including SceTouch_stub.
**Expected / observed:** build succeeds with warnings enabled; no hardware test.
**Evidence:** native build output. Runtime touch bounds come from panel info.
**Result:** PARTIAL — BUILD CONFIRMED, input behavior PENDING hardware.
**Next:** test pointer IDs/release phases and multi-page mod selection on device;
connect pointer events to the recovered KeyData/Controller in the next phase.

## 2026-10-04 — Attempt 010 — Original engine and internal-format cross-check

**Goal:** complete the platform/format audit without turning inferred schemas
into confirmed decoders. **Baseline:** 38d156e.
**Hypothesis:** original DEX bytecode and independent JADX output can resolve
community-state discrepancies and destructive DAD decompilation omissions.
**Changes:** engine dispatch, render call mapping, audio/save/download/Bluetooth
boundaries, internal-table probe and audio probe; metadata-only evidence.
**Procedure:** compare original class methods with GdGohan snapshot, validate
all 19 PACs and candidate internal layouts, ffprobe all 36 Ogg streams.
**Expected / observed:** raw animation DAC and converted gamedata/text DAC
differ; CNV coordinates are BE; state 390 is next-rival selection/load; save
allocation is 12906 bytes. All 36 audio streams are Vorbis at 44100 Hz.
**Evidence:** ENGINE_MAP.md, RESOURCE_FORMATS.md, PLATFORM_SERVICES.md,
RENDER_MAPPING.md and evidence/internal_tables.json / audio_inventory.json.
**Result:** PARTIAL — source/container facts established; full animation,
collision, save interoperability and native platform behavior remain PENDING.
**Next:** verify bootstrap on device, then port original metadata/draw functions.

## 2026-10-04 — Attempt 011 — Final packaging and bounded-resource stress tests

**Goal:** package reproducible native output and retain useful symbols/relink
inputs. **Baseline:** ef18f65.
**Hypothesis:** the reviewed bootstrap builds with explicit C++14 and license
notices, while rejecting oversized PNG allocation and large mod-list edge cases.
**Changes:** explicit language standard/notices, ux0:data parent creation,
258-folder/255-byte-name VFS test and valid-CRC 4096-square PNG rejection test.
**Procedure:** ASan/UBSan host tests plus real release VitaSDK package build;
inspect ELF ABI, SELF header and VPK members; retain exact hashes/build log.
**Expected / observed:** host tests pass; native compilation and packaging pass.
No device or emulator is available, so selector/GPU/touch execution is untested.
**Evidence:** tests/test_core.cpp, tests/test_image.cpp and final
evidence/build_validation.json; symbols/relink archive includes the build log.
**Result:** PARTIAL — BUILD CONFIRMED; HARDWARE/VITA3K PENDING.
**Next:** run first-milestone device sequence in BUILD.md and capture runtime.log.

## 2026-10-04 — Attempt 012 — Compare original and community Android14 APK

**Baseline:** a04e264. **Goal:** identify exact packaging/code differences and
usable data before assuming ordinary PAC or generic community-mod support.
**Procedure:** inspect both ZIPs, DEX/manifest/signing subjects with androguard
4.1.4, ext.o/ext.u bytecode, ARM Thumb libabc GL upload/inflate path; compare
seven libraries with public SWB f4a275d; compare original PNG pixels using Pillow.
**Observed:** community assets profile has 106 encoded PACs; 390 RGBA textures
including nested SPR decode with bounded raw DEFLATE. 65 textures match original
PNG after floor-alpha premultiplication; 37 identical/18 changed/2 absent/89 added
logical resources. Seven native helpers identical to public SWB. Publisher and
actual Android14/Vita behavior remain unconfirmed.
**Evidence:** ANDROID14_APK.md, evidence/android14_{inventory,comparison}.json;
reproducible audit_apk.py/compare_apks.py and pinned community14.py metadata.
**Result:** SUCCESS (static/container/pixel facts); runtime INCONCLUSIVE.
**Next:** preserve encoded data, normalize only confirmed aliases at import,
and implement bounded native PAC/image decoding with correct alpha handling.

## 2026-10-04 — Attempt 013 — Non-destructive community import

**Baseline:** 8569c70. **Changes:** raw/assets/profile selection; confirmed alias
normalization; format-3 provenance; --mod destination isolation; codec validation.
FAFD uses four suffix digits (FAFD0012 → charf0012), cross-checked against the
actual 13 filenames before publishing the importer.
**Procedure:** nine Python regressions; original 57-file and community 144-file
extraction; compare every output byte/hash to the APK; verify 13 character triplets.
**Observed:** all pass; originals stay intact; alias collisions, wrong codec,
unsafe paths and ambiguous layout fail before resource publication.
**Evidence:** tests/test_extractor.py; updated comparison report; ANDROID14_APK.md.
**Result:** SUCCESS (host import). **Next:** native table/image regressions and
correct premultiplied-alpha preview; no gameplay support claim from extraction.

## 2026-10-04 — Attempt 014 — Native community tables and RGBA

**Baseline:** 861165b. **Changes:** PacEncoding per file, retained unknown metadata,
bounded raw-DEFLATE image decoder, explicit alpha state, codec-aware preview/log.
**Procedure:** g++ C++14 warnings-as-errors + ASan/UBSan; existing VFS/PAC/PNG
regressions; new metadata-first/corrupt table/DEFLATE/index/budget tests; read both
real corpora and nested SPR. Host libpng 1.6.43 source built locally for headers
and static link because this environment lacks its development package.
**Observed:** CORE PASS on all 125 outer files; original PNG PASS 51; COMMUNITY
PASS 137 total containers and 470 textures (80 original, 390 community). Synthetic
RGBA bytes/alpha match exactly; invalid data is rejected without partial images.
**Evidence:** tests/test_community.cpp, tests/test_core.cpp, tests/test_image.cpp;
evidence/android14_validation.json. **Result:** SUCCESS (native C++ host only).
**Limit:** no VitaSDK/CMake/device here; new ARM build/GPU behavior unconfirmed.
**Next:** build this source with the documented matching VitaSDK; test original
and Android14 common previews/logs. Then recover converted tables, sprite behavior,
character/menu/combat and WAV services along original engine boundaries.

## 2026-10-04 — Attempt 015 — Begin actual engine data initialization

**Baseline:** 9134e65. **Goal:** move from image preview toward the original
startup path with both APK profiles, without inventing gameplay.
**Changes:** native converted game/text table reader, per-PAC codec after VFS
resolution, original InitGameData two-resource initialization in main.
**Procedure:** check original DEX InitGameData/binCnv and community ext.u constants;
warnings-as-errors C++14, ASan/UBSan, both extracted APKs and mixed-codec overlay.
**Observed:** 271+1 records per APK; 262/39783 comparable game cells changed and
preserved. Bounds, row stride, unsigned bytes, record XOR, failed-load clearing,
ordinary fallback and corrupt override rejection pass. Initial LeakSanitizer
execution could not inspect /proc tasks; repeat disables only leak detection.
**Result:** SUCCESS (host data initialization); Vita build/execution PENDING.
**Limit:** main still ends in the diagnostic atlas. No menu/combat or new playable
VPK delivered. AOT Java-to-C investigated as a possibility, not implemented or
validated on Vita; do not describe it as an available Android runtime.
**Next:** port the original sprite/action interpreter and task/state dispatch
for menu → character selection → one complete battle, using this same database
and both existing texture/container codecs. Original-only gameplay also requires
the character resources absent from the supplied original APK.

## 2026-10-04 — Attempt 016 — Prepare private Vita data package

**Baseline:** 2c3fecd. **Changes:** prepare_vita_data.py composes the validated
raw/community importers under data/DBTapBattle/game and mods/Android14, without
conversion. Exclusive same-filesystem link publishes only a verified archive.
**Procedure:** re-extract both supplied APKs, validate all ZIP CRCs and SHA-256s;
check 57 original + 144 community files. Nine importer regressions still pass.
**Observed:** 83,321,665-byte ZIP; 205 members; unchanged payloads, two provenance
manifests, Spanish installation instructions and SHA256SUMS. ZIP SHA-256:
f1b899f8bd03821d46db72d561591908b9a38f3c84c7907762530504f84056ea.
**Result:** SUCCESS (private downloadable user data). No commercial data is
uploaded to Git or GitHub Releases. A data package does not establish gameplay.

## 2026-10-04 — Attempt 017 — Restore SDK and compile current native bootstrap

**Source:** 2c3fecd. **Procedure:** official vdpm rootless signed 2026.08 channel
in a fresh isolated SDK, managed core 2026.08.1-1 plus the exact renderer packages
from BUILD.md; CMake Release configure/build, ARM ELF inspection and VPK ZIP CRC.
**Observed:** current dual-profile PAC/image/table source compiles and packages.
VPK 717864 bytes; hash in BUILD.md. **Result:** BUILD CONFIRMED. No emulator or
Vita run; no menu/combat. The diagnostic is not published as a playable release.

## 2026-10-04 — Attempt 018 — Preserve original Java core via AOT

**Goal:** test a route preserving original game methods rather than rewriting
all Game1..17 handlers. **Procedure:** dex2jar 2.4 on the original APK; handwritten
EngineProbe calls Init/Run, InputProbe calls original KeyData/Controller; ECJ
3.37.0, TeaVM 0.12.3 C, JVM/native output comparison, GCC host/Vita builds.
**Observed:** unadapted full engine produces 203 diagnostics (44 unique), no C;
JVM throws Android Stub! at BluetoothAdapter. Correcting CLI/classpath isolates
input: press/hold/release key 16, pointer coordinates/speeds, independent IDs,
slot bounds/reuse and Unicode stdout match byte-for-byte between JVM and C.
**Vita changes:** missing uchar.h/UTF-16, GNU POSIX detection, mmap reservation,
clock/fiber hooks, unreachable-return fallback and unused Date backend addressed
for the isolated probe. Fresh adapted output links to ARM hard-float and converts
to VELF/SELF with a standalone 16-MiB Newlib heap: 445118 bytes, SHA-256
a293566b84b2685ad6bda7b3faffe433d80de1da13178303263c1d3275dd25c7.
UTF-16 ASan/UBSan and generated host output with the custom layer pass.
**Result:** SUCCESS (input AOT/native build); full engine INCOMPLETE; device
execution PENDING. This is not connected to the VPK or a production-engine
decision. No generated commercial sources/binaries or APKs are committed.
**Next:** replace actual Android/local-resource/render/text/audio/save boundaries
and obsolete network startup, then verify original Init→Run→menu→selection→battle.
Both resource profiles must remain supported; altered Java mods need separate
behavior recovery. See tools/aot/README.md for full reproduction and limits.


## 2026-10-04 — Attempt 019 — Generate the complete original Init/Run core

**Baseline:** e54cf31. **Changes:** handwritten Android/GLES service interfaces,
NativePlatform/VitaGles imports and original engine entry point. A hash-pinned ASM
adapter redirects only GameData.Init(GlobalWork,String,int,int) resource I/O to
VFS, then invokes the original byte-array Init with conversion/filter preserved.
**Procedure:** ECJ 3.37.0 + TeaVM 0.12.3, original dex2jar 2.4 JAR, no Android
framework JAR. Compare JAR payloads; repeat using generate.py in a fresh directory.
**Observed:** first adapter pass leaves three file/resource reflection diagnostics;
resource overload adaptation resolves all three. Both final runs generate 456
classes/3989 methods without diagnostics. Only GameData.class changes among 106
JAR entries; all 105 other payloads are identical. No original/decompiled/generated
commercial sources are committed. **Result:** COMPILER SUCCESS, not execution.
**Limits:** 55 native service imports need implementations/linking; no native
Init/Run execution yet. Shift_JIS and real Date support must be provided. Offline
network/catalog/Bluetooth boundaries report unavailable/disconnected, never fake
a purchase/download/connection. Production CMake remains the diagnostic bootstrap.
**Next:** connect bounded dual-profile VFS/image data, native client buffers, actual
fonts/audio/saves/input, link the core and execute menu→selection→one complete battle.

## 2026-10-05 — Attempt 020 — Native resource bridge and full C compilation

**Changes:** bounded in-memory PAC normalization for the original GameData
decoder; native GL client-buffer ownership, image upload and per-profile save
services. Ordinary containers remain byte-identical. Community images retain
their entry index and premultiplied payload; nested SPR containers and converted
table directories are adapted without modifying user files.
**Procedure:** ASan/UBSan resource fixture traversal and comparison against the
existing native decoders; compile generated full core on the host; compile GL
and resource services for both host and Vita.
**Observed:** 125 PAC files, 137 containers and 470 images pass, including every
table cell, raw loading/mk/audio resources, malformed directories and unsafe
names. TeaVM emits an empty initializer for the pinned original static byte
bEventFlagBuf. Hash/field-guarded generation substitutes its JVM default zero;
the original TCBManajer.class is unchanged. The resulting full C core compiles.
**Result:** RESOURCE/COMPILER SUCCESS. Linking and actual rendered gameplay are
separate checks; production CMake still builds the diagnostic application.

## 2026-10-05 — Attempt 021 — Text boundary and graphics-buffer lifecycle

**Baseline:** 9a4285c plus recovered uncommitted native-service work.
**Procedure:** complete core generation with TeaVM 0.12.3; exhaustive Shift_JIS
decoder comparison against Java; preceding core native ASan execution.
**Observed:** 65,792 decode cases match. The preceding core renders original
logos/dialog and runs 1,800 frames with heap-backed client buffers. Community
text was mojibake because it is UTF-8. Converting it to standard Shift_JIS fails
on circled digits and corporation symbols. **Correction:** select encoding at
the GetString platform boundary per resolved table; retain payload, string
lengths and characters. Both tables independently fall back to original.
Fresh generation: 460 classes/4,002 methods without diagnostics. Native menu
progression and combat remain under test, not playable-release evidence.

## 2026-10-05 — Attempt 022 — Execute the original offline title/menu

**Baseline:** 9a4285c plus recovered service adapters and charset fixes.
**Procedure:** generate and link the original complete engine with native EGL/GL,
FreeType and Vorbis services; ASan enabled, leaks disabled for the driver.
**Observed:** Android14 resource preflight succeeds; original logs, animated
title and main menu render; injected touch events enter the menu. The original
profile independently runs 900 frames and renders readable Japanese text.
**Corrections:** InitGameData bypasses the string resource overload, so encoding
must be recorded by the actual native read; SetString has four additional charset
boundaries. Font surfaces follow the original batched flash lifecycle. Locally
verified initial character triplets set the original installed-data flag; local
update checking returns no pending remote items only when required PACs exist.
Remote HTTP requests still fail honestly. No gameplay handler is replaced.
**Result:** HOST MENU CONFIRMED; character selection/combat under test. Vita ARM
build and hardware execution are separate checks, not established by host GL.

## 2026-10-05 — Attempt 023 — Real Vita touch/audio and Android14 character selection

**Goal:** progress the full original engine on physical Vita through real menu
input and character selection for both installed data profiles.

**Baseline:** builds 00.08–00.10 after the vitaGL selector, audio-thread and
initial-resume fixes.

**Changes:** calibrate the front panel from its active area, prevent Vita face
buttons from being translated accidentally to Android Back during gameplay, and
map arbitrary SceTouch report IDs to stable original-engine pointer slots 0–4.

**Test procedure:** run Original and Android14 on a physical PS Vita, navigate
with the front touchscreen, listen for game audio and continue into character
selection. Inspect the captured TeaVM/runtime log rather than treating a clean
process exit as a native crash.

**Observed:** build 00.10 has responsive touch on hardware and audible audio.
Original ran 1054 frames in the captured session. Android14 accepted touch through
the menus, reached character selection, displayed a character and then exited
cleanly at 1502 frames. TeaVM traced a caught NPE to `TCBManajer.Game3()` with
the reported state md=1018. Some screen transitions show a 1–2 second black
interval; this is retained as a separate loading/performance observation.

**Root-cause follow-up:** original bytecode state 1012 loads `ChrGameData[3]`
from `charXX.pac` with conversion 2/filter 187, enters 1013, and sets md=1018
before the final table reads. The Community14 bridge normalized the outer PAC and
RGBA textures but left each encoded `bin` converted-table header untouched.
The original `binCnv()` therefore rejected the character table and later Game3
dereferenced null GameData arrays. Host reproduction confirms all 13
`char00..12.pac` BIN entries decode to 43 records with the established
Community14 GameDataTable codec; the generalized regression covers all 68
Community14 BIN entries.

**Evidence:** `docs/evidence/vita_hardware_touch_character_00.10.json`, submitted
`runtime.log`, commits `3977953a2209f5c52e81eb5ca84f3068ae46cd80`
and `e3a79b1e73baab5d648112697fcb2d0f4ac32770`.

**Result:** PARTIAL. Touch, audio, menu navigation and character-selection entry
are HARDWARE CONFIRMED. Community BIN normalization is HOST CONFIRMED and awaits
00.11 hardware verification. Character-selection continuation and a complete
battle are not yet confirmed.

**Next action:** build 00.11 with Community14 BIN normalization and retest
Android14 character selection first. After functional progression is stable,
profile the 1–2 second black transition stalls separately rather than mixing a
performance optimization into the correctness fix.

## 2026-10-05 — Attempt 024 — Normalize Community14 character BIN tables and build 00.11

**Goal:** fix the clean Android14 exit after the first character appears without
patching `Game3` or replacing original character-selection logic.

**Baseline:** hardware-confirmed build 00.10, whose TeaVM log ends in a caught
`TCBManajer.Game3()` NPE with reported `md=1018` after 1502 frames.

**Changes:** `src/engine_resources.cpp` now applies the already-verified
Community14 converted-table metadata decoder to encoded `bin` entries before
the unchanged original Java `GameData.Init(..., conversion=2, ...)` path sees
them. `tests/test_engine_resources.cpp` compares all Community14 BIN tables
record-by-record before/after normalization. CNV and unrelated DAC payloads
remain on their distinct schemas. Vita version advances to 00.11.

**Host test:** the actual Android14 `char00.pac` through `char12.pac` from the
private Vita dataset were passed through the production `readEngineResource()`
path. All 13 normalized BIN tables decode as Original format with 43 records each,
with matching positions, dimensions and cell values:
`CHAR BIN NORMALISE PASS: 13/13, 43 records each`.

**Vita build:** Release ARM build completed successfully through ELF, VELF, SELF
and VPK. `DBTapBattle-Vita-00.11-charbinfix.vpk` is 2,210,294 bytes, SHA-256
`a39688235f3751689fd64c924bb7f11fc8a8212e29928705a3449db34292efe6`.
`eboot.bin` SHA-256 is
`1f7967da03617df53909bafe829c9301c88cc5fbade0815594de2f8746f83c49`.
VPK ZIP integrity passes, SFO is `DBTB00001` / `00.11`, and no icon0 is packaged.

**Evidence:** `docs/evidence/vita_character_bin_fix_00.11.json`.

**Result:** HOST DATA CONFIRMED + BUILD CONFIRMED. The fix specifically addresses
the reproduced null character GameData table. Hardware confirmation of selection
beyond the first rendered character and a complete battle is still pending.

**Next action:** test Android14 first, browse several characters and proceed
toward battle. If that succeeds, test Original again for regression and then
profile the separate 1–2 second black transition stalls.

## 2026-10-05 — Attempt 025 — Recover interrupted 00.17 and reduce PVF I/O

**Baseline:** `fddbba5`, including voice interpolation (`910f9d3`), GL client
state tracking (`5608826`) and PVF metric reuse (`0205adf`). These commits were
already on main when the interrupted session was resumed; no delivered full
00.17 build or hardware confirmation was found.

**Hardware evidence:** submitted 00.16 runtime.log shows text draws of 18–184 ms
and uploads of 19–20 ms. User confirms cards/startup fixed, but character
switching still stalls, some voices are rough, and battles run at 35–45 FPS.
The log does not yet identify the combat CPU/GPU bottleneck.

**Change:** open the identical selected system font with
`SCE_PVF_MEMORYBASEDSTREAM`, falling back to file streaming if PVF rejects it.
VitaSDK's pvf.h defines mode 0 as FILEBASEDSTREAM and mode 1 as MEMORYBASEDSTREAM.
This avoids font seeks on each new glyph without changing text, layout or sizes.

**Check:** current VitaSDK 2026.08 GCC 15.2.0 compiled vita_text.cpp successfully;
`git diff --check` passes. A real-device timing improvement is still PENDING.

### Follow-up — GLES client arrays

The Java GLES adapter duplicated each NIO buffer, allocated fresh byte arrays,
and serialized float/short values for every pointer/index call. Reuse typed
scratch arrays for direct buffers and borrow an active backing array for heap
buffers. Preserve position, limit and arrayOffset; native attributes remain
copied into their own storage before returning to Java. This removes per-draw
conversion allocations without batching/reordering the original draws.

ECJ 3.37.0 compiled the adapter successfully. Full TeaVM regeneration, buffer
range checks and Vita compilation are the next checks; hardware FPS is PENDING.

### Follow-up — Audio, frame measurements and complete build 00.18

Recovered a concurrent committed follow-up, `1351457`, reducing mixer state-lock
work to 64-sample chunks and using Q16 interpolation with a 32.32 phase. Retained
it; sample chunk duration is not a measured bound on lock contention.

Cache clip frame counts and clamp gains once at playback start, removing integer
sample-count divisions and gain clamps from each output sample. Preserve original
22050 Hz mono voices, three voice channels, track pitch and clip ordering. The
original APK SoundEffect bytecode independently confirms AudioTrack is created
at 22050 Hz (its buffer-size query at 44100 Hz is not the playback rate).

Add main-thread frame summaries every 120 frames and atomic audio statistics.
`run_ms` includes Java/GL/resource work; `swap_ms` includes GPU/pacing waits;
`interval_max_ms` includes cooperative tasks after present. These are elapsed
wall times, not CPU utilization counters. Record resource, texture/text and OGG
load costs, draws, native copied bytes, clipped output samples and blocks whose
mix calculation exceeds the 1024-frame audio deadline. No file writes occur in
the audio worker.

Set public clock requests to CPU 444, bus 166, GPU 222, crossbar 166 MHz and log
return values plus effective clocks. Keep 960x544, original update frequency,
resources/mod fallback and separate profile saves. No overclock plugin is needed.

**Host checks:** the real mixer passes ASan/UBSan tests for fractional/extreme
interpolation, 22050→48000 duration, stereo, loops, three-channel rejection and
clipping counters (`tests/test_vita_audio.cpp`; platform calls mocked). The Java
adapter passes active direct/read-only ranges, backing-array slice offsets,
shrinking limits, steady buffer reuse and oversize rejection via
`tools/aot/engine/tests/run_gles_buffer_probe.py` (native imports mocked).
Complete current Java adapters regenerated through pinned TeaVM 0.12.3; complete
ARM engine compiled and packaged as VPK 00.18. Device smoothness, voice quality
and a stable 60 FPS remain PENDING user hardware testing.

## 2026-10-05 — 00.19 text regression and audio peak control

**Hardware result for 00.18:** user reports stable 60 FPS in battles, raspy
voices persist, previously working text disappears. Latest supplied runtime.log
boots 3302305; steady battle windows are 59.9 FPS. No mix-computation deadline
misses; summed PCM clips 1345 channel samples across this session. Exact metrics
and log SHA are in `evidence/vita_hardware_performance_00.18.json`. Selection
resource/texture pauses remain separate from steady battle performance.

**Text:** 0205adf replaced `scePvfGetCharImageRect` with CharInfo bitmap dimensions.
A host probe of the real adapter reproduces zero coverage when those dimensions
are zero but the image rectangle is valid. Restore the authoritative rectangle
once per visible glyph/size; retain memory font, cached advances/bearings,
deferred off-screen glyphs and dirty-row uploads. First eight glyphs report
rectangle and nonzero pixel count to distinguish PVF from upload failures.
The same probe then passes ASCII/CJK coverage, bounds, cache reuse, deferred
rasterization and failed-image handling under ASan/UBSan. Device recovery is
still PENDING testing 00.19.

**Audio:** retain original PCM16 mono 22050 Hz voices and fixed-point linear
resampling to 48000 Hz. The original SoundEffect audio worker skips the 44-byte
WAV header; the adapter's RIFF parser retains the same source samples and supports
additional chunks. All 198 gen.apk voices match their PCM payload byte-for-byte
and add no clipping when mixed alone (host ASan/UBSan). 36 source clips contain
4140 samples exactly at the PCM rails; this is source evidence, not proof of
all audible roughness. No filtering or asset rewriting is introduced.

Replace hard saturation of the summed signal with a stereo-linked block peak
limiter and gradual ~100 ms release. Use an 8 KiB fixed scratch buffer, keep
64-frame lock portions, and add no output buffering/delay or per-block allocation.
Below-threshold audio at unity remains unchanged. Host probes verify shape and
stereo balance, extreme overlap, release across calls, non-1024 block counts,
RIFF metadata/truncation, interpolation/duration and the original three-channel
limit. `audio_overload_samples` counts pre-limiter overflow, while
`audio_clip_samples` now counts actual post-limiter saturation. Source PCM
rate/peak/rail counts are logged at voice load. Audible improvement and Vita
audio-thread cost remain PENDING hardware testing.

Native source changes are separate commits 749fdb5 (text), 8654f66 (audio);
full-engine version 00.19 is bb78269. Keep original graphics optimizations, clocks,
960x544, game rules, mod fallback and profile saves.
