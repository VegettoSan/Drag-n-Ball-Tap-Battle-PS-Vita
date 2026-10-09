# Current status — 2026-10-09, v1.2 PS Vita Controls (release ready)

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

## Documentation alignment — 2026-10-09

All 78 pre-existing tracked Markdown files were reviewed, and the
[documentation index](DOCUMENTATION_INDEX.md) adds the 79th. Current subsystem,
build, validation, installation/extractor and next-work pages now match v1.2.
Every page links to current status/contract while preserving dated build and
APK evidence. The user is testing stable v1.2; no outcome is recorded yet.
These are Markdown-only updates; the delivered executable/VPK is unchanged.

## v1.2 working controls promoted to main

The user approved combat and hidden pads, the L/R swap and X character
confirmation, then Start resume and finally Circle beyond pause in Test 5.
Dialogue X still failed and is removed at the user's request; advance text by
front touch. The retained script guard blocks stale combat input during text.
[Final Test 5 report](evidence/vita_controls_hardware_report_test_5.json).

v1.2 is a fresh local full-engine build, APP_VER `01.02`, TITLE_ID `DBTB01178`.
It updates the stable bubble and uses each profile's existing `save.bin`.
`save-controls-test.bin` belongs to the separate test bubble and is not migrated
or overwritten. Existing `vita-controls.cfg` preferences retain their meaning.
The English launcher offers **PS VITA CONTROLS** (hidden pads) first and
**TOUCH ONLY** second before every profile launch, remembering the highlight.
No core task, combat or controller implementation is changed.

The v1.1 VisualQuality resource/memory baseline is retained. Its earlier
hardware result and the separate controls tests do not certify every mod or
the exact rebuilt v1.2 package. Release/package evidence is recorded in
[release notes](RELEASE_v1.2.md). Publication is prepared for the maintainer.

Final VPK: `Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk`, 2750706 bytes.
SHA-256 `343aee505f77fa743339111fa7cf29f1e9bda333e49bddb6be166933d7bac1fc`. Full local engine/package and LiveArea checks pass;
JVM controls, native preferences and Python regressions pass (41 run / 20 skipped).
Build source `f6e9adaa792d38c4f3a7c7b27d112ae54b41ede5`.
No release workflow was dispatched and no new GitHub release is claimed.

## Historical Test 5 candidate (superseded by v1.2)

User tested Test 4 (01.05): Start resumes the main pause menu; Circle goes
back only within pause. Circle outside pause and X dialogues did not work.
Back buttons occupy the same location when needed; real dialogue taps work
anywhere. Profile and runtime log were not supplied. This is partial acceptance,
not approval of all Test 3/4 shortcuts. Test 2's earlier acceptance still stands.
[Hardware report](evidence/vita_controls_hardware_report_test_4.json).

On `test/vita-controls`, Test 5 removes the pause-only graphic requirement for
Circle and the non-universal demo/text-object markers for X. Live audited
original CheckBack consumers receive a single contact-0 touch at (40,24);
interactive script 811 receives X as an ordinary Begin anywhere, letting the
original script accept or ignore it. Prior-frame bTaskSkip cannot block these
menu/script contacts; original Run resets it after consuming input. Audited
Yes/No confirmations suppress shortcuts. Script type 9 ignores X while allowing
a separate menu back listener to remain available. Menu/script
contexts outrank stale combat tasks, and pending back cancels if its consumer
changes even while a script remains alive. Start resume behavior is retained.

English selector order is **PS Vita controls** first / **Touch only** second.
Stored mode values keep their meaning and remembered highlight; neither saves
nor folders migrate. Bubble `DBTBCT001` / `01.06` updates previous test versions
and keeps isolated test progress. New JVM checks execute original Game1 back
navigation and original Game4 finished/markerless text branches; all 37 back
consumer modes, holds, touch priority and exclusions pass. Native preferences
and Python regressions pass. Full local build/package evidence is in
[Test 5](TEST_VITA_CONTROLS_5.md). At delivery its hardware result was pending;
the final report above approves Circle and rejects dialogue X. Main integration
is now complete; the existing core patch pipeline remains unchanged.
[Updated audit](VITA_MENU_SHORTCUTS_2026-10-09.md).

## Historical physical controls research — 2026-10-08, no runtime change at that stage

The original settings and combat pad were inspected in the supplied APKs.
Original mode 1 offers a virtual stick and six buttons; mode 2 is gesture input.
A new host JVM probe passes type-1/type-4 direction/button press, hold, release
and five simultaneous logical contacts. Recommended implementation feeds Vita
controls as synthetic contacts to original KeyData, with a native selector
choice and profile-local option applied through the existing save service.
The tutorial temporarily forces mode 2 and save reloads restore ConfigData[4].
Those boundaries require explicit handling; physical gameplay remains absent
from the current VPK. No engine/runtime code was changed by this research.
See [research and implementation plan](VITA_CONTROLS_RESEARCH_2026-10-08.md).

Pad visibility research additionally found an adapter-only resource overlay:
the original renderer skips DAC frames with image -1. All nine supplied APKs
have an isolated pad-action closure of 22 actions/25 image fields; a hypothetical
50-byte in-memory overlay leaves unrelated DAC bytes and source files unchanged.
Linked button/stick layers must also be included. This is structural evidence,
not an integrated invisible-pad feature or a hardware rendering result.
See [visibility design and evidence](VITA_PAD_VISIBILITY_RESEARCH_2026-10-08.md).

## Historical v1.1 VisualQuality release baseline

The user has **approved the 2026-10-08 VisualQuality experimental build on
physical PS Vita** with the high-resolution protected `dbz_mobile_v9` profile.
Character selection, first fight, victory and successive battles proceed without
the previously reproduced crashes or long loading deadlocks. Attached
`runtime.log` has 340 performance windows, 317 at 58+ FPS, 78
`[TextureCompact]` messages, 7 indexed AAC tracks and 3 combat cache
boundaries; no logged `std::bad_alloc`. Heavy-load transitions can take longer.

**Known accepted limitation:** some high-resolution mod textures appear blurry
because the general dynamic-C14U renderer uses RGBA4444 and adaptively reduces
physical GPU texture resolution to stay within PS Vita memory limits. Not all
community mods are verified. No per-mod quality exceptions were added.

**v1.1 release package:** `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk`,
SHA-256 `9953e8c99ce59a5b4b55dab3ae2caec788c39ffe1edb19a2d4e5833c958ee6bd`,
APP_VER `01.01`, TITLE_ID `DBTB01178`. Its `eboot.bin` matches the hardware
approved VisualQuality VPK bit-for-bit; only APP_VER SFO metadata was changed.
VPK ZIP and LiveArea integrity pass. The previous v1.0 remains historical.

GitHub release `1.1` is published; the existing asset and its recorded hash
remain unchanged by v1.2 preparation. Full instructions: [v1.1 release notes](RELEASE_v1.1.md)
and [Web/Windows install guide](INSTALLATION_AND_EXTRACTION.md).

---

## Historical status — 2026-10-07 v1.0 / 00.34

> **Authoritative current contract:** [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).
> Any later section in this document that mentions `game/`, `mods/`, an
> unconditional Original selector entry, or old profile-save paths describes
> the historical build named by that section. The current public release is
> **v1.0 / APP_VER 01.00 / TITLE_ID DBTB01178**; 00.34 remains the exact
> hardware-tested gameplay baseline.

