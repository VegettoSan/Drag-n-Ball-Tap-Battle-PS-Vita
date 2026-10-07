# Attempts Log

> **Historical document notice — current v1.0 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for v1.0. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


## 2026-10-07 — 00.34 — recenter themed buttons and labels

**Observed on physical Vita:** after the no-orb background correction, profile
labels could sit over the silver bevel instead of remaining inside the cyan
interior.

**Cause:** selector rows still used the older right-shifted button coordinates
(`x=248`) and left-anchored text (`x=282`). Long profile names could extend
into the metallic edge.

**Fix:** center the 664 px Gen button at `x=148`; move the one-star marker with
the row; center labels at `x=480`; fit text dynamically to a 530 px usable cyan
region and vertically center each glyph row inside the 46 px button height.

**Source:** `0da8684805d1510caf93130a22eed523a854c1d6`.

**CI:** Vita native build/smoke PASS; VitaSDK/private-tool export PASS.

**User-test VPK:** `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk`

**SHA-256:** `24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03`

Physical verification of this exact geometry is pending.

## 2026-10-07 — 00.34 — remove embedded blue orb from selector background

**Observed on physical Vita:** the previous fullscreen-background attempt still
showed a large blue energy orb in the lower-left area and left transparent/black
space around it.

**Cause:** `select0_background.png` is a 512×512 atlas-like derivative, not one
full-screen rectangular background. Its desired cyan/grid art is the continuous
upper band; the separate blue orb is embedded later in the transparent lower
part. Cropping only the transparent right edge still sampled the orb vertically.

**Fix:** detect the continuous populated band from the top. For the approved
asset this resolves to 482×320 px. Runtime now crops both U and V to that region
and stretches only it to 960×544. The lower orb region is excluded completely.

**Source:** `18559dcea316076bb1225080bb8506dd439df6f6`.

**Build evidence:** Vita native smoke/build for that commit PASS. Complete VPK
generated locally and LiveArea validation PASS.

**User-test VPK:** `DBTapBattle-Vita-00.34-No-Blue-Orb-Fix.vpk`

**SHA-256:** `4f0abc4aba15c766847657d152de9cff50877df3f7b13a169f06be4226d2367b`

Physical verification of this corrected VPK is pending.

## 2026-10-07 — 00.34 — unified profiles + fullscreen selector + opening-profile transition

**Goal:** remove the artificial Original/mod split, make extractor output identical
to the Vita runtime namespace, fill the selector viewport with the existing Gen
background and show visible feedback while a selected profile begins loading.

**Runtime changes:**

- one current data root: `ux0:data/DBTapBattle/profiles/`;
- selector enumerates only real first-level profile folders;
- no unconditional Original or Original-missing row;
- empty `profiles/` shows **NO GAME DATA FOUND**;
- selected resources and mutable save stay inside the same profile;
- background samples only its non-transparent horizontal content and stretches
  that region to 960×544;
- confirmation presents **OPENING PROFILE / LOADING GAME DATA...** before the
  selector textures are released and control passes to the original engine.

**Extractor changes:** Windows extractor 1.5 emits only
`data/DBTapBattle/profiles/<sanitized APK filename>/`, records
`runtime_contract: profiles-v1`, does not use known APK hashes for visible
profile naming, and writes the exact runtime root into `RESULTADO.json`.

**Evidence:**

- runtime/selector source checkpoint `63bc0f90d33d4a5d8d90c4816ff0f0ae07272751`;
- Vita engine native smoke run `37679405794`: PASS;
- private tool export run `37679405502`: PASS;
- Windows extractor 1.5 regression run `37688246446`: PASS;
- complete user-test VPK:
  `DBTapBattle-Vita-00.34-Selector-UX-Fix.vpk`;
- VPK SHA-256:
  `e06ded147eead1c7ee8e5a558552d5129a98b5916395c59780125782c8d55c92`.

**Observed:** build/tool evidence only. Physical Vita result is pending; do not
promote 00.34 over the 00.33 hardware checkpoint until the user reports the
selector, profile-opening transition and gameplay regression result.

## 2026-10-07 — 00.34 — Restyle the native data selector from Gen/select0.pac

**Goal:** make the Vita-only profile chooser look like it belongs to Dragon Ball
Tap Battle without using character art or changing the original engine.

**Baseline:** 00.33 is hardware-confirmed for Loading recovery, dynamic rosters
and the reproduced Invasion repeated-fight crash fix.

