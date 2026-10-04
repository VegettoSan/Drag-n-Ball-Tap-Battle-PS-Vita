# Failures and Dead Ends

This file exists to prevent repeated work. A failed approach is valuable when its conditions and evidence are recorded precisely.

## Entry template

```markdown
## YYYY-MM-DD — Failure NNN — Short title

**Attempt**
Link/reference to the corresponding entry in `ATTEMPTS.md`.

**What failed**
Exact symptom.

**Conditions**
Commit/build, device, resource files and configuration.

**Evidence**
Logs, dump, screenshot, hash, return code, etc.

**Likely cause**
Confirmed cause or clearly marked hypothesis.

**Do not repeat**
The exact unchanged action that should not be tried again.

**A retry is justified only if**
What must materially change before trying again.
```

---

No port implementation failures have been recorded yet.

## Important policy

A result that is merely untested is **not** a failure. A hypothesis is **not** a confirmed cause. Record both accurately so later work does not build on false certainty.

## 2026-10-04 — Baseline extractor integrity weaknesses

**Attempt:** 006; baseline 1e3699b source review.
**What failed:** unguarded manifest write ignored --overwrite, nested raw files
were skipped, and earlier resources were published before discovering a late
conflict/CRC failure. Manifest loss follows directly from unconditional write.
**Do not repeat:** direct incremental extraction without full preflight/staging.
**Different approach:** validated path/collision budget plus staged CRC checks;
regressions now exercise manifest preservation and corrupt late records.

## 2026-10-04 — Baseline VFS path and presence mistakes

**Attempt:** 007.
**What failed:** std::string containing NUL passed safety validation but C I/O
would resolve only its prefix. originalDataPresent used active-mod resolve and
accepted any existing common.pac path, including directories. Legitimate names
containing '..' were incorrectly excluded from discovery.
**Do not repeat:** substring-only traversal checks, exists-and-not-directory as
regular-file validation, or active-overlay resolution to check original data.
**Different approach:** validate components/control bytes, require regular files,
and check game/common.pac independently. All covered by host regressions.
