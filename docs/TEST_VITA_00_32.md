# PS Vita physical test — 00.32 Loading regression

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Historical build sheet: its identities/results apply to the named build only.
<!-- DBTB_DOC_STATUS:END -->

00.32 exists specifically to undo the 00.31 infinite-loading regression while
retaining the 00.31 roster, Shop, memory and text/audio fixes.

## Root cause confirmed

The original APK's Downloader uses `isDownload()==true` to mean **the async
request is still running**. 00.31 returned true forever for Vita's offline stub,
so the TCB state remained stuck after requesting:

```text
http://smap-ai.channel.or.jp/dragonball_tap/device/screensize.csv
```

Both Invasion and Samu logs show this exact terminal state while rendering keeps
running at about 60 FPS.

00.31 also added a deep `auditInstalledData()` before entering the original
engine. On Vita this took about 13 s for Invasion and 34 s for Samu because every
character PAC was opened and parsed.

## 00.32 behavior

- offline Downloader completes immediately: `isDownload=false`, no data;
- original engine follows its normal failed/offline catalog path;
- startup roster detection uses file presence/contiguity only;
- deep PAC parsing is not performed before engine startup;
- dynamic roster/save synchronization from 00.31 remains;
- Shop return fix remains;
- large-PAC memory policy remains;
- Samu direct MP3/AAC/Vorbis remains;
- Invasion UTF-8 fallback remains.

## What to test first

1. Launch Samu. It must leave Loading and reach the normal title/menu.
2. Launch Invasion. It must also leave Loading.
3. Note approximate time from profile selection to game startup.
4. If both pass, continue the pending 00.31 checks:
   - Samu visible character count should be 92;
   - Shop must not close the game;
   - run 3–5 consecutive Invasion fights to stress the previous bad_alloc path.

If Loading still loops, send the complete runtime.log. The last non-Perf line is
especially important.
