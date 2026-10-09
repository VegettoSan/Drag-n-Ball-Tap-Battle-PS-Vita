# Current runtime/data contract — v1.1 Universal Mod Support (00.34 historical stable baseline)

## Experimental controls branch only

On `test/vita-controls`, the launcher offers two remembered choices before
every profile startup: **Touch only** (mode 0) and **PS Vita controls**
(mode 2, hidden pads). The test bubble is `DBTBCT001` / APP_VER `01.05`; its native
save path is `profiles/<Profile>/save-controls-test.bin`, initially copied from
the stable `save.bin` or the approved seed. Stable release paths below remain
the release contract. `vita-controls.cfg` holds only the Vita mode. See
[controls protocol and limits](TEST_VITA_CONTROLS_4.md). Test 1's legacy visible
mode 1 still parses, highlights the Vita row and becomes hidden mode 2 when
confirmed. The sidecar schema stays `DBTC1:<0|1|2>\n`; new selections write
only 0 or 2. Test 4 updates the same test bubble and retains previous test progress.

All launcher-owned labels, help text, loading/empty-state messages and fallback
selector text are English. Profile names come from user folder names and are
not translated or renamed. Original game/mod menus and dialogue keep their
resource language. Touch choices and preference values remain identical.
The controls guide is [an English vector schematic](VITA_CONTROLS_REFERENCE.md).

L activates original rage/pad 6; R activates the fourth special shortcut/pad 5.
In ready character selection, left/right still pulse the original arrows and
X pulses the original confirmation pad at (240,140), code `0x4100`, only in
selection mode 0. Holding X does not repeat; scene changes require release
before another press. Start in the main pause menu and Circle at a visible audited back button emit
one original contact-0 tap at (40,24). Circle also backs out of character
selection when its original back panel is present. X emits a Begin tap at
(240,280) for a live interactive script with visible text and original ready
markers. Scripted dialogue takes priority over lingering combat tasks. Original
engine code decides reveal/advance/resume; the adapter does not rewrite script
or pause state. Loading, held controls and excluded confirmation dialogs do
not synthesize these shortcuts. Nested pause settings return with Circle;
Start resumes only the main pause menu. Other menu choices remain touch-operated.

Vita modes overlay save-read config byte 4 with original pad mode 1 and retain
the touch preference on disk during original writes. Gameplay input remains
`KeyData.Set/Clear`; no task/command/controller implementation is replaced.
Hidden mode uses a sparse per-stream view of the normalized effect animation
image fields, preserving the immutable shared cache and source PACs.

> This file is the source of truth for the **current** runtime and extractor
> contract. Historical test documents may mention older `game/` and `mods/`
> layouts because those paths were correct for those specific builds.

## Current package identity

- release candidate ready to publish: **v1.1 Universal Mod Support**, hardware approved on the `dbz mobile v9` profile
- Vita APP_VER: `01.01`
- TITLE_ID: `DBTB01178`
- executable baseline: hardware-confirmed VisualQuality 2026-10-08, identical bytes in the v1.1 VPK
- historical v1.0 / 00.34: previous known-good gameplay baseline (unchanged, retained for rollback)

Historical 00.34 test records intentionally retain `DBTB00001`, because that
was the Title ID of the exact VPK tested on hardware. The v1.0 release changed
package identity, not the runtime/data contract below. The v1.1 candidate preserves the same unified profile and save paths.

## Runtime root

All playable datasets are independent first-level profile directories under:

```text
ux0:data/DBTapBattle/profiles/
```

There is no special runtime `game/` directory and no separate `mods/` root.

Examples:

```text
ux0:data/DBTapBattle/profiles/gen/
ux0:data/DBTapBattle/profiles/Invasion/
ux0:data/DBTapBattle/profiles/DragonBallZuperSamuGamerYT/
```

## Selector contract

The selector (runtime inherited from 00.34) enumerates only directories that actually exist directly
inside `profiles/`.

- There is no unconditional **Original** row.
- There is no **Original (missing)** / **ORIGINAL - DATA MISSING** row.
- A folder called `Original` appears only if the user actually has a
  `profiles/Original/` directory.
- If no profile directories exist, the selector shows **NO GAME DATA FOUND** and
  tells the user to prepare a Tap Battle APK with the Web or Windows extractor.
- The folder name is the selector display name.
- Renaming the folder changes the selector label; PAC files do not need editing.

The Gen-derived selector theme uses the four packaged assets under
`app0:/selector/`. The background is drawn using its non-transparent content
bounds and stretched to the complete 960×544 Vita viewport.

After a profile is confirmed, the Vita port presents the 00.34 themed **OPENING PROFILE** /
**LOADING GAME DATA...** transition before the original engine starts loading
that profile. This is presentation only; it does not change original game logic.

## Resource isolation

Once selected, a profile resolves resources only from:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

There is no cross-profile fallback. A missing resource is a compatibility error,
not permission to borrow the same filename from another profile.

## Save contract

Each profile owns:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

The VPK carries the read-only master seed `app0:/save.bin`. The runtime copies
that seed only if the selected profile has no save yet. Existing progress is not
overwritten by profile switching or a VPK update.

APK-bundled `save.bin` files are recorded by the extractor for provenance but
are not installed as mutable Vita progress.

## Extractor contract

Two user-facing extractors implement the same contract:

- **Web Extractor 1.0:** https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/
- **Windows Extractor 1.5:** `tools/windows/Extract_APK_for_Vita.bat`

The Web Extractor processes APK bytes locally in the browser and does not upload
them. Both tools must produce the same `profiles-v1` structure and save policy.

### Output

Both extractors must produce:

```text
data/DBTapBattle/profiles/<sanitized APK filename>/
```

for **every** supported APK. Codec/layout detection never chooses the visible
profile name.

Examples:

```text
gen.apk
-> data/DBTapBattle/profiles/gen/

tap battle android 14.apk
-> data/DBTapBattle/profiles/tap_battle_android_14/

TAP BATTLE INVASION BETA 3.apk
-> data/DBTapBattle/profiles/TAP_BATTLE_INVASION_BETA_3/
```

Both extractors validate archive safety, CRCs, file sizes, protected PAC profile
structure, collisions, SHA-256 provenance, `common.pac`, and the supported
contiguous character-triplet namespace. Neither writes a runtime `game/` or
`mods/` directory. APK-bundled `save.bin` is never installed as profile
progress.

Web-specific implementation and validation evidence is documented in
[WEB_DATA_TOOL](WEB_DATA_TOOL.md); Windows-specific details remain in
[WINDOWS_DATA_TOOL](WINDOWS_DATA_TOOL.md).

## Evidence status

v1.1 is the latest accepted hardware-tested candidate: the high-resolution `dbz_mobile_v9` profile entered and completed successive fights on physical Vita using the VisualQuality runtime, although some protected textures remain blurry because they are reduced to protect GPU memory. The v1.1 VPK contains that exact executable and changes APP_VER metadata only. See [v1.1 release notes](RELEASE_v1.1.md) and [installation guide](INSTALLATION_AND_EXTRACTION.md).

Historically, 00.34 was the first hardware-confirmed stable checkpoint. The exact tested VPK is:

```text
DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk
SHA-256: 24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03
source: 0da8684805d1510caf93130a22eed523a854c1d6
```

The user reports the VPK stable and functional on physical Vita, with no problem
found so far in the exercised selector, profile-loading and gameplay paths.
