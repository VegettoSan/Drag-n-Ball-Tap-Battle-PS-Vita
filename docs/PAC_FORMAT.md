# PAC Container Format

Status: ordinary outer format validated against original and Gen APKs; the
pinned Community14 encoded variant is supported per file. Current runtime is
full engine 00.22; physical selection recovery is pending after 00.21 rejects
original filter 187. Audio/menu recovery is confirmed in 00.21. See
[CURRENT_STATUS](CURRENT_STATUS.md).

This document describes the outer `.pac` container. Internal formats such as `spr`, `act`, `cnv`, `dac`, `gdt`, etc. require their own reverse-engineering notes.

## Header

The file begins with a little-endian 16-bit entry count:

```c
uint16_t entry_count;
```

Immediately after it are `entry_count` table entries, each exactly 16 bytes:

```c
struct PacEntryDisk {
    uint32_t offset;     // relative to start of PAC data block
    uint32_t size;       // resource length in bytes
    char     type[4];    // e.g. "png\0", "spr\0", "act\0"
    uint32_t reserved;   // observed as 0 in validated files
};
```

Therefore:

```text
data_base = 2 + entry_count * 16
absolute_resource_offset = data_base + entry.offset
```

These ordinary outer-directory numeric fields are little-endian. Internal
CNV, text, animation and save schemas have their own signed/endian contracts.

## Validation example: `back00.pac`

The supplied APK contains `res/raw/back00.pac`.

Observed:

```text
entry_count = 8
data_base    = 130 bytes
```

Validated table:

| # | Offset | Size | Type |
|---:|------:|-----:|------|
| 0 | 0 | 73589 | png |
| 1 | 73589 | 10074 | png |
| 2 | 83663 | 768 | act |
| 3 | 84431 | 4070 | cnv |
| 4 | 88501 | 50 | dac |
| 5 | 88551 | 1201763 | spr |
| 6 | 1290314 | 35 | bin |
| 7 | 1290349 | 220 | gdt |

The first resource at byte 130 begins with the standard PNG signature:

```text
89 50 4E 47 0D 0A 1A 0A
```

This confirms that offsets are relative to the PAC data block, not to the beginning of the file.

## Types already observed in the original APK

Examples include:

```text
png
spr
act
cnv
dac
gdt
bin
bmp
dat
plt
db
```

Not every PAC contains every type.

## Parser validation rules

A safe reader must reject a PAC when:

- `data_base` exceeds file size.
- `offset + size` overflows.
- `data_base + offset + size` exceeds file size.
- the table cannot be read completely.

Do not assume resource types are null-terminated; always treat the field as exactly four bytes.

## Mod compatibility implication

Because the container is simple and self-contained, the Vita port should read community PAC replacements directly instead of unpacking/repacking them into a Vita-specific container.

## Independent corpus validation — 2026-10-04

All 19 bundled PACs, across common/effect/font/gamedata/select/text/back/bobj/
card/demo families, were checked. Six nested SPR containers also obey the outer
PAC layout. Outer counts are 3–14, all reserved fields zero, contiguous payloads,
no overlap and no tail. There is no alignment requirement (e.g. PNG offsets may
be odd). ZIP compression is independent of PAC; offsets refer to uncompressed
PAC bytes. No alternate/empty original PAC version was observed.

The native reader was compiled on host and Vita. Every entry of all 19 originals
was read on host. It handles four-byte non-terminated tags, rejects truncated/
out-of-bounds tables, exposes no partial entry list after a failed open, checks
backing length before reads, and applies a default 16-MiB per-entry read budget.
Caller may select a different budget; this is a resource policy, not a discovered
original-format limit. Files outside signed 32-bit seek range are unsupported.
Unknown tags/reserved values are retained for future mods, not rejected merely
because they differ from this corpus. Zero-count synthetic PACs parse safely.
Subsequent native support includes the pinned Community14 variant and earlier
physical menu/selection/combat; this does not certify every payload schema/mod.

## Encoded Android14 profile

