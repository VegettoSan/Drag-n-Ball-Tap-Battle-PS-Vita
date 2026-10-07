# Current runtime/data contract — 00.34 stable

> This file is the source of truth for the **current** runtime and extractor
> contract. Historical test documents may mention older `game/` and `mods/`
> layouts because those paths were correct for those specific builds.

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

The 00.34 selector enumerates only directories that actually exist directly
inside `profiles/`.

- There is no unconditional **Original** row.
- There is no **Original (missing)** / **ORIGINAL - DATA MISSING** row.
- A folder called `Original` appears only if the user actually has a
  `profiles/Original/` directory.
- If no profile directories exist, the selector shows **NO GAME DATA FOUND** and
  tells the user to prepare a Tap Battle APK with the Windows extractor.
- The folder name is the selector display name.
- Renaming the folder changes the selector label; PAC files do not need editing.

The Gen-derived selector theme uses the four packaged assets under
`app0:/selector/`. The background is drawn using its non-transparent content
bounds and stretched to the complete 960×544 Vita viewport.

After a profile is confirmed, 00.34 presents a themed **OPENING PROFILE** /
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

## Windows extractor contract

The Windows extractor must produce:

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

The extractor validates archive safety, CRCs, file sizes, protected PAC profile
structure, collisions, SHA-256 provenance, `common.pac`, and the supported
contiguous character-triplet namespace. It never writes a runtime `game/` or
`mods/` directory.

## Evidence status

00.34 is the current hardware-confirmed stable checkpoint. The exact tested VPK is:

```text
DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk
SHA-256: 24a723504a121e804d0ae6cae31fb0bf464b97e4c8f1bd7c7a96f239d0e55e03
source: 0da8684805d1510caf93130a22eed523a854c1d6
```

The user reports the VPK stable and functional on physical Vita, with no problem
found so far in the exercised selector, profile-loading and gameplay paths.
