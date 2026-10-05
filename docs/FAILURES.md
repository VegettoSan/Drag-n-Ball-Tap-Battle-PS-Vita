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

The initial baseline had no recorded failures. Confirmed findings from the
subsequent audit are recorded below.

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

## 2026-10-04 — Build environment setup failures

**Attempt:** 008 (environment, not a game runtime failure).
**Observed:** direct git push lacks a local HTTPS credential; authenticated
GitHub commit/ref operations succeeded instead. Latest nightly hard-float
package database lacked several renderer dependencies; use complete 2026.08
channel. A raw toolchain tarball has no pacman ownership records, so package
installation conflicts with existing files; install a consistent managed core
into the isolated toolchain root, then dependencies. Do not mix soft/hard float.
LeakSanitizer is unsupported under this runner's /proc restrictions; disable
only leak detection rather than dropping ASan/UBSan entirely.

## 2026-10-04 — Reject uniform internal-table and wiki state assumptions

**Attempt:** independent internal/motor audit following 005.
**What failed:** treating every CNV/DAC/GDT as binCnv's u16+8-byte table yields
out-of-range offsets on the real corpus. That conversion is mode-selected;
raw DAC animation and CNV rectangles have different encodings/endian fields.
Wiki 390='active fight' and 193='character selection' do not describe original
case handlers. DAD output also omitted valid original save writes.
**Evidence:** internal_tables.json; original DEX/jadx handler and method inspection.
**Do not repeat:** uniform generic decoder, wiki-only battle dispatcher, or
compiling decompiler output without bytecode and resource validation.
**Different approach:** per-schema decoders, original task dispatch ranges,
original bytecode/API evidence and corpus regression tests.

## 2026-10-04 — Original extractor/PNG assumptions do not fit community APK

**Attempt:** 012–014. The baseline raw-only extractor cannot import the supplied
assets/ dataset. Encoded metadata interpreted as ordinary PAC count/offsets fails;
PNG decoding cannot interpret its raw-DEFLATE RGBA, and straight-alpha blending
would darken premultiplied RGB. Renaming filenames alone is insufficient.
**Different approach:** verified per-file codec, canonical name import with
unchanged bytes, bounded DEFLATE RGBA and explicit alpha state. Do not declare
new constants/code mods compatible because they share the APK title/version.
Local apt cannot switch its sandbox UID here; PNG headers were obtained by
building the pinned official source locally, without altering system packages.
No missing-toolchain issue has been interpreted as a runtime/game failure.

## 2026-10-04 — Unadapted Android engine does not become a native game by AOT

**Attempt:** 018. Full Init/Run probe: 203 diagnostics/44 unique with the basic
classpath; missing GL/JSON are classpath gaps, while Android/network/security
and reflection require proper native services. JVM Android stubs throw immediately.
Do not claim compiler failure proves the core cannot be reused or that adding
stub classes supplies rendering/gameplay. Retest after actual boundary replacement.

Tooling corrections: Java needed proxy settings and an explicit Maven plugin
coordinate. TeaVM's greedy -p ate the main class; use JVM classpath plus --.
Compiling generated roots with -I shadows C string.h/time.h; compiling all.c
alongside its constituent files duplicates definitions. Build all.c once.

Stock TeaVM C also assumes GNU means POSIX and Newlib has uchar.h; Vita needs
explicit runtime adaptation. GCC 15 rejects a fallback non-void return and
Newlib lacks GNU timegm. The isolated input patch addresses only tested needs,
omitting unused Date rather than inventing it. Device/full-engine scheduler and
filesystem behavior remain unverified. No placeholder game release is published.


## 2026-10-04 — Adapter pass still reached Android streaming I/O

Attempt 019's first pass still needed Context.openFileInput/getResources and
ResourceMiner reflection, because GameData has a separate String loading overload.
Adding raw-ID stubs would not supply a working resource stream. The revised pass
redirects that one verified overload to the VFS and reuses the original byte-array
decoder; full generation then passes. The input-only Date omission is unsuitable
for this full core. Native service implementations are still required before linking.
# 2026-10-05 — TeaVM empty static-byte initializer

Full C compilation first failed on `bEventFlagBuf = ;`. The pinned original
field is a static byte without ConstantValue, so its JVM initial value is zero.
Generation now verifies both the class hash and field shape before replacing
the one defective C initializer with zero; gameplay bytecode remains unchanged.
The corrected output compiles. Do not reuse the uncorrected generated output.

## 2026-10-05 — Direct-buffer GC and lossy charset conversion

Attempt 021: ASan locates the old direct-buffer fault in TeaVM GC.freeBufferContent,
triggered after rendering allocations. Use heap-backed Java buffer views with
native-owned GL copies; do not suppress GC or enlarge its heap to hide the fault.
Community UTF-8 cannot be losslessly converted to ordinary Shift_JIS: U+3231
and U+2460 are present but unavailable. Resolve charset per table at the platform
text boundary instead of replacing characters or rejecting the whole table.
Original startup still requests initial downloadable data; installed-resource
offline startup remains to be validated.

## 2026-10-04 — Vita 00.03 treated successful vitaGL init as failure

**What failed:** the first full-engine hardware VPK went black and crashed before
showing any selector/game UI.
**Evidence:** `docs/evidence/vita_hardware_vgl_init_00.03.json` plus the submitted
runtime log and psp2core dump.
**Confirmed cause:** `vglInitExtended()` returns whether framebuffer resolution
fallback happened, not whether initialization succeeded. Native 960x544 returns
`GL_FALSE`; the port incorrectly returned startup failure and TeaVM threw.
**Do not repeat:** never use the return value of `vglInitExtended()` as a generic
success/failure flag.
**Different approach:** initialize vitaGL, treat the return only as a resolution-
fallback indicator, and log the post-init checkpoint.

## 2026-10-04 — Vita 00.04 selector used disabled immediate-mode pool

**What failed:** hardware showed the vitaGL logo and completed renderer/mod scan,
then data-aborted before the Original/Android14 selector became visible.
**Evidence:** `docs/evidence/vita_hardware_selector_crash_00.04.json`. The dump
maps the fault to `glVertex3f`, called by the custom selector `rect()` helper, with
DFAR `0x00000000`.
**Confirmed cause:** the full engine intentionally starts vitaGL with
`legacy_pool_size=0` because the original game renderer uses GLES client arrays,
but the custom selector still used `glBegin/glVertex3f` immediate mode.
**Do not repeat:** no `glBegin/glVertex*` UI while the legacy pool is disabled.
**Different approach:** selector and diagnostic quads now use
`glVertexPointer`/`glTexCoordPointer` plus `glDrawArrays(GL_TRIANGLE_FAN)`.