## Universal protected DragonTap loader — experimental source, not yet hardware validated

New Web and Windows extractors automatically inspect the shared PRIVATE-MOD Dalvik initializer to recover PAC keys and aliases without adding a hard-coded profile. For new protected variants, extraction creates a versioned **dbtb_codec.json** inside the selected profile (raw PAC bytes unchanged). The Vita native resource bridge strictly parses this metadata on selection, resets previous profile state and uses the preexisting PAC/GameData/image/WAV decoders with the `C14U` texture marker. All changes are isolated to data extraction and native resource loading; the original TeaVM/AOT gameplay logic has not been modified.

CI: [Web end-to-end DEX/ZIP fixture](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37718130522), [Windows 20 tests](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37718063263), [native host regression](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37717841023), native Vita build smoke (dummy **nonplayable** AOT). See [DRAGONTAP_PRIVATE_UNIVERSAL](DRAGONTAP_PRIVATE_UNIVERSAL.md) for schema and limitations. **Published v1.0 VPK remains unchanged; a new full-engine test VPK and actual Vita gameplay acceptance are still pending.** The Python CLI extractor was not yet given this DEX reader.

## DBFZ v22 mod integration — source-only candidate, hardware pending

A new user-supplied `Dbfz v22.apk` (`5fe0b98d45822cc060a95ef8d9bfa069b4005d7897d83c540db160134c4af67f`) uses a fourth protected Community14-family PAC codec and renamed asset families. The updated Web and Windows extractors recognize this audited profile, and the native PAC/texture bridge has corresponding decode support, **without modifying original AOT gameplay logic or the already published stable v1.0 artifact**.

The corpus contains 261 PAC files, 58 contiguous character triplets (00..57), protected game/text tables, and 10 AAC/M4A files mislabeled `.ogg`. Audited metadata and implementation details: [DBFZ V22 APK](DBFZ_V22_APK.md). Standalone installation path: `ux0:data/DBTapBattle/profiles/Dbfz_v22/`.

**Not a hardware acceptance claim**: this source update requires a separately built test VPK and Vita gameplay/regression evidence. Public v1.0 / 00.34 remains the stable hardware-tested checkpoint.

## v1.0 public release identity

The first stable public release is:

- file: `Dragon-Ball-Tap-Battle-PS-Vita-v1.0.vpk`
- APP_VER: `01.00`
- TITLE_ID: `DBTB01178`
- SHA-256:
  `15eb056274db6f3ad561c3befb670833c348f536c3073590b9768b04f74ee594`
- gameplay/runtime source baseline:
  `0da8684805d1510caf93130a22eed523a854c1d6`

This v1.0 package is derived from the exact 00.34 stable build. The Title ID
migration changes `sce_sys/param.sfo` identity only; the gameplay executable
and packaged game-facing resources are unchanged. Because the exact 00.34 VPK
was the binary physically exercised, its historical metadata remains recorded
below as `TITLE_ID DBTB00001`. A new physical launch/install check of the
DBTB01178 package is still the correct final publication sanity check.

## 00.34 stable hardware-confirmed VPK

The latest complete user-test package is:

- `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk`
- SHA-256:
  `24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03`
- runtime/selector source:
  `0da8684805d1510caf93130a22eed523a854c1d6`
- physical Vita result: **HARDWARE CONFIRMED — stable and functional**

Current runtime/data behavior:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

There is no current `game/` or `mods/` split. The selector lists only real
profile directories, has no synthetic Original row, and shows a no-data message
when `profiles/` is empty. Profile saves live inside the selected profile and
are seeded once from `app0:/save.bin`.

The selector row buttons are now centered on the Vita viewport. Their labels
are now centered inside the cyan interior and dynamically shrink before touching
the silver bevels.

The selector background now uses only the continuous 482×320 cyan/grid band
from the top of the 512×512 Gen-derived source and stretches that region to
960×544. The separate blue energy orb stored in the lower transparent portion
is explicitly excluded. After profile confirmation, a themed
**OPENING PROFILE / LOADING GAME DATA...** screen is presented before the
original engine begins loading the selected dataset.

Web Extractor 1.0 and Windows Extractor 1.5 target the same `profiles-v1`
contract and always derive the visible profile folder from the APK filename.
The web version runs entirely in the browser and downloads a Vita-ready ZIP;
the selected APK is not uploaded.

Build/tool evidence for the current candidate:
- Vita engine native smoke on selector/runtime checkpoint: `37679405794` PASS.
- Private build-tool export on the same checkpoint: `37679405502` PASS.
- Windows extractor 1.5 `profiles-v1` regression: `37689580096` PASS (latest cleanup/test run; earlier contract run `37688246446` also passed).
- Web Extractor 1.0 core: syntax/unit validation PASS; real APK package tests
  PASS for original, Gen, Android14, Spanish Android14, Invasion Beta 3 and Samu.
  The Samu stress case generated a 405,409,508-byte ZIP with 92 characters and
  passed independent ZIP CRC validation.
- GitHub Pages deployment: run `37704582648` PASS. GitHub reports the live URL:
  https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/
  (an earlier deployment request hit a transient GitHub HTTP 500; the subsequent
  deployment completed successfully).

**00.34 is now the latest HARDWARE CONFIRMED checkpoint.** The user reports
the exact VPK above is stable and functional in the tested real-Vita session,
with no problem found so far across the exercised selector, profile loading and
gameplay paths. 00.33 remains historical evidence for the protected-PAC
repeated-fight repair.

## Web Extractor 1.0 / GitHub Pages

A static browser extractor now lives under `web/` and is deployed with
`.github/workflows/pages.yml` to:

https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

It processes APK bytes locally, validates ZIP/CRC/SHA-256, applies the same
audited protected PAC aliases as Windows Extractor 1.5, checks roster structure,
excludes APK-local `save.bin`, writes manifests/checksums and produces a
downloadable `data/DBTapBattle/profiles/<Profile>/` ZIP.

The web UI reuses the exact validated Gen selector theme and has dedicated
desktop, portrait-mobile and landscape-mobile layouts. See
[WEB_DATA_TOOL](WEB_DATA_TOOL.md) for implementation, limits and validation.

## 00.34 candidate update — unified APK profiles

The current 00.34 source candidate now uses a single public/runtime dataset root:

`ux0:data/DBTapBattle/profiles/<Profile>/`.

The previous special `game/` plus `mods/` split has been removed from the
current source. The boot selector enumerates only directories that are actually
present under `profiles/`; it no longer creates an unconditional Original row,
so `ORIGINAL - DATA MISSING` is gone.

If `profiles/` is empty, the selector remains on its themed/fallback screen and
shows a no-game-data message instructing the user to prepare a Tap Battle APK with
the extractor and copy it to `ux0:data/DBTapBattle/profiles/`.

The current Web Extractor 1.0 and Windows Extractor 1.5 emit every APK as an
independent profile named from the APK filename. APK layout/codec detection remains automatic and separate
from naming. Renaming a folder under `profiles/` changes the selector display
name without modifying PAC files or re-extracting the APK.

Profile saves now live at
`ux0:data/DBTapBattle/profiles/<Profile>/save.bin`, still seeded once from the
VPK master save when absent.

This layout is now **HARDWARE CONFIRMED in 00.34** using the exact stable VPK
recorded at the top of this file.

## 00.34 candidate — Gen-styled first-screen data selector