**Changes:** extracted/cropped only background/header/button/one-star-ball visuals
from the supplied Gen `assets/select0.pac`; stored them as a hash-pinned split
Base64 ZIP; materialize four PNGs at build time; package them under
`app0:/selector/`; render the first screen with client-array vitaGL drawing.
The previous flat selector is preserved as runtime fallback. No gameplay Java,
PAC parser, profile resolution or save logic was changed.

**Observed:** native VitaSDK smoke CI compiles, links and packages the 00.34
selector successfully (run 37622687132). Publication/actionlint/theme regressions
also pass (run 37622780686).

**Result:** BUILD CONFIRMED / hardware appearance pending.

**Full publication attempt:** run 37623204195 intentionally tried to build a
00.34 prerelease through the existing private-source workflow. It stopped at the
source gate because `DBTB_ORIGINAL_APK_URL` is empty. No TeaVM generation,
compilation or publication ran, so this is an infrastructure/input blocker rather
than a code regression.

**Next action:** configure the existing private original-APK URL secret, build the
full 00.34 VPK, then test it on a physical Vita: photograph the first selector,
verify D-pad/stick/X/touch/Circle, and enter at least one normal gameplay path to
ensure 00.33 behavior is unchanged. See `TEST_VITA_00_34.md`.

## 2026-10-05 — manual full-engine release/prerelease automation

User requested separate manual workflows after confirming 00.24 on Vita. Added
two dispatch callers with a shared pinned original-APK → dex2jar/TeaVM → VitaSDK
full build, exact version/source/core/LiveArea/CRC validation and draft-first
publication of compiled binaries only. Stable and prerelease tags/latest flags
are separate; existing tags/releases are never overwritten. Private inputs and
generated source/logs remain ephemeral and excluded from uploads. Static
actionlint, 10 publication regressions, three LiveArea regressions and staging
against the real hardware-confirmed 00.24 VPK pass. Original-APK music fixtures
also preserve all 17 PCM tracks under the allocation-ceiling regression. Full
hosted publication is not claimed: the private download secret is required.

## 2026-10-05 — 00.24 physical retest confirmed

The user tested the exact delivered 00.24 VPK and reports it works very well.
The prior battle-start crash is resolved in this test. No new log or exhaustive
profile/character matrix was supplied. Runtime source f5672d4d, VPK SHA-256
0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345.
Preserve both PAC streaming and exact PCM allocation; use test_vita_ogg and
test_vita_resources before future memory optimizations.

## 2026-10-06 — 00.24 exact BGM PCM allocation

User confirmed 00.23 LiveArea-Fixed presentation, then supplied an Android14
characters 12/03 battle-start crash. Log retains native PAC streaming but ends
with `std::bad_alloc`. Core stack points to old `decodeOgg` vector resize;
its 0x910e00 (9,506,304-byte) request is reproduced by private `bgm_03.ogg`.
Changed Ogg decoding to exact frame-count allocation/direct writes with
format/length validation and cache-only resource reclamation beforehand.
Newlib/TeaVM heaps and original gameplay remain unchanged. Private APK-derived
code/data are generated locally, never committed or sent to CI.

All 17 real BGM tracks compare byte-for-byte against the legacy decoder. A 6 MiB
single-request ceiling fails legacy bgm_03 and permits every fixed track. That
track's C++ peak falls from 14,260,324 to 5,454,432 bytes. Existing audio DSP/setup
and native resource tests pass ASan/UBSan with Vita/GL mocked; active streams and
live textures survive reclamation. 00.24 device result pending.

**Reading checkpoint — 2026-10-05 / 00.21.** Entries retain the source/build and
evidence available when recorded. Historical pending items can be superseded;
do not treat them as current blockers or retroactively promote their success.
The current full-core AOT status, delivered artifact and open physical checks
are in [CURRENT_STATUS](CURRENT_STATUS.md). Commands/mock scope are in
[VALIDATION](VALIDATION.md); reusable lessons in [PORTING_GUIDE](PORTING_GUIDE.md).

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

## 2026-10-05 — 00.20 selective PAC loading, reuse and voice reconstruction

