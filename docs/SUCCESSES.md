# Confirmed Successes

> **Historical document notice — current v1.0 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for v1.0. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


## 2026-10-07 — Web Extractor 1.0 — local browser packaging validated

A new static GitHub Pages extractor under `web/` implements the current
`profiles-v1` contract without requiring Windows. The APK stays on the user's
device; the page uses browser File/Blob/stream APIs and its Content Security
Policy blocks network connections from the extractor itself.

Confirmed before publication:

- JavaScript syntax/unit checks PASS;
- exact validated Gen selector artwork reused by the web UI;
- responsive layouts for desktop, portrait mobile and landscape mobile;
- original DBTapBattle APK extraction PASS;
- Gen APK extraction PASS, 13-character roster;
- Android14 protected extraction PASS using `community14-a210795b`;
- Spanish Android14 extraction PASS using `community14-es-d594affc`;
- Invasion Beta 3 extraction PASS using
  `community14-invasion-05aa0c5e`, 22 characters `00..21`;
- Samu extraction PASS, 92 characters `00..91`, 383 gameplay/data files;
- generated Invasion, Spanish and Samu Vita ZIPs reopened with CRC validation PASS;
- APK-bundled `save.bin` remains excluded; Samu explicitly exercised this path.

GitHub Pages deployment run `37703911061` completed successfully after build,
tests, selector materialization and artifact upload all passed. GitHub reports
the live URL as
https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/.
An earlier deployment API call returned a transient HTTP 500, then succeeded on
the subsequent deployment.

See [WEB_DATA_TOOL](WEB_DATA_TOOL.md).

## 2026-10-07 — 00.34 HARDWARE CONFIRMED — stable unified profiles and selector

The user tested `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` on a
physical PS Vita and reports it **stable and functional**, with no issue found so
far in the exercised session.

Exact artifact:

- SHA-256:
  `24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03`
- source:
  `0da8684805d1510caf93130a22eed523a854c1d6`

Hardware-confirmed in the reported test:

- unified `profiles/` data selection;
- no artificial Original/missing row;
- fullscreen background without the blue orb;
- centered themed buttons;
- labels centered inside the cyan/blue button area;
- profile-opening/loading transition;
- successful transition into the game;
- no crash or new functional regression observed so far.

00.34 is now the current stable hardware checkpoint. 00.33 remains historical
evidence for the protected-PAC repeated-fight allocation repair.

## 2026-10-07 — 00.34 build/tool success — unified profiles and selector UX

**Scope:** superseded by the HARDWARE CONFIRMED 00.34 result above.

- Runtime source uses only `profiles/<Profile>/` for current datasets and saves.
- Selector no longer synthesizes Original/missing rows.
- Empty profile root has an explicit no-data state.
- Gen background visible bounds fill the 960×544 selector viewport.
- Confirming a profile presents a themed opening/loading indication.
- Vita engine native smoke `37679405794` passed for source checkpoint
  `63bc0f90d33d4a5d8d90c4816ff0f0ae07272751`.
- Windows extractor 1.5 `profiles-v1` regression `37688246446` passed.
- Complete user-test VPK SHA-256:
  `e06ded147eead1c7ee8e5a558552d5129a98b5916395c59780125782c8d55c92`.

This build-only note is historical. The later exact 00.34 button/text-center
VPK was tested on a real Vita and promoted to HARDWARE CONFIRMED above.

## 2026-10-07 — 00.34 Gen-styled selector builds and packages in VitaSDK CI

**Scope: BUILD CONFIRMED; physical Vita pending.** The native first-screen data
selector now renders with four non-character visual derivatives from the supplied
Gen `assets/select0.pac`: grid/energy background, beveled header, beveled menu
button and a one-star Dragon Ball marker. The profile/VFS/input contract is
unchanged, selector textures are released before entering the engine, and a
legacy flat-selector fallback remains available if any embedded texture fails.

The split Base64 payload reconstructs to the pinned ZIP SHA-256
`90418a27c6681ee644d5cc383e31fc73248a5c412527839d216b61bcc2516c12`;
each materialized PNG also has a pinned hash/dimensions contract. VitaSDK native
smoke run [37622687132](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37622687132)
passes with the themed renderer and VPK packaging, while publication/actionlint
validation run [37622780686](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37622780686)
passes the selector regression suite. This does not supersede the 00.33
hardware-confirmed gameplay checkpoint.

## 2026-10-05 — publication guardrails validated

