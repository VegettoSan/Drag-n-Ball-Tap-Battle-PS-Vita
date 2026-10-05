# Confirmed Successes

Only add items here when they are demonstrated by evidence. Distinguish PC-side format validation from behavior confirmed on real Vita hardware.

## 2026-10-04 — Original APK is suitable for source-level reconstruction

**Scope:** confirmed from supplied APK inspection.

- No `lib/*.so` native game libraries were found in the APK.
- The game logic is primarily Java/Dalvik rather than a closed native ARM game binary.
- Original game resources are directly accessible in the APK under `res/raw/`.
- Original audio includes OGG resources.

**Why this matters:** the port can focus on reconstructing platform/game layers in native Vita code instead of binary translation of an Android native executable.

## 2026-10-04 — PAC outer container layout validated

**Scope:** confirmed on PC against the supplied original APK.

- Entry count is a little-endian `uint16_t`.
- Each table record is 16 bytes: `uint32 offset`, `uint32 size`, `char type[4]`, `uint32 reserved`.
- Entry offsets are relative to the data block at `2 + count * 16`.
- `back00.pac` entry 0 resolves exactly to a PNG signature.
- All eight `back00.pac` entries resolve inside file bounds.

**Why this matters:** original PAC files can be consumed directly by the Vita port and community PAC replacements can potentially remain unchanged.

## 2026-10-04 — Original runtime data can be extracted without conversion

**Scope:** FORMAT CONFIRMED against the supplied APK.

- 57 regular files were found under `res/raw/` and extracted byte-for-byte.
- The dataset contains 19 `.pac` files and 36 `.ogg` files plus auxiliary resources.
- Per-file SHA-256 hashing works and can be written to a manifest.
- Example validated hash: `back00.pac` = `a19c425b0496aadc780d3a12b47fad91363423c9b5944407bdd5f74d64f18012`.

**Why this matters:** users can prepare `ux0:data/DBTapBattle/game/` directly from their own APK without repacking or changing the original resources.

## Success levels used by this project

- **FORMAT CONFIRMED** — validated against original files on PC.
- **BUILD CONFIRMED** — compiles/links successfully with VitaSDK.
- **VITA3K CONFIRMED** — observed working in Vita3K.
- **HARDWARE CONFIRMED** — observed working on a real PS Vita.

Whenever possible, promote discoveries through these levels rather than assuming PC-side success guarantees Vita behavior.

## 2026-10-04 — Complete inventory and hardened extractor

**Scope:** FORMAT CONFIRMED / host validation.
19 outer PACs and all nested SPR containers audited; metadata/hashes committed
in evidence/apk_inventory.json. All 57 extracted raw SHA-256 values match that
independent report. Five regression groups cover exact bytes, UTF-8/nesting,
unsafe paths/collisions, manifest overwrite, symlinks and late CRC failures.

## 2026-10-04 — Native PAC and mod overlay validated on host

**Scope:** FORMAT CONFIRMED; not hardware confirmation.
Actual src/pac.cpp reads every entry from all 19 original PACs. Host regressions
exercise malformed/truncated tables, 32-bit overflow offsets, empty PACs,
non-terminated tags, per-entry allocation budget, changed backing file length,
failed-open table isolation, mod precedence/fallback, original detection,
nested paths, UTF-8/spaces, traversal/NUL rejection and symlink escapes.

## 2026-10-04 — Vita bootstrap compiles and packages

**Scope:** BUILD CONFIRMED.
A real VitaSDK GCC 15.2.0 hard-float build creates ARM ELF, Sony VELF, SELF and
VPK. Host decoder reads all 51 outer-PAC PNGs and rejects truncated PNGs.
This does not confirm startup, selector, texture display or shader compiler on
Vita3K/hardware. Those remain explicitly PENDING.

## 2026-10-04 — Audio and internal-table corpus checks

**Scope:** FORMAT CONFIRMED for the stated fields only.
All 36 Ogg files are Vorbis, 44100 Hz: 17 stereo BGM and 19 mono SE.
Six SPR payloads validate as nested PACs. Seventeen raw DAC animation tables
have in-bounds action-index start addresses. This confirms table headers/index
arithmetic, not complete animation command decoding or native audio playback.

## 2026-10-04 — Final source-pinned native package

