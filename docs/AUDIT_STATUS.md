# Audit status — 2026-10-05

The 2026-10-04 bootstrap audit was followed by full original-core AOT integration
and physical Vita testing. This page replaces its obsolete current-state table;
chronological details remain in [ATTEMPTS](ATTEMPTS.md). The latest test package
is 00.21; its startup recovery is pending. See [CURRENT_STATUS](CURRENT_STATUS.md).

| Area | Present conclusion | Evidence / boundary |
|---|---|---|
| Original APK | 77 ZIP files, 91 DEX classes, 57 raw resources, no native engine .so | [APK_AUDIT](APK_AUDIT.md); original character data absent |
| Community14 | 144 assets, verified XOR/DEFLATE/table/ADPCM contracts | [ANDROID14_APK](ANDROID14_APK.md); exact source hash, not universal mod codec |
| Gen dataset | 147 actual assets; empty raw stubs; ordinary PACs with hybrid charset | [Original+Characters](ORIGINAL_PLUS_CHARACTERS_APK.md) |
| PAC/SPR/images/tables | Native adaptation plus original parser/interpreters | [RESOURCE_FORMATS](RESOURCE_FORMATS.md); host corpus checks, earlier hardware rendering |
| Engine | Original Java task/core methods generated privately with TeaVM | [ENGINE_MAP](ENGINE_MAP.md); no handwritten replacement combat |
| Renderer/text | GLES bridge, real FBOs, PVF glyph service, immutable texture reuse | Earlier hardware gameplay/text; newest cache performance pending |
| Input | Stable touch mapping and original Controller | Touch confirmed; full physical gameplay mapping pending |
| Audio | Vorbis/PCM backend, voice reconstruction/limiter/cache, restored setup priority | 00.20 setup failure; 00.21 host fixes, hardware pending |
| Saves | Profile-local save.bin with atomic publication path | Implementation/host tests; exhaustive compatibility pending |
| Frame rate | 00.18 steady combat reaches logged 59.9/user-observed 60 FPS | Not a universal all-version/all-mode guarantee |
| Online/multiplayer | Local dataset checks, HTTP rejection, disconnected Bluetooth | Remote services and synchronized multiplayer unimplemented |
| Packaging | Full private ARM engine ELF/VELF/SELF/VPK 00.21 verified | Public CI smoke tests only native linking; no complete public relink-kit claim |

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
- 00.20's broad host pass missed platform thread setup rejection; 00.21 adds
  injected failure paths but still needs a physical test.
- Save interoperability, return/suspend lifecycle, arbitrary mods, all secondary
  modes, complete font coverage and physical controls lack a full device matrix.
- mod.json/selector Unicode labels, remembered choice and log rotation are open.
- Verify full-engine notices/attribution and relink materials before public
  distribution; [THIRD_PARTY](THIRD_PARTY.md) records actual delivered scope.

Next: run [00.21 test](TEST_VITA_00_21.md), then cold/repeated selection timing,
recorded voices and retained text/FPS checks. [PORTING_PLAN](PORTING_PLAN.md)
tracks subsequent work rather than restarting the completed atlas milestone.
