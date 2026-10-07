DRAGON BALL TAP BATTLE - WINDOWS DATA EXTRACTOR 1.4

Requirements
- Windows 10 or Windows 11
- Built-in Windows PowerShell 5.1
- No Python, Java, 7-Zip, administrator rights, or Internet connection required

QUICK START
1. Extract the entire extractor ZIP into one folder.
2. Keep Extract_APK_for_Vita.bat and Extraer_APK_para_Vita.ps1 together.
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
