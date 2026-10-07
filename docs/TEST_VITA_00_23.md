# PS Vita hardware test — 00.23

> **Historical document notice — current 00.34 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for 00.34. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


Date: 2026-10-05  
Source checkpoint: `0e17b0bac33c47698b414b67a839c839f0e555ce`  
Test VPK: `DBTapBattle-Vita-00.23-battle-memory-test.vpk`  
VPK SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`

## Purpose

Retest the exact progression that failed in 00.22 after replacing the whole-PAC
TeaVM bridge with the original streaming `GameData.Init` parser backed by a native
Vita resource stream.

## Previous failure being retested

00.22 already preserved the earlier audio and character-selection improvements, but
starting a fight aborted while creating a 4,739,319-byte managed bridge array for
`char00.pac`. The stack reached `NativePlatform.readGameData` /
`ResourceAdapter.load` / `GameData.Init` / `TCBManajer.SetLoad` / `Game1` and the
runtime reported lack of free memory.

## 00.23 change

- preserve the original streaming parser and its `Dispose` ordering;
- patch only Android resource-opening expressions;
- provide a native-backed `NativeResourceStream` with explicit ownership/close;
- read original PAC entries incrementally instead of first copying the whole PAC to
  one TeaVM-managed `byte[]`;
- keep the already accepted audio, text, texture/resource cache and selection-filter
  fixes.

## Physical Vita result

**PASS for the tested session.** The user reported that everything exercised was
working as expected and no error was found at this point. In particular:

- audio/voices remained correct, without the prior rasp;
- character selection remained responsive;
- the game no longer crashed when starting a fight;
- battle startup and gameplay worked normally during the reported test.

No new `runtime.log`/`psp2core` failure artifact was supplied because no failure was
observed in this session.

## Interpretation

00.23 was the hardware checkpoint for this historical test and resolved the reproduced 00.22
battle-start managed-allocation regression. It is not an exhaustive certification
of every character, mode, mod, repeated-battle sequence or long-duration memory
behavior. Those remain regression work for later checkpoints.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.33 (2026-10-07):** physical Vita testing
> confirms the reproduced Invasion repeated-fight/Saitama→Freezer crash is fixed
> after the protected-PAC ownership-transfer repair. The recent hardware sequence
> also confirms Loading recovery and dynamic installed rosters, including Samu's
> 92 characters. Scope is limited to tested paths; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
