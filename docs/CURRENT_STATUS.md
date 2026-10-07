# Current status — 2026-10-06 (America/Bogota), 00.24 hardware / 00.28 candidate

## 00.28 candidate — standalone APK-derived profiles

00.28 changes the runtime data contract so every selected APK-derived profile is
autonomous. `game/` is now only the optional Original profile. When
`mods/<Profile>/` is selected, `GameVfs` resolves PAC/data/audio **only** from
that directory and reports `missing selected profile resource: ...` rather than
borrowing from Original. Saves remain profile-local.

This is deliberate: Android14, Español and Invasion are independently runnable
APKs even though their inventories differ from Original/Gen. If the preserved
original TeaVM core asks for a resource that a modified APK does not need, 00.28
exposes that as a compatibility gap to adapt from APK/DEX evidence instead of
masking it with cross-profile fallback.

Selector behavior also supports a mod-only installation. When
`game/common.pac` is absent and one or more profiles exist, the first profile is
initially selected; `ORIGINAL - DATA MISSING` cannot be launched.

The standalone contract has two levels of non-hardware evidence:

- synthetic CI proves a missing file in the selected profile is **not** satisfied
  by an identically named file still present in `game/`;
- a real-APK host matrix with an empty `game/` accepts Gen (13 characters),
  Android14 (13), Español (13), Invasion (22) and ZuperSamu (92). Invasion also
  explicitly fails resolution of absent `bobj00.pac` inside its own profile
  rather than falling back, while its installation audit remains valid because
  that omission is part of the audited APK contract.

00.28 retains all 00.27 direct-audio work: Vorbis remains on libvorbisfile,
MP3/AAC-M4A are detected by content and decoded directly with Vita
`SceAudiodec`, and source assets are not transcoded or renamed.

Full physical-test artifact:

- `DBTapBattle-Vita-00.28-Standalone-Profiles.vpk`
- 2,648,911 bytes
- VPK SHA-256:
  `4411302f1b7e673fe49c98bb9ce0b7fe47ed086a34e1ad03025735411d07cab2`
- eboot SHA-256:
  `5473e2fd7e1ea04cd9c207af61a440ef88d265b2338a093b9a616e1762b37f12`
- ELF SHA-256:
  `595052aec345e8b08f29cc650e0cfa085c2d37d9103769b778c73778808e9bf6`
- runtime source marker: `fa9d7b6`
- APP_VER `00.28`, TITLE_ID `DBTB00001`
- LiveArea validation: PASS.

This build is host/build validated, not yet hardware-confirmed. The critical
device test is to leave `game/` empty and run a profile all the way through
menu, selection, battle, audio and save. Any selected-profile missing-resource
message should be investigated in the port, not fixed by copying a file from
Original.

Evidence:
[vita_standalone_profiles_00.28.json](evidence/vita_standalone_profiles_00.28.json).
Physical test:
[TEST_VITA_00_28](TEST_VITA_00_28.md).

## Historical 00.27 candidate — Samu + Invasion direct source audio


00.27 supersedes the 00.26 Samu import-time audio conversion approach. The user
explicitly requires the mod to work with the files **exactly as they are stored
in the APK**. The hardware-confirmed 00.24 path remains the regression baseline;
00.27 is a new physical-test candidate, not yet hardware-confirmed.

The audited Samu dataset is still 92 characters (00..91), with the Vita-side
offline gate covering the complete two-digit namespace 00..99. No original
selection/combat logic is rewritten. The important change in 00.27 is entirely at
the Vita audio boundary:

- the 17 files keep their original `bgm_XX.ogg` names and bytes;
- actual Vorbis (`bgm_12/13`) continues through the proven libvorbisfile path;
- the 12 MP3-backed `.ogg` files are detected from content and decoded with
  Vita `SceAudiodec` MP3;
- the 3 AAC/M4A-backed `.ogg` files are parsed as ISO-BMFF, their original AAC
  access units are fed to Vita `SceAudiodec` AAC, and no source file is rewritten;
- decoded PCM enters the same 48 kHz mixer used by the existing Vita backend.

`tools/prepare_samu_mod.py` is now a validation/extraction helper only. It is
pinned to the audited APK/DEX hashes, verifies all 92 triplets and the known
12 MP3 + 3 AAC/M4A + 2 Vorbis matrix, but leaves all 384 extracted assets
byte-for-byte unchanged. Its manifest explicitly records
`payloads_unchanged: true`.

