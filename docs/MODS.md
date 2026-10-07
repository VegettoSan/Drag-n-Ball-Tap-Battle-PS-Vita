# Mod compatibility model — current 00.34 contract

> Historical test documents may use older `game/` and `mods/` paths. The
> current runtime contract is [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).

## Selection model

At startup the port scans only:

```text
ux0:data/DBTapBattle/profiles/
```

Each first-level directory becomes one selector row. No profile type is special.

Example:

```text
profiles/
├── gen/
├── Invasion/
└── DragonBallZuperSamuGamerYT/
```

The selector shows those three folders and nothing else. A folder named
`Original` is just another profile and appears only if it actually exists.

## Standalone profile behavior

If `Invasion` is selected and the original engine requests `effect.pac`, the
only valid source is:

```text
ux0:data/DBTapBattle/profiles/Invasion/effect.pac
```

Missing files are compatibility errors. There is no cross-profile fallback and
no entry-level PAC merge.

## Profile names

The Windows extractor derives the profile folder from the APK filename.
Codec/layout detection does not rename it.

Users may rename the final profile directory to change the label shown in the
Vita selector. PAC files do not need to be edited.

## Save isolation

Each profile owns:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

The VPK's read-only `app0:/save.bin` is copied only when that profile has no
save yet. Existing saves are never replaced merely by switching profile or
updating the VPK. APK-local saves are not installed automatically.

## Compatibility tiers

### Tier A — resource replacement

PAC, texture, audio or data replacements using formats already understood by the
port/original engine.

**Target:** direct compatibility.

### Tier B — added characters/cards/data using original formats

Requires the preserved original engine/adapters to accept the installed
indexes/counts/loading behavior.

The current roster audit supports contiguous complete
`charXX/chardemoXX/charfXXXX` triplets in `00..99`, rejects incomplete/gapped
extensions, and has hardware evidence for the tested Invasion 22-character and
Samu 92-character paths.

### Tier C — APK/Dalvik code modifications

A mod may change `classes.dex` mechanics, limits, menus or modes. The Vita port
does not dynamically execute that changed Android bytecode.

**Target:** reuse compatible assets/data and explicitly port only evidenced
behavioral differences when necessary.

### Tier D — protected/private formats

Protected Android14-family APKs require an audited codec/alias profile. Unknown
constants are rejected rather than guessed.

Audited protected families currently include Android14, Spanish Android14 and
Invasion Beta 3.

## Audio compatibility

Filename extension is not treated as codec proof. The Vita path detects actual
media content. Ordinary Vorbis uses libvorbisfile; supported MP3/AAC paths use
Vita decoding. This is required for the mislabeled BGM found in Invasion and
Samu.

## Windows import route

For normal users, use:

```text
tools/windows/Extract_APK_for_Vita.bat
```

Extractor 1.5 outputs:

```text
data/DBTapBattle/profiles/<sanitized APK filename>/
```

It validates the APK/ZIP, CRCs, safe paths, collisions, `common.pac`,
protected PAC structure where applicable, hashes and roster structure. It does
not install DEX/native Android code as Vita runtime code.

The older Python tools remain engineering/forensic helpers for pinned historical
tests. Their old `--mod`/fixture examples are not the current user-facing Vita
installation layout unless explicitly updated to `profiles/`.

## Current evidence

- 00.33: hardware-confirmed repeated-fight protected-PAC ownership fix.
- Recent hardware sequence: Loading recovery, dynamic rosters including Samu 92.
- 00.34: current user-test candidate adds unified `profiles/`, real-only
  selector entries, fullscreen selector background handling and themed
  profile-opening transition. Physical confirmation is pending.

## New-mod evidence checklist

Record:

- APK/source hash;
- profile folder/output manifest;
- source layout and PAC codec;
- standalone completeness;
- character triplet/count bounds;
- text charset behavior;
- actual BGM codecs;
- whether DEX logic differs;
- profile save continuity/isolation;
- first/repeated selection, battle/results and longer-session behavior.

See [APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md),
[COMMUNITY_MOD_PROFILES](COMMUNITY_MOD_PROFILES.md),
[WINDOWS_DATA_TOOL](WINDOWS_DATA_TOOL.md) and
[CURRENT_STATUS](CURRENT_STATUS.md).
