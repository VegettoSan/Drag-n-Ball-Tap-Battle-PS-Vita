# DBFZ v22 APK — protected resource profile (2026-10-07)

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Pinned source/research facts retain their corpus; dated runtime proposals are historical.
<!-- DBTB_DOC_STATUS:END -->

> **Status:** format-level interoperability implemented in source and static APK validation complete. **Not confirmed on PS Vita hardware.** The codec path is included in the v1.1 baseline retained by prepared v1.2, but this exact DBFZ profile still lacks a reported on-console result. Earlier v1.0 artifacts do not gain changes committed after their build.
>
> **Runtime install contract:** `ux0:data/DBTapBattle/profiles/Dbfz_v22/`, from the Windows/Web extractor's sanitized `Dbfz v22.apk` stem. The profile is standalone: no copying from Original or other profiles.

## Provenance (local user-supplied APK only)

| Field | Audited value |
|---|---|
| File | `Dbfz v22.apk` |
| APK SHA-256 | `5fe0b98d45822cc060a95ef8d9bfa069b4005d7897d83c540db160134c4af67f` |
| DEX SHA-256 | `11d60c43184a61743781062ce9260c293cbba948fc097d9c10daff44e778085a` |
| APK bytes | 450,149,820 |
| Resource layout | `assets/` (299 files; 261 protected .pac files) |
| PAC profile ID | `community14-dbfz-11d60c43` |
| PACs structurally audited | 261/261; 7,165 outer directory entries |
| Extra nested SPR | 16 containers |

No APK, binary PAC, DEX, native library, audio, or commercial images should be added to the public repository. Preserve the original APK and raw extracted PAC bytes.

## Diagnosis of extractor rejection

Before this audit, both public extractors only identified three Community14 protected alias/codec profiles. DBFZ uses distinct protected filenames and XOR metadata, so the generated profile had no `common.pac` and would fail the standalone completeness check. The native PAC detector additionally assumed protected first-word counts had bit `0x8000` set; DBFZ does **not**. The pre-existing VPK 1.0 therefore cannot be presumed to recognize it simply because a new extractor successfully writes the files.

Resolution: add a fourth **audited** profile to all extractors and native readers, remove only the over-specific count-bit heuristic, and keep the exact directory bounds/type/unique-profile checks. Do not bypass `common.pac`, insert placeholder resources or merge different installed profiles.

## Codec metadata (hexadecimal)

| Member | Value |
|---|---|
| `count_xor` | `0x39AE` |
| `offset_xor` | `0xAFC6643C` |
| `size_xor` | `0x64CE617B` |
| `image_width_xor` | `0x4B8B` |
| `image_height_xor` | `0xC03A` |
| `table_count_xor` | `0xF00D` |
| `table_position_xor` | `0x4BCC7D9E` |
| `table_width_xor` | `0x5AB5` |
| `table_height_xor` | `0x44F7` |
| `wav_size_xor` | `0x39AE` |

Protected table layout follows `count = le16(source[0:2]) XOR count_xor`, `base = 2 + 16*count`. For entry `i`, `offset = le32(row+0) XOR offset_xor XOR i`, `size = le32(row+4) XOR size_xor XOR i`, `tag = be32(row+8) XOR i`. Validate `base + offset + size <= file_length` without integer overflow. Require a known tag and a unique matching profile.

| Tag | Encoded type key |
|---|---|
| BIN | `0xA4C74FE3` |
| CNV | `0x0E995397` |
| DAC | `0x82F9572B` |
| RGBA | `0x10445923` |
| SPR | `0x04BEE884` |
| WAV | `0x3E602FA3` |
| PLT | **5 literal `plt\0` records in the observed corpus**, not an inferred XOR type key |
| ACT | Not observed; do not invent a type key |

Packed textures use BE16 width and height XOR the two image keys and entry index, followed by raw DEFLATE RGBA data. A normalized texture is tagged with Vita bridge marker `C14D` so the native image bridge invokes the DBFZ decoder. Converted BIN/DAC records use the table keys above and existing validated GameData contracts. Protected WAV uses the decoded-size key; no engine-side audio format invention is necessary.

> **2026-10-07 DEX correction:** the `Lext/o;.<clinit>` resource alias array, independently identified across four protected APKs, proves DBFZ maps `46C3.pac → common.pac` and `2B98.pac → demo_00.pac`. An earlier guess reversed these two; that guess has been corrected in the Web, Windows and Python extractors and regression tests. Source-file presence alone would not have caught the semantic mapping error.
>
> This initializer also stores the per-APK offsets, sizes, texture dimensions and GameData XOR constants. It is feasible to read these parameters programmatically from `classes.dex` and emit per-profile metadata instead of maintaining hard-coded mod families. See `docs/DRAGONTAP_PRIVATE_UNIVERSAL.md` for the proposed universal compatibility layer.

