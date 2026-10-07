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

## Standalone profile behavior

If `MyMod` is selected and the game requests `effect.pac`, the only valid runtime source is:

```text
ux0:data/DBTapBattle/mods/MyMod/effect.pac
```

If it is absent, that selected dataset reports a missing-resource error. It is **not** replaced with `game/effect.pac`.

The runtime treats each extracted APK/profile as an independent installation, matching the user's requirement that Gen/Android14/community datasets work even when `game/` is empty.

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

## Current compatibility checkpoint — 00.33 hardware-confirmed

The physical-Vita baseline is 00.24: the supplied legacy Android14 profile
reaches selection/battle with the later text, audio, selection, PAC-streaming
and battle-audio fixes retained. Ordinary Gen assets are also host-validated.
Spanish and Invasion Beta 3 protected layouts have separately audited codec profiles. Invasion is hardware-confirmed standalone for the tested gameplay/textures/audio paths; the earlier result-text corruption is no longer observed after the charset fix, and 00.33 also hardware-confirms the reproduced repeated-fight allocation crash is resolved. See
[COMMUNITY_MOD_PROFILES](COMMUNITY_MOD_PROFILES.md) and
[CURRENT_STATUS](CURRENT_STATUS.md).

- Missing file in the selected profile → explicit selected-profile error. There is no automatic `game/` fallback. A full PAC remains a complete file, not an entry-level merge.
- Original PACs plus the audited protected Android14, Spanish and Invasion
  profiles support per-file outer PAC decoding, converted GameData tables,
  PNG/private RGBA and protected WAV wrappers. Unknown constants/aliases still
  require a separately audited profile and fail instead of being guessed.
- `classes.dex` from a mod is not dynamically executed. The port compiles the
  original APK core privately to native C; changed mod mechanics require explicit
  behavior adaptation and separate evidence.
- Names/spaces/UTF-8 remain in paths. Folder name labels the selector; its font
  is ASCII-limited. mod.json, metadata icons and previews remain unimplemented.
- 00.33 retains the 00.30 save model: Original and every mod have an independent writable `save.bin` inside that profile. Each missing profile save is initialized once from the exact VPK `app0:/save.bin` seed; existing progress is never overwritten and APK-local saves are not installed.
- Format-valid data may still depend on APK/Dalvik code changes. The local-data
  gate keeps the 13-character baseline and now audits the complete two-digit
  character namespace 00..99 (up to 100 contiguous complete triplets). It
  rejects incomplete triplets and gaps followed by later character data.
  Invasion supplies 22; Samu supplies 92. 00.32 hardware testing confirms the dynamic roster path exposes the complete installed roster for the tested mods. This is still resource compatibility, not
  a universal mod API or a claim that every theoretical slot has content.
- Invasion is also a Tier-C code mod: compared with Android14, 372 of 595 common
  code-method signatures have different DEX instructions. Do not port those
  changes wholesale; investigate only a concrete missing mechanic with evidence.
- Audio compatibility is independent of PAC compatibility. 00.30 retains content detection of the
  actual BGM codec from file content: ordinary Vorbis keeps the existing
  libvorbisfile path, while MP3 and AAC/M4A are decoded directly with Vita
  `SceAudiodec`. The logical `.ogg` filename is not treated as codec proof.
  This also provides the required boundary for Invasion's mislabeled BGM. Invasion has physical gameplay/audio evidence on Vita; exhaustive per-track coverage remains separate.
- Zuper/SamuGamerYT is a separate **Gen-derived Tier B+ case**: its DEX and
  manifest are byte-identical to Gen, but it expands `char/chardemo/charf`
  from 13 to 92 indices using ordinary PACs. The Vita audit covers all 92
  supplied triplets and retains the complete ID 00..99 namespace without
  modifying original gameplay logic. Its 12 MP3 + 3 AAC/M4A + 2 Vorbis BGM are
  now consumed **byte-for-byte as extracted from the APK**; no transcoding,
  renaming or repacking is required. Two empty-but-valid charf PACs and
  nonstandard `u`/`.pn` entry tags are preserved as source data.
