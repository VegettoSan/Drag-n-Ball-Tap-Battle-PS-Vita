# Platform services — original contracts and current Vita adapters

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

> Current filesystem/selector contract: [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).

Current prepared v1.2 uses APP_VER `01.02` and TITLE_ID `DBTB01178`.
It retains the v1.1 VisualQuality loader/memory baseline and approved Vita
controls. Original-code facts refer to the pinned original APK, not the
community archive. The core executes through private AOT; host, build and
hardware evidence remain separate in [CURRENT_STATUS](CURRENT_STATUS.md).

## Audio

Original SoundEffect uses MediaPlayer for BGM, SoundPool for 19 SE in a 20-stream
pool and three AudioTrack channels for character PCM. BGM gain derives from
ConfigData[2]/100 × 0.7; SE from ConfigData[3]/100 × 0.6; voice from
ConfigData[3]/100. BGM stop/recreate/loop and original queued request order are
preserved at the adapter boundary. Original AudioTrack playback is **22050 Hz,
mono PCM16**; its 44100-Hz buffer-size query is not the playback rate.

| Current backend | Contract / source |
|---|---|
| OGG | libvorbisfile decodes whole clips on load; 17 stereo BGM and 19 mono SE, 44100 Hz source in supplied APKs |
| MP3 / AAC / M4A | Content-sniffed compressed BGM through Vita SceAudiodec; file-backed indexed AAC/M4A retained from v1.1 |
| Voices | RIFF chunk walking or headerless PCM16; original bank IDs zero-based, 3 active voice channels |
| Community voices | Verified indexed wrapper/ADPCM decoded during PAC normalization before PCM load |
| Output | Stereo 48000 Hz, 1024-frame blocks via SceAudio; fixed native worker buffer |
| Voice conversion | 16-tap Hann-windowed sinc, 256 phases, Q14 table, 32.32 sample phase; low-rate character voices only |
| Other conversion | BGM/effects retain their existing linear sample path |
| Peak control | Stereo-linked block limiter; quiet unity retained, gradual release, no extra output queue |
| Reuse | 2 MiB source+PCM LRU across bank release, exact bytes after hash; cleared on audio disposal |
| Synchronization | Atomic state lock released between 64-sample mix portions; native worker, no per-block logging/allocation |
| Setup | 00.21 uses restored 0x10000100; exact port/create/start errors, owned-handle cleanup, failure latch until disposal |

`vita_audio.cpp` and `SoundEffect.java` implement these boundaries. PCM files are
not rewritten or pre-expanded to 48 kHz. Vorbis decoding/loading remains on the
calling game path; only mixing/output runs in the audio worker. Do not describe
this as an implemented asynchronous streaming/prefetch decoder.

Earlier voice/setup/filter failures in 00.19–00.22 were corrected in
subsequent hardware tests. Clean voices/audio were reported in the later tested
paths. Source clipping and output gaps remain separate metrics; broader
character/phrase coverage is not implied. [Validation](VALIDATION.md).

## Touch and physical controls

`VitaControls.update` runs before original `Run`. Physical buttons become
synthetic contacts only in audited original contexts; Controller, task state,
combat commands and pause state remain original. No global Android Back is
injected. Original scale is 320/screen_height; 960×544 produces scaled width
564 and the original horizontal offsets. Raw touch IDs map to stable slots
0–4, which real fingers retain ahead of synthetic contacts.

The English launcher prompts before every profile launch: **PS VITA CONTROLS**
first, then **TOUCH ONLY**, remembering each profile's highlighted choice.
`vita-controls.cfg` stores input preference only. Legacy mode 1 confirms as
hidden mode 2; touch mode 0 retains its meaning and the missing-file default.

| Input | Native selector | Original-game loop in Vita mode |
|---|---|---|
| Front touch | Select/confirm visible rows | Original gestures, dialogues and other menu choices |
| D-pad / left stick | Navigate rows | Eight combat directions; D-pad left/right changes character |
| X | Confirm | Attack; confirm a ready character; neutral during dialogues |
| Square / Triangle | Triangle also cancels | Special shortcuts 1 / 2 in combat |
| Circle | Cancel/back | Special shortcut 3 in combat; original Back in audited menus |
| L / R | No additional launcher action | Rage when available / special shortcut 4 |
| Start | No general launcher shortcut | Pause; resume from the main pause menu |
| Select / right stick | Unassigned | Unassigned |

Vita mode overlays save-read configuration byte 4 with original virtual-pad
mode 1; save writes preserve the user's stored touch preference. Hidden pads
use a sparse per-stream DAC image-field overlay; immutable PAC cache/source
bytes remain unchanged. Touch-only mode emits no physical game shortcuts.

Menu Back and Start resume use a single contact-0 Begin at (40,24), combat
pause uses (240,30). A real finger on 0 is never evicted; pending input cancels
when its live consumer changes. Held shortcuts require release/repress after
scene changes. Loading/resume and audited Yes/No prompts exclude shortcuts.
Scripts shield stale combat input; dialogue X was retired after hardware
failure. Original tutorial stays tactile. Start resumes only the main pause
menu; Circle returns from nested settings first.

Retained controls were approved in the user's test sequence, most recently
Circle in Test 5. The exact stable v1.2 package's physical retest is pending.
See [controls reference](VITA_CONTROLS_REFERENCE.md) and
[current contract](CURRENT_RUNTIME_CONTRACT.md). Launcher labels are English;
profile names and game/mod text retain their language.

## Text/render/lifecycle

