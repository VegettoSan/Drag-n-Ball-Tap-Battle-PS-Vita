# Experimental character-selection cache — 2026-10-07

**Status: source change committed in the experimental branch; no Vita hardware validation yet.**
**Do not update the v1.0 release or call this an established fix.**
Branch: `perf/character-switch-metadata-cache`.
Baseline: public v1.0, original runtime source `0da8684`, TITLE_ID `DBTB01178`, APP_VER `01.00`.

## User's Vita evidence

User-reported behavior: character selection briefly stops for approximately 1/4 second, while the v1.0 release remains playable. Submitted `runtime.log` is a session opening Invasion on 2026-10-07 at 19:18:54 (Vita local time). No crash is reported in this session.

Counted only resource rows matching `charNN.pac`:

| Operation | Count | Cache hits | Mean logged duration | Maximum |
| --- | ---: | ---: | ---: | ---: |
| `filter=251` (character BIN metadata) | 17 | 0 | 125.2 ms | 128.0 ms |
| `filter=187` (character filtered data/voice) | 15 | 0 | 300.3 ms | 360.4 ms |

`char14.pac filter=251` repeats at approximately 125, 123.5, 123.9 and 123.7 ms in the same session. This is evidence of redundant source work, **not** proof that every user-perceived freeze is the metadata read. The `[Perf]` reports include much larger event-driven loading stalls while entering/leaving fights; those are outside this scoped selection optimization. Normal idle/selection windows return to ~60 FPS.

## Identified source behavior

At baseline `src/resource_cache.hpp`, a source PAC over 2 MiB causes `clear()` **before** every read and is not retained afterwards, regardless of which original `GameData.Init` exclusion mask was requested. Some `charNN.pac` files have multi-MiB physical sources but produce small `filter=251` metadata results (~11–13 KiB). Adjacent/returning selections therefore repeatedly open/read/rebuild the same tiny metadata. The intended original `filter=187` and `filter=251` exclusion behavior must remain exact.

## Experimental implementation

Only `src/resource_cache.hpp` changes; `readEngineResource`, original TeaVM/Java engine, stream ownership, PAC normalization, textures, audio, save, selector and mod extractors remain untouched.

- Retain *only* `charNN.pac` filter-251 outputs with retained vector capacity at most 24 KiB.
- Separate LRU: at most **256 KiB**; this reservation is **subtracted from** the former 8 MiB normal resource-cache budget, so total retained capacity remains no more than **8 MiB**. Non-character and larger-than-limit resources follow the old handling.
- Keep metadata during large character filtered requests with `filter=187` or `251`. Still discard the regular LRU **before** the large source read. Retain cache keys using the resolved physical path, exact filter and file size/mtime/ctime, preserving mod-profile isolation and invalidation.
- Explicitly clear **both** LRUs on a large *unfiltered* (`filter=0`) character PAC import, on profile/resource reinitialization and on existing idle-memory reclamation. A battle must not accumulate 256 KiB of selection-only metadata over the previous baseline's large-import peak.
- Cache hits still use exactly the same already-normalized byte array; no changes to gameplay/sound/rendering behavior.

**Limits:** This may reduce repeated selection delays after metadata has first been read. It does not accelerate the **first** cold `filter=251` read for a never-before-selected character, and it cannot eliminate genuine `filter=187` or demo/texture loading. Do not claim a universal 0-ms character transition without real Vita measurements.

## Synthetic host regression

`tests/test_character_selection_cache.cpp` constructs an entirely synthetic >3 MiB character PAC and checks:

1. `filter=251` returns the expected small BIN-only payload.
2. A large `filter=187` load does not evict the small result; subsequent `251` returns the same owner as a hit.
3. Source replacement with changed mtime forces an actual miss and changed bytes.
4. A large `filter=0` load clears metadata **before** its full allocation and does not retain it.
5. Explicit `clear()` removes all entries and `used()` never exceeds 8 MiB.

Run from the repository root (Linux, host g++ required):

```sh
g++ -std=c++14 -O2 -Wall -Wextra -Isrc \
  tests/test_character_selection_cache.cpp \
  src/engine_resources.cpp src/pac.cpp src/game_data.cpp src/vfs.cpp \
  -o /tmp/dbtb-test-character-selection-cache
/tmp/dbtb-test-character-selection-cache
```

**Host regression passed in GitHub Actions** on the experimental branch: run [37708484312](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37708484312), job `synthetic-cache-regression`, compilation and run completed successfully and logged `CHARACTER SELECTION CACHE PASS`. This is a host test only: neither full VitaSDK engine compilation nor hardware playback has occurred yet. Any experimental VPK must still be verified on the device.

## Planned physical Vita A/B acceptance

Preserve the public v1.0 VPK and its executable identity. Publish any experiment only as a clearly named test artifact; never overwrite the v1.0 release or migrate/delete `ux0:data/DBTapBattle/profiles/` and its independent saves.

On both the original APK and Invasion (plus Samu if available):

1. Enter character select and go character A → B → A several times; note the first visit versus return visit.
2. Compare the baseline and candidate `runtime.log` rows for `charNN.pac filter=251`: after an initial miss, revisit is expected to log `cache=hit` with markedly less `us=`.
3. Visit many characters, start/finish multiple fights (including Saitama vs. Freezer on Invasion), revisit selection and check for crashes, black textures, missing voices, audio glitches, saves, or increased fight-entry pauses.
4. Check repeating `[Perf]` reports for `resource_cache_hits` and `run_max_ms`; this is a comparison, not proof that every stall is fixed.
5. If a crash happens, capture `runtime.log` plus `psp2core` and revert to the untouched v1.0 release.

**Promotion criterion:** passing synthetic host regression, actual Vita testing with original + at least the tested mods, and no regression in fighting, memory or data isolation. Until then this remains an unverified optimization attempt.


## Full-engine experimental VPK build attempt (2026-10-07)

An isolated workflow was added at
`.github/workflows/vita-character-cache-vpk.yml` to compile the **actual**
original TeaVM engine plus native adapters from this branch, run the
character-cache regression, validate LiveArea and upload a test-only VPK
without modifying `main` or publishing/overwriting the stable v1.0 release.

First attempt: [GitHub Actions run 37708790788](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37708790788).
**Blocked before compilation** because repository Actions secret
`DBTB_ORIGINAL_APK_URL` is not configured. No VPK was produced or uploaded.
The attached local `DBTapBattle.apk` hashes to the pinned
`b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`,
but GitHub runners have no access to chat-local uploaded files.

To enable the existing workflow without committing commercial APK content:

1. Host the exact original `DBTapBattle.apk` at a **private, direct HTTPS**
   download URL usable by non-interactive CI for the duration of the build.
2. Add it under **Settings → Secrets and variables → Actions** as the
   repository secret `DBTB_ORIGINAL_APK_URL`.
3. Re-run workflow run 37708790788 from GitHub Actions; after a successful full
   build the downloadable artifact will be named
   `DBTapBattle-v1.0-character-cache-experimental-<run-id>`, containing
   `Dragon-Ball-Tap-Battle-PS-Vita-v1.0-Character-Cache-Test.vpk`,
   `SHA256SUMS.txt`, and `README.txt`.

**Never distribute a native-smoke/dummy VPK as the playable test build.**
A passed synthetic host test is not a release-ready VPK.
