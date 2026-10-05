#!/usr/bin/env python3
"""Synchronize every Markdown document to the hardware-validated 00.23 checkpoint.

This is intentionally additive for historical files: old failures remain evidence,
while a common checkpoint block makes the current state unambiguous everywhere.
"""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
DATE = "2026-10-05"
VERSION = "00.23"
SOURCE = "0e17b0bac33c47698b414b67a839c839f0e555ce"
VPK_SHA256 = "8dd286423b09abb1ce11d82b314bd0e89a5a728f31e4226ba3207b1054b584dd"

CHECKPOINT = f"""<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — {VERSION} ({DATE}):** build `{VERSION}` from source
> commit `{SOURCE[:8]}` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> {VERSION} the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
"""

DETAILS = {
    "README.md": f"""## PS Vita hardware status — {VERSION}

The current validated checkpoint is **{VERSION}**. A real-hardware test on {DATE}
reported the game working normally with no error observed in that session: the
previously repaired audio remained clean, character selection remained responsive,
and the game successfully entered and played a fight instead of crashing during
character PAC loading. The tested VPK SHA-256 is `{VPK_SHA256}` and its source
checkpoint is `{SOURCE}`.

The 00.22 battle-start crash remains documented as a historical failure. 00.23 fixes
that specific regression by preserving the original streaming `GameData.Init`
parser and replacing only Android resource opening with a Vita-backed native stream,
avoiding the multi-megabyte managed bridge allocation that exhausted TeaVM memory.

""",
    "docs/CURRENT_STATUS.md": f"""## Authoritative hardware checkpoint — {VERSION} ({DATE})

**Status:** hardware validated for the tested path; no error observed in the user's
00.23 session.

Verified together on a real PS Vita in the current progression:

- startup and menu flow continue to work;
- text remains visible;
- audio/voices remain clean (the prior rasp/worker issue did not regress);
- character selection remains responsive (the prior 1–2 second selection stalls did
  not return in the reported session);
- starting a fight now succeeds;
- the fight can be played without the 00.22 battle-start crash.

The specific 00.22 failure was a TeaVM managed-allocation abort while bridging the
entire `char00.pac` (~4.74 MiB) into one Java `byte[]` before the original parser's
disposal order could free the prior owner. 00.23 restores the original streaming
parser contract and exposes the PAC through a Vita native `InputStream` bridge, so
Java receives individual original payload arrays instead of one whole-PAC bridge
array.

This is a **checkpoint, not an exhaustive certification**: every character, mode,
mod dataset, repeated battle sequence and long-duration memory behavior still need
broader regression coverage before a final release claim.

Test artifact SHA-256: `{VPK_SHA256}`  
Source checkpoint: `{SOURCE}`

""",
    "docs/ATTEMPTS.md": f"""## {DATE} — 00.23 real-hardware battle-memory retest — SUCCESS

**Input:** the 00.23 battle-memory test VPK built from `{SOURCE}`.  
**Reason:** 00.22 reached the character selector with clean audio and responsive
switching, but aborted when starting a fight while allocating a full ~4.74 MiB PAC
bridge array in TeaVM managed memory.

**Change under test:** restore the original streaming `GameData.Init` path; replace
only Android resource opening with `NativeResourceStream`; keep original entry
parsing, filters, conversion, `Dispose` ordering and close/finally behavior.

**Hardware result:** user reports everything exercised in this session working as
expected and no error found. Audio remained correct, character selection remained
responsive, battle startup succeeded and gameplay proceeded normally.

**Outcome:** accepted as the current 00.23 hardware checkpoint. Continue regression
coverage rather than reopening the removed whole-PAC bridge design.

""",
    "docs/SUCCESSES.md": f"""## {DATE} — 00.23: first reported session with battle startup working after the memory crash

A physical Vita test of 00.23 completed the path that failed in 00.22. Clean audio
and responsive character selection were preserved, the fight started successfully,
and the user reported no error during the tested session.

The key reusable success is architectural: the port no longer copies an entire PAC
into a TeaVM-managed `byte[]` merely to feed code that already had an original
streaming parser. The Vita adapter now supplies a native-backed `InputStream` while
preserving original parsing and resource-lifetime semantics.

Test VPK SHA-256: `{VPK_SHA256}`.

""",
    "docs/FAILURES.md": f"""## Resolution note — 00.22 battle-start managed-memory abort resolved by 00.23

The 00.22 failure is retained above as evidence. Its observed trigger was the
whole-PAC managed bridge allocation (`char00.pac`, 4,739,319 bytes) during battle
startup. 00.23 removed that bridge path and restored the original streaming parser
with native stream ownership.

**Real-hardware retest ({DATE}):** battle startup and gameplay succeeded; the user
reported no error in the tested session. Treat the 00.22 failure as **resolved for
this reproduced path**, while keeping broader repeated-battle/long-session memory
regression testing open.

""",
    "docs/DECISIONS.md": f"""## Decision — preserve original streaming PAC parsing across the Vita boundary (00.23)

**Decision:** do not bridge full multi-megabyte character PAC files into one managed
TeaVM array when the original engine already provides a streaming parser. Patch only
the Android-specific stream-opening expressions and back them with a native Vita
resource stream whose handle owns/pins the resource for the stream lifetime.

**Why:** 00.22 demonstrated that a correct parser can still fail on constrained
hardware if an adapter changes its allocation topology. The full `char00.pac`
bridge added a ~4.74 MiB managed allocation at exactly the battle transition. The
00.23 hardware retest succeeded after that allocation was removed.

**Constraint:** keep original entry decoding, filters, conversion, `Dispose`
ordering, exceptions/finally and close behavior unless a separately evidenced Vita
incompatibility requires a narrower adaptation.

""",
    "docs/ENGINE_MAP.md": f"""## 00.23 GameData/PAC runtime path

Current battle resource flow:

`TCBManajer.SetLoad/Game1` → original `GameData.Init(...)` streaming parser →
`ResourceAdapter.open(...)` → `NativeResourceStream` → native resource cache/VFS.

The native stream pins its resource owner until close and supports ranged reads.
The Java side sees the original parser's per-entry allocations rather than a whole
PAC bridge array. This path is hardware-validated through successful battle startup
in the reported 00.23 session.

""",
    "docs/PLATFORM_SERVICES.md": f"""## Native resource streams — hardware checkpoint 00.23

The Vita platform layer now exposes open/size/read/close operations used by
`NativeResourceStream`. Stream handles pin their native resource owner, remain valid
across cache activity, reject invalid ranges, and are closed idempotently. This
service exists to preserve the original engine's streaming PAC parser without
copying the full archive into TeaVM-managed memory.

The design fixed the reproduced 00.22 battle-start allocation failure on real
hardware in the 00.23 test session.

""",
    "docs/PAC_FORMAT.md": f"""## Vita loading contract validated in 00.23

PAC bytes and entry semantics are unchanged. The important port-side rule is to
preserve the original streaming parser for large character archives. Vita provides
a native-backed `InputStream`; it does not repack the PAC and does not materialize
the entire archive as a Java bridge array. This allocation-shape correction is what
allowed the reproduced battle-start path to pass on hardware in 00.23.

""",
    "docs/RESOURCE_FORMATS.md": f"""## 00.23 resource-loading note

Resource **formats** did not change in the battle-memory fix. The corrected layer is
transport/lifetime: character PACs are exposed to the original Java parser as a
stream backed by native Vita storage/cache ownership. Keep this distinction in
future ports: format conversion and bridge allocation strategy are separate concerns.

""",
    "docs/PORTING_GUIDE.md": f"""## Reusable lesson from 00.22 → 00.23: preserve streaming allocation topology

On memory-constrained targets, an adapter can break a working original parser even
when it returns byte-identical data. The failed 00.22 adapter first allocated an
entire ~4.74 MiB PAC in TeaVM managed memory; the original method then performed its
own normal work. That additional peak was enough to abort at battle startup.

The successful 00.23 approach patches only platform-specific opening and leaves the
original stream parser/lifetime structure intact. For future ports, prefer a native
stream/handle bridge over whole-file managed copies for large resources, and test
allocation **shape and timing**, not only total file size or final decoded content.

""",
    "docs/PORTING_PLAN.md": f"""## Milestone update — 00.23 battle-start blocker closed for the reproduced path

The immediate blocker carried from 00.22 (managed OOM while starting a fight) is now
closed by the streaming PAC bridge and a successful physical-Vita retest. The next
phase is regression breadth rather than another rewrite: repeated battles, more
characters, both supported datasets/mod paths, longer sessions, control adaptation
and release-quality performance/build reproducibility.

""",
    "docs/BUILD.md": f"""## 00.23 hardware-tested package note

The source checkpoint `{SOURCE}` introduces the streaming PAC bridge used by the
successful 00.23 hardware test. The tested package is
`DBTapBattle-Vita-00.23-battle-memory-test.vpk`, SHA-256 `{VPK_SHA256}`.

That package validated the runtime repair on hardware. It should be distinguished
from a future release-quality reproducible package: the interactive build session
used split compilation of TeaVM generated C to fit the build runner, with the large
`TCBManajer.c` translation unit compiled at `-O0` while the rest retained the normal
build settings. Do not infer final performance characteristics from that packaging
exception. The source/runtime fix itself is the 00.23 checkpoint.

For installation, update the VPK without deleting `ux0:data/DBTapBattle/` or saves.

""",
    "docs/VALIDATION.md": f"""## Latest hardware validation — 00.23

See [TEST_VITA_00_23](TEST_VITA_00_23.md). The acceptance path that failed in 00.22
now passes on a physical Vita: audio remains clean, character switching remains
responsive, battle startup succeeds and gameplay proceeds without an error observed
in the reported session.

This does not close exhaustive regression. Keep exact VPK/source hashes and extend
testing to repeated battles, additional characters, datasets/mods and long sessions.

""",
    "tools/aot/README.md": f"""## 00.23 AOT checkpoint

The full-engine AOT path now preserves the original streaming `GameData.Init`
resource parser and supplies Vita resources through `NativeResourceStream`. This
avoids a full PAC `byte[]` bridge allocation in the TeaVM heap. The resulting 00.23
runtime path passed the previously crashing battle-start transition on real hardware.

""",
    "tools/aot/engine/README.md": f"""## 00.23 engine-generation checkpoint

`PatchResourceInit` must keep the original streaming loader structure and replace
only Android-specific resource-opening expressions. `ResourceAdapter.open` returns a
`NativeResourceStream` backed by native open/size/read/close imports. Do not restore
the former whole-PAC `byte[]` shortcut: it caused the reproduced 00.22 battle-start
managed-memory abort and 00.23 hardware testing validates the streaming repair.

""",
}

