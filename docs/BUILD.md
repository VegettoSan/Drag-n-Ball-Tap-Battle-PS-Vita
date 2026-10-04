# Build and First Test

## Requirements

- VitaSDK configured through the `VITASDK` environment variable.
- vitaGL and its current dependencies installed in the same VitaSDK environment.
- CMake 3.16+.

The initial CMake link list follows current vitaGL sample conventions and may need adjustment if the locally installed VitaSDK/vitaGL package differs. Any such adjustment must be logged in `ATTEMPTS.md`.

## Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j$(nproc)
```

Expected artifact:

```text
build/dbtb_vita.vpk
```

Current title ID:

```text
DBTB00001
```

## Prepare original data

On PC:

```bash
python3 tools/extract_apk_data.py /path/to/DBTapBattle.apk ./original-data
```

Copy the extracted files to:

```text
ux0:data/DBTapBattle/game/
```

Do not copy the APK itself to the repository.

## Optional mods

Create one folder per mod:

```text
ux0:data/DBTapBattle/mods/MyMod/
```

Place only the files the mod overrides there. Missing files fall back to `game/`.

## Expected first bootstrap behavior

1. App starts and initializes vitaGL.
2. It scans `mods/`.
3. A selector displays `ORIGINAL` plus detected mod folder names.
4. D-pad moves selection.
5. Cross confirms.
6. Triangle exits the selector.
7. The selected VFS resolves `common.pac`.
8. The PAC parser validates its original table.
9. A green `PAC OK` screen appears on success; red `PAC ERROR` appears on failure.
10. `ux0:data/DBTapBattle/runtime.log` records the boot, selected dataset and PAC result.

## What to provide after the first Vita test

- Exact commit/build used.
- `runtime.log`.
- Photo/screenshot of the selector and result screen.
- Whether Original and at least one mod folder are both detected correctly.
- Any crash dump if generated.

Do not mark the bootstrap as successful until it is built and observed on Vita/Vita3K.
