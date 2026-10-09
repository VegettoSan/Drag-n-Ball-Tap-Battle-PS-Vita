# Reusable porting guide — lessons from Tap Battle on Vita

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

## v1.2 adapter-only physical controls

The integrated controls translate Vita buttons to the original KeyData pointer
interface; they never write task, combat, pause or controller state. Hidden pads
use a sparse per-stream DAC overlay, preserving source/cache bytes. Per-profile
settings select Vita mode or original touch input before each launch. The stable
build uses `DBTB01178` / `01.02` / `save.bin`.

Host original-script acceptance did not predict dialogue X behavior on hardware.
After Test 5 failed it, the feature was retired at the user's request. Scripts
still guard against stale combat input; real dialogue touches remain intact.
Circle was accepted in Test 5, Start resume earlier, and combat/hidden pads and
character confirmation in preceding tests. See [current contract](CURRENT_RUNTIME_CONTRACT.md),
[controls reference](VITA_CONTROLS_REFERENCE.md) and [release notes](RELEASE_v1.2.md).

Original lessons began at 2026-10-05 / full-engine 00.22 and were extended
through v1.1 and v1.2. This guide distinguishes reusable
methods from game-specific facts. [CURRENT_STATUS](CURRENT_STATUS.md) identifies
what actually worked on hardware. It is not a claim that every Java/Android game
can use this exact pipeline or that Tap Battle is finished.

## 1. Identify the real engine before selecting a loader

Inventory APK archives, native libraries, DEX classes, resource paths and missing
downloaded content; pin the input hash. The original Tap Battle APK has no native
engine .so: its core is Java/Dalvik. The Community14 libabc.so is a resource helper,
not a native replacement engine. This justified original-core AOT here; a game
with a native ARM engine could require a different approach entirely.

Compare alternate APKs independently. Shared helper hashes establish shared
bytes, not publisher identity or unchanged combat mechanics. A generic certificate
is not an author. Preserve unknown fields and state the limits of provenance.

Reusable code: `tools/audit_apk.py`, `tools/compare_apks.py`, extractor path/CRC/
manifest validation. Non-reusable without audit: aliases, XOR constants, class
names, state ranges, character counts and downloaded-package naming.

## 2. Preserve behavior and replace platform boundaries

The original task/Controller/animation/combat code is the authority. The chosen
pipeline compiles original bytecode privately rather than writing approximate
menus or combat. Handwritten Android-facing classes provide resources, rendering,
text, audio, files, time and lifecycle. Unsupported HTTP/Bluetooth fail or remain
disconnected; required services must not pretend to succeed as empty no-ops.

| Boundary | Tap Battle implementation | Porting lesson |
|---|---|---|
| Android resource overload | PatchResourceInit → ResourceAdapter → native PAC bridge | Preserve parser and filter contract while changing I/O |
| GL10/GL11 calls | VitaGles → C ABI → vitaGL | Keep state/draw order, types and memory lifetime |
| Canvas/Paint text | StringTexture → PVF service | Metrics, image coverage and text bounds are separate contracts |
| SoundPool/MediaPlayer/AudioTrack | Native Vorbis/PCM mixer | Recover IDs, rates, channel limits and lifecycle before optimizing |
| Android files/save | GameVfs plus dedicated profile save bridge | Selected-profile read isolation and atomic save write ownership |
| Java timing/background tasks | Patched Vita time/fibers and EventQueue pump | A working render loop may still starve cooperative jobs |

See [ENGINE_MAP](ENGINE_MAP.md), [RENDER_MAPPING](RENDER_MAPPING.md) and
[PLATFORM_SERVICES](PLATFORM_SERVICES.md) for source-to-adapter mappings.

## 3. Make AOT changes narrow and reproducible

Use the original APK hash, pinned dex2jar/ECJ/TeaVM/JDK and fresh private output.
Patch expected method/string/field shapes and fail when they differ. dex2jar
whole-class bytes can vary; blindly comparing a previous generated class hash
is not a reliable semantic validation method. Check required boundaries instead.

The full generator retains original byte-array GameData parsing and task methods.
It fixes a pinned TeaVM empty static-byte initializer with verified JVM default
zero, builds the Shift_JIS registry and adapts table-specific UTF-8 boundaries.
No original generated C/JAR is published as handwritten project source.

Vita is not a full POSIX runtime: replace mmap/mprotect allocation expectations,
wall/monotonic clocks and cooperative waits explicitly. Keep reachable Date/UTC
conversion; the input-only probe's Date removal cannot be copied into the game.
Compile all.c once and keep its root off global -I paths to prevent libc header
shadowing. Keep managed GC heap, Newlib heap and resource/GPU allocations distinct.
A valid ELF must still pass VELF/SELF/package creation. See [BUILD](BUILD.md).