TEST_0023 = f"""# PS Vita hardware test — 00.23

Date: {DATE}  
Source checkpoint: `{SOURCE}`  
Test VPK: `DBTapBattle-Vita-00.23-battle-memory-test.vpk`  
VPK SHA-256: `{VPK_SHA256}`

## Purpose

Retest the exact progression that failed in 00.22 after replacing the whole-PAC
TeaVM bridge with the original streaming `GameData.Init` parser backed by a native
Vita resource stream.

## Previous failure being retested

00.22 already preserved the earlier audio and character-selection improvements, but
starting a fight aborted while creating a 4,739,319-byte managed bridge array for
`char00.pac`. The stack reached `NativePlatform.readGameData` /
`ResourceAdapter.load` / `GameData.Init` / `TCBManajer.SetLoad` / `Game1` and the
runtime reported lack of free memory.

## 00.23 change

- preserve the original streaming parser and its `Dispose` ordering;
- patch only Android resource-opening expressions;
- provide a native-backed `NativeResourceStream` with explicit ownership/close;
- read original PAC entries incrementally instead of first copying the whole PAC to
  one TeaVM-managed `byte[]`;
- keep the already accepted audio, text, texture/resource cache and selection-filter
  fixes.

## Physical Vita result

**PASS for the tested session.** The user reported that everything exercised was
working as expected and no error was found at this point. In particular:

- audio/voices remained correct, without the prior rasp;
- character selection remained responsive;
- the game no longer crashed when starting a fight;
- battle startup and gameplay worked normally during the reported test.

No new `runtime.log`/`psp2core` failure artifact was supplied because no failure was
observed in this session.

## Interpretation

00.23 is the current hardware checkpoint and resolves the reproduced 00.22
battle-start managed-allocation regression. It is not an exhaustive certification
of every character, mode, mod, repeated-battle sequence or long-duration memory
behavior. Those remain regression work for later checkpoints.
"""

