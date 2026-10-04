# PAC Container Format

Status: **validated against the supplied original APK** (`DBTapBattle.apk`).

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

All integer fields observed so far are little-endian.

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
Native runtime on Vita and modified/community PAC variants remain PENDING.

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
