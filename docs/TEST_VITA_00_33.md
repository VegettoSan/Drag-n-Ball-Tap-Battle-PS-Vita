# PS Vita physical test — 00.33 Invasion Saitama -> Freezer

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Historical build sheet: its identities/results apply to the named build only.
<!-- DBTB_DOC_STATUS:END -->

## Hardware result — PASS (2026-10-07)

The user retested 00.33 on a physical PS Vita and reports **several consecutive
fights without a crash**. The reproduced 00.32 Saitama -> second fight vs Freezer
failure did not recur, so the protected-PAC duplicate-allocation bug is considered
**resolved in the tested hardware scope**.

This confirms the 00.33 ownership-transfer fix on device. It does not claim that
every possible mod, mode, character pairing or arbitrarily long session has been
exhaustively tested. Machine-readable evidence:
[evidence/vita_hardware_00.33.json](evidence/vita_hardware_00.33.json).


## Purpose

Retest the exact 00.32 hardware crash with the smallest evidenced fix.

00.32 already confirmed:

- the startup Loading loop is fixed;
- dynamic profile rosters work on hardware, including all 92 Samu characters;
- Invasion text remained readable in the tested path.

The remaining reproducible failure is Invasion with Saitama (char15), when the
story advances to the second fight against Freezer (char05).

## Confirmed 00.32 crash path

The supplied `psp2core` symbolicates the uncaught `std::bad_alloc` through:

```text
operator new
std::vector<unsigned char>::operator=
normalise(...)
normaliseEnginePac(...)
readEngineResource(...)
EngineResourceCache::read(...)
dbtb_resourceFiltered
dbtb_openResourceStream
NativePlatform.openGameData
ResourceAdapter.open
GameData.Init
```

The failing return address is the vector copy at the end of protected-PAC
normalisation. `char15.pac` is about 4.05 MiB on disk and about 4.64 MiB after
normalisation. GCC 15 lowered the previous conditional move expression to vector
copy-assignment on this path, requiring another contiguous multi-MiB allocation.

00.33 replaces that changed/protected path with `output.swap(out)`, transferring
ownership without allocating another PAC-sized buffer. The ordinary unchanged
path still copies `input` intentionally.

## Test

1. Install 00.33 over 00.32. Do not delete profile data or saves.
2. Select Invasion.
3. Choose Saitama.
4. Complete his first fight normally.
5. Continue to the second fight against Freezer.
6. Confirm the fight starts and can be completed.
7. If successful, continue for at least 3–5 fights/transitions to stress repeated
   protected character PAC loading.
8. Also make a short Samu launch/roster check to ensure 92-character support did
   not regress.

## Expected

- No `std::bad_alloc`.
- Saitama -> Freezer second fight starts.
- Invasion text/audio/textures remain as in 00.32.
- Samu still shows its complete 92-character roster.
- 00.32 startup/offline-loading behavior remains fixed.

## If it still crashes

Send:

- complete `runtime.log`;
- new `psp2core-*.psp2dmp`;
- exact character/fight transition;
- whether the crash occurs before the VS/battle screen or after rendering begins.

Do not alter or replace Invasion PACs for this test.
