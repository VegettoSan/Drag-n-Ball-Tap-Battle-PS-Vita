# Dragon Ball Tap Battle LiveArea

<!-- DBTB_DOC_STATUS:START -->
> **Current public release:** v1.0 / APP_VER `01.00` / TITLE_ID `DBTB01178`.
> Gameplay/presentation baseline: hardware-confirmed 00.34. This file may also
> describe earlier component validation; see
> [CURRENT_STATUS](../../docs/CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


Approved LiveArea art extracted from the supplied `gen.apk` and adapted for PS Vita.

Packaging contract:

- `icon0.png`: 128x128, indexed 8-bit PNG, opaque, <= 128 KiB.
- `bg0.png`: 840x500, indexed 8-bit PNG, opaque, <= 128 KiB.
- `startup.png`: 280x158, indexed 8-bit PNG, transparency allowed, <= 128 KiB.
- `pic0.png`: 960x544, indexed 8-bit PNG, **exactly 256 palette entries**, opaque, <= 1024 KiB.
- `template.xml`: minimal `style="a1"` layout with `bg0.png` and a `startup.png` gate.

The gate uses the original Dragon Ball Tap Battle logo as the launch image over the
Shenlong background, matching the visual language of the game's original startup screen.

## Repository representation

The approved PNG bytes are stored under `encoded/*.png.b64`. This is only a
transport representation: `tools/decode_livearea_assets.py` reconstructs the exact
PNG bytes into the build directory before `vita_create_vpk` runs. It checks SHA-256,
dimensions, indexed PNG format, palette/transparency requirements and size limits, and
CMake fails closed if any check fails.

The supplied Ready ZIP and failed Final VPK had a 192-entry `pic0.png` palette.
The corrected source pads that PLTE to 256 entries; every decoded RGBA pixel and every IDAT byte is unchanged. This repairs a documented Vita splash requirement that the earlier hash/header checks missed. The corrected LiveArea was subsequently installed and accepted on physical Vita and remains packaged in the 00.34 hardware baseline and v1.0 release.
Reproduce the lossless correction with:

```sh
python3 tools/normalize_livearea_palette.py /path/to/old/pic0.png /path/to/fixed/pic0.png
```

MetalSyntax toolkit reference: `MetalSyntax/psvita-port-toolkit-cli` commit
`516e612b7470496a878cebb130da80361326bc44`, `psvita_toolkit/livearea.py` and
`docs/dev-notes/livearea.md`. Dimensions, file limits and the minimal a1 gate match
that toolkit. Its generic mode=P check alone does not require a 256-entry palette;
the splash-specific rule comes from
https://gist.github.com/Hammerill/64411eebf071b93396b7d310ba8d6776 .
The XML content revision is now 2 to identify the corrected presentation payload.

Do not resize, recompress or re-encode the corrected assets during packaging.
`tools/validate_livearea_vpk.py` verifies the final VPK paths and exact PNG hashes.