## 4. Treat GC, arrays and GL ownership as a real boundary

Direct NIO buffers caused a TeaVM GC freeBufferContent failure in this port.
Suppressing GC or inflating the heap would conceal the lifetime problem.
Heap-backed Java views plus native-owned client data solve the observed boundary.
Buffer position, limit, arrayOffset, read-only/direct modes and index widths all
matter; test them, including shrinking limits and repeated reuse.

Keep cached imported textures immutable. Content hashes only shortlist candidates:
compare exact source bytes and sampling mode before reuse. Track live owners and
evict only idle GPU objects. Text/FBO targets are mutable and must remain outside
that cache. Preserve original flush/blend order rather than assuming batching can
reorder translucent sprites. Host GL mocks establish ownership behavior, not GPU
fidelity. Real-device text/pixels and frame timing are separate checks.

## 5. Carry the original data filter down to actual reads

The original stream GameData loader skipped excluded entry types. Reading a whole
PAC then filtering in memory preserved results but lost that I/O benefit.
00.20 restores exclusion before payload reads. Retain all directory slots/types/
reserved fields and ordering even when a payload is omitted: original index-based
lookups must not shift. Nested SPR and converted BIN/DAC are distinct schemas.

00.21 exposes another contract error: a 0..127 native range guard rejects the
original Game3 mask 187 (BIN/WAV allowed) and other paths 251 (BIN only). Type flags
are bit tests; their OR does not define the legal range of an int mask. Test actual
core call sites and higher/sign bits instead of only invented seven-bit samples.
The new regression fails on previous code and passes after removing that guard;
00.22 initially awaited selection testing; later hardware reports approved
selection and battle startup. That earlier pending status is historical.

Separate disk bytes, bridge copies, decoding and GPU uploads. A 26-character-PAC
host probe measured 91,081,701 → 11,707,264 source bytes with filter 33; this
87.146% reduction is not a measured selection-speed or FPS improvement.

The 8/4/2 MiB PAC/texture/voice caches here are design budgets, not transferable
hardware guarantees. Cache keys must include resolved dataset and conversion/
filter variant; exact byte checks prevent collision aliasing. Respect invalidation,
active owners and eviction. Document additional live/managed/driver memory.
First visits and evicted revisits still load resources; no async prefetch is
implemented in this checkpoint.

## 6. Diagnose audio in separate stages

| Stage | What to verify | What the result cannot prove |
|---|---|---|
| Source | RIFF/chunks or wrapper, channels/rate, PCM samples, rail counts | Source rail samples do not identify every audible artifact |
| Decode | Byte-exact PCM or bounded ADPCM reconstruction, entry index | Successful decode does not prove correct event-to-voice IDs |
| Resample | Duration, tone, DC, bounds, spectrum and edges | A synthetic DSP improvement is not an audible Vita confirmation |
| Mix | Three voice channels here, gain/stereo, overlap, peak control | Zero clipping does not exclude delivery/scheduling gaps |
| Worker/output | Port/create/start errors, output submission timing, teardown | Zero late mix computations does not prove no underruns |

Tap Battle's source AudioTrack uses 22050 Hz mono PCM16. A 44100-Hz minimum-buffer
query is not the playback rate. Community wrappers require their own verified
ADPCM path; never play encoded bytes or a WAV header as PCM. Original voice IDs
are zero-based; a one-slot error produced neighboring/repeated speech.

The 16-tap/256-phase Q14 filter suppresses a measured spectral image 32.3 dB
without changing duration; the peak limiter controls added mix clipping. Rough
voices remained audible in 00.19, so neither change alone is a proven complete
cure. Request a recording by phrase/character alongside source and counters.

00.20 changed an encoded thread priority based on numeric ordering. On Vita,
worker setup failed and the original game caught a BGM exception and exited.
Restore the tested value, log actual syscall/error, test failure ownership and
avoid reopening for every clip. An always-successful mock cannot validate kernel
scheduling. Do not infer a universal legal priority range from this session.

## 7. Test lifecycle and cooperative progress explicitly

First active frame must supply the original resume edge; otherwise required text
surfaces were null and the game exited without a native crash. Stable logical
touch slots must replace raw Vita IDs outside the original Controller range.
Physical Circle/Triangle must not silently become Android Back in gameplay.

Original AutoCardTask runs through TeaVM cooperative EventQueue. Rendering
continued while an unpumped task stayed queued indefinitely. The entry loop now
processes one ready event after present. Preserve units/order and measure work
outside present; a steady FPS average can hide a long loading/background pause.

## 8. Preserve datasets and save ownership

