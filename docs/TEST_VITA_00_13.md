# PS Vita hardware test — 00.13

**Archived test sheet.** These expectations describe 00.13 at publication and
are not current unresolved-engine milestones. Later tests restore visible text
(00.19), while rough voices and character pauses remain unresolved. Latest
00.22 selection recovery is pending after 00.21 starts audio/reaches menu. See [CURRENT_STATUS](CURRENT_STATUS.md) and
[TEST_VITA_00_22](TEST_VITA_00_22.md). Do not infer this exact Gen APK hash or every
00.13 fix is hardware-confirmed from a later generic Original-profile test.

Build 00.13 is a focused follow-up to the hardware-tested 00.12. It retains the 00.12 text glyph cache/partial texture upload and Community14 voice decoding, while correcting two hardware regressions and adding the hybrid charset handling needed by the user-supplied Original+Characters APK.

## What is already hardware confirmed from 00.12

- Real game startup, front touch, menus, character selection and battle remain functional.
- Dialogue/text creation no longer causes the multi-second stalls observed before 00.12.
- Community character voice data is decoded sufficiently to produce audible speech.
- 00.12 introduced compressed/thin-line text in both Original and Android14.
- 00.12 voice identity is shifted: some expected voices are missing and other requests play the wrong/repeated clip.

## 00.13 changes under test

1. Text geometry now builds a cached line envelope from scaled `ScePvfCharInfo` representative glyphs. It does **not** return to per-sentence glyph metric/rasterization work. The glyph cache and dirty-row `glTexSubImage2D` path remain enabled.
2. Character voice playback IDs are zero-based, matching original `SoundEffect.wave[id]` and `TCBManajer`'s `iReqSENo - 80` request. Vita also limits concurrent streamed voices to the original three AudioTrack channels.
3. Text encoding is selected from normalized GameData content when strong UTF-8 is present. This supports the hybrid Original+Characters dataset whose PAC directory is ordinary/original but whose `text00.pac` strings are UTF-8.

## Test sequence

### Original / existing game data

- Start Original and navigate screens that contain several text sizes.
- Confirm glyphs have normal height/position rather than thin horizontal lines.
- Confirm the text no longer causes the old multi-second stalls.

### Android14

- Enter character selection and a real battle.
- Trigger several different voice events for the selected character.
- Confirm the first voice is no longer missing and that different events select the expected different clips rather than the previous neighboring clip.
- Confirm menus/dialogue text has normal geometry and retains the 00.12 performance improvement.

### Original+Characters (`gen.apk`) dataset

- Extract its real `assets/` data as `ux0:data/DBTapBattle/game/`. The earlier
  sheet advised omitting the bundled save for that test. Current extraction
  preserves it and the game uses the selected profile's save.bin; back up
  existing progress and decide intentionally whether to install that supplied save.
  See [ORIGINAL_PLUS_CHARACTERS_APK](ORIGINAL_PLUS_CHARACTERS_APK.md).
- Start the Original slot in the selector.
- Verify common menus/text, character selection and battle.
- Pay particular attention to `text00` strings: this dataset uses ordinary PAC headers with UTF-8 `text00`, while its character/game tables remain Shift_JIS.

## Evidence to return

If a visual/audio problem remains, preserve `ux0:data/DBTapBattle/logs/runtime.log`. A screenshot is especially useful for any remaining text positioning issue. A `psp2core` dump is only needed if there is an actual native crash.
