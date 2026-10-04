# Confirmed Successes

Only add items here when they are demonstrated by evidence. Distinguish PC-side format validation from behavior confirmed on real Vita hardware.

## 2026-10-04 — Original APK is suitable for source-level reconstruction

**Scope:** confirmed from supplied APK inspection.

- No `lib/*.so` native game libraries were found in the APK.
- The game logic is primarily Java/Dalvik rather than a closed native ARM game binary.
- Original game resources are directly accessible in the APK under `res/raw/`.
- Original audio includes OGG resources.

**Why this matters:** the port can focus on reconstructing platform/game layers in native Vita code instead of binary translation of an Android native executable.

## 2026-10-04 — PAC outer container layout validated

**Scope:** confirmed on PC against the supplied original APK.

- Entry count is a little-endian `uint16_t`.
- Each table record is 16 bytes: `uint32 offset`, `uint32 size`, `char type[4]`, `uint32 reserved`.
- Entry offsets are relative to the data block at `2 + count * 16`.
- `back00.pac` entry 0 resolves exactly to a PNG signature.
- All eight `back00.pac` entries resolve inside file bounds.

**Why this matters:** original PAC files can be consumed directly by the Vita port and community PAC replacements can potentially remain unchanged.

## 2026-10-04 — Original runtime data can be extracted without conversion

**Scope:** FORMAT CONFIRMED against the supplied APK.

- 57 regular files were found under `res/raw/` and extracted byte-for-byte.
- The dataset contains 19 `.pac` files and 36 `.ogg` files plus auxiliary resources.
- Per-file SHA-256 hashing works and can be written to a manifest.
- Example validated hash: `back00.pac` = `a19c425b0496aadc780d3a12b47fad91363423c9b5944407bdd5f74d64f18012`.

**Why this matters:** users can prepare `ux0:data/DBTapBattle/game/` directly from their own APK without repacking or changing the original resources.

## Success levels used by this project

- **FORMAT CONFIRMED** — validated against original files on PC.
- **BUILD CONFIRMED** — compiles/links successfully with VitaSDK.
- **VITA3K CONFIRMED** — observed working in Vita3K.
- **HARDWARE CONFIRMED** — observed working on a real PS Vita.

Whenever possible, promote discoveries through these levels rather than assuming PC-side success guarantees Vita behavior.