**Hardware evidence for 00.19:** user confirms text is visible again, voices
still sound bad and character switching still stalls. Its current log contains
40 performance windows, zero output clipping/overload and zero late mix blocks;
maximum main-thread run is 3578.65 ms. Repeated resource/texture/voice bank loads
coincide with pauses. Per-bank decode accounting includes synchronous diagnostic
logging, so that measurement does not isolate PCM parsing. See
`evidence/vita_hardware_text_audio_00.19.json`.

**Original-engine evidence:** original APK GameData.Init stream bytecode skips
payloads excluded by its type filter: PNG/RGBA=1, ACT=2, BIN=4, CNV=8, DAC=16,
SPR=32, WAV=64. The port instead read the entire PAC, copied entries and applied
the filter later in byte-array Init. Restore selective payload reads before
normalization while retaining directory slots, types, reserved fields and order.
Normalize Community14 fields without touching excluded entries; avoid a second
container open and unnecessary plain-PAC copies. The original parser/gameplay
bytecode remains unchanged.

**Reuse:** an 8 MiB PAC-result LRU keys resolved profile path and filter and checks
file size/mtime/ctime; profile changes clear it and saves are separate. Cache
immutable imported textures up to 4 MiB of source bytes plus GPU RGBA cost,
checking hash collisions with exact bytes and preserving filtering mode and
live ownership. Delete only idle LRU textures; mutable render targets are uncached.
Cache voice inputs plus decoded PCM up to 2 MiB across voice bank releases;
audio disposal clears it. Reduce per-voice diagnostics to the first three loads.

**Voice conversion:** replace linear low-rate character voice interpolation with
a 16-tap Hann-windowed sinc table, 256 phases and Q14 coefficients. Preserve
source PCM, original 22050 Hz playback, 32.32 phase and 48000 Hz output duration;
BGM/effect paths are unchanged. Retain stereo-linked peak control. Increase audio
worker priority and measure gaps between blocking output submissions separately
from mix computation; gaps alone do not prove hardware underruns.

**Host checks:** ASan/UBSan probes pass for 26 character PACs and filters 1/33/64/127,
identical selected payloads, ordinary/Community14 directories, cache invalidation
and bounds. Filter 33 requests 11,707,264 rather than 91,081,701 source bytes
(87.146% reduction); not a measured Vita latency result. Native resources probe
uses real PNG decoding and mocked GL and passes ownership/modes/collisions/idle
eviction/live protection/render-target release. Audio probe passes exact DC,
coefficient/PCM bounds, source boundaries, RIFF, collision-safe PCM reuse,
limiter, stereo/loops/duration and three channels. An 8 kHz tone at 22050 Hz
reduces its 14050 Hz resampling image by 32.3 dB with fundamental amplitude
within 5%; audible Vita quality is still pending.

Full resource ASan/UBSan regression passes 125 files, 137 containers, 470 textures,
68 BIN tables and 198 decoded WAVs. Python 12 tests, JVM buffer probe and existing
text probe pass. Current adapters regenerate through TeaVM 0.12.3 (465 classes,
4059 methods) and the complete ARM engine packages as 00.20. VPK CRC/SFO/eboot
and full-engine source identification pass. Implementation commits: 9ef0c57,
dc32531, 78b2b1d; packaged source 7f19f80. Hardware voice quality, cold/warm
character latency, retained text and battle FPS remain PENDING.

## 2026-10-05 — 00.21 audio worker startup regression repair

**Hardware result for 00.20:** the user reports Original closes before the menu.
The new 7f19f80 log records 20 output-port opens followed by 20 worker setup
failures. Engine initialization passes and resources/text run, then at frame 570
state 693 catches `IllegalStateException: BGM load failed: bgm_16` and leaves the
game loop. BGM loading calls ensureAudio() before file resolution/decoding; its
failure here is not evidence of a corrupt OGG/PAC. All three supplied APKs have
a 278463-byte OggS bgm_16 entry (the Gen raw entry is an empty placeholder;
its assets entry is populated). See `evidence/vita_hardware_audio_startup_00.20.json`.

**Correction:** 2e71d51 restores 0x10000100, the priority used by the working
00.19 worker and the VitaSDK thread-creation example. The 00.20 change to
0x10000080 is the leading regression; the old log combines create/start failures
and omits error codes, so neither exact failing syscall nor valid priority range
is asserted. Split output-port, thread-create and thread-start failures, log each
operation and hex/decimal result, release only owned handles once and latch setup
failure until disposal. Preserve PAC filters/caches, voice DSP, PVF and graphics.
Fix resource diagnostics' unsupported `%zu` formatting, which printed literal
`zu` and shifted the reported duration on Vita's printf.