**Scope:** BUILD CONFIRMED / host validation.
Clean Release rebuild from c13f4a2f312726294a1b637e545f908cf110e5d0
succeeds. ARM ELF hard-float attributes, SELF header, VPK CRC/member set and
SFO DBTB00001 / 00.02 were checked. Exact VPK hash is in
evidence/build_validation.json. Host regressions pass on 19 PACs / 51 PNGs,
including 258 mod directories, a 255-byte component and oversized valid-CRC
IHDR rejection. Runtime confirmation still requires Vita3K or physical hardware.

## 2026-10-04 — Community Android14 packaging recovered

**Attempt:** 012. Verified ext.o/ext.u profile, all 106 outer and six SPR tables,
390 raw-DEFLATE RGBA textures; 65 original-derived textures match after exact
floor-alpha premultiplication. All 36 Ogg files and mk.bin are identical. The
seven libabc libraries match the public SWB byte-for-byte, establishing a shared
loader rather than an author identity. Metadata-only evidence committed; no
commercial payload, Java or Android helper binary redistributed.

## 2026-10-04 — Original and community APK imports verified

**Attempt:** 013. Nine extractor regressions pass. Real APK imports preserve all
57 original and 144 community entries byte-for-byte. Confirmed PAC aliases become
canonical names, including all 13 char/chardemo/charf triplets. --mod keeps the
original installation intact; unknown data stays preserved and profile/conflict
errors occur before publishing resources.

## 2026-10-04 — Native host decoding of both APK formats

**Attempt:** 014. Original VFS/PAC tests and PNG regressions remain passing. Native
ASan/UBSan corpus reads 137 containers, decodes 470 textures, including all 390
community textures and 80 originals with nested SPR. Explicit alpha state avoids
applying alpha twice to community pixels. Corrupt tables/DEFLATE/index/budget
cases fail cleanly. This is HOST CONFIRMED, not a new Vita build/device result.

## 2026-10-04 — Native initial game/text tables for both APKs

**Attempt:** 015. The original InitGameData pair now loads natively, using each
resolved PAC's codec. Both supplied APKs decode 271 game records + one text
record. ASan/UBSan tests preserve modified values and verify mixed-profile mod
override/fallback, row/column bounds and atomic failed loads. This is HOST
CONFIRMED; the Vita program still shows a diagnostic atlas after initialization.

## 2026-10-04 — Private dual-profile Vita data ZIP

**Attempt:** 016. Both APKs packaged under the real ux0: data layout, with 201
unchanged runtime files, provenance manifests and per-file hashes. Archive CRC
and content hashes pass. prepare_vita_data.py reproduces the package from the
user's APKs and refuses overwrites; no game assets enter the source repository.

## 2026-10-04 — Current ARM build and original input AOT experiment

**Attempts:** 017–018. Current native bootstrap source 2c3fecd packages correctly
with the matching SDK (still diagnostic). The original APK's KeyData/Controller
passes a JVM/native C parity probe, including button edges and pointer behavior.
An isolated runtime adaptation links to a real Vita ARM ELF and valid SELF.
UTF-16 host tests pass. This demonstrates a reusable Java-core experiment, not
full-engine porting or device execution; the full Init/Run remains blocked by
unadapted Android dependencies. Generated game code stays outside Git.


## 2026-10-04 — Complete original core generation

Attempt 019 generates the reachable original Init/Run engine with TeaVM 0.12.3:
456 classes and 3989 methods, no compiler diagnostics; a fresh reproduction passes.
TCBManajer/Game1..17, TCB/ObjReq, input/controller, graphics batching and the
byte-array GameData/SpriteData decoder remain supplied original bytecode. One
hash-pinned Android resource overload is adapted. Native linking/execution, full
menu/combat and playable release remain pending. See tools/aot/engine/README.md.
# 2026-10-05 — Full-core native compilation and resource adaptation

The original engine's generated C compiles on the host after a guarded TeaVM
static-byte default repair. ASan/UBSan checks pass for 125 PAC files, 137 nested
containers and 470 images across both profiles. Original bytes, community image
indices, table cells and unknown metadata are preserved by the memory adapter.
GL/resource services also compile for Vita. This does not confirm device play.

## 2026-10-05 — Complete charset mapping and retained Unicode

Attempt 021: all 65,792 single/two-byte Shift_JIS decode cases match Java.
The resource-specific charset boundary keeps community UTF-8 strings intact and
respects original fallback. Generation passes; native execution is a separate check.

