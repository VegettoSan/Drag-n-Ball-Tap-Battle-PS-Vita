DRAGON BALL TAP BATTLE - WINDOWS DATA EXTRACTOR 1.3

Requirements
- Windows 10 or Windows 11
- Built-in Windows PowerShell 5.1
- No Python, Java, 7-Zip, administrator rights, or Internet connection required

QUICK START
1. Extract the entire DBTapBattle extractor ZIP into one folder.
2. Keep Extract_APK_for_Vita.bat and Extraer_APK_para_Vita.ps1 together.
3. Drag one or more Dragon Ball Tap Battle APK files onto Extract_APK_for_Vita.bat.
   You can also double-click the BAT and choose the APK files from the file picker.
4. When the tool reports READY, open the generated package.
5. Copy the package's data folder to the root of ux0: with VitaShell.
6. Confirm the final path is:
     ux0:data/DBTapBattle/
   NOT:
     ux0:data/data/DBTapBattle/
7. Launch the Vita port and select the extracted profile.

RUNTIME LAYOUT
- Original:
    ux0:data/DBTapBattle/game/
- Mods / alternate APK datasets:
    ux0:data/DBTapBattle/mods/<Profile>/

The current Vita runtime treats every selected profile as standalone. It does not
borrow missing resources from game/ or another mod profile.

KNOWN PROFILE NAMES
- Original APK -> game/ -> selector name: Original
- gen.apk -> mods/Gen/
- Zuper/Samu -> mods/ZuperSamu/
- Android14 -> mods/Android14/
- Spanish Android14 -> mods/Espanol/
- Invasion -> mods/Invasion/
- Other supported APKs -> mods/<sanitized APK filename>/

SAVE FILES
Each profile has its own save:
- Original: ux0:data/DBTapBattle/game/save.bin
- Mod:      ux0:data/DBTapBattle/mods/<Profile>/save.bin

The extractor intentionally does NOT install save.bin from an APK. On first launch
of a profile, the VPK creates that profile's save from its bundled seed if no
save already exists. When updating data, keep a backup of the existing save.bin.

WHAT THE EXTRACTOR DOES
- Preserves gameplay payload bytes.
- Detects res/raw and assets APK layouts.
- Canonicalizes only audited protected Android14-family PAC aliases.
- Supports dynamic character rosters in the Vita 00..99 namespace.
- Checks ZIP sizes, CRCs, path safety, collisions, and SHA-256 hashes.
- Writes dbtb_manifest.json for each profile.
- Does not extract Android DEX/classes/native libraries as Vita gameplay code.

IMPORTANT
The tool prepares game data only. The VPK is installed separately.
A mod that changes Android code may still require additional Vita-port support.
The first supplied Original APK does not include all downloaded character packs;
the extractor cannot invent files that are not present in the source APK.
