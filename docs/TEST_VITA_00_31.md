# PS Vita physical test — 00.31

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


00.31 is the hardware-regression candidate built from the user's 00.30 test.

## What 00.30 proved

- Zuper/Samu now boots and reaches the menu.
- Invasion text corruption seen in 00.28 was not reproduced in the short 00.30 test.
- Samu direct MP3/AAC/Vorbis playback passes the previous title transition.
- Invasion later crashes on a new fight with `std::bad_alloc`.
- Samu reports 92 complete character triplets but only exposes the first 13 with
  the approved seed.
- Shop exits the game because the Vita adapter throws for Android marketplace.
- Offline update/catalog startup pauses roughly 25 seconds.

## 00.31 changes to validate

### Dynamic character roster / save

The approved seed contains visibility/download/open flags for 00..12 and zero for
13+. 00.31 audits the selected profile, then synchronizes only the three original
ConfigData character flags for every complete installed triplet:

- record base: `character * 100 + 30`
- +1: GetCharctorBuy / SetCharVisible
- +2: GetCharDL
- +85: CharOpen

The exact Gen core also hardcodes 90-entry character arrays/loops. A verified ASM
patch changes only `bCharIndex`, `bCharVersionSv`, `bCharNoSv` capacity to
100 and replaces the 90 bounds in `CharVisibleInit` / `ClearCharDLALL` with
the audited installed-character count. Unrelated card/Bluetooth/UI constants
equal to 90 are untouched.

Expected with Samu: 92 characters (00..91), not 13 and not invented 92..99.

### Invasion repeated-fight memory

The 00.30 log ends after a full `char15.pac` load with:

```text
terminate called after throwing an instance of 'std::bad_alloc'
```

Large normalized character PACs were being retained in the 8 MiB LRU until after
the next multi-MiB resource had already been materialized. 00.31 clears stale LRU
ownership before source files larger than 2 MiB and does not cache those large
results. Active resource streams keep their own shared ownership.

Expected: run at least three consecutive fights, including the same transition
that crashed 00.30.

### Shop

Android's original `SmapStart` launches a separate Activity and later completes
through `SmapEnd`. Vita has no Android marketplace Activity. 00.31 now completes
that lifecycle edge synchronously and returns to the game instead of throwing
`UnsupportedOperationException`.

Expected: entering Shop must not close the application. Android purchasing itself
is not claimed to be implemented.

### Startup catalog wait

The original Gen `Downloader` is already an offline stub: SetURL=false,
GetData=empty array, GetSize=0 and isDownload=true. The previous Vita adapter
returned null/-1/false, making the preserved state machine wait roughly 25 seconds.
00.31 matches the original stub semantics.

Expected: the "checking/downloading updates" stage should pass quickly instead of
waiting 10–25 seconds.

## Save isolation

00.30's independent save policy remains:

```text
Original -> ux0:data/DBTapBattle/game/save.bin
Mod      -> ux0:data/DBTapBattle/mods/<Profile>/save.bin
```

Every profile is initially seeded from the exact VPK `app0:/save.bin`. 00.31 may
set installed-character flags to 1 in the selected profile save; it does not copy
progress between profiles.

## Send back after test

- complete `runtime.log`;
- any new `psp2core`;
- how many Samu characters are visible;
- approximate startup wait time;
- whether Shop returns cleanly;
- number of consecutive Invasion fights completed;
- screenshot only if a text/render issue appears.
