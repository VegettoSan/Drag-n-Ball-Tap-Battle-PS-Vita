DRAGON BALL TAP BATTLE - WINDOWS DATA EXTRACTOR 1.5 / VITA RELEASE v1.1

WEB EXTRACTOR ALTERNATIVE
If you do not have a Windows PC, use Web Extractor 1.0:
  https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

It runs locally in a modern browser, does not upload the APK, and produces the
same profiles-v1 data layout expected by the Vita port.

Requirements
- Windows 10 or Windows 11
- Built-in Windows PowerShell 5.1
- No Python, Java, 7-Zip, administrator rights, or Internet connection required

RUNTIME CONTRACT
The current Vita runtime contract is profiles-v1:
  ux0:data/DBTapBattle/profiles/<Profile>/

The selector shows only first-level folders that actually exist there. It has no
built-in Original entry and never reads game/ or mods/ as current profile roots.

QUICK START
1. Extract the entire extractor ZIP into one folder.
2. Keep Extract_APK_for_Vita.bat, Extraer_APK_para_Vita.ps1 and
   PrivateModDex.ps1 together. This last file is needed for NEW protected mods.
3. Drag one or more Dragon Ball Tap Battle APK files onto Extract_APK_for_Vita.bat.
   You can also double-click the BAT and select APK files.
4. Wait until the tool reports READY.
5. Copy the generated package's data folder to the root of ux0: with VitaShell.
6. Confirm the final profile path is:
     ux0:data/DBTapBattle/profiles/<Profile>/
7. Launch the Vita port and select the profile.

PROFILE NAMING
Every APK uses its APK filename as the Vita profile folder name.

Examples:
  gen.apk
  -> data/DBTapBattle/profiles/gen/

  tap battle android 14.apk
  -> data/DBTapBattle/profiles/tap_battle_android_14/

  TAP BATTLE INVASION BETA 3.apk
  -> data/DBTapBattle/profiles/TAP_BATTLE_INVASION_BETA_3/

The APK layout/codec may be detected internally as Gen-style, Android14,
Spanish, Invasion, or another supported format, but this does NOT change the
profile folder name.

The Vita selector displays the profile folder name.

If you want another display name, rename the folder after extraction or directly
on the Vita. No PAC files need to be changed and you do not need to extract the
APK again.

UNIFIED DATA DIRECTORY
There is no separate game/ and mods/ layout anymore.

All datasets live here:

  ux0:data/DBTapBattle/profiles/

Only folders that exist there appear in the Vita selector.

If profiles/ is empty, the Vita selector reports that no game data is installed
and asks the user to prepare a Tap Battle APK with this extractor.

SAVE FILES
Each profile has its own save:

  ux0:data/DBTapBattle/profiles/<Profile>/save.bin

The extractor does NOT install save.bin from an APK.
The VPK creates a profile save from its bundled seed only if that profile does
not already have a save.bin.

Back up save.bin before deleting or replacing an existing profile folder.

WHAT THE EXTRACTOR DOES
- Preserves gameplay payload bytes.
- Detects res/raw and assets layouts.
- Detects audited protected Android14-family layouts.
- Canonicalizes only audited protected PAC aliases.
- Supports dynamic character rosters in the Vita 00..99 namespace.
- Checks ZIP CRCs, sizes, unsafe paths, collisions, and SHA-256 hashes.
- Writes dbtb_manifest.json for each profile.
- Does not copy Android DEX/classes/native libraries as Vita gameplay code.

IMPORTANT
The tool prepares game data only. The VPK is installed separately.
A mod that changes Android code may still require Vita-side compatibility work.

UNIVERSAL PRIVATE MODS (VITA v1.1 HARDWARE TESTED)
- Previously unknown DragonTap PRIVATE variants are recognized by statically
  inspecting classes.dex. No Android execution, external Python, or mod keys.
- Such profiles include dbtb_codec.json next to the unchanged PAC payloads.
- Install that sidecar together with the profile's other extracted files.
- The v1.1 Universal Mod Support VPK reads dbtb_codec.json. The older v1.0
  VPK does not; do not use v1.0 with newly recognized PRIVATE profiles.
- Original and known protected mod paths remain on their audited codecs.
- Some large mod sprites/atlases can exhaust PS Vita GPU memory and cause a
  crash or freeze at fight loading. For stability, v1.1 uses a generic compact
  texture path and can REDUCE RESOLUTION on some protected high-resolution
  images. Therefore some graphics may appear blurry. Source PAC bytes remain
  intact; there is NO mod-specific quality exception.
- Compatibility is not guaranteed for all Android code-changing mods.
- Full Spanish/English-friendly instructions and memory warnings:
  https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/blob/main/docs/INSTALLATION_AND_EXTRACTION.md
