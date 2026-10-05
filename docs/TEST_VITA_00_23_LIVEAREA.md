# PS Vita test — 00.23 LiveArea-only repack

Historical test instructions. For the corrected package after the reported Final
failure, use [the Fixed VPK test](TEST_VITA_00_23_LIVEAREA_FIXED.md). The Final
package's native CI executable was not the hardware-tested engine, and its splash
palette did not satisfy the 256-entry rule.

This test is intentionally narrower than the 00.23 gameplay validation. The runtime
binary is unchanged from the hardware-tested 00.23 package; this package adds only
the PS Vita presentation files used by the bubble and LiveArea.

## Exact package

- File: `DBTapBattle-Vita-00.23-LiveArea.vpk`
- SHA-256: `382927c8032fda1db5ec21078a006026daa78ef93f3bdfce7c8484f880fd4e50`
- Base hardware-tested package: `DBTapBattle-Vita-00.23-battle-memory-test.vpk`
- Base SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`
- Title ID: `DBTB00001`
- Version: `00.23`

The repack retains every original VPK entry byte-for-byte, including the tested
`eboot.bin` (SHA-256
`fad6135add0a29a3f0ff67dca9cee79c04233502cf2bb7cfa944537887c3894f`)
and `param.sfo` (SHA-256
`eaecfecbc28f342176dd45445d3a2208b2b5c56f3839a81d306e8a8c9d9f76da`).
It adds only:

- `sce_sys/icon0.png`
- `sce_sys/pic0.png`
- `sce_sys/livearea/contents/bg0.png`
- `sce_sys/livearea/contents/startup.png`
- `sce_sys/livearea/contents/template.xml`

See [LiveArea evidence](evidence/vita_livearea_00.23.json).

## Install

1. Do **not** delete `ux0:data/DBTapBattle/` and do not replace the current
   dataset/save merely for this test.
2. Install `DBTapBattle-Vita-00.23-LiveArea.vpk` over the existing
   `DBTB00001` application with VitaShell.
3. Record whether VitaShell completes installation normally. In particular,
   `0x8010113D` must not appear; that historical failure came from an incompatible
   manually injected RGBA icon and is not expected with the validated indexed PNGs.

## Visual acceptance

Before launching the game, verify:

- the bubble shows the approved Goku icon;
- opening the bubble displays the Shenlong/starfield LiveArea background;
- the launch gate displays the Dragon Ball Tap Battle logo;
- no white/blank placeholder, corrupt image or malformed layout appears.

A Vita screenshot/photo is useful evidence for this portion because CI can verify
package bytes and XML structure but cannot establish the final shell rendering on
physical hardware.

## Runtime regression check

Launch the game without changing the data folder. A short regression pass is enough:

1. reach the menu;
2. confirm text and audio still behave as in the prior 00.23 test;
3. enter character selection;
4. choose characters;
5. start one battle and play briefly.

Because the executable is byte-identical to the already hardware-tested 00.23
binary, this is a packaging regression check rather than a new engine build.

## Report back

For a PASS, report that installation succeeded, describe whether the icon,
background and gate logo look correct, and state whether the short game launch/battle
check remained normal.

If installation fails, report the exact VitaShell error code and do not delete the
existing data/save. If the game itself crashes, provide `runtime.log` and the
`psp2core` dump if one is generated.

## Acceptance boundary

A successful device test promotes only the LiveArea/install presentation of this
repack to hardware-confirmed. It does not expand the existing 00.23 gameplay claim
to every character, mode, mod, repeated battle or long-duration session.
