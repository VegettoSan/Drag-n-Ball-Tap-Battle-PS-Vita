# Corrected 00.23 LiveArea device test

Package: `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk`
SHA-256: `19fae90627b1ddf4f42902ec228b0c50d992cb7c3cce6d3fbb6d8a8843d9edee`
Title ID/version: `DBTB00001` / `00.23`

This package uses the exact executable, SFO and notices from the hardware-tested
00.23 battle-memory package. The failed Final VPK instead contained the native CI
probe; do not use it. The corrected package adds the approved art, pads only pic0's
palette to 256 slots without changing pixels, and uses the toolkit's a1 gate layout
with content revision 2. Host checks pass; new shell acceptance is pending.

1. Install the Fixed VPK over the existing application with VitaShell.
2. Keep `ux0:data/DBTapBattle/` and its save/profile data intact.
3. Verify the Goku bubble, Shenlong background and centered Tap Battle launch logo.
4. Launch and repeat a short menu/character selection/battle check.

Report a screenshot/photo of the LiveArea and whether installation/launch succeeds.
If installation fails, send the exact VitaShell error code. If old art persists,
close the app and restart the console once before another screenshot. A crash during
launch/gameplay needs `runtime.log` and a core dump if generated. Do not delete saves
or treat the CI probe as a fallback game.
