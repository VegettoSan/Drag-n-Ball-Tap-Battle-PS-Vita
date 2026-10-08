# DragonTap_Util PRIVATE MOD — universal recognition research
_Date: 2026-10-07; source work on main; no new Vita hardware approval._

## Attribution and confidence

The historical community tutorial “Como Funciona El DragonTap_Util 2.6” (SankBlogs, 2020-08-09), https://sank-youtube.blogspot.com/2020/08/como-funciona-el-dragontaputil-2.html, documents `com.neon.dragonhack` output, ordinary “Make MOD.APK”, and **“Make PRIVATE MOD.APK”**, with per-mod package name and extra resource protection. This is a strong candidate for the common source of the protected APKs, but **not definitive attribution** without comparing the original tool executable.

The following are direct observations from the four user-provided protected APKs, not guesses based on name resemblance:

| APK | DEX short SHA | Static resource loader | PAC directories validated | Contiguous characters |
|---|---|---|---:|---:|
| tap battle android 14.apk | `f4e52c47` | `Lext/o;.<clinit>` | 106/106 | 13 |
| DBTB en español para Android 14.apk | `d594affc` | `Lext/o;.<clinit>` | 106/106 | 13 |
| TAP BATTLE INVASION BETA 3.apk | `05aa0c5e` | `Lext/o;.<clinit>` | 139/139 | 22 |
| Dbfz v22.apk | `11d60c43` | `Lext/o;.<clinit>` | 261/261 | 58 |

**612/612 protected PAC outer directories pass** header/table type/bounds checks when decoded using parameters recovered from each APK's own DEX, with **zero hard-coded mod profiles** in the scanner. The four APKs also share an identical arm64 `libabc.so` helper (SHA-256 prefix `7e98a974c49f24b3`). This establishes a common decoding *schema*, not a guarantee that all gameplay alterations are data-only.

## Decisive discovery: aliases AND XOR keys are in classes.dex

For these four APKs, a single Dalvik class initializer (`Lext/o;.<clinit>`) sets:

1. A 17-string array of four-character uppercase hexadecimal aliases (2 slots repeat).
2. A consecutive 17-int static-field block with the per-APK obfuscation keys.

**Array slot contract, confirmed across all four:**

| Array index | Logical name | Filename rule |
|---|---|---|
| 0 | `common` | fixed stem |
| 1 | `select0` | fixed |
| 2, 3 | `back` | same stem + 2 decimal digits |
| 4, 5 | `bobj` | same stem + 2 decimal digits |
| 6 | `char` | + 2 digits |
| 7 | `chardemo` | + 2 digits |
| 8 | `charf` | + 4 digits |
| 9 | `effect` | fixed |
| 10 | `demo_00` | fixed |
| 11 | `demo_08` | fixed |
| 12 | `font00` | fixed (may not appear in APK) |
| 13 | `card_preview` | fixed |
| 14 | `gamedata` | fixed |
| 15 | `text00` | fixed |
| 16 | `card` | + 3 digits |

**17-integer initializer block, index 0–16:**

| Index | Interpretation |
|---|---|
| 0–6 | Encoded type-mask inputs in order `act,bin,cnv,dac,rgba,spr,wav` |
| 7 | `table_position_xor` |
| 8 | `pac_offset_xor` |
| 9 | `pac_size_xor` |
| 10 | Shared type mask, XOR with each integer from slots 0–6 to recover recognized type keys |
| 11 | `pac_count_xor`; also `wav_decoded_size_xor` for this family |
| 12–13 | `image_width_xor`, `image_height_xor` |
| 14 | `table_count_xor` |
| 15–16 | `table_width_xor`, `table_height_xor` |

Existing protected PAC record formula:

```
count  = read_le16(pac, 0) XOR pac_count_xor
base   = 2 + count * 16
offset = read_le32(pac, 2 + entry * 16) XOR pac_offset_xor XOR entry
size   = read_le32(pac, 6 + entry * 16) XOR pac_size_xor XOR entry
type   = read_be32(pac, 10 + entry * 16) XOR entry
```

Each decoded region must be strictly within the PAC. The literal `plt\0` tag is a special case, observed in each of four protected corpora, not an XOR-obfuscated invented code.

### Prior DBFZ alias correction

