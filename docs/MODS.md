# Mod Compatibility Model

## Goal

Allow the Vita port to use ordinary Tap Battle community data/assets with as little repacking as possible.

## Selection model

At startup the port scans:

```text
ux0:data/DBTapBattle/mods/
```

and presents:

```text
Original
<mod directory 1>
<mod directory 2>
...
```

`Original` is always available as the first logical entry.

## Overlay behavior

If `MyMod` is selected and the game requests `effect.pac`:

```text
ux0:data/DBTapBattle/mods/MyMod/effect.pac
```

is tried first. If absent, the port falls back to:

```text
ux0:data/DBTapBattle/game/effect.pac
```

A mod can therefore contain only changed files.

## Compatibility tiers

### Tier A — Direct resource replacement

Examples: PAC, texture, audio or data replacements using formats already supported by the original game.

**Target:** direct compatibility.

### Tier B — Added characters/cards/data using original formats

Requires the preserved original engine/adapters to support the same indexes/counts/loading behavior used by the mod.

**Target:** high compatibility after the relevant engine limits/data rules are understood.

### Tier C — APK/Dalvik code modifications

A mod may change `classes.dex` to alter mechanics, limits, menus or modes.

**Target:** assets remain reusable, but code behavior must be explicitly ported to C/C++; Android bytecode is not executed by the Vita port.

### Tier D — Protected/private mods

If a mod encrypts/obfuscates its resource package, normal PAC compatibility cannot be assumed.

**Target:** case-by-case support only when the format is legally/technically available to analyze.

## Metadata

A Vita-side `mod.json` may describe a mod without changing its original resources:

```json
{
  "name": "Example Mod",
  "author": "Author",
  "version": "1.0",
  "description": "Optional description"
}
```

The initial selector intentionally works without JSON and uses the folder name. Metadata parsing can be added later without breaking existing folders.

## Non-destructive rule

The port must never require a mod installer to rewrite `game/`. Mod activation is a runtime decision only.

## Current compatibility checkpoint — 00.24 baseline + audited mod profiles

The physical-Vita baseline is 00.24: the supplied legacy Android14 profile
reaches selection/battle with the later text, audio, selection, PAC-streaming
and battle-audio fixes retained. Ordinary Gen assets are also host-validated.
Spanish and Invasion Beta 3 protected layouts now have separately audited codec
profiles and synthetic/runtime-normalisation coverage, but those two datasets
still need a physical-Vita test before being called hardware-confirmed. See
[COMMUNITY_MOD_PROFILES](COMMUNITY_MOD_PROFILES.md) and
[CURRENT_STATUS](CURRENT_STATUS.md).

- Missing file → original fallback. Existing malformed/non-regular override →
  explicit error; a full PAC replaces the entire file, not individual entries.
- Original PACs plus the audited protected Android14, Spanish and Invasion
  profiles support per-file outer PAC decoding, converted GameData tables,
  PNG/private RGBA and protected WAV wrappers. Unknown constants/aliases still
  require a separately audited profile and fail instead of being guessed.
- `classes.dex` from a mod is not dynamically executed. The port compiles the
  original APK core privately to native C; changed mod mechanics require explicit
  behavior adaptation and separate evidence.
- Names/spaces/UTF-8 remain in paths. Folder name labels the selector; its font
  is ASCII-limited. mod.json, metadata icons and previews remain unimplemented.
- Save is only the selected `game/save.bin` or `mods/<Profile>/save.bin`, never
  a shared fallback. Bundled saves are preserved; choose imports intentionally.
- Format-valid data may still depend on APK/Dalvik code changes. The local-data
  gate keeps the 13-character baseline but now accepts contiguous complete
  character triplets up to the original core's 31 loaded GameData slots;
  Invasion supplies 22. This is resource compatibility, not a universal mod API.
- Arbitrary protected containers and synchronized multiplayer are unsupported.

## Import routes

```sh
# Base original resources; does not include downloaded character triplets.
python3 tools/extract_apk_data.py original.apk /private/install/game
# Original-style populated assets plus characters; auto ignores empty raw stubs.
python3 tools/extract_apk_data.py gen.apk /private/install-gen/game
# Pinned Community14 aliases/codec, confined to its own profile.
python3 tools/extract_apk_data.py community.apk /private/install --mod Android14
# Audited Spanish/Invasion protected APKs are detected by their own profiles.
python3 tools/extract_apk_data.py spanish.apk /private/install --mod Espanol
python3 tools/extract_apk_data.py invasion.apk /private/install --mod Invasion
# Other mods only when their layout/names/format contracts are supported.
python3 tools/extract_apk_data.py mod.apk /private/install --mod MyMod
```

The extractor preserves data bytes, records hashes/aliases and refuses ambiguous
or conflicting imports. Android .so/DEX are omitted. Unknown extensions can be
preserved without claiming a runtime decoder. Do not install an encoded
Community14 dataset over the only base copy just to fix a missing resource;
its absent bobj00/font00 need base fallback. See [ANDROID14_APK](ANDROID14_APK.md),
[DATA_LAYOUT](DATA_LAYOUT.md) and [Original+Characters](ORIGINAL_PLUS_CHARACTERS_APK.md).

## Performance and cache limits

Original GameData filter semantics now reach disk I/O. Bounded native PAC,
immutable-texture and PCM caches reduce repeated work on host; no hardware
selection-latency improvement is yet confirmed. Cold/evicted loads still do
I/O, conversion and upload. Restart after modifying installed files; caches are
not a general hot-reload API. Resource codec does not alone select text charset:
Gen's ordinary text00 is UTF-8 while its game/character tables use Shift_JIS.

## Compatibility evidence for a new mod

Record source hash/profile, complete triplets/shared fallback, file codec and
counts, character/card table bounds, charset, save expectations and whether
Java logic differs. Test first/repeated selection, multiple voice events, text,
actual battle/results and independent saves on the target build. Promote only
those observed features. Shared helper-library bytes establish lineage but not
publisher identity or a universal installed-mod loader.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.24 (2026-10-05, America/Bogota):** the user
> confirms `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` works on the physical Vita
> after the Android14 battle-start crash. Runtime source `f5672d4d`, VPK SHA-256
> `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
> The original PAC streaming repair remains; Ogg PCM now uses one exact allocation
> instead of transient vector doubling, with cache-only resource reclamation.
> The approved LiveArea is retained. This is a user-confirmed test checkpoint,
> not exhaustive character/profile/mode or long-session certification. Historical
> records keep their original artifact and evidence scope.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