# 2026-10-05 — Original title and main menu executed on the host

The original complete core reaches its animated title and main menu with the
supplied Android14 assets, responding to injected original touch events. The
original profile also renders its startup and Japanese text. These are ASan
host OpenGL runs; Vita hardware, selection and complete battle remain pending.

## 2026-10-04 — vitaGL startup and data-profile scan on real PS Vita

**Scope: HARDWARE CONFIRMED for renderer bootstrap and VFS scan only.**
The 00.04-installfix VPK displayed the vitaGL splash on a physical PS Vita. The
runtime log then confirmed native 960x544 initialization without framebuffer
fallback and completed the data/mod profile scan, finding the installed Android14
profile. The following crash was isolated to the port's own immediate-mode selector,
not to vitaGL initialization or the original game engine. See
`docs/evidence/vita_hardware_selector_crash_00.04.json`.

## 2026-10-05 — Original game boots and runs on real PS Vita

**Scope: HARDWARE CONFIRMED for the Original profile startup/runtime path.**
Build 00.08 displayed the actual original game on a physical PS Vita without a
crash. The hardware log confirms vitaGL startup, the profile selector, Original
VFS initialization, native audio startup, `TCBManajer.Init()` success and two
continuous `TCBManajer.Run()` sessions reaching 1314 and 624 frames. The prior
frame-498 text/lifecycle failure is resolved. Front-touch calibration and physical
button mapping remained the next input-specific blockers; see
`docs/evidence/vita_hardware_game_boot_00.08.json`.

## 2026-10-05 — Front touch, audio and menu navigation on real PS Vita

**Scope: HARDWARE CONFIRMED through Android14 character-selection entry.**
Build 00.10 fixed the Vita touch-ID bridge by mapping hardware report IDs onto
the original engine's logical pointer slots 0–4. On a physical PS Vita the front
touchscreen then responded correctly throughout the game. The submitted runtime
log confirms the 1919×1087 active touch grid, multiple Begin events mapped to
logical slot 0, and continuous original-engine execution.

The user also confirmed native game audio is audible. The Original profile ran
1054 frames in the captured session. The Android14 profile ran 1502 frames,
accepted real touch navigation through the menus and reached character selection,
where at least one character rendered before a caught Java exception ended the
session cleanly. No `psp2core` was produced.

Short black intervals of roughly 1–2 seconds were observed during some screen
transitions. They are currently recorded as a loading/transition performance
issue, not as a crash or rendering correctness result. Character-selection
continuation and a complete battle remain pending hardware confirmation. See
`docs/evidence/vita_hardware_touch_character_00.10.json`.

## 2026-10-05 — Android14 reaches and plays a real battle on PS Vita

**Scope: HARDWARE CONFIRMED through actual combat.**
Build 00.11 fixes the Community14 top-level character GameData BIN metadata and,
on a physical PS Vita, proceeds beyond the former first-character exit into an
actual battle. Character selection, front-touch navigation, game sound and combat
execution are therefore hardware confirmed for the Android14 profile. The user
reported gameplay holding 60 FPS in normal operation, with intermittent stalls;
that frame-rate observation is not an instrumented benchmark and is recorded as
user-observed hardware behavior rather than a timing guarantee.

The submitted 00.11 `runtime.log` contains two Android14 sessions, both reaching
`ORIGINAL ENGINE INIT PASS`, starting the Vita audio output/worker and accepting
multiple normalized front-touch events without a fatal/NPE trace. Remaining
adapter defects observed during real combat are character voice/WAV playback and
dialogue text stalls/layout drift; these are addressed separately in build 00.12.
See `docs/evidence/vita_hardware_battle_00.11.json`.

## 2026-10-05 — 00.16 cards and startup; 00.18 build/host checks

**00.16 HARDWARE CONFIRMED:** user reports the ability-card processing and long
startup delay are fixed. Character-switch pauses, rough voices and combat
35–45 FPS remain user-observed issues in this same report; do not describe 00.16
as uniformly 60 FPS.

**00.18 HOST/BUILD CONFIRMED:** current TeaVM generation and complete Vita ARM
ELF/VELF/SELF/VPK packaging succeed. Mixer ASan/UBSan and JVM client-buffer
behavioral probes pass. In-memory PVF font opening, buffer reuse and faster audio
mixing are implemented, but their performance on physical Vita is still PENDING.