## Filenames — canonical mappings

The Windows and Web extractors must rename only these verified top-level `assets/*.pac` aliases. All file payload bytes and source mappings remain in `dbtb_manifest.json`.

| Logical Vita filename | APK name |
|---|---|
| `common.pac` | `46C3.pac` |
| `select0.pac` | `CC4B.pac` |
| `effect.pac` | `7D98.pac` |
| `demo_00.pac` | `2B98.pac` |
| `demo_08.pac` | `D6E1.pac` |
| `card_preview.pac` | `B727.pac` |
| `gamedata.pac` | `AC9E.pac` |
| `text00.pac` | `D791.pac` |
| `backXX.pac` | `8ED7XX.pac`, `XX=00..13` |
| `bobjXX.pac` | `DC70XX.pac`, `XX=01..14` |
| `charXX.pac` | `128BXX.pac`, `XX=00..57` |
| `chardemoXX.pac` | `CD4AXX.pac`, `XX=00..57` |
| `charfXXXX.pac` | `4BD8XXXX.pac`, `XXXX=0000..0057` |
| `cardXXX.pac` | `FDD0XXX.pac`, `XXX=000..050` |

The 58 character triplets are present and contiguous (`00..57`). `font00.pac` is absent, as in other independently running Community14 variants, so this **must not** be treated as evidence of corruption or substituted from another installed profile.

## Resource and audio audit

- 7,165 outer PAC table entries were checked for directory bounds: 5,608 RGBA, 123 BIN, 261 CNV, 261 DAC, 891 WAV, 16 SPR and 5 literal PLT.
- 16 nested SPR directories were encountered. Of the RGBA entries, 361 sampled image payloads were decompressed and checked for decoded dimensions/byte count.
- 123 protected BIN converted tables were parsed. `gamedata.pac` contains 271 GameData records; `text00.pac` contains 1.
- 36 `.ogg` audio assets consist of 26 actual Ogg/Vorbis files and 10 AAC/M4A files stored with misleading extension. The existing Vita audio bridge sniffs codec bytes; leave payloads untouched. Playback on real Vita remains to be tested.

## Implementation and safety boundaries

| Component | Change |
|---|---|
| `tools/community14.py` | New audited profile |
| `web/extractor-core.mjs` | Web profile detection, canonical paths and PAC CRC/structure checks |
| `tools/windows/Extraer_APK_para_Vita.ps1` | Windows profile, with unsigned-decimal constants for Windows PowerShell 5.1 |
| `src/community_profiles.hpp` | Enum and codec; full bounds/type unique detection without assuming high count bit |
| `src/pac.cpp` | Preserve the observed literal `plt\0` type |
| `src/engine_resources.cpp` | Normalize the DBFZ resource structure, `C14D` marker and palette |
| `tools/aot/engine/native/resources.cpp` | Recognize `C14D` for RGBA bridge |
| `tests/` | Synthetic Python/JavaScript/Windows/C++ profile regressions |

**Do not change** original AOT game rules, other codec constants, profile save isolation, fallback behavior, audio transcoding or the already-released stable VPK based on this static evidence. DBFZ's `classes.dex` is not loaded by the Vita engine; changed Dalvik logic is an independent, presently unverified compatibility tier.

### PS Vita acceptance procedure (new build only)

1. Produce a **new, distinctly versioned** test VPK from the source that includes this profile; never replace the stable v1.0 user artifact.
2. Extract `Dbfz v22.apk` with updated Web or Windows tools. Copy the resulting `data/DBTapBattle/profiles/Dbfz_v22/` under `ux0:`.
3. Confirm the theme selector lists `Dbfz_v22`, then reaches menu without long loading/hang or crash.
4. Confirm all 58 characters are available, character preview and voices are correct, and test battles with early/mid/late roster indices and repeated transitions.
5. Test audio, results screens, save per-profile isolation and suspend/resume.
6. Collect `ux0:data/DBTapBattle/runtime.log` (and any core dump) if issues arise, including source APK hash and precise reproduction.
7. Recheck Original, Gen, Android14, Spanish, Invasion and Samu on the same new candidate before marking full compatibility.

**Evidence levels:** structurally audited = yes; source changes committed = yes; GitHub CI checks = report per current run; on-console gameplay = **pending**.