The next test build is **00.34**. Its only intended presentation change is the
native Vita data-set selector shown before the original engine starts. The old
flat dark list is replaced, when the embedded theme loads correctly, by a menu
composed from four non-character visual elements extracted/cropped from the
supplied `gen.apk` `assets/select0.pac`: the blue/cyan grid-energy background,
beveled title bar, beveled menu button and one-star Dragon Ball marker.

The selector still owns only data-profile choice; it does **not** replace or alter
the original Tap Battle title/menu/combat logic. D-pad/stick, X, touch and Circle
retain the same selector semantics. The four textures are loaded once from
`app0:/selector/`, released before entering the game, and the already-tested
plain selector remains as a safe fallback if any packaged PNG is missing or fails
to decode/upload.

The theme is stored in Git as a hash-validated split Base64 ZIP reconstructed by
`tools/materialize_selector_theme.py`. The VPK contains only the four derived PNGs,
not `gen.apk`, `select0.pac`, characters, music or a playable data set. Release
validation pins each embedded PNG SHA-256 so a damaged/replaced theme cannot be
published silently.

**BUILD CONFIRMED for the native Vita smoke target:** the 00.34 CMake packaging and
the new `src/ui.cpp` selector renderer both compile/link/package successfully in
VitaSDK CI. The full 00.34 VPK has now been tested on physical Vita and is the current
hardware-confirmed stable checkpoint.

A one-shot full-prerelease attempt (Actions run
[37623204195](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37623204195))
stopped **before generation/compilation** because the repository does not currently
have the private `DBTB_ORIGINAL_APK_URL` secret configured. That run is not a
selector/code failure and produced no playable VPK.

## 00.33 hardware-confirmed — Invasion repeated-fight allocation fix

00.33 is now the current physical-Vita development checkpoint.

The 00.32 retest had already confirmed that the Loading loop was fixed and that
extended mod rosters load correctly, including Samu's full 92-character roster.
Its remaining reproducible failure was Invasion: Saitama (char15) advancing to
his second fight against Freezer (char05) terminated with native
`std::bad_alloc`.

The supplied 00.32 `psp2core`, symbolicated against the exact matching ELF,
resolved the allocation through `std::vector<unsigned char>::operator=` inside
protected-PAC `normalise()`. Invasion `char15.pac` is 4,054,165 bytes on disk
and normalizes to 4,638,744 bytes. GCC 15 lowered the previous conditional move
expression to another vector copy on the changed path, requesting a second
multi-MiB contiguous allocation after the normalized PAC already existed.

00.33 replaces that ambiguous assignment with explicit ownership transfer:
`if (changed) output.swap(out); else output = input;`. No source PAC bytes,
DEX behavior or original gameplay logic are modified.

**Physical result:** the user reports several consecutive Invasion fights on
00.33 with no crash. The reproduced Saitama -> Freezer failure did not recur and
is considered **resolved in the tested hardware scope**.

Current artifact:

- `DBTapBattle-Vita-00.33-Invasion-Saitama-Freezer-Fix.vpk`
- size: 2,650,664 bytes
- VPK SHA-256: `d241499a356ac11c523909a84b0c383910ef7a387efcfdc2c05d3581be86fd77`
- eboot SHA-256: `bc0a0d4293e5b416084d02050dd6b3c17529cc63fab00bfe7a48d0310303d43b`
- ELF SHA-256: `6c55a58f59277bee0d2632dbefca8c1a938d577457b867489d3605a11cee7dbe`
- APP_VER `00.33`, TITLE_ID `DBTB00001`
- runtime checkpoint `71b95d54ad6eef0ebd2043eb96269f6e7a4e1370`
- LiveArea: PASS.

Confirmed in the recent hardware sequence:

- Loading loop fixed;
- dynamic per-profile rosters working, including Samu 92;
- Invasion text appears correct in the tested path;
- Samu direct compressed BGM transition works;
- repeated Invasion fights no longer reproduce the protected-PAC bad_alloc.

Open coverage is now broader regression work: Shop return behavior still needs a
clean explicit hardware report, along with return-to-menu/suspend-resume,
additional modes/mods and longer sessions.

See [TEST_VITA_00_33](TEST_VITA_00_33.md),
[build evidence](evidence/vita_build_00.33.json) and
[hardware evidence](evidence/vita_hardware_00.33.json).


### Documentation reconciliation — 00.33

After the hardware confirmation, the repository documentation was swept end to
end. All Markdown documentation is now aligned around **v1.0 / APP_VER 01.00 /
TITLE_ID DBTB01178** as the current public identity, with **00.34** retained as
the exact hardware-confirmed gameplay/runtime baseline. Historical test reports
keep their original observations, hashes, APP_VER and Title IDs; only stale
"current status" banners and obsolete install-path claims are corrected.

The active documentation contract is therefore:

- v1.0 / DBTB01178 is the current public release identity; 00.34 is the exact hardware-confirmed gameplay/runtime baseline;
- the Invasion Saitama -> Freezer repeated-fight crash is RESOLVED in the tested scope;
- Samu's 92-character dynamic roster is hardware-confirmed;
- the Loading loop regression is resolved;
- profile resources and mutable saves remain isolated;
- old test documents are historical evidence, not current blockers.


## 00.32 candidate — fix 00.31 infinite Loading

Physical 00.31 logs from both Invasion and Zuper/Samu show the engine alive at
about 60 FPS but stuck forever immediately after the obsolete
`device/screensize.csv` request. Direct inspection of the pinned APK proves the
Downloader contract was inverted in 00.31: `isDownload()==true` means the
request is still running. The Vita offline stub now returns false immediately
with no data so the original TCB state takes its normal offline/failure path.

00.31 also performed a deep PAC audit before entering the engine, producing
roughly 13 seconds of profile-init delay for Invasion and 34 seconds for Samu.
00.32 uses a presence/contiguity scan for the runtime roster instead. Deep PAC
validation remains in tooling/tests rather than the boot path.

Complete test VPK:
`DBTapBattle-Vita-00.32-Loading-Loop-Fix.vpk`, SHA-256
`07a8ab63923e4913cc1810a5658c84ef925d3eddf08ac04b81658c683956b4b9`.

00.31 dynamic roster/save synchronization, Shop return behavior, large-PAC memory
policy, Invasion text fallback and Samu direct compressed audio are retained.
Hardware retest subsequently passed on 00.33; this 00.32 section remains historical.

## 00.31 candidate — dynamic roster, repeated-fight memory, Shop and startup

The user's physical 00.30 test confirms Samu now boots and the previous Invasion
text defect was not seen in the tested path. It also supplied four concrete new
failures/limits:

- Samu audits 92 complete characters but the seed exposes only 00..12.
- Invasion crashes on a later fight with `std::bad_alloc` after loading a
  multi-MiB character PAC.
- Shop exits through the Vita adapter's intentional Android-marketplace
  `UnsupportedOperationException`.
- The obsolete catalog/update state pauses roughly 10–25 seconds.

00.31 addresses each at the narrowest evidenced boundary:

1. The selected profile save synchronizes original ConfigData character flags
   +1/+2/+85 for every audited installed character. The verified Gen core's
   character-only arrays `bCharIndex`, `bCharVersionSv`, `bCharNoSv` are
   extended 90→100, and only `CharVisibleInit` / `ClearCharDLALL` replace
   their 90 bound with the audited installed count. Unrelated constants remain
   untouched.
2. Character PAC source files over 2 MiB clear stale resource-LRU ownership
   before normalization and are not retained afterwards, reducing the repeated
   battle allocation peak that ended 00.30 with `std::bad_alloc`.
