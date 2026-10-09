# Mod compatibility model — v1.2 with inherited universal mod support

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

> Historical test documents may use older `game/` and `mods/` paths. The
> current runtime contract is [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).
> v1.2 uses APP_VER `01.02` and TITLE_ID `DBTB01178`. It retains the v1.1 VisualQuality loader/memory baseline and adds approved Vita controls. The published 1.1 remains the previous release.

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

Both the Web Extractor 1.0 and Windows Extractor 1.5 derive the profile folder from the APK filename.
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

## Controls and asset-mod compatibility

Every profile offers **PS VITA CONTROLS** first (hidden pads) and **TOUCH ONLY**
second before launch; the highlight is remembered per profile. Input goes through
the original KeyData/Controller boundary. Pad hiding changes only the runtime
DAC view, not PAC files or shared cached bytes. Unknown resource structures can
decline the overlay; nine supplied APKs have structural/corpus evidence, not
universal hardware certification. Custom DEX task/pad changes need an audit.

X attacks and confirms ready characters. Circle goes back in audited original
menus; Start pauses/resumes the main pause screen. Dialogues, tutorial gestures,
other menu choices and Yes/No prompts use front touch. See
[control reference](VITA_CONTROLS_REFERENCE.md) and
[runtime contract](CURRENT_RUNTIME_CONTRACT.md).

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

Audited protected families include Android14, Spanish Android14, Invasion and
DBFZ. For many additional DragonTap PRIVATE variants, the Web and Windows
extractors derive codec aliases/XOR values *from the APK's own DEX initializer*,
validate PAC structure, and write a profile-local `dbtb_codec.json` when required.
The v1.2 Vita bridge retains the v1.1 path and reads this metadata without executing Android code.
Neither a successful decode nor a complete character roster proves that every
customized mod works: DEX logic changes may require separate porting.

### High-resolution sprites / GPU memory (v1.1 limitation retained in v1.2)

Some mods have large sprite atlases/effect assets that can exhaust Vita system/GPU
memory during character selection, fight startup or between fights. The universal
C14U texture bridge uses compact RGBA4444 uploads and an **adaptive memory-aware
quality policy**. It may reduce GPU texture resolution to prevent out-of-memory
crashes. Therefore **some characters, effects, UI elements or backgrounds may
look blurry while others are sharp**. This is a tradeoff intentionally kept in
the hardware-approved v1.1 resource baseline retained in v1.2. The extractor preserves the original PACs.

The policy is generic across compatible protected formats and depends on resource
dimensions and memory usage; **there are no mod-name-specific configuration
files or per-mod quality exceptions**. Older original/audited code paths are not
overwritten. Loading high-resolution mods may take longer than ordinary datasets.

[Hardware and memory investigation](DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md).

## Audio compatibility

Filename extension is not treated as codec proof. The Vita path detects actual
media content. Ordinary Vorbis uses libvorbisfile; supported MP3/AAC paths use
Vita decoding. This is required for the mislabeled BGM found in Invasion and
Samu.

## User extraction routes

For normal users, use either:

- Web Extractor 1.0: https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/
- Windows Extractor 1.5: `tools/windows/Extract_APK_for_Vita.bat`

The web version processes the APK locally in the browser and downloads a
Vita-ready ZIP; the APK is not uploaded. Both extractors output:

```text
data/DBTapBattle/profiles/<sanitized APK filename>/
```

Both validate the APK/ZIP, CRCs, safe paths, collisions, `common.pac`,
protected PAC structure where applicable, hashes and roster structure. Neither
installs DEX/native Android code as Vita runtime code.

The older Python tools remain engineering/forensic helpers for pinned historical
tests. Their old `--mod`/fixture examples are not the current user-facing Vita
installation layout unless explicitly updated to `profiles/`.

## Current evidence

- v1.2: approved physical controls and hidden pads are integrated into main;
  stable package 01.02 / DBTB01178 was freshly built and is being tested.
  Dialogue X was retired; no exact stable-v1.2 hardware result is recorded yet.

- v1.1 VisualQuality: physical Vita accepts repeated fights of `dbz_mobile_v9`, no reproduced prior crash/freeze, though some reduced-resolution textures are blurry. `runtime.log` shows 340 performance windows, 317 of which are 58+ FPS; this is a tested-session result, not a blanket performance promise.

- 00.33: hardware-confirmed repeated-fight protected-PAC ownership fix.
- Recent hardware sequence: Loading recovery, dynamic rosters including Samu 92.
- Historical v1.0: its public release inherited the hardware-confirmed 00.34 runtime and
  uses unified `profiles/`, real-only selector entries, fullscreen selector
  background handling and the themed profile-opening transition. The exact
  00.34 gameplay/runtime baseline was confirmed on physical Vita; a freshly
  repackaged DBTB01178 v1.0 binary should still receive its own install/launch
  sanity check.

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
[WEB_DATA_TOOL](WEB_DATA_TOOL.md),
[WINDOWS_DATA_TOOL](WINDOWS_DATA_TOOL.md) and
[CURRENT_STATUS](CURRENT_STATUS.md).