The same compressed-BGM backend is intentionally profile-agnostic and has now
been checked against the supplied **TAP BATTLE INVASION BETA 3** APK. Invasion
keeps 22 contiguous character triplets (00..21) under its protected PAC profile
and changes seven BGM: five MP3 plus two AAC-LC/M4A. The two AAC files are
44.1 kHz stereo with maximum access-unit sizes 455 and 548 bytes, safely below
Vita's 1536-byte AAC ES limit; the MP3 files are valid at 44.1/48 kHz. No audio
conversion is required.

`tools/prepare_invasion_mod.py` is pinned to the audited APK/DEX hashes, uses
the existing Community14 canonical alias extraction, validates all 22 triplets
and the 5 MP3 + 2 AAC/M4A + 10 Vorbis BGM matrix, and requires
`payloads_unchanged: true`. Invasion omits `bobj00.pac` and `font00.pac` by design; the supplied protected APKs are autonomous with that inventory. In 00.28 these omissions are no longer hidden by cross-profile fallback; a differing original-core request is treated as an explicit compatibility issue.

The exact same 00.27 VPK binary is therefore the physical-test candidate for
both Samu and Invasion. Resource compatibility for Invasion does not imply that
all behavior unique to its heavily modified `classes.dex` is already ported;
any such mismatch must be isolated after the resource/audio path passes.

Public validation passes:

- Community mod profiles run `37544623252` — PASS at `926eb6b0`;
- Vita engine native smoke run `37544588962` — PASS with
  `SceAudiodec_stub` linked.

The first 00.27 Samu-named VPK was superseded after direct APK review showed
that the protected Android14/Spanish/Invasion datasets are valid without
`bobj00.pac`. The corrected physical-test candidate is:

- `DBTapBattle-Vita-00.27-Samu-Invasion-Corrected.vpk`
- 2,647,984 bytes
- VPK SHA-256:
  `311a820f948e337b0626b7b46e6dcb2ca0628b81364941b11d4c837867ee1b96`
- eboot SHA-256:
  `12106a98ecb2c6f0f4401e0879705edf57f4d7fcbee495351df07f24fdd241ec`
- ELF SHA-256:
  `aaae0778d094d17ac51bab175ce79ce350dfce49b990b10e490fe20db66cb3a6`
- runtime marker: `7fec715`
- APP_VER `00.27`, TITLE_ID `DBTB00001`
- TeaVM: 467 classes / 4086 methods
- LiveArea validation: PASS.
- installed-data gate: no longer requires `bobj00.pac`; VFS fallback remains optional compatibility when requested at runtime.

Because the interactive runner could not finish the monolithic generated C unit
at normal optimization within the command window, the private test build splits
the TeaVM remainder into ten compilation units at `-O1`, keeps the large
`TCBManajer.c` at `-O0` (the already documented interactive-build technique),
and leaves native Vita adapters including direct audio at `-O2`. Use this
artifact for functional roster/audio validation; final release performance still
requires the standard reproducible build recipe.

Evidence: [corrected 00.27 mod-compat build](evidence/vita_mod_compat_00.27_corrected.json).
Physical test protocol: [TEST_VITA_00_27](TEST_VITA_00_27.md).

## Historical 00.26 candidate — Samu roster + rejected conversion import path

00.26 keeps the hardware-confirmed 00.24 gameplay/audio path and the 00.25
protected-profile work. **00.24 remains the latest physical-Vita-confirmed
artifact until 00.26 is tested on hardware.**

The Vita-side offline installation audit no longer stops at 31 character slots.
It now validates the complete two-digit resource namespace **00..99** (up to 100
contiguous triplets), while preserving the 13-character minimum, rejecting
partial triplets and rejecting a gap followed by later character data. This
change is an adapter/gate correction only; the original TeaVM game logic is not
rewritten. Synthetic coverage now exercises 13, 22, 92 and 100 contiguous
triplets.

