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

## Current compatibility checkpoint — 00.22

Earlier build 00.11 runs the supplied Android14 profile through character
selection and a real Vita battle. Ordinary Gen assets are host-validated and
used as an alternate base profile. This does not certify arbitrary asset mods,
code-modified mechanics, every added index or a complete mode matrix. Latest
00.21 worker/menu recovery is confirmed; its original selection-mask rejection
is repaired in 00.22 with physical recovery pending; see
[CURRENT_STATUS](CURRENT_STATUS.md).

- Missing file → original fallback. Existing malformed/non-regular override →
  explicit error; a full PAC replaces the entire file, not individual entries.
- Original and verified Community14 PACs, converted GameData tables, PNG/private
  RGBA and Community14 WAV wrappers are supported per resolved file. Unknown
  private constants/aliases need another audited profile.
- `classes.dex` from a mod is not dynamically executed. The port compiles the
  original APK core privately to native C; changed mod mechanics require explicit
  behavior adaptation and separate evidence.
- Names/spaces/UTF-8 remain in paths. Folder name labels the selector; its font
  is ASCII-limited. mod.json, metadata icons and previews remain unimplemented.
- Save is only the selected `game/save.bin` or `mods/<Profile>/save.bin`, never
  a shared fallback. Bundled saves are preserved; choose imports intentionally.
- Format-valid data may still exceed original counts or depend on new code.
  Complete character/shared local checks match the supplied 13-triplet datasets,
  not a universal mod API.
- Arbitrary protected containers and synchronized multiplayer are unsupported.

## Import routes

```sh
# Base original resources; does not include downloaded character triplets.
python3 tools/extract_apk_data.py original.apk /private/install/game
# Original-style populated assets plus characters; auto ignores empty raw stubs.
python3 tools/extract_apk_data.py gen.apk /private/install-gen/game
# Pinned Community14 aliases/codec, confined to its own profile.
python3 tools/extract_apk_data.py community.apk /private/install --mod Android14
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