3. Vita Shop completes the Android Smap lifecycle edge synchronously and returns
   to the game instead of throwing. Android purchasing is still not implemented.
4. 00.31 attempted to model the offline Downloader with
   empty data, size 0, SetURL false and `isDownload=true`. Hardware testing then
   proved that polarity was wrong: `true` means the async request is still in
   progress. 00.32 corrects this to immediate completion with `isDownload=false`.

Complete physical-test artifact:

- `DBTapBattle-Vita-00.31-Roster-Shop-Memory-Startup-Test.vpk`
- size 2,653,793 bytes
- SHA-256 `85b28d7a080c2bc5806ca3c269a7fe15b2be84565c60ddca243dd3fad0e6e699`
- eboot SHA-256 `be043461ae79f0ce789a7389f8d4ba315f8121172ae138f505a8ea33728134a6`
- source checkpoint `bf283ae5c5e1d0e7c19b540dd89b0105edfa3a4b`
- native smoke and private export: PASS
- LiveArea: PASS

Physical acceptance remains pending. See
[TEST_VITA_00_31](TEST_VITA_00_31.md) and
[evidence](evidence/vita_build_00.31.json).

## Historical 00.30 candidate — same Samu/Invasion fixes, independent seeded saves



00.30 keeps the targeted Samu decoder-handoff and Invasion UTF-8 fallback from
00.29, but supersedes the experimental global mutable save model.