Manual Release/Prerelease definitions pass actionlint. Ten tests verify original
input/provenance checks, rejection of native-only probes, corrupted/private
release assets, preservation of existing tags, separate latest/prerelease flags
and refusal to publish incomplete drafts. Real 00.24 ELF/SELF/VPK staging passes
with 467 classes/4086 methods, approved LiveArea, original hash and compiled-only
symbols. GitHub-hosted [validation run 37395626518](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37395626518)
also passed actionlint and all 13 publication/LiveArea tests on commit
`b17bb49ef1ab257ea74f68353a907b4f538c1c89`. These are static/host results,
not a completed full remote build or publication.

## 2026-10-05 — 00.24 confirmed on physical Vita

User report: “Ya funciono, queda super bien”. The exact 00.24 full-build VPK
works after the Android14 native Ogg battle-start failure. LiveArea remains
confirmed. This promotes the previously pending device retest for this artifact,
not arbitrary mods or every character/mode/long session. See
[evidence](evidence/vita_battle_audio_00.24.json).

## 2026-10-06 — LiveArea on device; exact PCM allocation on host

User confirms 00.23 LiveArea-Fixed presentation on the physical Vita. The new
00.24 decoder preserves every PCM sample/frame/channel/rate of all 17 private
BGM tracks compared to the legacy loader. Under a 6 MiB single-request ceiling,
legacy bgm_03 throws bad_alloc and all fixed tracks succeed. bgm_03 C++ peak drops
8,805,892 bytes. Native audio/DSP and resource ownership tests pass ASan/UBSan
(Vita audio/GL APIs mocked); cache reclamation retains active streams/textures.
These do not establish 00.24 hardware battle recovery, which remains pending.

**Reading checkpoint — 2026-10-05 / 00.21.** Entries retain the source/build and
evidence available when recorded. Historical pending items can be superseded;
do not treat them as current blockers or retroactively promote their success.
The current full-core AOT status, delivered artifact and open physical checks
are in [CURRENT_STATUS](CURRENT_STATUS.md). Commands/mock scope are in
[VALIDATION](VALIDATION.md); reusable lessons in [PORTING_GUIDE](PORTING_GUIDE.md).

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
## 2026-10-05 — Full-core native compilation and resource adaptation

The original engine's generated C compiles on the host after a guarded TeaVM
static-byte default repair. ASan/UBSan checks pass for 125 PAC files, 137 nested
containers and 470 images across both profiles. Original bytes, community image
indices, table cells and unknown metadata are preserved by the memory adapter.
GL/resource services also compile for Vita. This does not confirm device play.

## 2026-10-05 — Complete charset mapping and retained Unicode

Attempt 021: all 65,792 single/two-byte Shift_JIS decode cases match Java.
The resource-specific charset boundary keeps community UTF-8 strings intact and
respects original fallback. Generation passes; native execution is a separate check.

## 2026-10-05 — Original title and main menu executed on the host

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

00.18 resource regressions also pass for the actual supplied APKs: 125 Original/
Android14 PAC files, 470 textures and 198 decoded WAV entries; gen.apk passes
108 PAC files, 405 PNGs and 198 aligned PCM voice entries. All 12 Python tests
pass. The native-only GitHub CI job is green at b6d859c (run 37357204717); that
job remains a smoke link probe and is not the delivered full game VPK.

## 2026-10-05 — 00.18 steady battle performance reaches 60 FPS on Vita

**Scope:** HARDWARE CONFIRMED by user report and current 3302305 runtime.log.
Steady battle windows record 59.9 FPS at 960x544 with original game behavior.
Requested clock API succeeds; effective CPU 444, bus 222, GPU 222, crossbar 166.
No measured mix-computation deadline misses. Resource-loading pauses, missing
text and raspy audio remain unresolved in this tested version; performance
success does not validate those systems. See
`evidence/vita_hardware_performance_00.18.json`.

## 2026-10-05 — Host regressions cover PVF rectangles and audio saturation

**Scope:** HOST CONFIRMED only, native PVF/audio hardware calls mocked.
`tests/test_vita_text.cpp` fails with 00.18's shortcut and passes after restoring
the independent image rectangle, including actual nonzero ASCII/CJK surface
coverage. `tests/test_vita_audio.cpp` passes ASan/UBSan with no post-limiter
clipping under overlap, preserved waveform/stereo ratios, bounded release,
unchanged quiet unity output and RIFF metadata/truncation handling.
Private validation reads/mixes all 198 supplied gen.apk RIFF voices, preserves
their PCM samples byte-for-byte and adds no clipping when each plays alone.
No source assets are committed. Audible quality/text recovery on Vita are pending.

