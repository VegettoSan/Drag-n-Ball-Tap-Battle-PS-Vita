# PS Vita test — 00.34 Gen-styled data selector

> **Status:** BUILD CONFIRMED in VitaSDK native smoke CI; physical Vita test pending.
> **Gameplay baseline:** 00.33 remains the current hardware-confirmed checkpoint.

## Purpose

00.34 changes only the native data-set selector displayed before the original
Dragon Ball Tap Battle engine starts. It must look like part of the game while
preserving the 00.33 gameplay/runtime behavior.

The visual theme is built from four non-character derivatives of the supplied
`gen.apk` `assets/select0.pac`:

- blue/cyan grid-energy background;
- beveled blue header bar;
- beveled cyan menu button;
- one-star Dragon Ball marker.

No Goku/Vegeta/other character artwork is used.

## What should be visible

Immediately after launching the VPK, before Tap Battle itself starts:

1. A blue/cyan Tap Battle-style background fills the selector.
2. A beveled header says **SELECT DATA SET**.
3. **DRAGON BALL TAP BATTLE VITA** appears as a small yellow subtitle.
4. Each installed profile is presented as a beveled cyan button with a Dragon
   Ball marker at its left.
5. The currently highlighted row is brighter than the other rows.
6. A matching bottom bar shows the controls.
7. Long profile names are reduced in font size rather than escaping the button.

The layout supports six visible rows and scrolls around the selected item when
more profiles are installed.

## Input regression checklist

- D-pad Up/Down changes the highlighted profile.
- Left stick Up/Down still changes the highlighted profile.
- X confirms.
- Front touch on a row confirms that exact row.
- Circle cancels the selector.
- Choosing **Original** still refuses to launch when Original data is missing.
- Selecting a mod still starts only that profile; no resource fallback to
  `game/` has been reintroduced.

## Runtime-safety contract

The four selector textures are loaded from `app0:/selector/` once, before profile
selection. They are deleted before the original engine starts, so they should not
consume gameplay texture memory.

If any theme PNG is missing/corrupt or fails to upload to vitaGL, the port must
fall back to the previously tested flat selector instead of crashing.

## What to test after entering the game

The selector redesign must not change 00.33 behavior. At minimum verify:

- one Original or Android14/Gen path reaches the normal title/menu;
- one Invasion fight starts and returns normally;
- if available, the former Saitama -> Freezer repeated-fight path still works;
- Samu roster/audio behavior remains unchanged.

## Evidence to return

A photo/screenshot of the new selector is the most important evidence. If
anything crashes or falls back to the old selector, also send
`ux0:data/DBTapBattle/logs/runtime.log` and any generated `psp2core`.

Relevant log lines:

- `Boot selector: Gen select0.pac visual theme loaded (no character art)`
- or, on safe fallback:
  `Boot selector: Gen visual theme unavailable, using safe legacy fallback`

## Build-side evidence

- Native smoke run: https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37622687132
- Workflow/publication validation: https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37622780686
- `VITA_VERSION`: `00.34`
- Theme ZIP SHA-256:
  `90418a27c6681ee644d5cc383e31fc73248a5c412527839d216b61bcc2516c12`

Do not mark 00.34 HARDWARE CONFIRMED until the first-screen appearance and at
least a basic gameplay regression have been observed on a real Vita.