The ordinary schema above remains the original contract. The supplied community
APK adds `community14-a210795b`: XOR-coded count/offset/size/type and BE-coded
image dimensions, raw DEFLATE premultiplied RGBA. Exact constants/formulas and
provenance are in ANDROID14_APK.md and tools/community14.py. Native PacFile
supports explicit Original/Community14 and defaults to per-file detection.
Auto looks for a high-bit count compatible with the pinned XOR table plus a
known decoded type ID; malformed identified tables fail without switching to
another codec. Unknown metadata retains its numeric type/reserved fields and
is never falsely labeled PNG. A caller with a deliberately ambiguous/unusual
container can specify the encoding; other private profiles are unsupported.

SPR tables restart entry indexes and use the same encoding. Host corpus tests
read both variants and all nested SPRs. RGBA decoding checks dimensions, decoded
allocation budget, exact output size and complete DEFLATE termination. Files
stay unchanged; images are decoded in memory and alpha state stays attached to
RgbaImage so ordinary PNG and premultiplied community data can coexist.

## Original GameData filter and selective reads — 00.20 onward

The original stream loader treats these bits as **exclusions**:

| Type | Exclusion bit |
|---|---:|
| PNG / verified RGBA texture | 1 |
| ACT | 2 |
| BIN | 4 |
| CNV | 8 |
| DAC | 16 |
| SPR | 32 |
| WAV | 64 |

Do not invert the mask as a requested-type set. ResourceAdapter forwards it to
`dbtb_resourceFiltered`; `readEngineResource` reads only allowed payloads and
rebuilds the bridge container in memory, preserving all directory slots/types/
reserved fields/order. Excluded payload bytes are omitted without attempting
their conversion; the original byte-array parser receives the same filter.
SPR nested normalization retains its separate schema/index rules. Source files
are never rewritten. Unknown types are preserved according to the verified
original dispatch/normalization contract, not guessed into a known format.

A single selected-payload file handle avoids per-entry reopen overhead. Plain
non-SPR full reads avoid unnecessary payload rebuilding after directory checks;
Community14 metadata/WAV/table adapters still run when required. The 8 MiB
retained-result LRU keys resolved path + filter and file size/mtime/ctime; live
shared/Java copies and temporary normalization memory lie outside that budget.

Host tests on 26 character PACs and filters 1/33/64/127/187/251 plus high/sign-bit edges compare exact selected
bytes and stable directories. Filter 33 requested 11,707,264 rather than
91,081,701 source bytes (87.146% reduction). This is not a Vita latency benchmark.
Commands and fixture layout: [VALIDATION](VALIDATION.md). Implementation:
`src/pac.cpp`, `src/engine_resources.cpp`, `src/resource_cache.hpp` and native
`resources.cpp`. Renderer texture and voice caches are separate layers.


## Mask range regression and original call sites — 00.22

The exclusion argument is a signed Java int bitmask, not an enum restricted to
0..127. Original Game3 selection supplies 187 (0xbb): exclude PNG/ACT/CNV/DAC/SPR,
retain BIN/WAV. Other original loading paths supply 251 (0xfb): retain BIN only.
Bit128 is unused by the observed original type tests and must not reject a load.
The adapter retains the exact mask, matching original bit tests, rather than
rewriting the core request or swallowing a null table later.

The 00.20/00.21 native range guard rejected char00 before opening its PAC.
00.21's caught Game3 NullPointerException follows that rejection; see
[evidence](evidence/vita_hardware_selection_00.21.json). The extended regression
fails at char00/filter 187 on the previous source and passes after removing the
guard. It checks exact selected bytes/slots, nonempty metadata, allowed voices,
and actual native resource-copy/cache behavior; physical 00.22 remains pending.

<!-- DBTB_00_23_DETAIL:START -->
## Vita loading contract validated in 00.23

PAC bytes and entry semantics are unchanged. The important port-side rule is to
preserve the original streaming parser for large character archives. Vita provides
a native-backed `InputStream`; it does not repack the PAC and does not materialize
the entire archive as a Java bridge array. This allocation-shape correction is what
allowed the reproduced battle-start path to pass on hardware in 00.23.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