The source DEX resolves `46C3.pac → common.pac` and `2B98.pac → demo_00.pac`. The earlier guess in the manually registered DBFZ codec accidentally swapped them. Extraction was *structurally* plausible but semantically incorrect; this discovery prompted a correction to all three extractors, regression cases and `DBFZ_V22_APK.md`. Do not trust a renamed archive merely because `common.pac` exists: validate it against the DEX mapping.

## Proposed **one-time** universal integration

The latest *stable* public VPK v1.0 (00.34 gameplay source) must remain untouched. Integrate this as a new, separately versioned candidate.

**Extractor side (Web and Windows, then engineering Python):**

1. Detect ordinary canonical `assets/` or `res/raw/` profiles using the existing safe path and ZIP rules.
2. For unknown protected `assets/*.pac`, inspect `classes.dex` without executing it. Find **exactly one** static initializer meeting the 17-alias + 17-key contract. Never infer keys from random arbitrary binary snippets.
3. Read the aliases and integers from its actual static initializer and construct an ephemeral profile; do not rely on DEX SHA or package name for normal recognition.
4. Validate **every** source PAC table with the derived count/offset/size and known type keys, check collisions after canonical renaming, ensure required resources and contiguous character triplets, verify ZIP CRCs and preserve payload bytes.
5. Write the canonical resource files and, for a protected profile, an optional new **versioned** `dbtb_codec.json` sidecar containing only validated codec parameters and source provenance. Exclude executable DEX/so from Vita data.
6. If initializer differs, metadata ambiguous or any PAC invalid, fail safely with an actionable diagnostic. Do not override security/compatibility checks to accept a file.

**PS Vita side (small, isolated bridge change, never rewrite TeaVM/AOT gameplay):**

1. At profile selection, resolve a safely bounded `dbtb_codec.json` inside *that selected profile only*. Parse a fixed schema (strict numbers, sizes, known strings, no paths or executable code).
2. For existing audited static profiles, preserve the current known decoding behavior. For unknown but validated dynamic profiles, retain one runtime-owned codec struct with exactly the existing PAC/GameData/image/WAV decoder fields.
3. Pass that profile into the PAC header decoder, GameData converted-table decoder, RGBA DEFLATE texture bridge (a `C14U` marker or explicit profile dimensions), and WAV wrapper. Do not alter the original AOT engine's gameplay decisions, counters, controls or screens.
4. Check the resolved codec at resource opening; reject unknown/mixed PACs and resource-overflow cases before allocation. Never silently reinterpret an unsupported protected PAC as ordinary.
5. Keep profile filesystem/save isolation and existing no-fallback policy. Test suspend/resume and switching profiles (do not leave pointers to a previous profile's codec).
6. Compile a **separate test VPK**. Re-test Original, Gen, Android14, Spanish14, Invasion, Samu, DBFZ, character selection, battle, results and extended session on hardware.

This sidecar design is proposed, **not currently implemented**. A Python **read-only proof-of-concept scanner** is provided in the conversation artifact `dragon_tap_universal_probe.py`, together with `dragon_tap_universal_probe_results.json`, so a future implementation can be verified against actual APK corpora without having hard-coded aliases or XOR constants.

## What cannot be promised

“Universal” here means **all APKs matching the audited DragonTap PRIVATE MOD generated-loader layout**, not arbitrary APK cryptography. Some mods may have replaced DEX logic, altered resource formats, different loader versions, incomplete/over-99 character rosters, unsupported media or gameplay changes that a resources-only port cannot execute. Unrecognized variants should produce an explicit unsupported-generator message. The proof-of-concept validates PAC **outer directories**; full data/texture/audio decoding and real hardware playability are separate acceptance stages.

## Validation acceptance gates

- Legacy known-codec/static extractors remain byte-for-byte compatible with previous input sources.
- Pure generator-independent DEX parser extracts all required names and integers correctly from four distinct APKs and validates 612/612 PAC directory structures.
- New Web and Windows extractors produce byte-for-byte matching canonical names/files/manifests for those four fixture APKs.
- Malformed/synthetic DEX, truncated PAC, wrong keys, path traversal, duplicate aliases, huge/empty PACs and mixed profiles reject gracefully.
- New Vita runtime decodes known profiles exactly as before; generic metadata path passes native tests and extended on-device character/battle/selector/suspend checks.
- Never promote to stable v1.0 replacement solely because source tests pass.