EVIDENCE = {
    "version": VERSION,
    "date": DATE,
    "source_commit": SOURCE,
    "test_vpk": "DBTapBattle-Vita-00.23-battle-memory-test.vpk",
    "test_vpk_sha256": VPK_SHA256,
    "device": "physical PS Vita",
    "result": "PASS for reported test session",
    "user_report": {
        "overall": "everything exercised working as expected; no error found at this point",
        "audio_voices": "working correctly; prior rasp remains resolved",
        "character_selection": "working correctly; prior stalls remain resolved",
        "battle_start": "succeeds; 00.22 crash not reproduced",
        "battle_gameplay": "working normally in reported session"
    },
    "resolved_regression": {
        "version": "00.22",
        "resource": "char00.pac",
        "whole_managed_bridge_bytes": 4739319,
        "classification": "TeaVM managed allocation failure at battle transition",
        "repair": "restore original streaming GameData parser with native-backed InputStream"
    },
    "failure_artifacts": "none supplied for 00.23 because no failure was observed",
    "scope_limit": "not exhaustive across every character, mode, mod, repeated battle or long session"
}

START = "<!-- DBTB_CURRENT_CHECKPOINT:START -->"
END = "<!-- DBTB_CURRENT_CHECKPOINT:END -->"
DETAIL_START = "<!-- DBTB_00_23_DETAIL:START -->"
DETAIL_END = "<!-- DBTB_00_23_DETAIL:END -->"