The VPK still ships the exact user-provided 12,906-byte `app0:/save.bin` seed,
SHA-256 `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.
When a profile is selected, the runtime checks only that profile's writable save:

- Original: `ux0:data/DBTapBattle/game/save.bin`
- Mod: `ux0:data/DBTapBattle/mods/<Profile>/save.bin`

If absent, it copies the VPK seed there once. If present, it loads the existing
profile save and never overwrites it. Thus every fresh dataset starts from the
same seed but can diverge independently afterward. The old 00.29 root
`ux0:data/DBTapBattle/save.bin` is ignored by 00.30.

Python and Windows extractors continue to exclude APK-bundled `save.bin` from
the installed data profile, while recording its presence/metadata. This prevents
a mod's packaged save from silently replacing the known VPK seed.

A complete 00.30 physical-test VPK is available:

- `DBTapBattle-Vita-00.30-Independent-Profile-Saves-Test.vpk`
- 2,654,057 bytes
- VPK SHA-256:
  `d823b9baddd667b09b1c407575a5698cd71276e1698e9d9845fb3086b3777379`
- eboot SHA-256:
  `c193a07463cbc7168bb1a5d5398f259dc4cdb0216b9d909b268df3b386faca2c`
- ELF SHA-256:
  `e5b38f5c4a5409d3ead2fc42929e3cf9e6a9fc1d931c0d54e0bf23262f59a236`
- runtime marker: `6d88bee`
- LiveArea: PASS.

Physical acceptance still requires Samu past the title, Invasion result-text
readability, and confirmation that progress diverges independently between at
least two profiles. See [TEST_VITA_00_30](TEST_VITA_00_30.md) and
[evidence](evidence/vita_build_00.30.json).

## Historical 00.29 candidate — Samu BGM handoff, Invasion text fallback, one global save


00.29 is built directly from the user's 00.28 physical Vita results.

**00.28 hardware findings:**

- Invasion runs as a standalone profile with `game/` absent.
- Invasion textures and direct external audio were reported working normally.
- Invasion's post-battle/result text showed corrupted glyph/string content while
  surrounding UI text (for example the character name and GANADOR/result frame)
  remained correct.
- Samu, both with and without `game/`, detected its 92-character dataset and
  reached the title flow, then exited.
- Both Samu logs identify the same concrete failure:
  `sceAudiodecCreateDecoder failed 0x807f0007` while replacing MP3 BGM.
- Direct APK inspection proves Samu `bgm_16.ogg` and `bgm_00.ogg` are
  byte-identical valid MP3 payloads. The failure therefore came from decoder
  lifetime, not damaged media: the backend was configured for one MP3 stream and
  00.28 tried to create the replacement decoder before destroying the active one.

**00.29 changes:**

1. `dbtb_bgmPlay` now clears the active Voice/Vorbis/compressed BGM under the
   audio lock before opening the replacement compressed track. This preserves
   the one-stream `SceAudiodec` contract and specifically fixes the observed
   Samu title transition without converting assets.
2. Character PAC text remains data-driven. Invasion `char20.pac` (Ranma) was
   decoded from the supplied APK and contains valid UTF-8, including
   `Ranma está disponible！`; the native content detector also classifies that
   character PAC as UTF-8. The original Gen `SetString` slot convention can
   miss the GameData object used by the modified Invasion DEX, so
   `ResourceAdapter.stringCharset` now keeps the exact per-object charset first
   and falls back to the detected active character-profile charset instead of
   blindly falling back to Shift_JIS.
3. Save state is intentionally global. The exact user-provided 12,906-byte
   `save.bin` is included in the VPK at read-only `app0:/save.bin`; if and
   only if `ux0:data/DBTapBattle/save.bin` is absent, boot copies the seed
   byte-for-byte there. Original and every mod/profile then read/write that one
   global save. Existing progress is never overwritten on profile switches or
   VPK updates.
4. APK-local saves are no longer installed by the Python or Windows dataset
   extractors. Their presence can still be recorded for provenance, but gameplay
   uses only the global 00.29 save.

Shared-save seed:

- size: 12,906 bytes;
- SHA-256:
  `64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`.

Public CI/native smoke has passed the BGM handoff source, VPK save materializer,
global save path and Vita link/package changes. A complete physical-test artifact
has also been built:

- `DBTapBattle-Vita-00.29-Samu-Invasion-SharedSave-Test.vpk`
- 2,653,396 bytes
- VPK SHA-256:
  `fc2a4ced375212eeb32609c199b95e0c58b55dfc6ccf07c0ce6da95f9a093e75`
- eboot SHA-256:
  `4f5f911adc27de89dea996670fdfd6d64b6a29b340f2f0826c1638eddfa6f06a`
- ELF SHA-256:
  `a3574f8777eb2d12a1db08af6a96bde509bff6787ee1d0af293d65b19e755f58`
- runtime source checkpoint: `1fe6e2dbff77b456846f7c4b6203c01383a5a549`
- APP_VER `00.29`, TITLE_ID `DBTB00001`
- LiveArea: PASS
- bundled save seed: 12,906 bytes with the approved exact SHA-256.

This interactive hardware-test build uses 24 TeaVM remainder units at `-O1`,
`TCBManajer.c` at `-O0`, and native adapters at `-O2` to fit the runner
execution window. Physical verification of Samu past the title, Invasion's
corrected result string and shared-save continuity remains the acceptance gate.

See [TEST_VITA_00_29](TEST_VITA_00_29.md) and
[evidence](evidence/vita_build_00.29.json).

## Historical 00.28 candidate — standalone APK-derived profiles



00.28 changes the runtime data contract so every selected APK-derived profile is
autonomous. `game/` is now only the optional Original profile. When
`mods/<Profile>/` is selected, `GameVfs` resolves PAC/data/audio **only** from
that directory and reports `missing selected profile resource: ...` rather than
borrowing from Original. At that historical 00.28 checkpoint saves were still profile-local; 00.29 supersedes that save policy with one global save.

This is deliberate: Android14, Español and Invasion are independently runnable
APKs even though their inventories differ from Original/Gen. If the preserved
original TeaVM core asks for a resource that a modified APK does not need, 00.28
exposes that as a compatibility gap to adapt from APK/DEX evidence instead of
masking it with cross-profile fallback.

Selector behavior also supports a mod-only installation. When
`game/common.pac` is absent and one or more profiles exist, the first profile is
initially selected; `ORIGINAL - DATA MISSING` cannot be launched.

The standalone contract has two levels of non-hardware evidence:

- synthetic CI proves a missing file in the selected profile is **not** satisfied
  by an identically named file still present in `game/`;
- a real-APK host matrix with an empty `game/` accepts Gen (13 characters),
  Android14 (13), Español (13), Invasion (22) and ZuperSamu (92). Invasion also
  explicitly fails resolution of absent `bobj00.pac` inside its own profile
  rather than falling back, while its installation audit remains valid because
  that omission is part of the audited APK contract.

00.28 retains all 00.27 direct-audio work: Vorbis remains on libvorbisfile,
MP3/AAC-M4A are detected by content and decoded directly with Vita
`SceAudiodec`, and source assets are not transcoded or renamed.

Full physical-test artifact:

- `DBTapBattle-Vita-00.28-Standalone-Profiles.vpk`
- 2,648,911 bytes
- VPK SHA-256:
  `4411302f1b7e673fe49c98bb9ce0b7fe47ed086a34e1ad03025735411d07cab2`
- eboot SHA-256:
  `5473e2fd7e1ea04cd9c207af61a440ef88d265b2338a093b9a616e1762b37f12`
- ELF SHA-256:
  `595052aec345e8b08f29cc650e0cfa085c2d37d9103769b778c73778808e9bf6`
- runtime source marker: `fa9d7b6`
- APP_VER `00.28`, TITLE_ID `DBTB00001`
- LiveArea validation: PASS.

This build is host/build validated, not yet hardware-confirmed. The critical
device test is to leave `game/` empty and run a profile all the way through
menu, selection, battle, audio and save. Any selected-profile missing-resource
message should be investigated in the port, not fixed by copying a file from
Original.

Evidence:
[vita_standalone_profiles_00.28.json](evidence/vita_standalone_profiles_00.28.json).
Physical test:
[TEST_VITA_00_28](TEST_VITA_00_28.md).

## Historical 00.27 candidate — Samu + Invasion direct source audio


00.27 supersedes the 00.26 Samu import-time audio conversion approach. The user
explicitly requires the mod to work with the files **exactly as they are stored
in the APK**. The hardware-confirmed 00.24 path remains the regression baseline;
00.27 is a new physical-test candidate, not yet hardware-confirmed.

The audited Samu dataset is still 92 characters (00..91), with the Vita-side
offline gate covering the complete two-digit namespace 00..99. No original
selection/combat logic is rewritten. The important change in 00.27 is entirely at
the Vita audio boundary:

- the 17 files keep their original `bgm_XX.ogg` names and bytes;
- actual Vorbis (`bgm_12/13`) continues through the proven libvorbisfile path;
- the 12 MP3-backed `.ogg` files are detected from content and decoded with
  Vita `SceAudiodec` MP3;
- the 3 AAC/M4A-backed `.ogg` files are parsed as ISO-BMFF, their original AAC
  access units are fed to Vita `SceAudiodec` AAC, and no source file is rewritten;
- decoded PCM enters the same 48 kHz mixer used by the existing Vita backend.

`tools/prepare_samu_mod.py` is now a validation/extraction helper only. It is
pinned to the audited APK/DEX hashes, verifies all 92 triplets and the known
12 MP3 + 3 AAC/M4A + 2 Vorbis matrix, but leaves all 384 extracted assets
byte-for-byte unchanged. Its manifest explicitly records
`payloads_unchanged: true`.

The same compressed-BGM backend is intentionally profile-agnostic and has now
been checked against the supplied **TAP BATTLE INVASION BETA 3** APK. Invasion
keeps 22 contiguous character triplets (00..21) under its protected PAC profile
and changes seven BGM: five MP3 plus two AAC-LC/M4A. The two AAC files are
44.1 kHz stereo with maximum access-unit sizes 455 and 548 bytes, safely below
Vita's 1536-byte AAC ES limit; the MP3 files are valid at 44.1/48 kHz. No audio
conversion is required.

`tools/prepare_invasion_mod.py` is pinned to the audited APK/DEX hashes, uses
the existing Community14 canonical alias extraction, validates all 22 triplets
and the 5 MP3 + 2 AAC/M4A + 10 Vorbis BGM matrix, and requires
`payloads_unchanged: true`. Invasion omits `bobj00.pac` and `font00.pac` by design; the supplied protected APKs are autonomous with that inventory. In 00.28 these omissions are no longer hidden by cross-profile fallback; a differing original-core request is treated as an explicit compatibility issue.

The exact same 00.27 VPK binary is therefore the physical-test candidate for
both Samu and Invasion. Resource compatibility for Invasion does not imply that
all behavior unique to its heavily modified `classes.dex` is already ported;
any such mismatch must be isolated after the resource/audio path passes.

Public validation passes:

- Community mod profiles run `37544623252` — PASS at `926eb6b0`;
- Vita engine native smoke run `37544588962` — PASS with
  `SceAudiodec_stub` linked.

The first 00.27 Samu-named VPK was superseded after direct APK review showed
that the protected Android14/Spanish/Invasion datasets are valid without
`bobj00.pac`. The corrected physical-test candidate is:

- `DBTapBattle-Vita-00.27-Samu-Invasion-Corrected.vpk`
- 2,647,984 bytes
- VPK SHA-256:
  `311a820f948e337b0626b7b46e6dcb2ca0628b81364941b11d4c837867ee1b96`
- eboot SHA-256:
  `12106a98ecb2c6f0f4401e0879705edf57f4d7fcbee495351df07f24fdd241ec`
- ELF SHA-256:
  `aaae0778d094d17ac51bab175ce79ce350dfce49b990b10e490fe20db66cb3a6`
- runtime marker: `7fec715`
- APP_VER `00.27`, TITLE_ID `DBTB00001`
- TeaVM: 467 classes / 4086 methods
- LiveArea validation: PASS.
- installed-data gate: no longer requires `bobj00.pac`; VFS fallback remains optional compatibility when requested at runtime.

Because the interactive runner could not finish the monolithic generated C unit
at normal optimization within the command window, the private test build splits
the TeaVM remainder into ten compilation units at `-O1`, keeps the large
`TCBManajer.c` at `-O0` (the already documented interactive-build technique),
and leaves native Vita adapters including direct audio at `-O2`. Use this
artifact for functional roster/audio validation; final release performance still
requires the standard reproducible build recipe.

Evidence: [corrected 00.27 mod-compat build](evidence/vita_mod_compat_00.27_corrected.json).
Physical test protocol: [TEST_VITA_00_27](TEST_VITA_00_27.md).

## Historical 00.26 candidate — Samu roster + rejected conversion import path

00.26 keeps the hardware-confirmed 00.24 gameplay/audio path and the 00.25
protected-profile work. **00.24 remains the latest physical-Vita-confirmed
artifact until 00.26 is tested on hardware.**

The Vita-side offline installation audit no longer stops at 31 character slots.
It now validates the complete two-digit resource namespace **00..99** (up to 100
contiguous triplets), while preserving the 13-character minimum, rejecting
partial triplets and rejecting a gap followed by later character data. This
change is an adapter/gate correction only; the original TeaVM game logic is not
rewritten. Synthetic coverage now exercises 13, 22, 92 and 100 contiguous
triplets.

For the audited `DragonBallZuperSamuGamerYT.apk`, the known dataset remains
**92 characters (00..91)**. Its DEX and manifest are byte-identical to Gen, so
no Samu-specific gameplay bytecode is being transplanted. The new
`tools/prepare_samu_mod.py` is pinned to the audited APK/DEX hashes, extracts
all 384 assets, verifies all 92 `char/chardemo/charf` triplets, and explicitly
normalizes only the 15 BGM whose contents are MP3/AAC despite their `.ogg`
names. They become real Ogg Vorbis 44.1 kHz stereo under the same logical
`bgm_XX.ogg` names. The two already-Vorbis BGM are left unchanged. Every
transformation is recorded in `dbtb_manifest.json`; PAC bytes are not rewritten.

This keeps the hardware-tested libvorbisfile mixer and exact-allocation repair
unchanged instead of adding an untested MP3/AAC decoder to the Vita executable.
The original APK itself is therefore still not direct-audio-compatible; the
prepared Vita dataset is the supported Samu import route.

Host/synthetic evidence: Community mod profiles run
`37540898687` passes the Samu preparation rules and the extended roster gate.
The 00.26 native VitaSDK smoke build is tracked separately from physical
gameplay. See [TEST_VITA_00_26](TEST_VITA_00_26.md).

A full private physical-test package was also generated from the pinned original
APK after the public/runtime checks passed. Identity:

- `DBTapBattle-Vita-00.26-Samu-Roster-Test.vpk`
- VPK: 2,604,860 bytes, SHA-256
  `749b9d32e6ed62a7b4593cb6f0b5af6dc2cabbc97cd9f25986757700879e18f5`
- eboot SHA-256:
  `1de9962f19cf9c39a1534e9547de14e3a2569950a6b712ca49ab871443f90830`
- full ELF SHA-256:
  `db580cd100ac330d88908a9db2cd71f53a50b70c2295700ea0a17fbba68e7e9e`
- embedded runtime source marker: `d7a4aa2`
- SFO: APP_VER `00.26`, TITLE_ID `DBTB00001`
- TeaVM generation: 467 classes / 4086 methods
- LiveArea validator: PASS for all five approved entries
- build layout: TeaVM remainder `-O1`, `TCBManajer.c` `-O0`, native Vita
  adapters `-O2`.

The split compile is an interactive-build packaging exception matching the
technique used by the hardware-tested 00.23 package; it is suitable for the
functional Samu test but not for final performance claims. Public CI also passed
Vita engine native smoke run `37541052112` and tool export run
`37541052003`. Physical Samu gameplay remains pending.

## 00.25 candidate — audited community mod profiles

00.25 is derived from the hardware-confirmed 00.24 path; **00.24 remains the
last physical-Vita-confirmed artifact until the user tests 00.25**. The existing
PAC streaming, exact Ogg allocation, clean voice path, responsive selection,
text fixes and approved LiveArea are retained.

New host/build work adds independent protected-resource profiles for the supplied
Spanish Android14 mod and TAP BATTLE INVASION BETA 3 instead of changing the
legacy Android14 constants globally. Detection is per PAC and requires one unique
fully in-bounds profile match. The Windows/Python extractors canonicalize only
audited aliases and preserve payload bytes. Invasion's complete contiguous
character triplets 00..21 are accepted by the local-data gate without weakening
the 13-character baseline; partial/non-contiguous extensions fail.

A deeper APK/media audit found an additional Invasion blocker that supersedes
the earlier "long Vorbis" assumption. Seven files keep the `.ogg` extension but
are not Vorbis: `bgm_03/06/07/14/15` are MP3 and `bgm_04/05` are AAC inside
M4A/ISO-BMFF. The current Vita BGM adapter opens music with libvorbisfile
(`ov_fopen`), so these seven cannot be decoded by the existing path. The bounded
Vorbis streaming branch therefore does **not** complete Invasion audio support.
This must be solved at the Vita audio/import boundary by content-based codec
detection plus a supported decoder/conversion path; the original engine must
continue requesting the same logical BGM names.

Synthetic CI now covers extractor aliases/profile uniqueness, protected PAC/image
decoding, converted GameData metadata and full engine-resource normalization for
Android14 + Spanish + Invasion. The three real supplied protected APK corpora were
also audited locally: Android14 106/106 PACs, Spanish 106/106 and Invasion 139/139
uniquely match their intended profile with no decoded directory extent outside a
file. This establishes format/host compatibility, **not** complete reproduction
of arbitrary changes made only in a mod's `classes.dex`.

See [community mod profiles](COMMUNITY_MOD_PROFILES.md), [mod compatibility](MODS.md),
the [deep APK technical reference](APK_TECHNICAL_REFERENCE.md), the
[Invasion audit](INVASION_BETA3_APK.md), and the
[00.25 physical test protocol](TEST_VITA_00_25.md).

A full private local 00.25 build using the original TeaVM core also completed:
467 classes / 4086 methods, VPK SHA-256
`39265deebeec6ff7954dc85cd2fd18300fa6de8cd546176e3413b9d2823c3173`.
The final clean VPK carries APP_VER `00.25`, TITLE_ID `DBTB00001`, source marker
`d186dc65`, the approved LiveArea and no game-data/APK payloads. Its eboot
SHA-256 is `a1c35f53070a600edabb310c53f95dea269b637206461e5a7e8370839fc5f380`
and full ELF SHA-256 is `c624a1f121a58da10d3d6bc481416d78a1799352c00b6c4d8c21a777ed93c591`. This artifact
is build/host validated and still awaits a physical-Vita run.

Real extracted overlays were exercised through the native C++ paths as well:
Spanish passes the offline gate with 13 characters and normalizes 106 PAC /
361 protected images / 198 WAV / 68 BIN; Invasion passes with 22 characters and
normalizes 139 PAC / 731 protected images / 344 WAV / 80 BIN. Those historical
00.25 host runs had a base `game/bobj00.pac` available because the then-current
gate required it. A later direct APK audit established that the protected APKs
are valid without `bobj00`, so 00.27 removes that artificial gate requirement.

## Deep APK audit — 2026-10-06

The six supplied APKs were re-audited directly from their ZIP/DEX/PAC/audio
bytes so future work does not need the binaries for already-known structure.

New source-of-truth documentation:

- [APK technical reference](APK_TECHNICAL_REFERENCE.md): manifest/DEX/signing,
  layout, PAC formats, codec constants, aliases, native libraries and semantic
  comparisons.
- [Canonical APK differences](APK_CANONICAL_DIFFERENCES.md): pairwise logical
  file presence/identity after resolving `res/raw`, `assets` and protected
  aliases.
- [Spanish Android14](SPANISH_ANDROID14_APK.md) and
  [Invasion Beta 3](INVASION_BETA3_APK.md): profile-specific details/outliers.
- [Exact exterior audio matrix](evidence/APK_AUDIO_MATRIX_2026-10-06.md).
- [Machine-readable evidence](evidence/apk_deep_structure_2026-10-06.json),
  including all 372 Android14→Invasion changed DEX signatures, canonical
  comparison counts and observed PAC outliers.

Important new findings: Invasion's seven changed BGM are five MP3 + two AAC/M4A
despite `.ogg` names; `char15`/ `char20` use non-uniform interleaved RGBA
layouts (101/98 entries), `char21` has 85 entries, and Spanish/Invasion
`card034` contains a valid one-record 0×0 converted table. Parsers must follow
the directory/type contract rather than fixed per-file layout assumptions.

This audit changed **documentation/evidence only**. It does not alter the runtime
or the hardware-confirmed 00.24 path.

A sixth mod, `DragonBallZuperSamuGamerYT.apk`, was then audited separately. It is
a Gen-derived ordinary-PAC build with **92 character triplets (00..91)** while
keeping Gen's `classes.dex` and `AndroidManifest.xml` byte-identical. Its 384
assets contain 345 PACs; 345/345 outer PACs are structurally valid, all 92
`charXX` contain a 43-record BIN, and 237 files are new versus Gen. This is
strong evidence of a data-driven large roster. The 00.26 Vita-side audit now
covers the complete two-digit namespace 00..99; no 92-character hardware claim
is made until the physical test protocol passes.

The same mod also broadens the media issue: **15/17 BGM named `.ogg` are
actually MP3 or AAC/M4A**, while all 19 SE remain baseline Vorbis. 00.26 adds an
explicit import-time Samu preparation path that normalizes those 15 files to
real Vorbis without changing their logical names or the original game core. It contains
two empty-but-valid `charf` PACs (20/21), 18 type-`u` URL metadata entries and
one malformed type `.pn` whose payload is a valid PNG. These are source
outliers to document, not a reason to globally weaken the runtime parser.

See [Zuper/SamuGamerYT APK audit](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md) and
[machine-readable evidence](evidence/dragonball_zuper_samugameryt_2026-10-06.json).

## Current hardware report — 00.24

Two manual full-engine publication entries are now implemented: Release and
Prerelease, with a shared builder, original-APK hash gate, audio regressions,
VPK/ELF/LiveArea validation, compiled-only symbols and draft-first publication.
Static/unit and real-VPK staging checks pass. GitHub-hosted validation run
[37395626518](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37395626518)
passed actionlint and all 13 publication/LiveArea tests. A live full build/publication
requires the private APK URL secret; see [setup](RELEASE_WORKFLOWS.md).

Before the 00.24 repair, the user confirmed that **00.23 LiveArea-Fixed presentation
worked on hardware**, but reported another battle-start crash with Android14 selected, characters 12/03
and `bobj03`. The earlier successful 00.23 test path remains historical evidence;
this extends coverage to a failing native-audio allocation path. The PAC streaming
repair remains active in the supplied log.

That 00.23 log ends in `std::bad_alloc`. Its supplied core's game-thread stack
returns to `decodeOgg` immediately after PCM vector growth, with a requested
9,506,304-byte allocation. The old decoder reproduces that exact request for
`bgm_03.ogg`: its C++ allocation peak is 14,260,324 bytes for a 5,454,332-byte
PCM track. This differs from 00.22's managed whole-PAC allocation failure.

00.24 allocates the exact Vorbis frame count once, decodes directly into that
buffer, checks channel/rate consistency and decoded length, and drops cache-only
PAC owners/idle imported textures before allocating. Active streams and textures
retain ownership. Newlib remains 96 MiB, TeaVM's maximum remains 48 MiB, and
original battle logic, music samples, voice DSP and approved LiveArea remain
unchanged. New Ogg diagnostics identify the track/frame count.

**Host confirmed:** all 17 supplied BGM tracks match the previous decoder sample
for sample; a 6 MiB single-allocation limit reproduces the old `bgm_03` failure
and permits all fixed loads. Fixed `bgm_03` C++ peak: 5,454,432 bytes, a reduction
of 8,805,892 bytes (excluding Vorbis C allocations and unrelated owners).
Audio setup/DSP and resource ownership probes pass with Vita APIs mocked.
**Hardware confirmed by the user on 2026-10-05 at 19:26 America/Bogota:**
the delivered 00.24 works and resolves the reported battle-start crash. The user
says it works very well; no new runtime log or exhaustive character/profile
matrix was supplied. The earlier failing log/core describe 00.23, not 00.24. See
[the result and broader regression procedure](TEST_VITA_00_24.md).

Hardware-confirmed package: `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk`, 2,663,883 bytes,
SHA-256 `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
Runtime source: `f5672d4d3fbf6b43cd699d7a5a2b80475e4db9f6`. Fresh generation
compiled 467 classes/4086 methods; the standard complete TeaVM amalgamation
compiled at -O1 and native services at -O2. Native smoke CI run `37392864767`
passed, separately from this local full-game build. All five presentation entries
match the hardware-confirmed 00.23 LiveArea-Fixed package byte-for-byte. Exact ELF
and private generated sources are retained with the candidate for crash analysis.
[Identity and measured allocation evidence](evidence/vita_battle_audio_00.24.json).

