# Local game data — current profile contract

No Dragon Ball Tap Battle asset dataset is stored in Git. Prepare data from a
user-owned APK outside tracked source, then copy the resulting package to Vita.

The current 00.34 runtime contract is:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

There is no special `game/` directory and no separate `mods/` root.

Use the Windows extractor for normal user installation:

```text
tools/windows/Extract_APK_for_Vita.bat
```

It writes each APK as:

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

The older Python extraction/preparation tools are retained for engineering,
forensics and pinned historical tests. Some of their command-line examples in
historical documents use the directory layout of the build they were testing;
they are not the current end-user installation contract.

See:

- [Current runtime contract](../docs/CURRENT_RUNTIME_CONTRACT.md)
- [Data layout](../docs/DATA_LAYOUT.md)
- [Windows extractor](../docs/WINDOWS_DATA_TOOL.md)
- [Current status](../docs/CURRENT_STATUS.md)
- [Build](../docs/BUILD.md)

Never commit extracted resources, APKs, user saves or generated commercial core
artifacts.