Resolve resources only inside the selected profiles/<Profile>/ dataset. Missing
or corrupt data is an explicit compatibility error; never borrow another
profile's files. An entire PAC is decoded as one resource, without merging its
entries with another PAC. Ordinary and protected files can coexist inside one
profile when their own formats are supported.
A container codec does not establish text charset: Gen's ordinary PAC has UTF-8
text00 but Shift_JIS game/character tables. Avoid lossy charset conversion.

Save progress belongs to one profile. Resources are read-only except the dedicated
save.bin contract; writes use a temporary file, fsync/close and rename. Avoid
silent save migrations or importing bundled default saves over current progress.
A valid container/base-common marker is not a complete installation check.

## 9. Keep evidence reusable

Capture version, embedded source commit, VPK/eboot hash, toolchain ABI, dataset
provenance and exact procedure with each result. Separate FORMAT, HOST, BUILD,
VITA3K and HARDWARE evidence; scope them to the tested feature, not the project
as a whole. A catch-and-exit may have no psp2core: original exception/stack logs
are essential. Keep old failed artifacts' records instead of overwriting them.

Use [VALIDATION](VALIDATION.md) for commands/log fields. Use [FAILURES](FAILURES.md)
before retrying an approach, [ATTEMPTS](ATTEMPTS.md) for experiments and
[SUCCESSES](SUCCESSES.md) for confirmed results. Pin upstream notices and describe
what a symbol bundle actually contains. Generated commercial code is not covered
by the licenses of the compiler/runtime/adapters merely because it compiles.

<!-- DBTB_00_23_DETAIL:START -->
## Reusable lesson from 00.22 → 00.23: preserve streaming allocation topology

On memory-constrained targets, an adapter can break a working original parser even
when it returns byte-identical data. The failed 00.22 adapter first allocated an
entire ~4.74 MiB PAC in TeaVM managed memory; the original method then performed its
own normal work. That additional peak was enough to abort at battle startup.

The successful 00.23 approach patches only platform-specific opening and leaves the
original stream parser/lifetime structure intact. For future ports, prefer a native
stream/handle bridge over whole-file managed copies for large resources, and test
allocation **shape and timing**, not only total file size or final decoded content.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## Reusable lesson from 00.23 → 00.24: budget transient native allocations

A correct managed stream adapter does not prevent native OOM. The Android14
battle-start crash returned from Ogg PCM vector growth with a 9,506,304-byte
request. bgm_03 needed only 5,454,332 PCM bytes, but incremental resize retained
the old allocation while requesting a doubled replacement: 14,260,324 C++ bytes
at peak. Cache budgets did not bound that transient demand.

For seekable Vorbis, query ov_pcm_total, bound dimensions/channels/rate and total
bytes, validate every chained stream, allocate once, and decode directly into the
exact buffer. Verify final decoded length and preserve PCM byte-for-byte. Reclaim
cache-only PAC owners and idle textures before the allocation; shared active
streams and referenced textures must survive. Keep the old music until a new
track has decoded successfully. Do not disguise allocation failure by stripping
music or substituting gameplay.

The regression test compares every sample/frame of all 17 supplied BGM tracks.
A 6 MiB single-request ceiling reproduces the old bgm_03 bad_alloc while every
fixed load succeeds. Its fixed C++ peak is 5,454,432 bytes; this measurement
excludes Vorbis C allocations and other live owners. The user confirms 00.24
works on physical Vita. Keep artifact identity and limited hardware scope with
that result; new rebuilds still require device testing.

## Reusable lesson from 00.32 → 00.33: verify the generated ownership handoff

Large-resource correctness is not finished when the normalized bytes are right.
The final C++ ownership transfer can create a second transient peak if the compiler
lowers a seemingly moving expression into copy assignment.

In 00.32, Invasion `char15.pac` normalized to roughly 4.64 MiB. The source used
`output = changed ? std::move(out) : input`; the matching Vita coredump and ELF
showed execution through `std::vector<unsigned char>::operator=` before
`std::bad_alloc`. The cache policy was not enough because the final handoff itself
asked for another large contiguous allocation.

The 00.33 fix uses an explicit branch: `output.swap(out)` for transformed data and
ordinary copy only for unchanged input. Physical testing then completed several
Invasion fights without reproducing the Saitama -> Freezer crash.

For future Vita ports:

- inspect allocation **shape**, not only final size and retained-cache budgets;
- symbolize the exact coredump against the exact ELF before changing gameplay;
- prefer explicit swap/move ownership for multi-MiB transformed buffers;
- add source/compiler regressions around allocation-sensitive handoffs;
- keep transformed source bytes immutable on disk and fix the platform adapter
  rather than repacking game data to hide memory bugs.