## 2026-10-05 — Text returns on Vita 00.19

**Scope: HARDWARE CONFIRMED by the user's test.** Previously missing text is
visible again with the restored PVF image-rectangle path. Latest 00.19 log also
records nonzero glyph coverage. Voices and character switching remain failures;
see `evidence/vita_hardware_text_audio_00.19.json`.

## 2026-10-05 — Selective PAC bytes and voice DSP verified on host; 00.20 builds

**Scope: HOST/BUILD CONFIRMED only.** The original filter is preserved before disk
reads for ordinary and Community14 PACs, with exact selected payloads and stable
directory slots. Across 26 character PACs, filter 33 reads 87.146% fewer requested
source bytes. Bounded PAC/PCM caches and immutable texture reuse pass invalidation,
collision, filtering, ownership and eviction checks under ASan/UBSan.

The new voice reconstruction filter reduces a measured resampling image by
32.3 dB for an 8 kHz source test without changing duration or original rates.
Full resource/audio/text checks, 12 Python tests and JVM buffer probe pass.
The complete regenerated original-engine ARM VPK 00.20 builds and its metadata,
CRC and eboot identity pass. These host results do not establish audible clarity,
physical selection latency or retained battle FPS; hardware testing is pending.
See `evidence/vita_pac_voice_build_00.20.json`.

## 2026-10-05 — Audio startup failure paths verified on host for 00.21

**Scope: HOST CONFIRMED, platform calls mocked.** The real native adapter passes
ASan/UBSan for successful setup reuse and failures during port-open, thread-create
and thread-start. Error operation/code logging, ownership cleanup, no repeated
setup during the same failed session and retry after disposal are verified.
DSP tests and 12 Python regressions remain green. This corrects test coverage
that previously assumed every thread setup succeeds. Restoring the working
priority is implemented; physical worker startup/menu recovery is pending.

The complete privately regenerated original engine also builds and packages as
00.21 at 07222bb. Native PAC/PNG/texture ownership checks pass ASan/UBSan; VPK
CRC/SFO/title/eboot and original-engine/source markers pass. See
`evidence/vita_audio_startup_build_00.21.json`. This is build/host evidence only.


## 2026-10-05 — Documentation checkpoint and future-port handoff reconciled

**Scope: DOCUMENTATION VALIDATION only.** All 29 pre-existing Markdown files
were reviewed/updated and three guides added: CURRENT_STATUS, VALIDATION and
PORTING_GUIDE. Local Markdown targets resolve; the current status/test sheet
matches the recorded 00.21 VPK source and SHA-256. Changes are Markdown only.

Build recipes identify full engine versus bootstrap/CI, preserve private AOT
inputs and specify adapter/mocked-test scope. Current PAC filters/cache ownership,
profile save paths and supplied APK differences are reconciled with source.
Historical test sheets retain their protocol and identify subsequent outcomes;
00.20 is marked with the startup regression, 00.21 physical recovery is pending.
This success does not add a gameplay/device/audio confirmation or a new VPK.


## 2026-10-05 — 00.21 audio worker and menu recover on physical Vita

**Scope: HARDWARE CONFIRMED by user and new 07222bb session.** Port opens, worker
starts and menu is reached. This closes the earlier startup checkpoint, not
selection/voice/battle coverage: char00 is rejected next and Game3 exits.
[Evidence](evidence/vita_hardware_selection_00.21.json).

## 2026-10-05 — 00.22 original filter regression covered and full VPK verified

**Scope: HOST/BUILD CONFIRMED only.** Previous source fails at real char00/filter187;
corrected stream tests pass 26 Gen/Community14 packs with 187/251 and high/sign-bit
masks, exact allowed payloads/slots, nonempty metadata and expected voice banks.
Native filtered import/copy/cache and PNG/ownership probes pass (GL mocked),
as does full 125-file/137-container/470-texture/68-table/198-WAV ASan/UBSan regression.
The full original core is recompiled to ARM ELF/VELF/SELF/VPK 00.22; CRC/SFO/eboot,
core symbols/source/version pass. Physical selection/voices/FPS remain pending.
[Build evidence](evidence/vita_selection_filter_build_00.22.json).

<!-- DBTB_00_23_DETAIL:START -->
## 2026-10-05 — 00.23: first reported session with battle startup working after the memory crash

A physical Vita test of 00.23 completed the path that failed in 00.22. Clean audio
and responsive character selection were preserved, the fight started successfully,
and the user reported no error during the tested session.

