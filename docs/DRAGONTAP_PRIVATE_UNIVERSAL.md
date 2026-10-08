# DragonTap_Util PRIVATE MOD — universal recognition research
_Date: 2026-10-07; source work on main; no new Vita hardware approval._

## Attribution and confidence

The historical community tutorial “Como Funciona El DragonTap_Util 2.6” (SankBlogs, 2020-08-09), https://sank-youtube.blogspot.com/2020/08/como-funciona-el-dragontaputil-2.html, documents `com.neon.dragonhack` output, ordinary “Make MOD.APK”, and **“Make PRIVATE MOD.APK”**, with per-mod package name and extra resource protection. This is a strong candidate for the common source of the protected APKs, but **not definitive attribution** without comparing the original tool executable.

The following are direct observations from the five user-provided protected APKs, not guesses based on name resemblance:

| APK | DEX short SHA | Static resource loader | PAC directories validated | Contiguous characters |
|---|---|---|---:|---:|
| tap battle android 14.apk | `f4e52c47` | `Lext/o;.<clinit>` | 106/106 | 13 |
| DBTB en español para Android 14.apk | `d594affc` | `Lext/o;.<clinit>` | 106/106 | 13 |
| TAP BATTLE INVASION BETA 3.apk | `05aa0c5e` | `Lext/o;.<clinit>` | 139/139 | 22 |
| Dbfz v22.apk | `11d60c43` | `Lext/o;.<clinit>` | 261/261 | 58 |
| dbs mobile tap battle v1.apk | `2392d43b` | `Lext/o;.<clinit>` | 117/117 | 14 |

**729/729 protected PAC outer directories pass** header/table type/bounds checks when decoded using parameters recovered from each APK's own DEX, with **zero hard-coded mod profiles** in the scanner. The original four APKs also share an identical arm64 `libabc.so` helper (SHA-256 prefix `7e98a974c49f24b3`). This establishes a common decoding *schema*, not a guarantee that all gameplay alterations are data-only.

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

Each decoded region must be strictly within the PAC. The literal `plt\0` tag is a special case, observed in all five protected corpora, not an XOR-obfuscated invented code.

### Prior DBFZ alias correction

The source DEX resolves `46C3.pac → common.pac` and `2B98.pac → demo_00.pac`. The earlier guess in the manually registered DBFZ codec accidentally swapped them. Extraction was *structurally* plausible but semantically incorrect; this discovery prompted a correction to all three extractors, regression cases and `DBFZ_V22_APK.md`. Do not trust a renamed archive merely because `common.pac` exists: validate it against the DEX mapping.

## Implemented universal bridge (experimental source; hardware pending)

The latest *stable* public VPK v1.0 (00.34 gameplay source) must remain untouched. Integrate this as a new, separately versioned candidate.

**Extractor side (Web and Windows implemented; engineering Python CLI remains separate):**

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

**Now implemented in source for Web, Windows and Vita:** the extractors detect an unseen PRIVATE DEX loader, remap protected names without rewriting PAC contents and emit a bounded `dbtb_codec.json`. Vita loads it per selected profile, resets it on profile change and supplies its keys to the existing PAC/GameData/RGBA/WAV decoders. The existing stable v1.0 release was not rebuilt or overwritten. Hardware/gameplay verification and a separately compiled full-engine VPK are **still pending**. The Python CLI extractor remains on registered codecs. A Python **read-only proof-of-concept scanner** is provided in the conversation artifact `dragon_tap_universal_probe.py`, together with `dragon_tap_universal_probe_results.json`, so a future implementation can be verified against actual APK corpora without having hard-coded aliases or XOR constants.

## What cannot be promised

“Universal” here means **all APKs matching the audited DragonTap PRIVATE MOD generated-loader layout**, not arbitrary APK cryptography. Some mods may have replaced DEX logic, altered resource formats, different loader versions, incomplete/over-99 character rosters, unsupported media or gameplay changes that a resources-only port cannot execute. Unrecognized variants should produce an explicit unsupported-generator message. The proof-of-concept validates PAC **outer directories**; full data/texture/audio decoding and real hardware playability are separate acceptance stages.

