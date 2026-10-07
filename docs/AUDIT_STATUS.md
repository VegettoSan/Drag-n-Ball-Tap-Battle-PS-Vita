# Audit status — 2026-10-07

The 2026-10-04 bootstrap audit was followed by full original-core AOT integration
and successive physical Vita tests. This page replaces its obsolete current-state
table; chronological details remain in [ATTEMPTS](ATTEMPTS.md). The current
public release is **v1.0** (APP_VER `01.00`, TITLE_ID `DBTB01178`), with
the **00.34** build as the hardware-confirmed gameplay/runtime baseline: Loading is fixed, dynamic rosters work on device,
and the reproduced Invasion Saitama -> Freezer repeated-fight `std::bad_alloc` no
longer occurs after the protected-PAC ownership-transfer fix. See
[CURRENT_STATUS](CURRENT_STATUS.md).

| Area | Present conclusion | Evidence / boundary |
|---|---|---|
| Original APK | 77 ZIP files, 91 DEX classes, 57 raw resources, no native engine .so | [APK_AUDIT](APK_AUDIT.md); original character data absent |
| Community14 | 144 assets, verified XOR/DEFLATE/table/ADPCM contracts | [ANDROID14_APK](ANDROID14_APK.md); exact source hash, not universal mod codec |
| Gen dataset | 147 actual assets; empty raw stubs; ordinary PACs with hybrid charset | [Original+Characters](ORIGINAL_PLUS_CHARACTERS_APK.md) |
| PAC/SPR/images/tables | Native adaptation plus original parser/interpreters | [RESOURCE_FORMATS](RESOURCE_FORMATS.md); host corpus checks, earlier hardware rendering; original 187/251 rejection fixed in 00.22 |
| Engine | Original Java task/core methods generated privately with TeaVM | [ENGINE_MAP](ENGINE_MAP.md); no handwritten replacement combat |
| Renderer/text | GLES bridge, real FBOs, PVF glyph service, immutable texture reuse | Earlier hardware gameplay/text; newest cache performance pending |
| Input | Stable touch mapping and original Controller | Touch confirmed; full physical gameplay mapping pending |
| Audio | Vorbis/PCM backend, voice reconstruction/limiter/cache, restored setup priority | 00.21 worker/menu recovery; clean audio/voices reported in later 00.22/00.23 hardware path |
| Saves | Profile-local save.bin with atomic publication path | Implementation/host tests; exhaustive compatibility pending |
| Frame rate | 00.18 steady combat reaches logged 59.9/user-observed 60 FPS | Not a universal all-version/all-mode guarantee |
| Online/multiplayer | Local dataset checks, HTTP rejection, disconnected Bluetooth | Remote services and synchronized multiplayer unimplemented |
| Packaging | v1.0 public VPK identity with DBTB01178; 00.34 remains the exact hardware-tested gameplay baseline with preserved historical hashes | Freshly rebuilt v1.0 artifacts still need their own install/launch sanity check; broader release-performance validation remains open |

## Completed corrections from the bootstrap audit

Extraction validates aliases/conflicts/CRC and preserves raw bytes. The 00.28 VFS
rejects unsafe/non-regular resources and isolates the selected APK dataset;
missing files do not fall back across profiles. PAC reads
validate extents and allocation budgets. Subsequent integration fixed missing
resume initialization, direct-buffer GC ownership, charset boundaries, stable
touch IDs, Community14 BIN/WAV normalization, card-task scheduling and PVF
rectangle usage. Current audio setup logs distinguish actual failed syscalls.

## Remaining risks and verification gaps

- Full downloadable character data is absent in the first original APK.
- A file named Original in the selector does not prove installed provenance.
- Imported Community14 assets do not reproduce altered Java mechanics.
- PAC LE fields do not imply CNV/text/save fields are LE.
- Format and build success do not establish audible fidelity or stable gameplay.
- Historical blocker chain: 00.20 thread setup -> 00.21 filter mask -> 00.22 whole-PAC managed allocation -> 00.23 streaming repair -> 00.31 Loading polarity regression -> 00.32 protected-PAC repeated-fight bad_alloc. 00.33 closes the latest reproduced allocation failure on hardware.
- Save interoperability, return/suspend lifecycle, arbitrary mods, all secondary
  modes, complete font coverage and physical controls lack a full device matrix.
- mod.json/selector Unicode labels, remembered choice and log rotation are open.
- Verify full-engine notices/attribution and relink materials before public
  distribution; [THIRD_PARTY](THIRD_PARTY.md) records actual delivered scope.

Next: broaden v1.0 / 00.34-baseline regression coverage to Shop return, return-to-menu,
suspend/resume, saves across more profiles, secondary modes, additional mods and
longer sessions. [PORTING_PLAN](PORTING_PLAN.md) tracks that work; resolved
loading/allocation blockers should not be reopened without new evidence.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current public release — v1.0 / DBTB01178 (2026-10-07):** gameplay/runtime
> baseline 00.34 is hardware-confirmed stable for the tested selector/profile/gameplay
> paths. The earlier 00.33 hardware sequence remains historical evidence for the
> protected-PAC repeated-fight repair, Loading recovery and dynamic rosters.
> Scope is limited to tested paths; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