The key reusable success is architectural: the port no longer copies an entire PAC
into a TeaVM-managed `byte[]` merely to feed code that already had an original
streaming parser. The Vita adapter now supplies a native-backed `InputStream` while
preserving original parsing and resource-lifetime semantics.

Test VPK SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current public release — v1.0 / DBTB01178 (2026-10-07):** the 00.34
> gameplay/runtime baseline is hardware-confirmed stable for the tested paths.
> Earlier 00.33 hardware evidence remains valid for the protected-PAC repeated-fight
> repair, Loading recovery and dynamic rosters. Historical artifact identities are
> preserved; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## 2026-10-05 — Corrected LiveArea packaging passes host verification

The repaired package retains every entry of the hardware-tested 00.23 VPK exactly,
including eboot/SFO. MetalSyntax's LiveArea checks pass; pic0 now has 256 palette
entries with identical decoded RGBA pixels and retained IDAT bytes. Three targeted
regressions pass: short-palette rejection/repair, PNG CRC rejection and prevention
of executable substitution. These are host/package successes; the new LiveArea
installation and rendering still need physical Vita confirmation.
See [evidence](evidence/vita_livearea_fixed_00.23.json).

## 2026-10-05 — Windows extractor verified

BAT/PS1 extraction passes 13 Windows PowerShell 5.1 CI tests, including actual
CMD argument transport, atomic failure cleanup and byte/hash/save preservation.
Private real-APK checks under PowerShell 7.4.6 verify every output byte for the
57-resource original, 144-resource Android14 and 147-resource Gen APKs.
Android14 needs 106 aliases but no payload conversion. Windows tool needs no
Python, Java, 7-Zip or administrator access on the user's PC. This is extraction
evidence, not new Vita gameplay coverage.
See [evidence](evidence/windows_extractor_2026-10-05.json).

## 2026-10-06 — Samu large-roster and audio preparation path validated on host

**Scope: HOST/IMPORT CONFIRMED; physical Vita pending.**

The Vita installation gate now accepts contiguous complete character triplets
across the full two-digit ID namespace 00..99. Regression coverage passes for
13 (baseline), 22 (Invasion), 92 (the audited Samu count) and all 100 two-digit
positions. This changes no original selection/combat method.

The Samu-specific importer is hash-pinned to the audited APK/DEX and validates
all 92 supplied triplets. Direct testing with the supplied APK converted its
12 MP3 + 3 AAC/M4A BGM to real Ogg Vorbis 44.1 kHz stereo while preserving
`bgm_XX.ogg` names; the two source Vorbis tracks remain unchanged. This lets
Samu use the existing 00.24 libvorbisfile path instead of adding an untested
runtime codec.

Synthetic CI run `37540898687` passes. This success does not yet claim that
all 92 characters or all normalized tracks have been exercised on a physical
Vita; use `TEST_VITA_00_26.md`.


## 2026-10-06 — 00.27 preserves Samu audio bytes and links direct Vita MP3/AAC decoding

**Scope: HOST/BUILD CONFIRMED; hardware pending.**

The conversion requirement from 00.26 has been removed. The audited Samu APK now
extracts all 384 assets unchanged, including its 12 MP3, 3 AAC/M4A and 2 Vorbis
BGM files behind the original `bgm_XX.ogg` names.

The Vita audio adapter detects codec from content. Vorbis retains the
hardware-established libvorbisfile path; MP3 uses `SceAudiodec` MP3, and M4A is
demultiplexed in memory so its original raw AAC access units can be decoded by
`SceAudiodec` AAC. No original selection/combat Java method is replaced.

Real-source parsing verifies 44.1 kHz stereo for all compressed Samu BGM and
valid M4A access-unit tables for `bgm_09/10/11`. Public CI passes Community mod
profiles run `37544623252` and Vita native smoke run `37544588962`.

A complete 00.27 full-engine VPK was built and validated:
`bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`.
It contains the expected original TeaVM symbols, direct compressed-audio symbol,
approved LiveArea and no APK/game-data files. Physical audible validation is the
remaining gate. See [evidence](evidence/vita_samu_direct_audio_00.27.json).

## 2026-10-06 — Protected profiles do not require bobj00