For the audited `DragonBallZuperSamuGamerYT.apk`, the known dataset remains
**92 characters (00..91)**. Its DEX and manifest are byte-identical to Gen, so
no Samu-specific gameplay bytecode is being transplanted. The new
`tools/prepare_samu_mod.py` is pinned to the audited APK/DEX hashes, extracts
all 384 assets, verifies all 92 `char/chardemo/charf` triplets, and explicitly
normalizes only the 15 BGM whose contents are MP3/AAC despite their `.ogg`
names. They become real Ogg Vorbis 44.1 kHz stereo under the same logical
`bgm_XX.ogg` names. The two already-Vorbis BGM are left unchanged. Every
transformation is recorded in `dbtb_manifest.json`; PAC bytes are not rewritten.

This keeps the hardware-tested libvorbisfile mixer and exact-allocation repair
unchanged instead of adding an untested MP3/AAC decoder to the Vita executable.
The original APK itself is therefore still not direct-audio-compatible; the
prepared Vita dataset is the supported Samu import route.

Host/synthetic evidence: Community mod profiles run
`37540898687` passes the Samu preparation rules and the extended roster gate.
The 00.26 native VitaSDK smoke build is tracked separately from physical
gameplay. See [TEST_VITA_00_26](TEST_VITA_00_26.md).

A full private physical-test package was also generated from the pinned original
APK after the public/runtime checks passed. Identity:

- `DBTapBattle-Vita-00.26-Samu-Roster-Test.vpk`
- VPK: 2,604,860 bytes, SHA-256
  `749b9d32e6ed62a7b4593cb6f0b5af6dc2cabbc97cd9f25986757700879e18f5`
- eboot SHA-256:
  `1de9962f19cf9c39a1534e9547de14e3a2569950a6b712ca49ab871443f90830`
- full ELF SHA-256:
  `db580cd100ac330d88908a9db2cd71f53a50b70c2295700ea0a17fbba68e7e9e`
- embedded runtime source marker: `d7a4aa2`
- SFO: APP_VER `00.26`, TITLE_ID `DBTB00001`
- TeaVM generation: 467 classes / 4086 methods
- LiveArea validator: PASS for all five approved entries
- build layout: TeaVM remainder `-O1`, `TCBManajer.c` `-O0`, native Vita
  adapters `-O2`.

The split compile is an interactive-build packaging exception matching the
technique used by the hardware-tested 00.23 package; it is suitable for the
functional Samu test but not for final performance claims. Public CI also passed
Vita engine native smoke run `37541052112` and tool export run
`37541052003`. Physical Samu gameplay remains pending.

## 00.25 candidate — audited community mod profiles

00.25 is derived from the hardware-confirmed 00.24 path; **00.24 remains the
last physical-Vita-confirmed artifact until the user tests 00.25**. The existing
PAC streaming, exact Ogg allocation, clean voice path, responsive selection,
text fixes and approved LiveArea are retained.

New host/build work adds independent protected-resource profiles for the supplied
Spanish Android14 mod and TAP BATTLE INVASION BETA 3 instead of changing the
legacy Android14 constants globally. Detection is per PAC and requires one unique
fully in-bounds profile match. The Windows/Python extractors canonicalize only
audited aliases and preserve payload bytes. Invasion's complete contiguous
character triplets 00..21 are accepted by the local-data gate without weakening
the 13-character baseline; partial/non-contiguous extensions fail.

A deeper APK/media audit found an additional Invasion blocker that supersedes
the earlier "long Vorbis" assumption. Seven files keep the `.ogg` extension but
are not Vorbis: `bgm_03/06/07/14/15` are MP3 and `bgm_04/05` are AAC inside
M4A/ISO-BMFF. The current Vita BGM adapter opens music with libvorbisfile
(`ov_fopen`), so these seven cannot be decoded by the existing path. The bounded
Vorbis streaming branch therefore does **not** complete Invasion audio support.
This must be solved at the Vita audio/import boundary by content-based codec
detection plus a supported decoder/conversion path; the original engine must
continue requesting the same logical BGM names.

Synthetic CI now covers extractor aliases/profile uniqueness, protected PAC/image
decoding, converted GameData metadata and full engine-resource normalization for
Android14 + Spanish + Invasion. The three real supplied protected APK corpora were
also audited locally: Android14 106/106 PACs, Spanish 106/106 and Invasion 139/139
uniquely match their intended profile with no decoded directory extent outside a
file. This establishes format/host compatibility, **not** complete reproduction
of arbitrary changes made only in a mod's `classes.dex`.

