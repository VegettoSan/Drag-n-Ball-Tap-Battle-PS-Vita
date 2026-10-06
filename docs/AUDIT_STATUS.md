# Audit status — 2026-10-05

The 2026-10-04 bootstrap audit was followed by full original-core AOT integration
and physical Vita testing. This page replaces its obsolete current-state table;
chronological details remain in [ATTEMPTS](ATTEMPTS.md). The current hardware checkpoint
is 00.23: the reported physical-Vita session preserves clean audio and responsive selection, enters a battle and plays without the 00.22 memory crash. See [CURRENT_STATUS](CURRENT_STATUS.md).

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
| Packaging | 00.23 hardware-tested VPK from source `0e17b0ba`; prior normal-build 00.22 evidence retained | 00.23 test package used split generated-C compilation; release-quality reproducible packaging remains open |

## Completed corrections from the bootstrap audit

Extraction validates aliases/conflicts/CRC and preserves raw bytes. VFS rejects
unsafe/non-regular overrides and preserves missing-file fallback. PAC reads
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
- 00.20's broad host pass missed platform thread setup rejection; 00.21 recovered the worker/menu. 00.21 then exposed the filter-mask guard, fixed in 00.22. 00.22 finally exposed the whole-PAC managed allocation at battle start; 00.23 replaces that bridge with the original streaming parser and passes the reproduced hardware path.
- Save interoperability, return/suspend lifecycle, arbitrary mods, all secondary
  modes, complete font coverage and physical controls lack a full device matrix.
- mod.json/selector Unicode labels, remembered choice and log rotation are open.
- Verify full-engine notices/attribution and relink materials before public
  distribution; [THIRD_PARTY](THIRD_PARTY.md) records actual delivered scope.

Next: extend [00.23 testing](TEST_VITA_00_23.md) to repeated battles, more characters/datasets, long sessions, lifecycle/save coverage and a release-quality normal build. [PORTING_PLAN](PORTING_PLAN.md) tracks that work rather than reopening resolved 00.20–00.22 blockers.

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
