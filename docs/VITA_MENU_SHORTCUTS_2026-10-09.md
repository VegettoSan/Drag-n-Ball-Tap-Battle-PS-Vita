# Vita menu shortcut audit — 2026-10-09

## Test 5 revision after physical failures

The audit below preserves Test 3's inspected paths and now-obsolete adapter
guards. Test 4 only confirmed pause shortcuts; other-menu Circle and X text
failed. [Report](evidence/vita_controls_hardware_report_test_4.json).

**Current rule:** live audited CheckBack coordinate consumers define back
input at (40,24), pointer 0. The common 806/action-10/resource-0x6000 panel is a
pause-specific signature, not a universal back-button requirement. The
consumer whitelist below remains, but sprite detection is removed. Live
sentinel confirmation tasks explicitly suppress shortcuts even if a parent
consumer is still active.

Interactive script 811 consumes iTouchStatus generated from any Begin among
all five contacts. A finished text with marker uses its ready branch; other
interactive types also test global touch without demo/text-object markers.
Test 5 sends a normal Begin at (240,280); original code ignores or acts on it
according to its own state. Only original type 9 is excluded as noninteractive.
No new state writes or forced script transitions. Original Run collects touch
state then resets bTaskSkip to false before task dispatch; the prior-frame
flag cannot reject new menu/script input. Combat retains its existing gate.

Priority: loading/resume/confirmation exclusions; pause; script; character
selection; audited back menus; combat. Script type 9 ignores X and never falls through to stale combat. A separate
audited back listener can still receive Circle during that automatic script. Back can coexist with a script;
its own consumer identity and frozen mode also participate in rearming and
queued-contact cancellation. Real fingers keep their slots.

Original-class tests now execute Game1 case 692 (back -> mode 693) and Game4
case 811 (finished-text and markerless global-touch branches), beyond checking
that KeyData/CheckBack sees a contact. No real game resources, bytecode or
generated commercial sources are committed. [Test 5](TEST_VITA_CONTROLS_5.md).

## Historical Test 3 adapter design (superseded)


## Source and scope

Private inspection of the supplied pinned original APK's dex2jar classes.
No original bytecode, generated C, APK or JAR is committed. User's victory
photo motivates dialogue handling; no profile/runtime-log identity is inferred.
Adaptation lives in VitaControls only. Existing main compatibility patches
remain unchanged; same-input regenerated main and candidate patched JARs have
identical entries. Controller/KeyData retain original bytes.

## Original paths and adapter guards

| Feature | Original path | Vita touch |
|---|---|---|
| Resume pause | Game11 task mode 847 calls CheckBack, ends pause panels and restores pause/fade state | Start rising edge, contact 0 at (40,24) |
| Visible back | Original CheckBack coordinate consumer + live common panel | Circle rising edge, contact 0 at (40,24) |
| Interactive text | Game4 task 811 reads original Run Begin/iTouchStatus and text-ready markers | X rising edge, free contact at (240,280) |

CheckBack accepts nonzero touch status in -10<x<100, 0<y<48 when not loading.
It has an Android-back route too; this adapter does not inject it. `bBackKey`
remains false. The original pause code performs resume; Vita does not write
`bPause`, fade, script fields or task modes to force navigation.

A common CreatePanelSingle task initializes mode 805 into 806. Visible back
requires work[0]=0, work[2]&0xff00=0x6000, work[4]=work[5]=0, initialized object,
object animation/action `ano=10` and object visibility bit 1 clear. work[1]
changes after initialization and is unsuitable for later action matching.
`bBackVisible` denotes background visibility, not this button; it is not used.

Audited coordinate consumers (37 task modes):
688, 692, 694, 695, 696, 699, 847, 855, 246, 283,
127, 131, 141, 143, 146, 149, 158, 179,
353, 360, 368, 375, 385, 409, 416, 421,
1086, 1104, 1108, 1117, 761,
1014, 1024, 1036, 1048, 1051, 1057, 1063.
Sentinel-coordinate confirmation cases 704, 851, 264, 286, 62, 74, 112 are
excluded. Standard visible-back menus may occur after battle with bGameStart
still set; that stale flag does not exclude otherwise valid back contexts.
Pause task 847 legitimately uses bTaskSkip; this one context is permitted.
BT mode 8 remains excluded. Nested settings 855 use Circle; Start is limited
to the main pause menu, not every paused task.

Dialogue requires non-pause/non-skip, live script 811 with work[0] != 9,
iDemoPushXPos != -1, iTextEnd != 0 and visible initialized text task 821 or 823.
Original script type 9 is noninteractive and excluded. Begin contacts reach
original iTouchStatus; the script decides typing completion or advancement.
Vita does not write iTextEnd/wSystemFlag. Dialogue is checked before combat
because a victory scene may retain its combat task temporarily.

## Input lifecycle and evidence

Synthetic pause/back need original pointer 0 because these menus read contact
0. A real finger on 0 is never evicted; one pending press waits until available.
Context, TCB identity or its frozen mode changing cancels the queue and requires
held buttons to release. This prevents queued back escaping into another menu
when the same TCB changes its mode. X text may use any free original contact.
Loading, hidden/inactive panels and absent ready markers cannot arm shortcuts.

Expanded tests/VitaControlsProbe calls original CheckBack with emitted touch
coordinates, and covers Start resume, Circle, finger priority, cancelled queues,
held controls, transitions, visible-panel guards, confirmation exclusions and
interactive/noninteractive script input. Existing combat/selection cases pass.
[Build/test evidence](evidence/vita_controls_test_3.json).
No Test 3 hardware result yet. Resource-only mods preserving original task/UI
contracts can share these paths; unknown custom game code is not certified.
