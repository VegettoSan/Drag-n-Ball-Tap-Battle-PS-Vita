# Audit completion and next milestone — 2026-10-04

Baseline 1e3699b; original APK hash and references pinned in APK_AUDIT.md.
All original native source/build/tool/documentation files were reviewed. Changes
are directly on main, published in small commits. Repository remains a
source-level reconstruction project, not an Android APK runner.

| Area | Result | Evidence / limit |
|---|---|---|
| APK architecture | FORMAT CONFIRMED | 77 ZIP files; 91 DEX classes; 57 raw resources; no native .so |
| Outer PAC | FORMAT CONFIRMED | All 19 parsed; every entry read by native host reader |
| Nested SPR | Container FORMAT CONFIRMED | Six nested packs; native animation/composition pending |
| PNG | Host decode confirmed | 51 exterior PNGs; corrupt/dimension-budget tests; GPU untested |
| Mod overlay | Host tests + Vita build confirmed | Original/mod precedence, missing fallback, bad override errors, 258 folders; real mod gameplay pending |
| Selector/input | BUILD CONFIRMED | D-pad/stick/Cross/back + touch events compiled; device behavior pending |
| Build/VPK | BUILD CONFIRMED | Real ARM compiler→VELF→SELF→VPK; final provenance/hashes in evidence |
| Audio | Format confirmed, playback PENDING | All 36 Vorbis streams probed; source channel/volume rules documented |
| Original game flow | Source map established | Actual task dispatch, state discrepancies recorded; no game loop implemented |
| Save | Source allocation/read/write confirmed | 12906 bytes; actual interoperability PENDING without a user save fixture |
| External data | Missing character triplets confirmed | Original download-wrapper naming recovered; survivor completeness/server status UNCONFIRMED |
| Bluetooth | Original transport/core boundary identified | RFCOMM and synchronization; native multiplayer PENDING |
| mod.json/Unicode typography | PENDING | Optional metadata ignored; folder names retained, ASCII diagnostic font |
| Vita3K/hardware | PENDING | No emulator/device execution in this environment |

## What was sound

Original PAC layout arithmetic, external immutable game data, file-level mod
fallback, staged roadmap and explicit verification categories were good starting
choices. No need to replace their architecture or repack assets.

## What needed correction

Extractor conflict/manifest handling, nested data preservation, CRC failure
publication; VFS NUL/component validation and original presence; PAC allocation
budget/backing revalidation/partial table state; missing entry→PNG step; unchecked
renderer init/upload; held-button screen transition; logging/provenance and
build dependency order. These were fixed and tested within their actual scope.

## Important discoveries and risks

1. The APK lacks full playable character data, not just optional cosmetic DLC.
2. GdGohan code/wiki is modified; state numbers, counts and adapters cannot be
   copied blindly. Actual original save writes must not be lost to DAD output.
3. PAC LE does not imply every internal field is LE; CNV uses BE rectangles.
4. DAC animation is not the same schema as converted gamedata/text DAC tables.
5. Entire PAC replacement may require a matching CNV/DAC/SPR set and engine
   logic; file lookup alone does not make a community mod playable.
6. Actual GPU memory/shader/alpha/FBO orientation, font rasterization, touch
   gestures and gameplay timing need original-reference/device evidence.
7. A valid VPK is a build artifact, not HARDWARE CONFIRMED.
8. The current preview opens one dataset then exits; returning to selector,
   persistence of last choice, richer metadata and log rotation remain pending.

## Next milestone

First validate this VPK on Vita: Original selector→common.pac 9 entries→first
512×512 atlas plus logs, then repeat with a mod override/fallback. After that,
implement the original GameData filter/metadata path, CNV DrawImage and DAC
_SetAct/_ActReqMain, followed by SPR DrawSprite and original task/panel menu.
Add audio/text/input services along the recovered boundaries. Obtain a user-owned
complete character dataset before character selection/battle work. Do not invent
menu coordinates, frame durations, collision formulas or a new combat system.

## Android14 follow-up — 2026-10-04

Audited supplied SHA a210795b alongside b84f98a3. Shared native helpers match the
GdGohan SWB exactly; publisher unconfirmed. Import/extraction, aliases, PAC/SPR
and DEFLATE RGBA have HOST CONFIRMED results: 9 Python regressions, 125 outer
PACs, 12 nested SPRs and 470 textures. New diagnostic source tracks alpha and
codec. Vita SDK/build/GPU verification of **this follow-up** remains PENDING;
previous build evidence applies only to its source hash. Added 13 triplets/51
cards are installed data, not implemented native character/battle behavior.
See ANDROID14_APK.md and evidence/android14_validation.json for scope/commands.
