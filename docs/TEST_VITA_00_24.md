# 00.24 — confirmed battle-start audio-memory recovery

<!-- DBTB_DOC_STATUS:START -->
> **Project checkpoint:** 00.33 is the current hardware-confirmed development
> checkpoint for the tested paths. This file may document an earlier component
> or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


**User confirmation — 2026-10-05 19:26 America/Bogota:** “Ya funciono, queda super bien”.
The delivered VPK is confirmed working after the previously reported battle-start
crash. SHA-256: `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`;
runtime source: `f5672d4d3fbf6b43cd699d7a5a2b80475e4db9f6`. No new log or
complete character/profile matrix accompanied this confirmation. The procedure
below remains useful for broader regression testing.

Install `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` as an update to DBTB00001.
Keep `ux0:data/DBTapBattle/` and saves. The approved 00.23 LiveArea artwork and
template are packed unchanged. This is a full original-engine test build.

1. Select Android14. Repeat the reported characters 12 versus 03 and stage 03
   where possible; start the fight and check music, voices and controls.
2. Return to selection, start several further battles, and test Original too.
3. If a crash persists, preserve `ux0:data/DBTapBattle/logs/runtime.log` and the
   new `psp2core-*.psp2dmp`. The boot must identify 00.24; new `Ogg decode:` lines
   show the exact track and decoded PCM allocation.

00.23's streaming parser, worker priority, voice reconstruction and original
selection masks remain in place. This change removes PCM vector doubling and
reclaims idle resource caches before Ogg decoding. It does not increase the
process heap or replace the original gameplay code.

Host tests reproduce the old BGM allocation failure, preserve every sample of
all 17 supplied BGM tracks, and verify active stream/live texture ownership after
cache reclamation. These are PC tests with Vita output/GL calls mocked. The user confirms
the device retest works; exhaustive scheduling/character/mode and repeated-session
stability coverage remains open.