<!-- DBTB_00_23_DETAIL:START -->
## Historical hardware checkpoint — 00.23 (2026-10-05)

**Status:** hardware validated for the tested path; no error observed in the user's
00.23 session.

Verified together on a real PS Vita in the current progression:

- startup and menu flow continue to work;
- text remains visible;
- audio/voices remain clean (the prior rasp/worker issue did not regress);
- character selection remains responsive (the prior 1–2 second selection stalls did
  not return in the reported session);
- starting a fight now succeeds;
- the fight can be played without the 00.22 battle-start crash.

The specific 00.22 failure was a TeaVM managed-allocation abort while bridging the
entire `char00.pac` (~4.74 MiB) into one Java `byte[]` before the original parser's
disposal order could free the prior owner. 00.23 restores the original streaming
parser contract and exposes the PAC through a Vita native `InputStream` bridge, so
Java receives individual original payload arrays instead of one whole-PAC bridge
array.

This is a **checkpoint, not an exhaustive certification**: every character, mode,
mod dataset, repeated battle sequence and long-duration memory behavior still need
broader regression coverage before a final release claim.

Test artifact SHA-256: `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd`  
Source checkpoint: `0e17b0bac33c47698b414b67a839c839f0e555ce`
<!-- DBTB_00_23_DETAIL:END -->


