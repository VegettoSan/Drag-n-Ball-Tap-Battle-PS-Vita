# 00.24 — Android14 battle-start audio-memory retest

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
cache reclamation. These are PC tests with Vita output/GL calls mocked. Device
startup, audio scheduling, gameplay and repeated-session stability remain pending.
