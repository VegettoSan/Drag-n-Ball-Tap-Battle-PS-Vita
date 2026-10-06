# PS Vita hardware test — 00.23

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

00.23 is the current hardware checkpoint and resolves the reproduced 00.22
battle-start managed-allocation regression. It is not an exhaustive certification
of every character, mode, mod, repeated-battle sequence or long-duration memory
behavior. Those remain regression work for later checkpoints.

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
