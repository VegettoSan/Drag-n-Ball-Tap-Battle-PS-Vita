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

Requires the reconstructed engine to support the same indexes/counts/loading behavior used by the mod.

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
