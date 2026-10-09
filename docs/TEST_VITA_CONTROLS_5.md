# Vita Controls Test 5 — broader back and dialogue input

Branch `test/vita-controls`; stable main unchanged. Test 4 confirmed Start
resume and Circle within pause but failed other-menu back and X dialogues.
This candidate revises the Vita adapter only and requires another Vita test.

## Install

Install **DBTapBattle-Vita-01.06-Controls-Test-5.vpk** over the previous test
bubble with VitaShell. Same **DB Tap Battle Controls Test**, TITLE_ID
`DBTBCT001`, APP_VER `01.06`. Profile resources and isolated
`save-controls-test.bin` remain; stable `save.bin` is untouched.

## Launcher order

| Position | Label | Stored mode |
|---|---|---|
| First | PS VITA CONTROLS | 2, hidden pads |
| Second | TOUCH ONLY | 0, original touch settings |

Every profile launch asks again and remembers the highlighted choice. Reordering
rows does not reinterpret saved values: 0 highlights Touch; 1/2 highlight Vita;
confirming legacy visible mode 1 writes hidden mode 2. A missing preference
keeps the existing Touch default, highlighted on the second row. No save/file
migration. All launcher-owned text remains English.

## Revised shortcuts

- **Circle:** one original contact-0 touch at (40,24), through an audited live
  original CheckBack consumer. No pause-specific sprite requirement. Works
  through the same coordinate region used by real back touches; the game
  decides the actual return. Character selection and menus with a simultaneous
  script can receive back too. Combat still uses Circle as special shortcut 3.
- **X:** one Begin at (240,280) while original interactive script 811 is alive.
  Dialogues consume any-screen Begin; no finished-text, demo-position or text
  sprite requirement is imposed by Vita. Original script state decides whether
  to reveal, advance/skip or ignore the touch. Type 9 is noninteractive and
  excluded. X attack and character confirmation retain their original paths.
- **Start:** retains pause entry and main pause resume. Nested pause settings
  return via Circle. No new action on unrelated menus.

Loading/resume and audited Yes/No confirmation tasks suppress shortcuts.
Previous-frame bTaskSkip no longer excludes menu/script input (Run resets it
before dispatch); combat's existing gate is retained. Menu/script input outranks
stale battle tasks. Real contacts keep priority; back waits for contact 0.
Held shortcuts do not repeat; task identity/mode transitions cancel pending
back, including when a dialogue remains alive. Release before pressing again.

## Check on physical Vita

1. Confirm the first row is **PS VITA CONTROLS**, the second **TOUCH ONLY**.
   Reopen profiles with each remembered mode and verify the correct highlight.
2. In Vita mode, use Circle in character selection and the other screens with
   the original back control. Confirm one press returns once; holding it must
   not exit multiple screens. Check Start pause/resume still works.
3. At the victory dialogue and other tappable text, release then press X.
   It should perform the same action as a screen tap. Repress to advance the
   next block; holding X must not skip several blocks or attack on transition.
4. Test real touch priority, loads and original Yes/No choices. Check existing
   combat controls and isolated test progress remain correct.

## Evidence scope

Original-class JVM tests execute actual Game1 navigation 692 -> 693 and Game4
script 811 finished/markerless touch branches; all 37 audited back consumers
without pause-sprite fixtures, scene queues, holds, loading and exclusions pass.
Native preference probe passes reordered rows, legacy normalization, persistence
and malformed/symlink fallback. Python: 41 pass, 20 existing gated skips.
Fresh private TeaVM generation and local full-engine ARM/VPK validation are
recorded in [artifact evidence](evidence/vita_controls_test_5.json). No workflow.
Original core/PatchResourceInit unchanged; inherited native visibility corpus
is not rerun and does not certify all mods. Unknown custom code/UI still needs
its own audit. **Test 5 hardware outcome is pending.**