**Host checks:** the real audio adapter passes ASan/UBSan for ready-path reuse,
injected port/create/start errors, handle cleanup, exact operation/error logging,
no repeated setup on subsequent loads, and recovery after disposal. Existing
DSP/limiter/PCM/three-channel checks still pass, including 32.3 dB spectral-image
reduction. Python's 12 regressions pass. The regenerated original engine has
465 classes and 4059 methods; build source 07222bb packages 00.21.
Physical worker startup and reaching the menu remain PENDING testing this VPK.

The native resource probe also passes ASan/UBSan using the real supplied Gen
PNGs/PACs with GL mocked. The first invocation used a flat extraction instead
of the GameVfs game/ layout; another symlink fixture was correctly rejected by
the VFS. A real game/ directory then passes all bridge/cache/texture checks.
The complete 00.21 build succeeds through ELF/VELF/SELF/VPK. CRC, APP_VER 00.21,
TITLE_ID DBTB00001, eboot equality and original-engine/source markers pass;
see `evidence/vita_audio_startup_build_00.21.json`.


## 2026-10-05 — Documentation reconciliation for the 00.21 checkpoint

**Goal:** make every repository Markdown consistent with actual implementation
and preserve reusable evidence for another port.

**Baseline:** latest delivered full-engine source 07222bb / VPK 00.21; user
hardware results through 00.20, no new 00.21 physical result.

**Changes:** reconcile build targets, private AOT generation, VFS/PAC filters,
cache ownership, audio setup/DSP, PVF/lifecycle, profile saves and APK identities.
Archive versioned test sheets with subsequent results; retain the journals.
Add CURRENT_STATUS, VALIDATION and PORTING_GUIDE for current navigation,
reproducibility, evidence levels and cross-project lessons. Correct unsupported
save-exclusion and complete-symbol-relink claims against actual tools/archives.

**Validation scope:** Markdown coverage, local links, recorded artifact metadata
and source-path/contract consistency. This task changes documentation only;
prior host/device results keep their original scope. No new gameplay, latency,
quality, save round-trip or latest hardware success is asserted.

**Next action:** test 00.21 on Vita with identified profile/manifest, preserve
runtime.log, then assess voice samples and cold/warm selection latency. Update
the checkpoint and evidence before claiming those issues fixed.


## 2026-10-05 — 00.22 preserve original character-selection filter masks

**Hardware result:** 00.21 starts its audio worker and reaches the menu. Selecting
Original then a character rejects char00 twice with invalid GameData filter;
Game3 catches NullPointerException at md=1018 and leaves at frame 1273. The log
contains an older 00.20 session; analyze only the new 00.21 marker onward.
No exact device dataset hash was supplied. See
[evidence](evidence/vita_hardware_selection_00.21.json).

**Cause:** source bytecode confirms Game3 uses mask 187 (0xbb: BIN/WAV allowed)
and other original LoadFilter paths use 251 (0xfb: BIN allowed). The native
00.20 selective-read adapter rejects anything outside 0..127 before opening the
file. Original GameData tests individual exclusion bits, not an enum range.
Do not reject the unused high/sign bits or change original task/parser behavior.

**Correction:** remove that range rejection, retaining the exact Java int mask,
known type tests, stable directories and cache keys. Extend real-corpus stream
and actual native resource/copy/cache probes for 187/251; include high/sign-bit
edge masks. The unchanged audio startup, text and DSP paths remain in place.

**Validation:** the pre-fix code fails the new regression at char00/filter187;
corrected ASan/UBSan stream, native resource and complete corpus probes pass.
26 Gen/Community14 character PACs preserve exact allowed payloads/slots; 187
retains BIN/WAV and 251 BIN only. Full corpus: 125 files, 137 containers,
470 textures, 68 tables, 198 WAVs. GL is mocked; no hardware fix is inferred.
Initial stale/flat fixtures were rejected; correct game/ plus mods/Android14
install roots are used for the recorded before/after checks.

**Next:** compile/verify full VPK 00.22, then retest first character, repeated
switches, voices, visible text and sustained battle FPS on Vita.


## 2026-10-05 — 00.23 restore original streaming PAC parser for battle memory