StringTexture owns actual 512×512 native surfaces. PVF memory-stream font loading
falls back to file mode, caches metrics/advances and rasterizes visible glyphs.
`scePvfGetCharImageRect` supplies image coverage separately from CharInfo metrics;
00.18's dimension substitution hid text and 00.19 restores visible glyphs.
Dirty-row upload avoids full repeated raster/upload work. Font-language/script
coverage and precise Android typography are not exhaustively verified.

VitaEngine sets the original first active `bResume` edge. It calls original
Init/Run/Dispose, presents, then pumps at most one ready TeaVM EventQueue event.
Without resume, text surfaces stayed null; without event progress, ability-card
processing stayed queued while rendering continued. These are real lifecycle
contracts. All return/suspend/restart combinations still need a device matrix.

## Files, resources and saves

Resources resolve through `GameVfs` inside exactly one active first-level
`profiles/<Profile>/` dataset. There is no special Original root or cross-profile
resource borrowing. Missing, malformed
or non-regular selected-profile resources are explicit errors; there is no
cross-profile fallback. Native ResourceAdapter applies original
GameData exclusion bits before disk reads and normalizes only selected verified
payload schemas in memory. Directory slots/order/reserved fields stay stable.
See [PAC_FORMAT](PAC_FORMAT.md) and [DATA_LAYOUT](DATA_LAYOUT.md).

Original ConfigData is 12906 bytes; _FILELoad/_FILESave/_FILESaveLen and original
partial offsets remain the semantic authority. Only `save.bin` is writable.

The independent-save principle introduced in 00.30 is retained with unified
profile paths from 00.34. The v1.2 VPK contains the exact user-approved
12,906-byte seed at read-only `app0:/save.bin` (SHA-256
`64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`),
but each selected profile owns a separate writable copy:

```text
Every profile: ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

If the selected profile has no save yet, boot copies the VPK seed into that
profile directory. Existing profile progress is never overwritten merely because
the profile is launched again, another profile is selected, or the VPK is updated.

Save loading is cached for the session, writes validate bounds and use exclusive
save.bin.tmp creation, write, fsync, close and rename before updating cached
state. A failed publication reports failure and removes the temporary file it
created; an already-existing temp file is not overwritten. Full Android
round-trip and arbitrary mod progression compatibility are still separate
validation work.

`vita-controls.cfg` is separate from progress. The test bubble
`DBTBCT001` uses `save-controls-test.bin`; stable `DBTB01178` uses `save.bin`
and does not automatically migrate test progress.

There is no cross-profile gameplay-resource or mutable-save fallback. The historical 00.29 root `ux0:data/DBTapBattle/save.bin` is no longer an active 00.30 input. Old `saves/shared` and `saves/<profile>` layouts remain historical/backups. APK-bundled saves may be recorded for provenance, but current extractors do not install them as gameplay state. Some save fields are BE; PAC LE
offsets do not imply universal endianness. SharedPreferences Smap identity and
billing SQLite are not the main gameplay save.

## Offline installed data

The first original APK has no charNN/chardemoNN/charf00NN triplets. The current installed-data service validates contiguous complete character
triplets (up to the two-digit 00..99 namespace) plus select0, effect and back00
through the active VFS/PAC reader. `bobj00` is not a universal requirement:
protected Android14-derived APKs are standalone without it. Selected profiles do
not require or consult a base dataset. Gen's populated assets and empty raw
stubs require assets extraction, not installing placeholders.

Original downloaded wrappers name entries charNN/chardemoNN/charf00NN; card
numbering derives from pack index/count. Original DEX references Smap device/news,
purchase and update endpoints. Endpoint presence does not establish surviving
server availability. The Vita downloader uses local completeness for the current
single-player path; remote items stay empty and HTTP is explicitly rejected.
No billing/browser/remote Android service is executed.

## Multiplayer and unsupported services

Original Bluetooth uses RFCOMM/SPP UUID
00001101-0000-1000-8000-00805F9B34FB and BTRev/BTSend task synchronization.
The Vita adapter reports disconnected. Replacing transport with networking
requires packet/timing and game-state validation; it is not a button remap.
No Vita multiplayer is implemented or tested at this checkpoint.

| Component | Treatment |
|---|---|
| Tasks/drawing/actions/Controller/combat | Original bytecode preserved privately |
| Android GL/Canvas/audio/files/time/input | Real Vita adapters |
| Local content completeness | VFS plus format checks |
| Catalog/billing/device identity/browser | Offline/unsupported boundary; not an invented native market |
| Bluetooth synchronization | Disconnected; multiplayer pending |

Future reuse guidance: [PORTING_GUIDE](PORTING_GUIDE.md). Specific native imports
are declared in `tools/aot/engine/native/dbtb_bridge.h`.

<!-- DBTB_00_23_DETAIL:START -->
## Native resource streams — hardware checkpoint 00.23

The Vita platform layer now exposes open/size/read/close operations used by
`NativeResourceStream`. Stream handles pin their native resource owner, remain valid
across cache activity, reject invalid ranges, and are closed idempotently. This
service exists to preserve the original engine's streaming PAC parser without
copying the full archive into TeaVM-managed memory.

The design fixed the reproduced 00.22 battle-start allocation failure on real
hardware in the 00.23 test session.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->

## 00.33 resource ownership checkpoint

The native resource service now has hardware-confirmed protection against a
specific large-PAC allocation failure. After a protected/community PAC has been
rebuilt in memory, changed data is handed to the cached resource owner with
explicit `swap` rather than an ambiguous conditional vector assignment. The
matching 00.32 coredump showed the old path entering vector copy-assignment and
then `std::bad_alloc`; 00.33 eliminates that duplicate multi-MiB allocation.

Physical Vita testing completed several Invasion fights without reproducing the
Saitama -> Freezer crash. Stream ownership, profile-local VFS behavior, save
isolation and source PAC bytes remain unchanged.