See [community mod profiles](COMMUNITY_MOD_PROFILES.md), [mod compatibility](MODS.md),
the [deep APK technical reference](APK_TECHNICAL_REFERENCE.md), the
[Invasion audit](INVASION_BETA3_APK.md), and the
[00.25 physical test protocol](TEST_VITA_00_25.md).

A full private local 00.25 build using the original TeaVM core also completed:
467 classes / 4086 methods, VPK SHA-256
`39265deebeec6ff7954dc85cd2fd18300fa6de8cd546176e3413b9d2823c3173`.
The final clean VPK carries APP_VER `00.25`, TITLE_ID `DBTB00001`, source marker
`d186dc65`, the approved LiveArea and no game-data/APK payloads. Its eboot
SHA-256 is `a1c35f53070a600edabb310c53f95dea269b637206461e5a7e8370839fc5f380`
and full ELF SHA-256 is `c624a1f121a58da10d3d6bc481416d78a1799352c00b6c4d8c21a777ed93c591`. This artifact
is build/host validated and still awaits a physical-Vita run.

Real extracted overlays were exercised through the native C++ paths as well:
Spanish passes the offline gate with 13 characters and normalizes 106 PAC /
361 protected images / 198 WAV / 68 BIN; Invasion passes with 22 characters and
normalizes 139 PAC / 731 protected images / 344 WAV / 80 BIN. Those historical
00.25 host runs had a base `game/bobj00.pac` available because the then-current
gate required it. A later direct APK audit established that the protected APKs
are valid without `bobj00`, so 00.27 removes that artificial gate requirement.

## Deep APK audit — 2026-10-06

The six supplied APKs were re-audited directly from their ZIP/DEX/PAC/audio
bytes so future work does not need the binaries for already-known structure.

New source-of-truth documentation:

- [APK technical reference](APK_TECHNICAL_REFERENCE.md): manifest/DEX/signing,
  layout, PAC formats, codec constants, aliases, native libraries and semantic
  comparisons.
- [Canonical APK differences](APK_CANONICAL_DIFFERENCES.md): pairwise logical
  file presence/identity after resolving `res/raw`, `assets` and protected
  aliases.
- [Spanish Android14](SPANISH_ANDROID14_APK.md) and
  [Invasion Beta 3](INVASION_BETA3_APK.md): profile-specific details/outliers.
- [Exact exterior audio matrix](evidence/APK_AUDIO_MATRIX_2026-10-06.md).
- [Machine-readable evidence](evidence/apk_deep_structure_2026-10-06.json),
  including all 372 Android14→Invasion changed DEX signatures, canonical
  comparison counts and observed PAC outliers.

Important new findings: Invasion's seven changed BGM are five MP3 + two AAC/M4A
despite `.ogg` names; `char15`/ `char20` use non-uniform interleaved RGBA
layouts (101/98 entries), `char21` has 85 entries, and Spanish/Invasion
`card034` contains a valid one-record 0×0 converted table. Parsers must follow
the directory/type contract rather than fixed per-file layout assumptions.

This audit changed **documentation/evidence only**. It does not alter the runtime
or the hardware-confirmed 00.24 path.

A sixth mod, `DragonBallZuperSamuGamerYT.apk`, was then audited separately. It is
a Gen-derived ordinary-PAC build with **92 character triplets (00..91)** while
keeping Gen's `classes.dex` and `AndroidManifest.xml` byte-identical. Its 384
assets contain 345 PACs; 345/345 outer PACs are structurally valid, all 92
`charXX` contain a 43-record BIN, and 237 files are new versus Gen. This is
strong evidence of a data-driven large roster. The 00.26 Vita-side audit now
covers the complete two-digit namespace 00..99; no 92-character hardware claim
is made until the physical test protocol passes.

The same mod also broadens the media issue: **15/17 BGM named `.ogg` are
actually MP3 or AAC/M4A**, while all 19 SE remain baseline Vorbis. 00.26 adds an
explicit import-time Samu preparation path that normalizes those 15 files to
real Vorbis without changing their logical names or the original game core. It contains
two empty-but-valid `charf` PACs (20/21), 18 type-`u` URL metadata entries and
one malformed type `.pn` whose payload is a valid PNG. These are source
outliers to document, not a reason to globally weaken the runtime parser.

See [Zuper/SamuGamerYT APK audit](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md) and
[machine-readable evidence](evidence/dragonball_zuper_samugameryt_2026-10-06.json).

## Current hardware report — 00.24