Direct audit of the supplied Android14, Spanish and Invasion APKs confirmed that
all three standalone protected builds omit `font00.pac` and begin their protected
`bobj` family at index 01. Therefore `bobj00.pac` is not a universal mod-profile
requirement. Commit `7fec715` removes the artificial `bobj00` requirement from
the Vita installed-data gate. The later 00.28 contract removes cross-profile VFS
fallback entirely; omitted resources stay visible as profile compatibility gaps. Community-profile
CI, installed-data regression CI and Vita native smoke all pass. The corrected
00.27 physical-test VPK has SHA-256
`311a820f948e337b0626b7b46e6dcb2ca0628b81364941b11d4c837867ee1b96`.

## 2026-10-06 — 00.28 real APK profiles pass with Original completely absent

**Scope: HOST/BUILD CONFIRMED; physical Vita pending.**

The selected-profile VFS is now isolated from `game/`. Regression coverage proves
that a missing selected-profile character PAC is rejected even when an identically
named file still exists under Original. With `game/` containing zero files, the
real extracted datasets pass the installation audit independently: Gen 13,
Android14 13, Español 13, Invasion 22 and ZuperSamu 92 characters.

A full 00.28 Vita VPK builds with direct Vorbis/MP3/AAC support retained:
`DBTapBattle-Vita-00.28-Standalone-Profiles.vpk`, SHA-256
`4411302f1b7e673fe49c98bb9ce0b7fe47ed086a34e1ad03025735411d07cab2`,
runtime marker `fa9d7b6`, LiveArea PASS. CI passes the profile suite and Vita
native smoke. This establishes the standalone filesystem/import contract, not
physical gameplay for every modified APK. See `TEST_VITA_00_28.md`.

## 2026-10-07 — 00.28 Invasion standalone hardware path works; 00.29 targeted fixes build

**HARDWARE CONFIRMED for 00.28:** Invasion runs without Original/`game/`; the user
reported textures and audio working correctly through gameplay. The remaining
visible defect is corrupted result/dialog text.

**DIAGNOSIS CONFIRMED:** Samu's standalone 92-character audit succeeds, and its
title exit is tied to decoder replacement error `0x807f0007`, independent of
whether `game/` exists. The adjacent Samu MP3 files involved are byte-identical,
which isolates decoder lifetime rather than source-media damage.

**BUILD/CI CONFIRMED for 00.29, hardware pending:** the compressed BGM handoff now
releases the old SceAudiodec stream first; protected character text gets a
content-detected charset fallback when the modified APK's SetString slot mapping
differs; and the exact approved save seed is packaged/read once into one global
writable save. These are not yet physical confirmation of the 00.29 fixes.

## 2026-10-07 — 00.30 VPK seed is profile-local

**HOST/CI CONFIRMED; physical save-isolation test pending.** The runtime now maps
Original to `game/save.bin` and a selected mod to `mods/<Profile>/save.bin`, while
keeping the approved `app0:/save.bin` only as a first-use seed. The seed hash and
VPK packaging regression remain unchanged. Existing per-profile saves are loaded
instead of re-seeded. Python/community profile suites pass with APK-local saves
excluded from installed datasets, and the Windows extractor regression passes the
new per-profile policy.

## 2026-10-07 — 00.32 dynamic rosters confirmed; 00.33 native crash path isolated

**HARDWARE CONFIRMED for 00.32:** the Loading regression is fixed and the user
reports all characters of the tested mods are now visible/loaded, including the
extended Samu roster. The earlier Invasion text issue was not observed in this
short test.

**COREDUMP/SYMBOL CONFIRMED for the remaining Invasion crash:** Saitama's second
fight against Freezer terminates with native `std::bad_alloc`; the matching
00.32 ELF resolves the allocation through `std::vector<unsigned char>::operator=`
inside protected-PAC `normalise()`, not through character gameplay logic or a
corrupt Freezer/Saitama asset. 00.33 removes that duplicate protected-PAC
allocation with ownership swap. Full build/packaging passes; physical 00.33
confirmation remains pending.

## 2026-10-07 — 00.33 repeated Invasion fights pass on physical Vita

**HARDWARE CONFIRMED.** The user reports several consecutive Invasion fights on
00.33 without a crash after the exact 00.32 `std::bad_alloc` was isolated to a
duplicate protected-PAC vector assignment. This validates the explicit
`output.swap(out)` ownership handoff on the affected real-device path.

The same testing session also preserves the already confirmed 00.32 fixes: the
startup Loading loop is gone and complete mod rosters are visible, including the
extended Samu roster. Invasion text remained correct in the tested path.

Scope remains bounded: this closes the reproduced Saitama/Freezer repeated-fight
crash, not every possible long-session/mod/mode combination. See
`evidence/vita_hardware_00.33.json`.
