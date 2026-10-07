# PS Vita physical test — 00.29 Samu / Invasion / shared save

> **Historical document notice — current 00.34 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for 00.34. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


<!-- DBTB_DOC_STATUS:START -->
> **Project checkpoint:** 00.33 is the current hardware-confirmed development
> checkpoint for the tested paths. This file may document an earlier component
> or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


## Purpose

Validate only the changes introduced after the user's 00.28 hardware test:

1. Samu must pass the title/BGM transition that failed with
   `sceAudiodecCreateDecoder 0x807f0007`.
2. Invasion result/dialog text must be readable while retaining the already
   working textures and direct audio.
3. Original and every mod must use one global
   `ux0:data/DBTapBattle/save.bin`.

Do not repopulate missing profile resources from `game/`. Each selected dataset
remains standalone.

## Install

Install the 00.29 VPK over the existing application. Keep
`ux0:data/DBTapBattle/` unless deliberately testing first-save seeding.

The VPK contains the exact approved seed at `app0:/save.bin`, but it is read-only.
The writable save is:

```text
ux0:data/DBTapBattle/save.bin
```

If that file already exists, 00.29 must preserve it.

### Optional seed test

Back up the current global save, delete only
`ux0:data/DBTapBattle/save.bin`, then launch once. A new file should appear with:

- size: 12,906 bytes
- SHA-256:
  `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`
  before the game subsequently modifies progress.

Do not delete profile datasets.

## Samu standalone test

Use only:

```text
ux0:data/DBTapBattle/mods/ZuperSamu/
```

`game/` may remain empty.

Expected:

- selector accepts ZuperSamu;
- log reports 92 character triplets;
- title passes into the menu instead of exiting;
- no `sceAudiodecCreateDecoder failed 0x807f0007`;
- transitions between MP3 BGM work;
- AAC/M4A and Vorbis BGM continue to play directly from original bytes.

Exercise characters 00, 20, 35, 42, 54, 79 and 91 when practical, including at
least one real battle with a high-index character.

## Invasion standalone test

Use:

```text
ux0:data/DBTapBattle/mods/Invasion/
```

`game/` may remain empty.

Expected:

- existing working textures remain unchanged;
- existing MP3/AAC/Vorbis playback remains unchanged;
- reproduce the winner/result screen from the 00.28 test;
- the message inside the blue dialogue/result box should now be readable rather
  than symbols such as `8@P dd...`;
- log may print one
  `Character text charset fallback: ... charset=UTF-8` diagnostic when the
  modified Invasion SetString slot convention misses the exact GameData map.

Test character 20 (Ranma) and character 21 if possible.

## Shared-save continuity

1. Launch one profile and make a visible progress/save change.
2. Exit normally.
3. Launch a different profile.
4. Confirm the compatible progress is visible there.
5. Exit and return to the first profile.
6. Confirm the same progress persists.

There should be one active save only:

```text
ux0:data/DBTapBattle/save.bin
```

Files such as `mods/ZuperSamu/save.bin`, `mods/Invasion/save.bin` or
`game/save.bin` are not runtime inputs in 00.29.

## If anything fails

Send:

- complete `ux0:data/DBTapBattle/logs/runtime.log`;
- `psp2core` dump if generated;
- selected profile;
- exact screen/action;
- character index/name;
- screenshot for text/render defects;
- BGM number if identifiable.

Do not copy a missing PAC from another profile as a workaround; the log should
remain evidence of the real compatibility gap.
