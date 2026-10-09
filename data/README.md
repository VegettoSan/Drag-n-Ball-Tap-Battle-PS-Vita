# Local game data — current profile contract

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](../docs/CURRENT_RUNTIME_CONTRACT.md) · [Status](../docs/CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

Prepared v1.2 VPK identity: APP_VER `01.02`, TITLE_ID `DBTB01178`; the published
1.1 is the previous release.

No Dragon Ball Tap Battle asset dataset is stored in Git. Prepare data from a
user-owned APK outside tracked source, then copy the resulting package to Vita.

The v1.2 runtime retains the unified profile contract:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

There is no special `game/` directory and no separate `mods/` root.

For normal user installation, choose either:

- Web Extractor 1.0:
  `https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/`
- Windows Extractor 1.5:
  `tools/windows/Extract_APK_for_Vita.bat`

Both write each APK as:

```text
data/DBTapBattle/profiles/<sanitized APK filename>/
```

The profile folder name is the selector label. Rename the folder if a different
display name is desired.

The selector enumerates only real first-level folders inside `profiles/`. It
does not create an unconditional Original entry. If no profile is installed, it
shows the no-game-data screen.

Each selected profile resolves PAC/audio/data only from its own directory. There
is no cross-profile resource fallback.

Each profile owns:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

The VPK copies its read-only `app0:/save.bin` seed only when that profile does
not already have a save. APK-bundled saves are not installed automatically.

The profile-local `vita-controls.cfg` remembers only the launch input choice.
Vita controls hide pads; Touch only retains original input. Stable progress stays
in `save.bin`; experimental `save-controls-test.bin` is separate and is not
automatically migrated. Keep any required `dbtb_codec.json` with its profile.

The older Python extraction/preparation tools are retained for engineering,
forensics and pinned historical tests. Some of their command-line examples in
historical documents use the directory layout of the build they were testing;
they are not the current end-user installation contract.

See:

- [Current runtime contract](../docs/CURRENT_RUNTIME_CONTRACT.md)
- [Data layout](../docs/DATA_LAYOUT.md)
- [Web extractor](../docs/WEB_DATA_TOOL.md)
- [Windows extractor](../docs/WINDOWS_DATA_TOOL.md)
- [Current status](../docs/CURRENT_STATUS.md)
- [Build](../docs/BUILD.md)

Never commit extracted resources, APKs, user saves or generated commercial core
artifacts.