This is the current handoff. It describes implementation and evidence separately.
Historical audit/test pages remain useful for their pinned APK/builds; their old
pending statements do not override this page. 00.21 and 00.22 failures are historical checkpoints.
The latest 00.24 user confirmation also resolves the distinct native Ogg memory crash reported after the 00.23 LiveArea repack.

## Historical 00.23 build identity

| Field | Value |
|---|---|
| Hardware-tested VPK | `DBTapBattle-Vita-00.23-battle-memory-test.vpk` |
| Version / title ID | `00.23` / `DBTB00001` |
| Source checkpoint | `0e17b0bac33c47698b414b67a839c839f0e555ce` |
| Battle-memory repair | original streaming `GameData.Init` + native-backed `InputStream` |
| Retained selection-mask repair | original masks 187/251 accepted |
| Retained audio repair | worker startup + clean voice output in reported hardware path |
| VPK SHA-256 | `8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd` |
| Toolchain family | VitaSDK 2026.08, GCC 15.2.0, hard-float |
| Private generator | dex2jar 2.4, ECJ 3.37.0, TeaVM 0.12.3, Java 17 |
| Hardware result | startup/menu/text/audio/selection/battle path passed; no error observed in this session |

[00.23 hardware evidence](evidence/vita_hardware_full_game_00.23.json) records the tested artifact and scope.
The package used for this hardware checkpoint was an interactive test build; see [BUILD](BUILD.md) for the split-compilation caveat before treating it as a release-quality performance artifact.