Two manual full-engine publication entries are now implemented: Release and
Prerelease, with a shared builder, original-APK hash gate, audio regressions,
VPK/ELF/LiveArea validation, compiled-only symbols and draft-first publication.
Static/unit and real-VPK staging checks pass. GitHub-hosted validation run
[37395626518](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37395626518)
passed actionlint and all 13 publication/LiveArea tests. A live full build/publication
requires the private APK URL secret; see [setup](RELEASE_WORKFLOWS.md).

Before the 00.24 repair, the user confirmed that **00.23 LiveArea-Fixed presentation
worked on hardware**, but reported another battle-start crash with Android14 selected, characters 12/03
and `bobj03`. The earlier successful 00.23 test path remains historical evidence;
this extends coverage to a failing native-audio allocation path. The PAC streaming
repair remains active in the supplied log.

That 00.23 log ends in `std::bad_alloc`. Its supplied core's game-thread stack
returns to `decodeOgg` immediately after PCM vector growth, with a requested
9,506,304-byte allocation. The old decoder reproduces that exact request for
`bgm_03.ogg`: its C++ allocation peak is 14,260,324 bytes for a 5,454,332-byte
PCM track. This differs from 00.22's managed whole-PAC allocation failure.

00.24 allocates the exact Vorbis frame count once, decodes directly into that
buffer, checks channel/rate consistency and decoded length, and drops cache-only
PAC owners/idle imported textures before allocating. Active streams and textures
retain ownership. Newlib remains 96 MiB, TeaVM's maximum remains 48 MiB, and
original battle logic, music samples, voice DSP and approved LiveArea remain
unchanged. New Ogg diagnostics identify the track/frame count.

**Host confirmed:** all 17 supplied BGM tracks match the previous decoder sample
for sample; a 6 MiB single-allocation limit reproduces the old `bgm_03` failure
and permits all fixed loads. Fixed `bgm_03` C++ peak: 5,454,432 bytes, a reduction
of 8,805,892 bytes (excluding Vorbis C allocations and unrelated owners).
Audio setup/DSP and resource ownership probes pass with Vita APIs mocked.
**Hardware confirmed by the user on 2026-10-05 at 19:26 America/Bogota:**
the delivered 00.24 works and resolves the reported battle-start crash. The user
says it works very well; no new runtime log or exhaustive character/profile
matrix was supplied. The earlier failing log/core describe 00.23, not 00.24. See
[the result and broader regression procedure](TEST_VITA_00_24.md).

Hardware-confirmed package: `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk`, 2,663,883 bytes,
SHA-256 `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
Runtime source: `f5672d4d3fbf6b43cd699d7a5a2b80475e4db9f6`. Fresh generation
compiled 467 classes/4086 methods; the standard complete TeaVM amalgamation
compiled at -O1 and native services at -O2. Native smoke CI run `37392864767`
passed, separately from this local full-game build. All five presentation entries
match the hardware-confirmed 00.23 LiveArea-Fixed package byte-for-byte. Exact ELF
and private generated sources are retained with the candidate for crash analysis.
[Identity and measured allocation evidence](evidence/vita_battle_audio_00.24.json).

<!-- DBTB_00_23_DETAIL:START -->
## Historical hardware checkpoint — 00.23 (2026-10-05)

**Status:** hardware validated for the tested path; no error observed in the user's
00.23 session.

Verified together on a real PS Vita in the current progression:

- startup and menu flow continue to work;
- text remains visible;
- audio/voices remain clean (the prior rasp/worker issue did not regress);
- character selection remains responsive (the prior 1–2 second selection stalls did
  not return in the reported session);
- starting a fight now succeeds;
- the fight can be played without the 00.22 battle-start crash.

The specific 00.22 failure was a TeaVM managed-allocation abort while bridging the
entire `char00.pac` (~4.74 MiB) into one Java `byte[]` before the original parser's
disposal order could free the prior owner. 00.23 restores the original streaming
parser contract and exposes the PAC through a Vita native `InputStream` bridge, so
Java receives individual original payload arrays instead of one whole-PAC bridge
array.

This is a **checkpoint, not an exhaustive certification**: every character, mode,
mod dataset, repeated battle sequence and long-duration memory behavior still need
broader regression coverage before a final release claim.

Test artifact SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`  
Source checkpoint: `0e17b0bac33c47698b414b67a839c839f0e555ce`
<!-- DBTB_00_23_DETAIL:END -->


