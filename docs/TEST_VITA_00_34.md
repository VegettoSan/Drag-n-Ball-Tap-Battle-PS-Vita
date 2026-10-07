# PS Vita test — 00.34 unified profiles + selector UX

> **Status:** complete playable VPK generated; physical Vita result pending.
> **Gameplay baseline:** 00.33 remains the latest hardware-confirmed checkpoint.

## Exact user-test artifact

- File: `DBTapBattle-Vita-00.34-Selector-UX-Fix.vpk`
- APP_VER: `00.34`
- TITLE_ID: `DBTB00001`
- VPK SHA-256:
  `e06ded147eead1c7ee8e5a558552d5129a98b5916395c59780125782c8d55c92`
- Runtime/selector source checkpoint:
  `63bc0f90d33d4a5d8d90c4816ff0f0ae07272751`

Documentation/extractor commits may be newer than this source checkpoint without
changing the VPK executable.

## Purpose

00.34 keeps the hardware-confirmed 00.33 game/runtime fixes and changes the data
installation/selection contract plus selector presentation.

Current data root:

```text
ux0:data/DBTapBattle/profiles/
```

Every first-level folder is one independent playable dataset. There is no special
`game/` root, no separate `mods/` root and no unconditional Original row.

## Selector appearance

The selector uses four non-character derivatives from the supplied Gen
`select0.pac`:

- `select0_background.png`
- `select0_header.png`
- `select0_button.png`
- `select0_ball_1.png`

Expected behavior:

1. Blue/cyan grid-energy background.
2. Beveled **SELECT DATA SET** header.
3. Yellow **DRAGON BALL TAP BATTLE VITA** subtitle.
4. One button and one-star Dragon Ball marker per installed profile.
5. Only real folders inside `profiles/` are shown.
6. No synthetic **Original**, **Original (missing)** or
   **ORIGINAL - DATA MISSING** row.
7. If there are no profiles, the selector shows **NO GAME DATA FOUND**.
8. The background uses its non-transparent content bounds and is stretched to
   fill the complete 960×544 Vita viewport.
9. Long profile names reduce text scale rather than escaping the button.

The packaged theme remains optional at runtime: if one of the selector PNGs is
missing/corrupt or cannot upload, the safe flat selector fallback remains
available instead of crashing.

## Profile-opening transition

After X/touch confirms a profile, the selector presents one completed themed
frame before the original engine starts loading:

```text
OPENING PROFILE
<selected folder name>
LOADING GAME DATA...
```

It reuses the same background/header/button assets. This screen is a visual
loading indication only; it does not introduce a fake gameplay progress value,
change PAC loading order or modify original game logic.

The transition is intentionally presented before destroying the selector
textures, so the user does not see an apparently frozen selector during the
initial profile startup.

## Input checklist

- D-pad Up/Down changes selection.
- Left stick Up/Down changes selection.
- X confirms.
- Front touch confirms the touched row.
- Circle cancels.
- More than six profiles scroll around the active item.

## Data-layout checklist

Prepare profiles with Windows extractor 1.5 or manually follow
[CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).

Verify on Vita:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

- selector lists exactly the installed first-level folders;
- folder rename changes selector label;
- selected profile never borrows resources from another profile;
- each profile keeps its own `save.bin`;
- VPK updates do not overwrite an existing profile save.

## Gameplay regression after selection

At minimum test:

- enter title/menu from one ordinary profile;
- enter title/menu from one protected profile if installed;
- start and finish an Invasion fight;
- if available, repeat the former Saitama -> Freezer path;
- verify Samu roster/audio remains functional;
- suspend/resume once from the selector and once after entering the game.

## Evidence to return

For success:

- photo/screenshot of selector;
- whether background fills all four screen edges;
- whether **OPENING PROFILE / LOADING GAME DATA...** is visible;
- profile(s) launched;
- whether at least one fight completed.

For any failure also provide:

```text
ux0:data/DBTapBattle/logs/runtime.log
```

and the generated `psp2core` when present.

Relevant log lines include:

```text
Boot selector: Gen select0.pac visual theme loaded (no character art)
Boot selector entered: <N> installed profiles
Boot selector opening profile: <Profile>
Selected profile: <Profile>
```

## Build/CI evidence

- Selector/runtime source `63bc0f9`: Vita engine native smoke
  run `37679405794` — **PASS**.
- Same source checkpoint: private build-tool export run `37679405502` —
  **PASS**.
- Windows extractor 1.5 `profiles-v1` regression run `37688246446` —
  **PASS**.
- Extractor implementation-only run `37688218032` — **PASS**.

These establish build/tool correctness, not physical Vita rendering/gameplay.

## Acceptance

Do not promote 00.34 to HARDWARE CONFIRMED until this exact VPK hash has been
observed on a physical Vita through selector + profile launch + basic gameplay.

If it passes, 00.34 becomes the new presentation/data-layout checkpoint while
00.33 remains the historical proof for the protected-PAC repeated-fight repair.
