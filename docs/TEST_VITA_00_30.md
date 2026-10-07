# PS Vita physical test — 00.30 Samu / Invasion / independent seeded saves

> **Historical document notice — current v1.0 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for v1.0. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


<!-- DBTB_DOC_STATUS:START -->
> **Current public release:** v1.0 / APP_VER `01.00` / TITLE_ID `DBTB01178`.
> The hardware-confirmed gameplay/runtime baseline is 00.34. This file may
> document an earlier component or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


00.30 keeps the Samu compressed-BGM handoff fix and Invasion UTF-8 character-text
fallback from 00.29. The only intentional architecture change from 00.29 is save
ownership.

## Save contract

The VPK contains one immutable seed:

```text
app0:/save.bin
```

Exact seed:

- size: 12,906 bytes
- SHA-256:
  `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`

Each profile gets its own writable copy when first selected:

```text
Original -> ux0:data/DBTapBattle/game/save.bin
Invasion -> ux0:data/DBTapBattle/mods/Invasion/save.bin
ZuperSamu -> ux0:data/DBTapBattle/mods/ZuperSamu/save.bin
Other mod -> ux0:data/DBTapBattle/mods/<Profile>/save.bin
```

The copy is created only if that profile has no save yet. A later launch must
load the existing profile save and must not reset it from the VPK.

The old 00.29 root file:

```text
ux0:data/DBTapBattle/save.bin
```

is not an 00.30 runtime input.

## Recommended clean isolation test

Back up existing save files first.

1. Delete only the profile saves you want to test, for example
   `mods/Invasion/save.bin` and `mods/ZuperSamu/save.bin`.
2. Launch Invasion once. Confirm `mods/Invasion/save.bin` appears.
3. Make a visible progress/change and exit normally.
4. Launch ZuperSamu. Confirm `mods/ZuperSamu/save.bin` appears independently.
5. Make a different progress/change and exit.
6. Return to Invasion and confirm its earlier state remains unchanged.
7. Return to Samu and confirm Samu retains its own state.

For a newly created profile save, checking it before the game modifies it should
match the VPK seed hash above.

## Samu regression

With `game/` absent if desired:

- ZuperSamu should detect 92 characters;
- pass the title into the menu;
- no `sceAudiodecCreateDecoder failed 0x807f0007`;
- MP3/AAC/Vorbis BGM should remain source-byte compatible;
- test a high-index character/battle if practical.

## Invasion regression

With `game/` absent if desired:

- textures/audio should remain as working in the 00.28 hardware test;
- reproduce the winner/result screen;
- the blue result/dialog text should be readable instead of mojibake;
- a one-time UTF-8 character charset fallback log is acceptable.

## Failure package

Send the complete `runtime.log`, any `psp2core`, selected profile, exact
screen/action, character index/name and screenshot for text/render failures.