This is the current handoff. It describes implementation and evidence separately.
Historical audit/test pages remain useful for their pinned APK/builds; their old
pending statements do not override this page. 00.21 and 00.22 failures are historical checkpoints.
The latest 00.24 user confirmation also resolves the distinct native Ogg memory crash reported after the 00.23 LiveArea repack.

## Historical 00.23 build identity

| Field | Value |
|---|---|
| Hardware-tested VPK | `DBTapBattle-Vita-00.23-battle-memory-test.vpk` |
| Version / title ID | `00.23` / `DBTB00001` |
| Source checkpoint | `0e17b0bac33c47698b414b67a839c839f0e555ce` |
| Battle-memory repair | original streaming `GameData.Init` + native-backed `InputStream` |
| Retained selection-mask repair | original masks 187/251 accepted |
| Retained audio repair | worker startup + clean voice output in reported hardware path |
| VPK SHA-256 | `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd` |
| Toolchain family | VitaSDK 2026.08, GCC 15.2.0, hard-float |
| Private generator | dex2jar 2.4, ECJ 3.37.0, TeaVM 0.12.3, Java 17 |
| Hardware result | startup/menu/text/audio/selection/battle path passed; no error observed in this session |

[00.23 hardware evidence](evidence/vita_hardware_full_game_00.23.json) records the tested artifact and scope.
The package used for this hardware checkpoint was an interactive test build; see [BUILD](BUILD.md) for the split-compilation caveat before treating it as a release-quality performance artifact.

The current presentation test is `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk`,
SHA-256 `19fae90627b1ddf4f42902ec228b0c50d992cb7c3cce6d3fbb6d8a8843d9edee`. It uses the exact hardware-tested 00.23
executable/SFO and retains every original package entry byte-for-byte, adding
only the five LiveArea files. `pic0.png` now has the required 256-entry palette
without changing any decoded pixels; the minimal MetalSyntax a1 gate uses XML
content revision 2. This package passes the toolkit and strengthened local
validators; the user subsequently confirmed its LiveArea works on real hardware.

The previous `DBTapBattle-Vita-00.23-LiveArea-Final.vpk` is **rejected**: its
`eboot.bin` matches the non-playable CI native link probe (run `37387861902`),
not the hardware-tested full engine. The user reported installation and
presentation failures. The supplied artwork also had a separate 192-entry
splash palette mismatch; its exact causal role in the installer failure is
unconfirmed because no VitaShell error code was supplied. The historical
LiveArea-only package/evidence remains a record of that earlier attempt, not
acceptance of the Final VPK.

See [corrected package evidence](evidence/vita_livearea_fixed_00.23.json) and
[the corrected device test](TEST_VITA_00_23_LIVEAREA_FIXED.md).

## Implementation versus observation

| Area | Current implementation | Verified scope / open limit |
|---|---|---|
| Core | Original APK Init/Run/task/combat code compiled privately to C; Android service adapters | Real menu/front touch/selection/combat on earlier Vita builds; not every mode/action certified |
| Profiles | Original/mod selector; file-level override/fallback; per-file codec | Earlier Vita profile selection and Android14 battle confirmed; arbitrary mods untested |
| Cards/startup | Cooperative EventQueue progresses one ready event after present; local-data checks | User reports fixed in 00.16 |
| Battle frame rate | Reused GLES client buffers, reduced adapter work; 960×544 | 00.18 steady battle windows 59.9 FPS, user reports stable 60; not every later build validated |
| Text | Memory-based PVF with fallback; glyph metrics/cache, image rectangles, visible-only rasterization, dirty uploads | Text recovery confirmed by user in 00.19; exhaustive script/font/layout fidelity untested |
| PAC I/O | Original exclusion filter plus 00.23 native-backed streaming InputStream; bounded native cache/stream handles | 187/251 masks and stream ownership pass host probes; physical selection and battle startup pass in 00.23 |
| Imported textures | Immutable byte/mode keyed cache; live-reference tracking and idle eviction | Native PNG/ownership host probes pass with GL mocked; hardware reuse/performance pending |
| Voice samples | Original PCM16 mono 22050 Hz or decoded Community14 wrapper; 3 voice channels | Format/host decoding verified; later physical tests report clean voices/audio |
| Voice output | 16-tap/256-phase Q14 reconstruction to 48000 Hz, peak limiter, PCM cache | Clean audible result reported on the physical 00.22/00.23 path; broader character/phrase matrix remains open |
| Audio startup | Restored `0x10000100`; exact open/create/start diagnostics, failure cleanup/latch | 00.21 worker/menu recovery confirmed and no audio regression reported in 00.23 |
| Saves | Active dataset's `save.bin`, max 12906 bytes; cached reads and temp/fsync/rename writes | Host ownership tests; full Android round-trip/mod progression matrix pending |
| Input | Stable slots mapped from Vita touch IDs; original coordinate transform and Controller | Touch gameplay confirmed; physical buttons serve selector, are neutral during game |
| Online / Bluetooth | Offline installed-data boundary; HTTP rejected; Bluetooth disconnected | Current local single-player path; multiplayer/billing/remote downloads unsupported |

