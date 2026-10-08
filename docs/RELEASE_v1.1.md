# Dragon Ball Tap Battle PS Vita v1.1 — Universal Mod Support

**Status:** hardware-tested release build, ready for maintainer publication.

**VPK:** `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk`  
**VPK SHA-256:** `9953e8c99ce59a5b4b55dab3ae2caec788c39ffe1edb19a2d4e5833c958ee6bd`  
**Vita TITLE_ID:** `DBTB01178` · **APP_VER:** `01.01`  
**Windows Extractor:** `DBTapBattle-Extractor-Windows-v1.1.zip` · SHA-256 `148480f1f5447086796eaa66ad3f97a45af7a15b5c32f03eadea929b5c4d44a2`  
**Web Extractor:** https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

## What's new

- **Expanded mod compatibility:** support for the original game and compatible community mods, including automatic recognition of additional DragonTap PRIVATE resource formats without embedding per-mod keys in the Vita executable.
- **Universal Web and Windows extractors:** every APK becomes an independent profile; compatible newly discovered PRIVATE formats receive a profile-local `dbtb_codec.json` automatically.
- **Improved stability:** reduced PAC memory peaks, incremental/file-backed AAC/M4A playback, compact GPU texture handling, and adaptive texture quality for especially heavy protected assets.
- **Existing behavior preserved:** profile selector, LiveArea, audio paths, independent saves, and previously audited profile decoders remain intact.
- **Progressive compatibility:** successful extraction does not guarantee compatibility with arbitrary Android/Dalvik gameplay-code modifications.

## Hardware validation

The final VisualQuality runtime was tested on a real PS Vita with the heavy **dbz mobile v9** profile. The user confirmed character selection, fight startup, victory, and multiple consecutive battles without reproducing the previous crash/freeze behavior.

The final `runtime.log` records 7 installed profile directories, 78 `[TextureCompact]` diagnostics, 7 indexed compressed BGM tracks, 3 combat-resource boundaries, and 340 performance reporting windows, 317 of them at 58 FPS or higher. No `std::bad_alloc` or crash is recorded in that log. These figures describe the tested session only and are not a blanket performance guarantee for every mod.

The original game and previously accepted mod paths retain their established resource-loading behavior. **Not every community mod has been tested.**

## Known limitation: texture sharpness

Some mods contain very large/high-resolution sprite atlases, backgrounds, effects, and UI assets. Loading all of them at full quality can exceed the PS Vita's limited system/GPU memory and cause `std::bad_alloc`, freezes, or crashes.

To prioritize stability, v1.1 uses compact GPU storage and a **generic memory-aware quality policy** for compatible high-resolution protected textures. It may **reduce the physical resolution of selected textures**, so some graphics can look blurry while others remain sharp.

This is intentional. The extractor does not damage or recompress the source PAC files, and there are **no mod-name-specific quality configurations**. Heavy mods can also take longer to load. Improving fidelity for these especially large assets is future work; v1.1 keeps the configuration confirmed stable on real hardware.

## Installation summary

1. Install **v1.1 VPK** with VitaShell. When updating, install it over the previous version without deleting your profile data.
2. **Windows:** extract the Windows tool ZIP, then drag your APK onto `Extract_APK_for_Vita.bat`.
3. **Web / Android / desktop:** open the [Web Extractor](https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/), select the APK, press **EXTRACT DATA FOR PS VITA**, and download the generated ZIP.
4. Extract the generated package and copy its **`data/` folder to the root of `ux0:`**.
5. The final profile path must be `ux0:data/DBTapBattle/profiles/<Profile>/`.
6. If the extractor generated `dbtb_codec.json`, keep it next to the profile PAC files.

See the [complete installation and extraction guide](INSTALLATION_AND_EXTRACTION.md) for detailed Windows and Web instructions.

## Reporting compatibility problems

If a mod crashes, freezes, fails to load, or displays incorrect graphics, open a GitHub Issue and include the VPK version, mod name/version, characters/mode, exact reproduction steps, and `runtime.log`. Attach `psp2core*.psp2dmp` when a crash dump exists.

Do not upload proprietary APKs or commercial game data to the repository.

## Package identity

The v1.1 VPK contains the **exact same `eboot.bin` as the hardware-approved VisualQuality experimental build**:

`bea473a4f1287702eafb2fe79e1b529d192b862bcd1ef63d681bf13f6a6d3af2`

Only `sce_sys/param.sfo` was changed to move `APP_VER` from `01.00` to `01.01`. `TITLE_ID DBTB01178`, LiveArea assets, selector assets, and the save seed remain unchanged. ZIP/CRC and LiveArea validation pass.

The previous v1.0 package remains the historical rollback baseline.

The VPK and extractors do not distribute the original APK or proprietary game data. Use an APK copy that you legally possess and extract its data locally.
