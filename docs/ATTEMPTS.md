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