- Arbitrary protected containers and synchronized multiplayer are unsupported.

## Import routes

```sh
# Base original resources; does not include downloaded character triplets.
python3 tools/extract_apk_data.py original.apk /private/install/game
# Original-style populated assets plus characters; auto ignores empty raw stubs.
python3 tools/extract_apk_data.py gen.apk /private/install --mod Gen
# Pinned Community14 aliases/codec, confined to its own profile.
python3 tools/extract_apk_data.py community.apk /private/install --mod Android14
# Audited Spanish/Invasion protected APKs are detected by their own profiles.
python3 tools/extract_apk_data.py spanish.apk /private/install --mod Espanol
python3 tools/extract_apk_data.py invasion.apk /private/install --mod Invasion
# Gen-derived 92-character Samu mod: validates the pinned APK and preserves
# every source asset byte-for-byte. 00.33 decodes MP3/AAC/Vorbis by content.
python3 tools/prepare_samu_mod.py DragonBallZuperSamuGamerYT.apk /private/install/mods/ZuperSamu
# Other mods only when their layout/names/format contracts are supported.
python3 tools/extract_apk_data.py mod.apk /private/install --mod MyMod
```

The generic extractor preserves data bytes, records hashes/aliases and refuses ambiguous
or conflicting imports. Android .so/DEX are omitted. Unknown extensions can be
preserved without claiming a runtime decoder. The Samu helper follows the same
non-destructive rule: it records codec/hash metadata but does not transform any
BGM or PAC payload. Do not install a protected profile over another dataset. `bobj00`/`font00` omissions observed in standalone protected APKs are not repaired by borrowing Original files; if the preserved original TeaVM core requests one on Vita, that is a port-compatibility gap to adapt explicitly from APK evidence. See
[APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md),
[ANDROID14_APK](ANDROID14_APK.md), [SPANISH_ANDROID14_APK](SPANISH_ANDROID14_APK.md),
[INVASION_BETA3_APK](INVASION_BETA3_APK.md),
[DRAGONBALL_ZUPER_SAMUGAMERYT_APK](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md),
[DATA_LAYOUT](DATA_LAYOUT.md) and [Original+Characters](ORIGINAL_PLUS_CHARACTERS_APK.md).

## Performance and cache limits

Original GameData filter semantics now reach disk I/O. Bounded native PAC,
immutable-texture and PCM caches reduce repeated work on host; no hardware
selection-latency improvement is yet confirmed. Cold/evicted loads still do
I/O, conversion and upload. Restart after modifying installed files; caches are
not a general hot-reload API. Resource codec does not alone select text charset:
Gen's ordinary text00 is UTF-8 while its game/character tables use Shift_JIS.

## Compatibility evidence for a new mod

Record source hash/profile, complete triplets/standalone completeness, file codec and
counts, character/card table bounds, charset, save expectations and whether
Java logic differs. Test first/repeated selection, multiple voice events, text,
actual battle/results and independent-save continuity/isolation across profiles on the target build. Promote only
those observed features. Shared helper-library bytes establish lineage but not
publisher identity or a universal installed-mod loader.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.33 (2026-10-07):** physical Vita testing
> confirms the reproduced Invasion repeated-fight/Saitama→Freezer crash is fixed
> after the protected-PAC ownership-transfer repair. The recent hardware sequence
> also confirms Loading recovery and dynamic installed rosters, including Samu's
> 92 characters. Scope is limited to tested paths; see [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->

### 00.33 protected-profile memory result

The remaining reproducible Invasion crash from 00.32 was not mod-specific game
logic. Its coredump resolved to a duplicate native vector allocation at the final
protected-PAC normalization handoff. 00.33 replaces that copy with explicit
ownership transfer and the user subsequently completed several Invasion fights
without a crash. Keep this fix generic to transformed PAC ownership; do not add
character- or mod-name conditionals.