**00.22 hardware:** user confirms clean voices and character switching without
observed stalls. Battle startup aborts in TeaVM's managed allocator while creating
char00's whole 4,739,319-byte bridge array. Stack identifies readGameData →
ResourceAdapter.load → GameData.Init → SetLoad → Game1. The supplied core is a
gzip-compressed ARM core; it does not contain the GC globals/managed heap, so the
exact live-memory/fragmentation split is not established. Archive metadata in
[evidence](evidence/vita_hardware_battle_memory_00.22.json).

**Original contract:** both original Init overloads begin with Dispose(gw). The
previous Vita adapter allocated the entire managed PAC before calling the byte
parser and thus before disposal. Restore the original String/InputStream parser;
patch only its Android raw-ID and openFileInput opening expressions. Keep the
original entry decoder, filters, conversion, skip/read, catch/finally and close
paths. NativeResourceStream pins a cached native PAC with an explicit handle;
Java reads individual original payload arrays. No larger heap or explicit GC.

**Checks:** compare both preserved original parsers using private normalized Gen
and Community14 character packs: 26 packs × 9 masks = 234 successful reloads,
identical GameData/SpriteData/BIN/WAV state, no surviving stream handles.
Largest PAC 4,819,351 bytes; largest Java stream read 380,395 bytes. GL and native
I/O mocked on JVM, so this measures parser/bridge behavior, not Vita total memory.
Real native cache/copy/PNG tests add pinned-owner survival after eviction, bad
ranges/EOF, independent streams, handle limit, idempotent close and profile reset;
ASan/UBSan PASS. LeakSanitizer cannot inspect /proc tasks in this runner; rerun
with detect_leaks=0 and verify explicit owner/handle assertions instead. Fresh
TeaVM generation has 467 classes / 4086 methods. Add allocation-free native OOM
requested/free/available/max/chunk diagnostics; preserve 8/48 MiB managed and
96 MiB Newlib budgets. Retain audio DSP/worker and texture cache behavior.

**Result:** host parser/resource checks pass; physical 00.23 battle startup,
repeat battles, memory headroom, texts and audible/selection regressions remain
PENDING. This removes the observed whole-PAC allocation, not every possible
memory limit. Deliver complete fresh engine, matching symbols and exact hashes.

<!-- DBTB_00_23_DETAIL:START -->
## 2026-10-05 — 00.23 real-hardware battle-memory retest — SUCCESS

**Input:** the 00.23 battle-memory test VPK built from `0e17b0bac33c47698b414b67a839c839f0e555ce`.  
**Reason:** 00.22 reached the character selector with clean audio and responsive
switching, but aborted when starting a fight while allocating a full ~4.74 MiB PAC
bridge array in TeaVM managed memory.

**Change under test:** restore the original streaming `GameData.Init` path; replace
only Android resource opening with `NativeResourceStream`; keep original entry
parsing, filters, conversion, `Dispose` ordering and close/finally behavior.

**Hardware result:** user reports everything exercised in this session working as
expected and no error found. Audio remained correct, character selection remained
responsive, battle startup succeeded and gameplay proceeded normally.

**Outcome:** accepted as the current 00.23 hardware checkpoint. Continue regression
coverage rather than reopening the removed whole-PAC bridge design.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## 2026-10-05 — Repair the failed Final LiveArea VPK

User reported both installation and presentation failure. Inspection found two
separate defects: `pic0.png` had 192 palette slots despite the documented 256-slot
splash rule, and the Final executable matched the CI native link probe exactly
(`450313712e506ae637501f57a5d521316304d0df74b611bdd416e0b19c360a22`).
That probe is not the game. We retrieved and hash-verified the hardware-tested
00.23 VPK, then rebuilt the presentation package with unmodified VitaSDK
`vita-pack-vpk`, retaining every original entry. PNG pixel identity, toolkit
validation, archive CRC/paths, stricter source/VPK validation and three regressions
pass. Repack input is now hash-pinned, and base executable/SFO identity is checked.
CI artifacts are renamed `dbtb-native-link-probe-NOT-PLAYABLE`.
Hardware confirmation of the new LiveArea is pending; no installer code was supplied.
See [evidence](evidence/vita_livearea_fixed_00.23.json).

## 2026-10-05 — Portable Windows APK extraction

