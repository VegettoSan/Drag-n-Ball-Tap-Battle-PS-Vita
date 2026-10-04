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
