# Documentation index and alignment audit — v1.2

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

## Start here

| Purpose | Authoritative page |
|---|---|
| Current implementation, artifact and user-test status | [CURRENT_STATUS](CURRENT_STATUS.md) |
| Runtime/input/data/save contract | [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md) |
| Installation and extraction | [INSTALLATION_AND_EXTRACTION](INSTALLATION_AND_EXTRACTION.md) |
| User button mapping | [VITA_CONTROLS_REFERENCE](VITA_CONTROLS_REFERENCE.md) |
| Exact release file, hash and publication instructions | [RELEASE_v1.2](RELEASE_v1.2.md) |
| Build/probe recipes | [BUILD](BUILD.md), [VALIDATION](VALIDATION.md) |
| Next work and evidence boundaries | [PORTING_PLAN](PORTING_PLAN.md), [AUDIT_STATUS](AUDIT_STATUS.md) |
| Rules for future changes | [PROJECT_RULES](PROJECT_RULES.md), [DECISIONS](DECISIONS.md) |

## Alignment audit — 2026-10-09

Reviewed all **78 pre-existing tracked Markdown files**. This index is the
79th. Current guides now describe prepared v1.2 (`01.02` / `DBTB01178`),
retaining the v1.1 VisualQuality resource/memory baseline and approved Vita
controls. The English launcher offers Vita first (hidden pads), Touch only
second, with per-profile remembered highlight. X attacks/confirms characters;
dialogues use touch. Start/Circle remain as approved in the controls tests.

Corrected obsolete physical-input neutrality, build version/generation counts,
cross-profile resource fallback instructions, extractor compatibility wording,
stable/test save ownership and the future-work checklist. Generic current-status
blocks link to the current contract. Historical version numbers, hashes, corpus
tables and failures are not rewritten as successes. Research mappings before
the L/R swap and dialogue-X retirement remain dated historical evidence.

The exact VPK built from `f6e9adaa792d38c4f3a7c7b27d112ae54b41ede5` is
unchanged by these Markdown edits. Its SHA-256 remains
`343aee505f77fa743339111fa7cf29f1e9bda333e49bddb6be166933d7bac1fc`.
The user is testing it; no new stable-v1.2 hardware result is recorded. The
published GitHub release verified during this audit is `1.1`; `1.2` is ready
for maintainer upload. No build/release workflow is dispatched by this audit.

This is a documentation consistency/link/source review, not a new APK corpus,
runtime or hardware test. The local full-engine/regression checks recorded for
v1.2 are inherited evidence; they are not rerun merely for prose changes.

Local documentation checks passed: **79 Markdown files, 750 local links and
anchors, no missing targets, balanced fenced blocks and no stale shared
current-version notes**. Historical 64-character hashes remain present. Git
whitespace checks pass, and only Markdown source is changed.

## Document inventory

The groups below state how each page should be read. The original audited bytes
and build identities stay authoritative within their recorded scope. Use the
current contract for present-day installation and input behavior.

### Current guides and contracts

- [README.md](../README.md)
- [assets/livearea/README.md](../assets/livearea/README.md)
- [assets/selector/README.md](../assets/selector/README.md)
- [data/README.md](../data/README.md)
- [docs/AUDIT_STATUS.md](AUDIT_STATUS.md)
- [docs/BUILD.md](BUILD.md)
- [docs/COMMUNITY_MOD_PROFILES.md](COMMUNITY_MOD_PROFILES.md)
- [docs/CURRENT_RUNTIME_CONTRACT.md](CURRENT_RUNTIME_CONTRACT.md)
- [docs/CURRENT_STATUS.md](CURRENT_STATUS.md)
- [docs/DATA_LAYOUT.md](DATA_LAYOUT.md)
- [docs/DECISIONS.md](DECISIONS.md)
- [docs/DRAGONTAP_PRIVATE_UNIVERSAL.md](DRAGONTAP_PRIVATE_UNIVERSAL.md)
- [docs/ENGINE_MAP.md](ENGINE_MAP.md)
- [docs/INSTALLATION_AND_EXTRACTION.md](INSTALLATION_AND_EXTRACTION.md)
- [docs/MODS.md](MODS.md)
- [docs/PAC_FORMAT.md](PAC_FORMAT.md)
- [docs/PLATFORM_SERVICES.md](PLATFORM_SERVICES.md)
- [docs/PORTING_GUIDE.md](PORTING_GUIDE.md)
- [docs/PORTING_PLAN.md](PORTING_PLAN.md)
- [docs/PROJECT_RULES.md](PROJECT_RULES.md)
- [docs/RELEASE_WORKFLOWS.md](RELEASE_WORKFLOWS.md)
- [docs/RELEASE_v1.2.md](RELEASE_v1.2.md)
- [docs/RENDER_MAPPING.md](RENDER_MAPPING.md)
- [docs/RESOURCE_FORMATS.md](RESOURCE_FORMATS.md)
- [docs/THIRD_PARTY.md](THIRD_PARTY.md)
- [docs/VALIDATION.md](VALIDATION.md)
- [docs/VITA_CONTROLS_REFERENCE.md](VITA_CONTROLS_REFERENCE.md)
- [docs/VITA_MENU_SHORTCUTS_2026-10-09.md](VITA_MENU_SHORTCUTS_2026-10-09.md)
- [docs/WEB_DATA_TOOL.md](WEB_DATA_TOOL.md)
- [docs/WINDOWS_DATA_TOOL.md](WINDOWS_DATA_TOOL.md)
- [tools/aot/engine/README.md](../tools/aot/engine/README.md)