Created BAT + Windows PowerShell 5.1 data tool with indexed environment argument
transport, automatic raw/assets/Community14 selection, separate profiles, exact
byte preservation, explicit ZIP CRC checks, manifest/file hashes and whole-batch
staging. The actual script extracted all three private supplied APKs under
PowerShell 7.4.6/Linux; independent Python checks verify 57/144/147 files and 106
Community14 alias renames with identical payloads. Windows CI first passed the
12 extraction tests but failed before starting BAT because Python's CRT quoting
was inappropriate for cmd.exe. The corrected harness in `5ef4549` passed all 13
tests on PowerShell 5.1/Windows Server 2025, including special-path BAT transport.
No runtime/VPK changes or new Vita gameplay evidence. See
[evidence](evidence/windows_extractor_2026-10-05.json) and
[usage/contract](WINDOWS_DATA_TOOL.md).

## 2026-10-06 — Attempt 030 — Samu 92-character roster / IDs 00..99 / BGM normalization

**Goal:** integrate the audited `DragonBallZuperSamuGamerYT.apk` without
rewriting the original engine, make all 92 supplied character triplets visible
to the Vita-side installation audit, retain headroom through two-digit ID 99,
and make the mod's mislabeled MP3/AAC BGM consumable by the already-tested
Vorbis backend.

**Source evidence:** APK
`1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d`;
DEX `cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729`
(byte-identical to Gen); 92 complete triplets `00..91`.

**Changes:** `auditInstalledData()` now scans the complete two-digit namespace
00..99 while retaining minimum/contiguity/completeness checks. No original
TeaVM gameplay method was changed. Added `tools/prepare_samu_mod.py`, pinned
to the audited APK/DEX, to normalize exactly the documented 12 MP3 + 3 AAC/M4A
BGM into Ogg Vorbis 44.1 kHz stereo and record all transformations. Version
candidate bumped to 00.26.

**Commits:** `7af4720` (roster audit), `063b779` (Samu preparer/tests),
`278da8f` (CI regression), `d7a4aa2` (00.26 version).

**Validation:** synthetic installed-data tests cover 13, 22, 92 and 100
contiguous triplets and reject >100 audit bounds. Community mod profiles run
`37540898687` PASS. Direct conversion against the supplied APK successfully
produced Vorbis 44.1 kHz stereo for all 15 non-Vorbis BGM; `bgm_12` and
`bgm_13` were already Vorbis and remain unchanged.

**Result:** HOST/IMPORT SUPPORT CONFIRMED. Physical Vita validation of the 92
characters, their voices/charf variants, high-index battles and the normalized
BGM matrix remains pending. 00.24 remains the hardware baseline.

### Attempt 030 follow-up — full 00.26 physical-test artifact

The pinned original APK `b84f98a3...` matched the private build gate. Fresh
dex2jar/TeaVM generation completed at 467 classes / 4086 methods. Public native
Vita smoke `37541052112` and tool export `37541052003` both passed.

The interactive runner could not finish the monolithic TeaVM `all.c -O1`
inside one execution window, so the already documented 00.23 split technique
was used for this physical-test package: all generated TeaVM C except
`TCBManajer.c` remains `-O1`, `TCBManajer.c` compiles at `-O0`, and
native Vita adapters remain `-O2`. ELF -> VELF -> SELF -> VPK completed.

Artifact: `DBTapBattle-Vita-00.26-Samu-Roster-Test.vpk`, 2,604,860 bytes,
SHA-256 `749b9d32e6ed62a7b4593cb6f0b5af6dc2cabbc97cd9f25986757700879e18f5`.
Eboot SHA-256 `1de9962f19cf9c39a1534e9547de14e3a2569950a6b712ca49ab871443f90830`;
ELF SHA-256 `db580cd100ac330d88908a9db2cd71f53a50b70c2295700ea0a17fbba68e7e9e`.
SFO is 00.26/DBTB00001, embedded runtime marker is `d7a4aa2`, and exact
LiveArea validation passes. This establishes a real full-engine test VPK, not
the CI native-link probe. Hardware result remains pending.


## 2026-10-06 — Attempt 031 — Samu audio directo sin conversión

**Reason for retry:** Attempt 030 proved the format matrix but its import-time
FFmpeg conversion does not satisfy the compatibility goal: the user requires the
mod to run from the APK assets exactly as supplied.

**Change:** the Samu helper no longer transcodes anything. It validates/extracts
384 assets byte-for-byte. The Vita BGM boundary now keeps the existing
libvorbisfile path for real Vorbis and adds content-detected MP3/AAC handling
through the system `SceAudiodec` API. AAC files remain inside their original M4A
container on disk; the adapter demuxes ISO-BMFF sample tables in memory and passes
the original AAC access units to the Vita decoder. No original TeaVM gameplay
method is changed.