## Validation acceptance gates

- Legacy known-codec/static extractors remain byte-for-byte compatible with previous input sources.
- Pure generator-independent DEX parser extracts all required names and integers correctly from five distinct APKs and validates 729/729 PAC directory structures.
- New Web and Windows extractors produce byte-for-byte matching canonical names/files/manifests for those four fixture APKs.
- Malformed/synthetic DEX, truncated PAC, wrong keys, path traversal, duplicate aliases, huge/empty PACs and mixed profiles reject gracefully.
- New Vita runtime decodes known profiles exactly as before; generic metadata path passes native tests and extended on-device character/battle/selector/suspend checks.
- Never promote to stable v1.0 replacement solely because source tests pass.

## Independent fifth-mod test — dbs mobile tap battle v1.apk

A new community APK was supplied after the initial four-mod research, making this an independent holdout test of the schema rather than a fitted dataset. Without pre-registering any DEX hash, alias, or XOR key, the read-only `dragon_tap_universal_probe.py` discovered a new loader `Lext/o;.<clinit>`, DEX SHA-256 `2392d43b74e81f601ecc771ec26453d444fbdecb48fffd4995f4b7b96f796de7`, and automatically resolved all 117 PAC names and record tables (117 valid, 0 invalid, 0 unknown types). Its character triplets cover 00–13 (14 complete), there are 51 cards 000–050, and mandatory common/select0/gamedata/text00 files are present after DEX-guided renaming. Example: `4AF4.pac→common.pac`, `259D.pac→gamedata.pac`, `36D713.pac→char13.pac`. `font00.pac` is absent, as in previously compatible protected variants.

Additional APK ZIP check: 155 non-directory assets (117 PAC, 36 audio with `.ogg` suffix, 1 PNG and 1 BIN), 129,925,365 unpacked asset bytes, no bad ZIP CRCs. Of the 36 `.ogg` files, 26 have an OggS stream header and 10 have an MP4/M4A `ftyp` header; the existing media converter needs to account for misleading extensions. APK SHA-256: `ce00bb66f2d19f5558bdd26bf6f80e8ec6193d20137a7dd0d1deb37066f2fc84`.

**Critical limitation:** source snapshots of the published Windows and Web extractors still hard-code only the first four alias families. Their recognition scores for this fifth APK are **0 for all four profiles**; `assets/common.pac` is absent (its actual name is `assets/4AF4.pac`). Accordingly their default extraction path falls back to generic assets and rejects with `common.pac was not found`. This is *not* a passing production extractor test, merely a passing **universal proof-of-concept** test. Implement DEX-powered extraction and Vita-side runtime codec metadata before promising that new protected mods are immediately playable; do not change stable VPK 1.0.

## Actual implementation and CI evidence

- `web/private-dex.mjs` + `web/extractor-core.mjs`: recognize new 17-alias/17-key DEX generator, statically validate every PAC, emit a per-profile numeric JSON sidecar, preserve ZIP CRC and file size limits. End-to-end synthetic extraction and malformed rejection passed [Pages CI](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37718130522).
- `tools/windows/PrivateModDex.ps1` + `tools/windows/Extraer_APK_para_Vita.ps1`: same detection and sidecar, no external Python. The portable Windows ZIP includes this new helper. All 20 Windows regression tests passed [Windows CI](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37718063263).
- `src/dynamic_codec.hpp` + `src/community_profiles.hpp` + `tools/aot/engine/native/resources.cpp` + `src/engine_resources.cpp`: single opt-in, profile-local codec with strict JSON schema and `C14U` texture marker. Synthetic fifth-mod code path and four existing codecs passed [native host regression tests](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37717841023); native Vita compilation also passed its link-smoke workflow, which uses a **non-playable mock TeaVM core**.
- Do not claim the full original AOT engine or hardware-combat flows were validated by these checks. Build the full VPK using the existing manual prerelease workflow and test with original and all mods on real Vita before promoting a stable version.