### Pinned audits and research

- [docs/ANDROID14_APK.md](ANDROID14_APK.md)
- [docs/APK_AUDIT.md](APK_AUDIT.md)
- [docs/APK_CANONICAL_DIFFERENCES.md](APK_CANONICAL_DIFFERENCES.md)
- [docs/APK_TECHNICAL_REFERENCE.md](APK_TECHNICAL_REFERENCE.md)
- [docs/DBFZ_V22_APK.md](DBFZ_V22_APK.md)
- [docs/DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md](DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md)
- [docs/DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md)
- [docs/INVASION_BETA3_APK.md](INVASION_BETA3_APK.md)
- [docs/ORIGINAL_PLUS_CHARACTERS_APK.md](ORIGINAL_PLUS_CHARACTERS_APK.md)
- [docs/SPANISH_ANDROID14_APK.md](SPANISH_ANDROID14_APK.md)
- [docs/VITA_CONTROLS_RESEARCH_2026-10-08.md](VITA_CONTROLS_RESEARCH_2026-10-08.md)
- [docs/VITA_PAD_VISIBILITY_RESEARCH_2026-10-08.md](VITA_PAD_VISIBILITY_RESEARCH_2026-10-08.md)
- [docs/evidence/APK_AUDIO_MATRIX_2026-10-06.md](evidence/APK_AUDIO_MATRIX_2026-10-06.md)
- [docs/evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART1.md](evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART1.md)
- [docs/evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART2.md](evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART2.md)
- [docs/evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART3.md](evidence/APK_LOGICAL_FILE_MATRIX_2026-10-06_PART3.md)
- [tools/aot/README.md](../tools/aot/README.md)

### Chronological ledgers

- [docs/ATTEMPTS.md](ATTEMPTS.md)
- [docs/FAILURES.md](FAILURES.md)
- [docs/SUCCESSES.md](SUCCESSES.md)

### Historical build sheets

- [docs/RELEASE_v1.1.md](RELEASE_v1.1.md)
- [docs/TEST_FULL_ENGINE_00_03.md](TEST_FULL_ENGINE_00_03.md)
- [docs/TEST_VITA_00_13.md](TEST_VITA_00_13.md)
- [docs/TEST_VITA_00_18.md](TEST_VITA_00_18.md)
- [docs/TEST_VITA_00_19.md](TEST_VITA_00_19.md)
- [docs/TEST_VITA_00_20.md](TEST_VITA_00_20.md)
- [docs/TEST_VITA_00_21.md](TEST_VITA_00_21.md)
- [docs/TEST_VITA_00_22.md](TEST_VITA_00_22.md)
- [docs/TEST_VITA_00_23.md](TEST_VITA_00_23.md)
- [docs/TEST_VITA_00_23_LIVEAREA.md](TEST_VITA_00_23_LIVEAREA.md)
- [docs/TEST_VITA_00_23_LIVEAREA_FIXED.md](TEST_VITA_00_23_LIVEAREA_FIXED.md)
- [docs/TEST_VITA_00_24.md](TEST_VITA_00_24.md)
- [docs/TEST_VITA_00_25.md](TEST_VITA_00_25.md)
- [docs/TEST_VITA_00_26.md](TEST_VITA_00_26.md)
- [docs/TEST_VITA_00_27.md](TEST_VITA_00_27.md)
- [docs/TEST_VITA_00_28.md](TEST_VITA_00_28.md)
- [docs/TEST_VITA_00_29.md](TEST_VITA_00_29.md)
- [docs/TEST_VITA_00_30.md](TEST_VITA_00_30.md)
- [docs/TEST_VITA_00_31.md](TEST_VITA_00_31.md)
- [docs/TEST_VITA_00_32.md](TEST_VITA_00_32.md)
- [docs/TEST_VITA_00_33.md](TEST_VITA_00_33.md)
- [docs/TEST_VITA_00_34.md](TEST_VITA_00_34.md)
- [docs/TEST_VITA_CONTROLS.md](TEST_VITA_CONTROLS.md)
- [docs/TEST_VITA_CONTROLS_2.md](TEST_VITA_CONTROLS_2.md)
- [docs/TEST_VITA_CONTROLS_3.md](TEST_VITA_CONTROLS_3.md)
- [docs/TEST_VITA_CONTROLS_4.md](TEST_VITA_CONTROLS_4.md)
- [docs/TEST_VITA_CONTROLS_5.md](TEST_VITA_CONTROLS_5.md)