**Real-APK format validation:** all 12 MP3 sources parse as 44.1 kHz stereo.
`bgm_09/10/11` contain 228/1578/228 AAC access units; maximum unit sizes are
1114/1143/1114 bytes. Representative source hashes/sizes remain identical before
and after extraction.

**Public validation:** Community mod profiles run `37544623252` PASS; Vita
engine native smoke run `37544588962` PASS after linking
`SceAudiodec_stub`. The earlier intermediate smoke failure at `de3b11e7` was
a missing linker dependency, corrected by `5e1808f6`.

**Full candidate:** 467 TeaVM classes / 4086 methods; APP_VER 00.27;
`DBTapBattle-Vita-00.27-Samu-DirectAudio-Test.vpk`, SHA-256
`bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`.
LiveArea and full-engine symbols pass. Interactive TeaVM remainder is split at `-O1`, `TCBManajer.c` is `-O0`,
and native adapters remain `-O2`; this is a functional codec/roster test and
not final release-performance evidence.

**Result:** BUILD/HOST DIRECT-AUDIO SUPPORT CONFIRMED. Audible MP3/AAC playback,
looping, transitions and high-roster gameplay remain pending on a physical Vita.
See `TEST_VITA_00_27.md`.

## 2026-10-06 — Attempt 032 — perfiles APK completamente independientes

**Goal:** allow a user to install only one extracted APK profile under
`mods/<Profile>/` and play without installing/populating `game/`.

**Reason:** Android14, Español and Invasion are standalone Android APKs despite
omitting resources present in Original/Gen. The previous overlay fallback could
silently borrow Original files, masking a Vita compatibility gap and violating
the intended installation model.

**Changes:** selected `GameVfs` profiles now resolve resources exclusively from
their own directory. Missing files report `missing selected profile resource`
and never fall back to `game/`. The installed-data gate understands that error,
no longer requires `bobj00`, and still enforces complete contiguous character
triplets plus shared select/effect/back data. If Original is absent, the selector
starts on the first installed profile and refuses to launch the empty Original
entry. Candidate version is 00.28.

**Regression proof:** a synthetic test leaves a matching `game/charf0000.pac`
in place, deletes only the selected profile's copy, and requires the audit to
fail. CI runs `37551313841` and `37551340654` pass; Vita native smoke
`37551372432` passes. A real-APK host matrix with an empty `game/` accepts Gen
13, Android14 13, Español 13, Invasion 22 and ZuperSamu 92. Invasion explicitly
cannot resolve `bobj00.pac` from anywhere else.

**Full candidate:** `DBTapBattle-Vita-00.28-Standalone-Profiles.vpk`, 2,648,911
bytes, SHA-256
`4411302f1b7e673fe49c98bb9ce0b7fe47ed086a34e1ad03025735411d07cab2`;
runtime marker `fa9d7b6`; LiveArea PASS. Direct MP3/AAC/Vorbis support from
00.27 is retained.

**Result:** HOST/BUILD STANDALONE CONTRACT CONFIRMED. Physical Vita validation
with `game/` empty is pending. If the original TeaVM core requests an asset a
modified APK omits, do not restore fallback; capture the request and port the
profile-specific loading behavior with evidence.

## 2026-10-07 — Attempt 033 — 00.28 hardware findings → 00.29

**Hardware input:** Invasion runs standalone with textures and external audio
working; its result/dialog box text is corrupted. Samu, both with and without
`game/`, detects 92 characters and exits after the title. Both Samu logs fail
while replacing MP3 BGM with `sceAudiodecCreateDecoder 0x807f0007`.

**Audio diagnosis/fix:** Samu `bgm_16.ogg` and `bgm_00.ogg` are byte-identical
valid MP3. SceAudiodec is initialized for one MP3 stream; 00.28 opened the next
decoder before closing the previous one. 00.29 serializes teardown under the
audio lock before opening the replacement. No audio conversion/repacking.

**Invasion text diagnosis/fix:** its Ranma character BIN contains valid UTF-8
(`Ranma está disponible！`) and native content sniffing identifies the character
PAC as UTF-8. Invasion's modified DEX has a substantially different SetString
method, so the preserved Gen slot expression can miss the per-GameData charset
map. 00.29 keeps exact mappings first and uses the detected active-character
charset only when that lookup misses.