Cache budgets are 8 MiB retained PAC-vector capacity, 4 MiB source+RGBA texture
cost and 2 MiB voice-input+PCM cost. These limits are **not** total process-memory
caps: live shared owners, Java copies, allocator/driver overhead and uncached
resources also use memory. First loads and eviction reloads still do work.

Clock requests are CPU 444, bus 166, GPU 222, crossbar 166 MHz. The 00.18 device
log reports effective bus 222; log API results/effective values instead of
assuming the requested value is the applied value. No higher clock profile is
established here.

## Latest observations and next work

1. **Confirm the LiveArea repack on hardware.** Install the corrected `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk` and verify VitaShell promotion, bubble icon, Shenlong background, launch gate/logo and a short launch/battle regression pass.
2. **Broaden 00.23 regression coverage.** Repeat battles, switch across more characters and revisit evicted resources to confirm the streaming fix under churn rather than only one successful progression.
3. **Retest both supported dataset paths/mod overlays.** Keep original fallback rules and record exact dataset hashes when comparing behavior.
4. **Measure release-quality performance.** The 00.23 hardware test package used split TeaVM compilation with `TCBManajer.c` at `-O0` because of the interactive build runner; create a normal reproducible full-engine package before making final FPS/performance claims for 00.23.
5. **Extend lifecycle/control coverage.** Return-to-menu, repeated launches, suspend/resume, save round-trips and physical Vita control adaptation remain separate work. Multiplayer/Bluetooth synchronization, billing/remote services and arbitrary code mods remain unsupported/unvalidated.

No currently reproduced crash is open in the 00.23 tested path. New failures should be recorded with exact VPK hash, dataset/profile, `runtime.log` and `psp2core` when produced.

## Evidence index

| Evidence | Scope |
|---|---|
| [00.11 battle](evidence/vita_hardware_battle_00.11.json) | Physical Android14 gameplay milestone |
| [00.18 performance](evidence/vita_hardware_performance_00.18.json) | User report, 15 steady battle windows, clocks and remaining regressions |
| [00.19 text/audio](evidence/vita_hardware_text_audio_00.19.json) | Text restored; voices/selection still fail; zero measured clipping in this session |
| [00.20 PAC/DSP build](evidence/vita_pac_voice_build_00.20.json) | Host format, I/O, cache and reconstruction tests |
| [00.20 startup failure](evidence/vita_hardware_audio_startup_00.20.json) | Physical worker setup failure and caught BGM exception |
| [00.21 build](evidence/vita_audio_startup_build_00.21.json) | Complete engine compilation and native setup/DSP/resource tests |
| [00.21 selection failure](evidence/vita_hardware_selection_00.21.json) | Physical worker/menu recovery, then rejected character mask and caught exception |
| [00.22 build](evidence/vita_selection_filter_build_00.22.json) | Before/after mask regression, full corpus and complete ARM artifact checks |
| [00.23 hardware](evidence/vita_hardware_full_game_00.23.json) | Physical Vita: clean audio, responsive selection, battle startup/gameplay pass; no error observed in reported session |
| [00.23 LiveArea package](evidence/vita_livearea_00.23.json) | Exact presentation-only repack: original tested VPK entries unchanged; LiveArea hashes/layout pass CI; physical shell test pending |

[Validation](VALIDATION.md) defines test scope and log interpretation.
[Porting guide](PORTING_GUIDE.md) explains reusable techniques and failures.

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
