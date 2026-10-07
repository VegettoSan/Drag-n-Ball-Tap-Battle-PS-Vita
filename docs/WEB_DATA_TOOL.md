# Web APK data extractor 1.0

The project includes a browser-based data extractor for users who do not have a
Windows PC.

Web app:

https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

The site is deployed from the repository by `.github/workflows/pages.yml`.
GitHub Pages deployment run `37703911061` completed successfully and reported
the URL above.

## Privacy model

APK processing is entirely local to the browser.

- The selected APK is read through the browser File/Blob APIs.
- No APK, extracted file, hash, filename or manifest is uploaded to this project.
- The page Content Security Policy sets `connect-src 'none'`, so the extractor
  itself cannot make network requests while processing an APK.
- GitHub Pages serves only the static HTML/CSS/JavaScript and the validated
  selector artwork.
- The output ZIP is built locally and downloaded/shared directly from the device.

The user still downloads the page itself from GitHub Pages, but the APK bytes are
not sent back to GitHub or to another server.

## Output contract

Web Extractor 1.0 targets exactly the same runtime contract as Windows Extractor
1.5:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

For one APK named `gen.apk`, the generated ZIP contains:

```text
data/
└── DBTapBattle/
    └── profiles/
        └── gen/
            ├── common.pac
            ├── ...
            └── dbtb_manifest.json

LEEME_COPIAR_A_VITA.txt
RESULTADO.json
SHA256SUMS.txt
```

The profile folder is derived from the APK filename using the same sanitizing
rules as the Windows tool. Multiple APKs can be selected in one browser session;
name collisions receive `_2`, `_3`, etc.

## Use from Android, desktop or tablet

1. Open the Web Extractor URL.
2. Tap/click **SELECT APK**.
3. Choose one or more Dragon Ball Tap Battle APK files that you legally possess.
4. Press **EXTRACT DATA FOR PS VITA**.
5. Keep the page open while CRC/SHA/PAC validation and ZIP generation finish.
6. Press **DOWNLOAD ZIP**. On devices that support file sharing, **SHARE ZIP**
   can also open the system share sheet.
7. Extract the downloaded ZIP.
8. Copy the ZIP's **data** folder to the root of `ux0:` using VitaShell,
   FTP, USB or another transfer method.
9. Confirm the final path is:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

10. Launch Dragon Ball Tap Battle PS Vita v1.0 (`TITLE_ID DBTB01178`) and select
    the profile.

Do **not** copy the generated ZIP itself into `ux0:data/`, and do not create
`ux0:data/data/DBTapBattle/`.

## What the browser extractor validates

The browser implementation mirrors the safety/compatibility rules of Windows
Extractor 1.5:

- validates the APK/ZIP32 central directory;
- rejects split ZIP/ZIP64 inputs and unsupported/encrypted entries;
- supports ZIP methods Store (0) and Deflate (8);
- verifies decompressed size and CRC-32 for extracted APK entries;
- computes SHA-256 for the source APK and extracted files;
- rejects unsafe paths, reserved Windows names, duplicate normalized names,
  file/directory collisions and unexpected non-regular entries;
- detects non-empty `res/raw/` versus `assets/` layouts;
- detects the three audited protected Android14-family codec profiles;
- maps only audited protected PAC aliases back to canonical names;
- validates protected PAC entry bounds/type evidence before publishing them;
- detects contiguous complete character triplets in the Vita `00..99`
  namespace;
- excludes APK-bundled `save.bin` from the generated profile;
- writes `dbtb_manifest.json`, `RESULTADO.json` and `SHA256SUMS.txt`;
- preserves gameplay/media payload bytes. The output ZIP itself uses Store
  entries, so the ZIP may be larger than the source APK.

The extractor does not execute Android DEX/classes and does not make a
code-changing Android mod automatically compatible with Vita.

## Safety limits

Current Web Extractor 1.0 limits are:

- APK: maximum 1 GiB;
- ZIP central directory: maximum 16 MiB;
- ZIP entries: maximum 8192;
- one extracted game-data file: maximum 64 MiB;
- extracted game data per APK: maximum 512 MiB;
- ZIP32 only.