**Save change:** exact user-provided 12,906-byte save is VPK seed; all profiles use
`ux0:data/DBTapBattle/save.bin`. Existing root save is never overwritten.
Extractors stop installing APK-local saves.

**Evidence level:** hardware findings are confirmed for 00.28; 00.29 fixes are
source/CI/build validated and require physical retest.

## 2026-10-07 — Attempt 034 — one VPK seed, independent save per profile

**Reason:** after reviewing how community mods store character/progression data,
the user rejected 00.29's single mutable global save. Different mods may reuse
slots differently, so sharing mutable state can couple otherwise independent APK
datasets.

**Change:** retain the exact 12,906-byte user-provided save in the VPK as the only
initial seed. On profile initialization, choose `game/save.bin` for Original or
`mods/<Profile>/save.bin` for a mod. If that file is missing, copy
`app0:/save.bin` once using the existing atomic temp/fsync/rename publication.
If it already exists, load it unchanged. The historical root 00.29 save is no
longer consulted.

**Importer policy:** APK-bundled saves remain excluded from installed datasets;
the manifest records their presence instead. This guarantees every fresh profile
starts from the same known VPK seed while remaining independently writable.

**Version:** 00.30. Native smoke and extractor/profile CI cover the source-policy
change; physical isolation validation remains pending.

## 2026-10-07 — Attempt 035 — 00.31 infinite Loading regression

**Hardware evidence:** both Invasion and Zuper/Samu remain alive at ~60 FPS but
never advance beyond Loading. Their last meaningful line is the rejected
`device/screensize.csv` request.

**Root cause:** the 00.31 Downloader stub inverted the original API meaning.
Direct inspection of the pinned APK proves `Downloader.isDownload()` returns the
DownloadTask `bDL` flag: true while the request is still running, false after the
worker completes. Returning true permanently therefore blocks the preserved TCB
state forever.

**Second startup regression:** 00.31 called the deep PAC audit before original
engine startup. Hardware timestamps show roughly 13 seconds for Invasion and 34
seconds for Samu between profile selection and VFS initialization.

**00.32 fix:** offline requests complete immediately with no data and the runtime
uses a presence/contiguity roster scan at startup instead of opening/parsing every
character PAC. The 00.31 roster, Shop, large-PAC memory and text/audio fixes are
retained.

## 2026-10-07 — Attempt 036 — 00.32 Invasion Saitama -> Freezer bad_alloc

**Hardware result:** 00.32 fixes the Loading loop and the user confirms complete
mod rosters now appear. Invasion still crashes reproducibly when Saitama advances
to his second fight against Freezer.

**Crash evidence:** the supplied Vita coredump resolves the native stack through
`operator new -> std::vector<unsigned char>::operator= -> normalise ->
normaliseEnginePac -> readEngineResource -> EngineResourceCache::read ->
GameData.Init`. The failing normalise return address is immediately after a
GCC-generated vector copy-assignment.

**Root cause:** protected Invasion `char15.pac` is ~4.05 MiB on disk and ~4.64 MiB
normalized. The source expression `output = changed ? std::move(out) : input`
looked like an ownership transfer but GCC 15 lowered the conditional to copy
assignment on the changed path. That requested a second PAC-sized contiguous
allocation after the normalized output already existed. It succeeds in some
transitions but fails after heap fragmentation in the reproduced second fight.

**00.33 change:** use explicit `if (changed) output.swap(out); else output = input;`.
Protected/changed PAC ownership transfers without allocation. Ordinary unchanged
PACs preserve their intentional byte-for-byte copy. No PAC, DEX or original
gameplay logic is modified. A source regression guards against reintroducing the
conditional assignment.

**Status:** full 00.33 VPK builds and LiveArea validates. Hardware retest of the
exact Saitama -> Freezer path is pending.

### Attempt 036 hardware follow-up — RESOLVED

00.33 was tested on a physical Vita after the coredump-driven ownership fix. The
user reports several fights completed without a crash, including continued
Invasion play after the previously reproducible transition. The 00.32
`std::bad_alloc`/protected-PAC duplicate-allocation issue is therefore closed in
the tested scope. Keep the explicit `output.swap(out)` ownership transfer and its
regression test; do not restore the conditional vector assignment.
