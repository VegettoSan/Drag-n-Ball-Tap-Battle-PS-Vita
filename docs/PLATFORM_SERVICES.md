# Audio, input, saves, downloads and multiplayer

All original-code statements below refer to the supplied APK 1.4, not the
modified Sketchware project. Current native runtime remains a bootstrap.

## Audio

ffprobe confirms all 36 Ogg streams as **Vorbis / 44,100 Hz**: 17 stereo BGM
tracks and 19 mono sound effects. Sizes/hashes and stream duration metadata are
committed separately. No conversion is necessary merely to read Ogg on Vita;
use a compatible Vorbis decoder and output PCM through a mixer/audio service.
Playback, resampling and scheduling still need implementation and device tests.

Original SoundEffect:

- MediaPlayer recreates BGM, sets left/right volume and the requested loop flag.
  Suspend stops it; resume calls PlayBGM again. Preserve this observed behavior
  before considering a seamless-resume feature.
- SoundPool reserves 20 streams, loads the 19 SE resources, plays at priority 1,
  loop count 0 and playback rate 1.0. Native mixer must retain overlaps/lifetimes.
- Three AudioTrack channels handle supplied wave/PCM buffers; wave storage has
  40 slots. Constructor uses 22050 Hz/legacy mono/PCM16 constants, while minimum
  buffer query uses 44100 Hz. Real missing character WAV examples must settle
  exact byte layout; do not feed an entire WAV header as PCM blindly.
- Core has a bounded SE request queue; BGM volume uses ConfigData[2]/100 × 0.7,
  SE ConfigData[3]/100 × 0.6, voice/PCM ConfigData[3]/100.

Recommended service contract: play/stop/loop BGM by VFS name, load/unload effect,
play effect with gain/priority, play owned PCM buffer, lifecycle pause/resume.
Decode/mix outside the render thread, reuse buffers, log errors without per-frame
spam. Backend choice remains PENDING until the original channels are exercised.

## Input

AndroidGLView converts event coordinates using `fScreenScale=320/screen_height`,
subtracts screen offsets, preserves pointer IDs and distinguishes down/move/up.
KeyData has ten slots; the core processes five touches in its observed per-frame
loop. Controller tracks tap position/duration, pull displacement, virtual pad
ranges, key state/history, joystick angle and gesture flags. Preserving only
'button pressed' would lose essential original gameplay.

The native input layer now produces stable Begin/Move/End pointer events and
neutral menu commands. Vita touch is normalized to bootstrap screen coordinates
from runtime panel bounds; future KeyData adapter must then apply the original
screen scale/offset. A gesture interpreter is not implemented yet.

| Control | Implemented bootstrap behavior | Proposed original-controller mapping (PENDING) |
|---|---|---|
| Front touch | Tap a visible Original/mod row | Pass stable pointers to original KeyData/Controller |
| D-pad / left stick | Up/down menu edges | Virtual movement pad/direction command, preserving command buffers |
| Cross | Confirm | Original primary virtual action; menu confirm |
| Circle | Back/exit selector/result | Original back/guard action after mode-specific command identification |
| Square | No action yet | Secondary action, only once original command ID is recovered |
| Triangle | Back/exit selector/result | Special/contextual original action, not hardcoded damage |
| L / R | No action yet | Original card/target/context actions if a matching command exists |
| Start | Exit diagnostic result | Pause/menu lifecycle command |
| Select | No action yet | Local options/control profile; never a mandatory gameplay mechanic |
| Right stick | No action yet | Optional pointer/camera only if an original action justifies it |

Held Cross is seeded at screen transitions, preventing accidental instant result
exit. Touch availability failures leave physical controls usable. Many mod rows
scroll with D-pad/stick. Unicode bytes are preserved for file access, but the
small diagnostic font only has ASCII glyphs; unsupported glyphs show '?' and
long labels clip. Full Unicode typography is PENDING, not falsely claimed.

## Saves and configuration

`TCBManajer.ConfigData` is **12906 bytes** in the original DEX. _FILELoad reads
save.bin and copies temp.length bytes into it; failure initializes defaults.
_FILESave writes the whole block; _FILESaveLen writes a partial range at its
original offset. Utility supports Android private files and external-storage
fallback; preserve ordering rather than merging all Android paths arbitrarily.

Data includes settings, unlock/download/version flags, equipped cards, progress
and stats; character fields use `char*100+30+offset`. Some multi-byte values
(e.g. XP fields) are BE even though PAC offsets are LE. The audit established
allocation and original read/write calls, **not save interoperability**. No
user Android save fixture was supplied; read/write/import tests remain PENDING.
SharedPreferences 'uniddata'/'unid' is used by Smap account/device identity, not
the primary combat save. Billing SQLite is not a gameplay save database.

Proposed writable layout: saves/original/save.bin and saves/<mod>/save.bin,
config/ for port preferences. Isolate incompatible mod progression, preserve
raw imported save and backup before any migration, use temp-write/rename for a
native full save. Never overwrite game/ or mod resources. Actual saving is not
implemented by the bootstrap.

## External data and offline startup

**Bundled APK != full downloaded installation.** It contains no charXX,
chardemoXX or charf00XX PAC families. Original CheckCharctorFiles checks these
three members for a character. pack_unpack uses a PAC-like downloaded wrapper:

| Package type | File naming in original code |
|---|---|
| Character | entry 0→charNN.pac; entry 1→chardemoNN.pac; entry 2→charf00NN.pac |
| Card pack | card number = pack_index*entry_count+1+entry_index; cardNNN.pac |

Original endpoints found in DEX: device/screensize.csv and news.csv under
http://smap-ai.channel.or.jp/dragonball_tap/; purchase under
https://smap-a.channel.or.jp/purchase; appversion.php on sd01.fas.ne.jp.
Asset URLs are obtained from Smap catalog data, not a verified fixed URL list.
Server availability and completeness of surviving packages are **UNCONFIRMED**;
no commercial external packages were downloaded or put in Git.

Native startup must resolve local data first and offer actionable missing-file
errors. No network request is present in current native code. A future importer
can accept a user-owned complete Android data folder or preserved package, but
must validate its naming/bounds. Do not silently unlock absent characters or
claim a community archive reconstructs every original version.

## Bluetooth and obsolete services

BluetoothSearch uses RFCOMM/SPP UUID 00001101-0000-1000-8000-00805F9B34FB with
client/server discovery; BluetoothManajer runs stream/read/write threads. Core
BTRev/BTSend gates task execution and handles synchronized battle data. Thus
transport is Android-specific, but synchronization/gameplay is part of the core.
Vita transport substitution (e.g. local network) requires packet/timing analysis;
it is not implemented or tested. Preserve single-player flow independently.

| Dependency | Classification | Vita treatment |
|---|---|---|
| TCB/task logic, data formats, gestures, combat/AI | Necessary gameplay | Reconstruct and preserve semantics |
| Activity/GLSurfaceView/Bitmap/Canvas/touch/files/audio | Platform-specific but needed service | Replace by Vita adapters |
| Smap catalog/download | Previously supplies missing resources | Local completeness/import; no startup dependency on server |
| Market billing, purchase DB, device identity | Prescindible for native local datasets | Keep outside core; no ported billing |
| Device screensize/news/update/browser APIs | Online/platform extras | Optional local defaults/status; never block start |
| Android RFCOMM APIs | Replaceable transport | Multiplayer PENDING; no impossible Android calls in native core |
| Analytics/advertising SDK | No dedicated SDK established in supplied 91-class DEX | Do not invent or include a dependency |