These are deliberate validation/memory limits. Very large mods can require
considerable RAM because browsers and mobile operating systems may keep temporary
Blob/stream buffers while producing the final ZIP.

The browser must support `DecompressionStream('deflate-raw')` for deflated APK
entries. If a device/browser does not provide that API or runs out of memory,
use a current browser or Windows Extractor 1.5.

## Responsive selector UI

The site uses the same four validated Gen-derived selector assets as the Vita
00.34/v1.0 selector:

- `select0_background.png`
- `select0_header.png`
- `select0_button.png`
- `select0_ball_1.png`

They are not duplicated as loose binary files in Git. The Pages workflow runs
`tools/materialize_selector_theme.py` and places the verified PNGs into the
deployment artifact. The web background samples only the approved top
482x320 cyan/grid region, so the lower blue orb remains excluded just as in the
final Vita selector.

The CSS has dedicated desktop, narrow-phone and short landscape layouts, plus
safe-area handling for devices with display cutouts.

## Validation evidence

The core JavaScript was syntax-checked and its unit tests passed before
publication. The same browser-oriented core was then executed against real
project APK inputs and generated Vita-ready ZIPs successfully:

| APK | SHA-256 | Detected layout | Codec | Result |
|---|---|---|---|---|
| `DBTapBattle.apk` | `b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b` | `raw` | original/unknown | PASS, 57 data files |
| `gen.apk` | `d52cbd7ef248d995ad17ba6ec8ec6fa08590a344ac2a9786e5ac839bf7715f28` | `assets` | original/unknown | PASS, 146 data files, 13 characters |
| `tap battle android 14.apk` | `a210795bf7ded8636a91bea96df051557229149feb310cf07baf16b0731e79c4` | `community14` | `community14-a210795b` | PASS, 144 data files, 13 characters |
| `DBTB en español para Android 14.apk` | `b38cc2c4ae3f20d1b1c6c1419a7b6b62ab57ea8954468874f6c5f8c40b39a098` | `community14` | `community14-es-d594affc` | PASS, 144 data files, 13 characters |
| `TAP BATTLE INVASION BETA 3.apk` | `caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d` | `community14` | `community14-invasion-05aa0c5e` | PASS, 177 data files, 22 characters |
| `DragonBallZuperSamuGamerYT.apk` | `1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d` | `assets` | original/unknown | PASS, 383 data files, 92 characters |

The generated Invasion, Spanish and Samu packages were independently reopened
as ZIP archives and passed full ZIP CRC validation. Invasion reported
`profiles-v1`, 22 characters (`00..21`) and the expected protected codec.
Spanish reported `community14-es-d594affc` and 13 characters. Samu reported
92 characters (`00..91`) and correctly omitted its APK-bundled `save.bin`
from the Vita profile.

Samu is intentionally an important stress case: its generated stored ZIP is
405,409,508 bytes. The core completes successfully on the development runner,
but low-memory phones may still hit browser/OS memory limits. That is a device
limit rather than permission to silently truncate the package.

This establishes the extraction/package core against all six supplied project
APK families. Device/browser UI and memory behavior should still be reported
separately when testing specific Android/iOS browsers.

Machine-readable evidence:
[`docs/evidence/web_extractor_1.0_2026-10-07.json`](evidence/web_extractor_1.0_2026-10-07.json).

## Development validation

From the repository root:

```sh
node --check web/extractor-core.mjs
node --check web/app.mjs
node tests/web_extractor_core.mjs
python3 tools/materialize_selector_theme.py assets/selector /tmp/dbtb-web-selector
```

The GitHub Pages workflow runs the JavaScript checks and materializes the
validated selector theme before it can upload/deploy the site.

## Relationship to the Windows extractor

Windows Extractor 1.5 remains supported. The two tools intentionally share the
same `profiles-v1` output semantics, protected alias maps, save policy and
character namespace.

Use the web extractor when a phone/tablet/browser is more convenient. Use the
Windows extractor when browser compatibility/memory is a limitation or when
working with automation on a PC.
