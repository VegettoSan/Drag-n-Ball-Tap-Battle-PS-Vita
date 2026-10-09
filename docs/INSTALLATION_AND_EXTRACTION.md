# Installation and data extraction — Dragon Ball Tap Battle PS Vita v1.2

This guide applies to **v1.2 — PS Vita Controls**, `TITLE_ID DBTB01178`, Vita
`APP_VER 01.02`. It retains the v1.1 VisualQuality baseline and adds the controls
approved in the user's tests. **Compatibility with every mod is not guaranteed.**

## 1. Install or update the VPK

1. Transfer `Dragon-Ball-Tap-Battle-PS-Vita-v1.2.vpk` to your PS Vita.
2. Install it with **VitaShell** over the currently installed version. **Do not uninstall the game first and do not delete its data.**
3. The `TITLE_ID` remains `DBTB01178`. v1.2 is a fresh full-engine build of the integrated controls. Existing profile `save.bin` files remain in place. Test bubble `DBTBCT001` and its `save-controls-test.bin` stay separate; test progress is not automatically migrated.
4. Existing profiles and save files remain under `ux0:data/DBTapBattle/profiles/`. A backup is still recommended before updating.

**The VPK does not contain the original APK or proprietary game data.** Prepare the data from an APK copy that you legally possess.

## Choose controls before launching a profile

The English launcher offers **PS VITA CONTROLS** first (physical controls,
hidden touch pads), then **TOUCH ONLY** (original touch input). It asks before
every profile launch and remembers that profile's highlighted choice.
Use X to confirm a character, Circle for available Back buttons, and Start to
pause/resume the main pause screen. Dialogues and other menu choices use touch.
See the [English controls diagram](VITA_CONTROLS_REFERENCE.md).

## 2A. Extract on Windows

1. Download `DBTapBattle-Extractor-Windows-v1.1.zip` from the existing 1.1 release; data extraction is unchanged.
2. Extract **all** files into the same folder. Keep `PrivateModDex.ps1` and `Extraer_APK_para_Vita.ps1` together with the BAT launchers.
3. Drag one or more Dragon Ball Tap Battle APK files onto `Extract_APK_for_Vita.bat`. You may also double-click the BAT and select the APK.
4. Wait until validation/extraction finishes. The tool creates a Vita-ready package under `Listo_para_Vita/`.
5. Using VitaShell (FTP or USB), copy the generated **`data` folder to the root of `ux0:`**, merging folders when requested.

The Windows extractor works on Windows 10/11 with the built-in Windows PowerShell 5.1. It does not require Python, Java, 7-Zip, administrator privileges, or an Internet connection while extracting.

## 2B. Extract from Android, desktop, or another modern browser

1. Open **https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/**.
2. Press **SELECT APK** and choose your APK, then press **EXTRACT DATA FOR PS VITA**.
3. Keep the page open until processing completes and press **DOWNLOAD ZIP**.
4. Extract the generated ZIP and copy its **`data/`** folder to the root of **`ux0:`** using VitaShell, FTP, USB, or another transfer method.
5. Verify the final profile location described below.

**Privacy:** APK processing happens locally in the browser. The Web Extractor does not upload your APK to the project. Very large APKs can require significant browser memory; use the Windows extractor if your phone/browser runs out of memory.

## 3. Verify the Vita data path

The correct layout is:

```text
ux0:
└── data/
    └── DBTapBattle/
        └── profiles/
            ├── gen/
            │   ├── common.pac
            │   ├── ...
            │   └── dbtb_manifest.json
            └── dbz_mobile_v9/
                ├── common.pac
                ├── char00.pac
                ├── ...
                ├── dbtb_manifest.json
                └── dbtb_codec.json   (only when generated)
```

Each APK becomes an independent profile whose folder name is derived from the APK filename. For some newly recognized PRIVATE mods, **the extractors automatically generate `dbtb_codec.json`**. Copy it together with the PAC files. Older profiles may work without it. Do not edit codec keys manually.

**Wrong path:** `ux0:data/data/DBTapBattle/`. Do not place the APK itself or the generated ZIP directly inside `profiles/`.

## 4. Saves and compatibility

Each profile owns its save at:

```text
ux0:data/DBTapBattle/profiles/<Profile>/save.bin
```

Updating the VPK does not require re-extracting existing profiles. Back up `save.bin` before manually replacing or deleting a profile folder.

The PRIVATE-format reader can recover compatible resource parameters from an APK, but **the Vita port does not execute modified Android/Dalvik code**. A mod that changes gameplay mechanics through DEX code may therefore remain incompatible even if its assets extract successfully.

### Important: high-resolution mods and PS Vita memory

Some mods contain **very large or high-resolution sprite atlases, backgrounds, effects, or UI images**. Loading all of them at full quality can exceed the PS Vita's limited system/GPU memory and cause `std::bad_alloc`, freezes, or crashes.

For stability, v1.1 uses a **generic memory-aware texture policy** based on resource properties and memory pressure. It can use compact GPU storage and **reduce the physical resolution of selected textures**. As a result, some graphics may look **blurry while others remain sharp**.

This is an intentional stability measure, not evidence of a damaged APK or failed extraction. The extractor preserves the source PAC bytes. There are **no per-mod quality exceptions or hard-coded mod-name configurations**. High-resolution mods can also take longer to load than the original game or lighter mods.

Improving the visual quality of these especially heavy assets is future work; v1.1 intentionally keeps the configuration that was confirmed stable on real hardware.

## 5. Reporting a problem

Open a GitHub Issue and include:

- VPK version;
- mod/APK name and version;
- character(s), mode, and exact steps that reproduce the problem;
- what you expected and what happened;
- whether it failed on the first fight or a later fight;
- `runtime.log`;
- `psp2core*.psp2dmp`, if a crash dump was produced.

Do not upload commercial APKs or proprietary game data to the repository.

Additional technical information: [Mod compatibility](MODS.md), [current project status](CURRENT_STATUS.md), [v9 memory investigation](DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md), [Web Extractor](WEB_DATA_TOOL.md), and [Windows Extractor](WINDOWS_DATA_TOOL.md).