The current presentation test is `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk`,
SHA-256 `19fae90627b1ddf4f42902ec228b0c50d992cb7c3cce6d3fbb6d8a8843d9edee`. It uses the exact hardware-tested 00.23
executable/SFO and retains every original package entry byte-for-byte, adding
only the five LiveArea files. `pic0.png` now has the required 256-entry palette
without changing any decoded pixels; the minimal MetalSyntax a1 gate uses XML
content revision 2. This package passes the toolkit and strengthened local
validators; the user subsequently confirmed its LiveArea works on real hardware.

The previous `DBTapBattle-Vita-00.23-LiveArea-Final.vpk` is **rejected**: its
`eboot.bin` matches the non-playable CI native link probe (run `37387861902`),
not the hardware-tested full engine. The user reported installation and
presentation failures. The supplied artwork also had a separate 192-entry
splash palette mismatch; its exact causal role in the installer failure is
unconfirmed because no VitaShell error code was supplied. The historical
LiveArea-only package/evidence remains a record of that earlier attempt, not
acceptance of the Final VPK.

See [corrected package evidence](evidence/vita_livearea_fixed_00.23.json) and
[the corrected device test](TEST_VITA_00_23_LIVEAREA_FIXED.md).

## Historical implementation versus observation — 00.23

| Area | Current implementation | Verified scope / open limit |
|---|---|---|
| Core | Original APK Init/Run/task/combat code compiled privately to C; Android service adapters | Real menu/front touch/selection/combat on earlier Vita builds; not every mode/action certified |
| Profiles | Original/mod selector; file-level override/fallback; per-file codec | Earlier Vita profile selection and Android14 battle confirmed; arbitrary mods untested |
| Cards/startup | Cooperative EventQueue progresses one ready event after present; local-data checks | User reports fixed in 00.16 |
| Battle frame rate | Reused GLES client buffers, reduced adapter work; 960×544 | 00.18 steady battle windows 59.9 FPS, user reports stable 60; not every later build validated |
| Text | Memory-based PVF with fallback; glyph metrics/cache, image rectangles, visible-only rasterization, dirty uploads | Text recovery confirmed by user in 00.19; exhaustive script/font/layout fidelity untested |
| PAC I/O | Original exclusion filter plus 00.23 native-backed streaming InputStream; bounded native cache/stream handles | 187/251 masks and stream ownership pass host probes; physical selection and battle startup pass in 00.23 |
| Imported textures | Immutable byte/mode keyed cache; live-reference tracking and idle eviction | Native PNG/ownership host probes pass with GL mocked; hardware reuse/performance pending |
| Voice samples | Original PCM16 mono 22050 Hz or decoded Community14 wrapper; 3 voice channels | Format/host decoding verified; later physical tests report clean voices/audio |
| Voice output | 16-tap/256-phase Q14 reconstruction to 48000 Hz, peak limiter, PCM cache | Clean audible result reported on the physical 00.22/00.23 path; broader character/phrase matrix remains open |
| Audio startup | Restored `0x10000100`; exact open/create/start diagnostics, failure cleanup/latch | 00.21 worker/menu recovery confirmed and no audio regression reported in 00.23 |
| Saves | 00.30: selected profile's `game/save.bin` or `mods/<Profile>/save.bin`, seeded once from exact VPK `app0:/save.bin`; cached reads and atomic writes retained | Seed/hash + native build pending latest smoke; physical profile isolation retest pending |
| Input | Stable slots mapped from Vita touch IDs; original coordinate transform and Controller | Touch gameplay confirmed; physical buttons serve selector, are neutral during game |
| Online / Bluetooth | Offline installed-data boundary; HTTP rejected; Bluetooth disconnected | Current local single-player path; multiplayer/billing/remote downloads unsupported |

Cache budgets are 8 MiB retained PAC-vector capacity, 4 MiB source+RGBA texture
cost and 2 MiB voice-input+PCM cost. These limits are **not** total process-memory
caps: live shared owners, Java copies, allocator/driver overhead and uncached
resources also use memory. First loads and eviction reloads still do work.

Clock requests are CPU 444, bus 166, GPU 222, crossbar 166 MHz. The 00.18 device
log reports effective bus 222; log API results/effective values instead of
assuming the requested value is the applied value. No higher clock profile is
established here.

## Historical next work — 00.23

1. **Confirm the LiveArea repack on hardware.** Install the corrected `DBTapBattle-Vita-00.23-LiveArea-Fixed.vpk` and verify VitaShell promotion, bubble icon, Shenlong background, launch gate/logo and a short launch/battle regression pass.
2. **Broaden 00.23 regression coverage.** Repeat battles, switch across more characters and revisit evicted resources to confirm the streaming fix under churn rather than only one successful progression.
3. **Retest both supported dataset paths/mod overlays.** Keep original fallback rules and record exact dataset hashes when comparing behavior.
4. **Measure release-quality performance.** The 00.23 hardware test package used split TeaVM compilation with `TCBManajer.c` at `-O0` because of the interactive build runner; create a normal reproducible full-engine package before making final FPS/performance claims for 00.23.
5. **Extend lifecycle/control coverage.** Return-to-menu, repeated launches, suspend/resume, save round-trips and physical Vita control adaptation remain separate work. Multiplayer/Bluetooth synchronization, billing/remote services and arbitrary code mods remain unsupported/unvalidated.

No currently reproduced crash is open in the 00.23 tested path. New failures should be recorded with exact VPK hash, dataset/profile, `runtime.log` and `psp2core` when produced.

## Evidence index

| Evidence | Scope |
|---|---|
| [00.11 battle](evidence/vita_hardware_battle_00.11.json) | Physical Android14 gameplay milestone |
| [00.18 performance](evidence/vita_hardware_performance_00.18.json) | User report, 15 steady battle windows, clocks and remaining regressions |
| [00.19 text/audio](evidence/vita_hardware_text_audio_00.19.json) | Text restored; voices/selection still fail; zero measured clipping in this session |
| [00.20 PAC/DSP build](evidence/vita_pac_voice_build_00.20.json) | Host format, I/O, cache and reconstruction tests |
| [00.20 startup failure](evidence/vita_hardware_audio_startup_00.20.json) | Physical worker setup failure and caught BGM exception |
| [00.21 build](evidence/vita_audio_startup_build_00.21.json) | Complete engine compilation and native setup/DSP/resource tests |
| [00.21 selection failure](evidence/vita_hardware_selection_00.21.json) | Physical worker/menu recovery, then rejected character mask and caught exception |
| [00.22 build](evidence/vita_selection_filter_build_00.22.json) | Before/after mask regression, full corpus and complete ARM artifact checks |
| [00.23 hardware](evidence/vita_hardware_full_game_00.23.json) | Physical Vita: clean audio, responsive selection, battle startup/gameplay pass; no error observed in reported session |
| [00.23 LiveArea package](evidence/vita_livearea_00.23.json) | Exact presentation-only repack: original tested VPK entries unchanged; LiveArea hashes/layout pass CI; physical shell test pending |

[Validation](VALIDATION.md) defines test scope and log interpretation.
[Porting guide](PORTING_GUIDE.md) explains reusable techniques and failures.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
