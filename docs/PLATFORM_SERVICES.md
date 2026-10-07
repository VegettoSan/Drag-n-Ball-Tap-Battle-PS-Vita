# Platform services — original contracts and current Vita adapters

Checkpoint 00.22 / 2026-10-05. Original-code facts refer to the pinned original
APK, not the community archive. The full original core executes through private
AOT; this is no longer an atlas-only bootstrap. [CURRENT_STATUS](CURRENT_STATUS.md)
and [VALIDATION](VALIDATION.md) qualify the hardware/host evidence.

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

00.19 voices still sound bad despite zero measured overload/clipping in its
latest session. 00.20's synthetic reconstruction check reduces an image 32.3 dB,
but its hardware worker fails setup before the menu. 00.21 worker/menu recovery is now confirmed on Vita. Its
selection then rejects legal mask 187 and exits; 00.22 removes that native range
guard. Physical selection recovery and audible voice quality remain pending. Source rail samples, clipping and output gaps are separate
facts; see [VALIDATION](VALIDATION.md) for counter meanings.

## Touch and physical controls

Original screen scale is 320/screen_height. VitaEngine sets 960×544, truncates
scaled width to 564 and applies original horizontal offsets before KeyData.
Raw Vita touch IDs are mapped to stable logical slots 0–4; Begin/Move/End and
Controller history/gesture processing remain original. KeyData has ten slots;
the observed core processes five touches. Front touch is hardware-confirmed in
earlier menu/selection/combat builds.

| Input | Native selector | Current original-game loop |
|---|---|---|
| Front touch | Select visible profile row | Original KeyData/Controller gestures |
| D-pad / left stick | Move/scroll profile rows | No gameplay gesture mapping |
| Cross | Confirm profile | Neutral |
| Circle / Triangle | Cancel selector | Android Back deliberately not emitted |
| Start | Diagnostic/selector behavior where present | Pause/back neutral; use original touch UI |
| Square, L/R, Select, right stick | No general gameplay binding | Mapping remains future work |

Held touch/button state is primed across selector transitions. Circle/Triangle
previously closed the game through original Back semantics; neutral physical
input now avoids that unintended exit. Do not advertise a controller scheme
from the older proposal. The selector's diagnostic font remains ASCII-limited;
UTF-8 paths are retained and in-game PVF text is a separate service.

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

Resources resolve through `GameVfs` inside exactly one active dataset. Original
uses `game/`; a selected profile uses only `mods/<Profile>/`. Missing, malformed
or non-regular selected-profile resources are explicit errors; there is no
cross-profile fallback. Native ResourceAdapter applies original
GameData exclusion bits before disk reads and normalizes only selected verified
payload schemas in memory. Directory slots/order/reserved fields stay stable.
See [PAC_FORMAT](PAC_FORMAT.md) and [DATA_LAYOUT](DATA_LAYOUT.md).

Original ConfigData is 12906 bytes; _FILELoad/_FILESave/_FILESaveLen and original
partial offsets remain the semantic authority. Only `save.bin` is writable.

Starting with 00.30, the VPK still contains the exact user-approved
12,906-byte seed at read-only `app0:/save.bin` (SHA-256
`64b050092a5be8921108e1a38ef4777ef69eb87ab3226d8c244eb9073755e0bb`),
but each selected profile owns a separate writable copy:

```text
Original: ux0:data/DBTapBattle/game/save.bin
Mod:      ux0:data/DBTapBattle/mods/<Profile>/save.bin
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
> **Current hardware checkpoint — 00.24 (2026-10-05, America/Bogota):** the user
> confirms `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` works on the physical Vita
> after the Android14 battle-start crash. Runtime source `f5672d4d`, VPK SHA-256
> `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
> The original PAC streaming repair remains; Ogg PCM now uses one exact allocation
> instead of transient vector doubling, with cache-only resource reclamation.
> The approved LiveArea is retained. This is a user-confirmed test checkpoint,
> not exhaustive character/profile/mode or long-session certification. Historical
> records keep their original artifact and evidence scope.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