def remove_block(text: str, start: str, end: str) -> str:
    while start in text:
        a = text.index(start)
        b = text.find(end, a)
        if b < 0:
            text = text[:a].rstrip() + "\n"
            break
        b += len(end)
        text = text[:a].rstrip() + "\n\n" + text[b:].lstrip("\n")
    return text


def append_block(path: Path, detail: str = "") -> None:
    text = path.read_text(encoding="utf-8")
    text = remove_block(text, START, END)
    text = remove_block(text, DETAIL_START, DETAIL_END)
    if detail:
        text = text.rstrip() + "\n\n" + DETAIL_START + "\n" + detail.rstrip() + "\n" + DETAIL_END + "\n"
    text = text.rstrip() + "\n\n" + CHECKPOINT.rstrip() + "\n"
    path.write_text(text, encoding="utf-8")


def insert_after_title(path: Path, detail: str) -> None:
    text = path.read_text(encoding="utf-8")
    text = remove_block(text, START, END)
    text = remove_block(text, DETAIL_START, DETAIL_END)
    lines = text.splitlines(True)
    pos = 0
    for i, line in enumerate(lines):
        if line.startswith("# "):
            pos = i + 1
            break
    block = "\n" + DETAIL_START + "\n" + detail.rstrip() + "\n" + DETAIL_END + "\n\n"
    text = "".join(lines[:pos]) + block + "".join(lines[pos:])
    text = text.rstrip() + "\n\n" + CHECKPOINT.rstrip() + "\n"
    path.write_text(text, encoding="utf-8")


# First create the dedicated 00.23 test/evidence records.
(ROOT / "docs" / "TEST_VITA_00_23.md").write_text(TEST_0023, encoding="utf-8")
(ROOT / "docs" / "evidence" / "vita_hardware_full_game_00.23.json").write_text(
    json.dumps(EVIDENCE, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
)

# Every Markdown file in the repository receives the current checkpoint block.
# Key living documents also receive a topic-specific 00.23 section.
for path in sorted(ROOT.rglob("*.md")):
    rel = path.relative_to(ROOT).as_posix()
    detail = DETAILS.get(rel, "")
    if rel in {"README.md", "docs/CURRENT_STATUS.md"} and detail:
        insert_after_title(path, detail)
    else:
        append_block(path, detail)

# Make the prior test explicitly point forward without rewriting its historical body.
old = ROOT / "docs" / "TEST_VITA_00_22.md"
if old.exists():
    text = old.read_text(encoding="utf-8")
    marker = "<!-- DBTB_00_22_RESOLUTION:START -->"
    end = "<!-- DBTB_00_22_RESOLUTION:END -->"
    text = remove_block(text, marker, end)
    resolution = f"""{marker}
## Resolution in 00.23

The battle-start memory failure recorded by this 00.22 test was retested after
restoring the original streaming PAC parser. The physical-Vita 00.23 session passed
the previously failing transition and gameplay proceeded normally with no error
observed. See [TEST_VITA_00_23](TEST_VITA_00_23.md).
{end}
"""
    # Keep the common checkpoint last.
    text = remove_block(text, START, END).rstrip() + "\n\n" + resolution + "\n" + CHECKPOINT
    old.write_text(text, encoding="utf-8")

print(f"Updated {len(list(ROOT.rglob('*.md')))} Markdown files to checkpoint {VERSION}")
