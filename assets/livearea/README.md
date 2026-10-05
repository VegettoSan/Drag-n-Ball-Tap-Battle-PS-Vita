# Dragon Ball Tap Battle LiveArea

Approved LiveArea art extracted from the supplied `gen.apk` and adapted for PS Vita.

Packaging contract:

- `icon0.png`: 128x128, indexed 8-bit PNG, opaque, <= 128 KiB.
- `bg0.png`: 840x500, indexed 8-bit PNG, opaque, <= 128 KiB.
- `startup.png`: 280x158, indexed 8-bit PNG, transparency allowed, <= 128 KiB.
- `pic0.png`: 960x544, indexed 8-bit PNG, opaque, <= 1024 KiB.
- `template.xml`: minimal `style="a1"` layout with `bg0.png` and a `startup.png` gate.

The gate uses the original Dragon Ball Tap Battle logo as the launch image over the
Shenlong background, matching the visual language of the game's original startup screen.

## Repository representation

The approved PNG bytes are stored under `encoded/*.png.b64`. This is only a
transport representation: `tools/decode_livearea_assets.py` reconstructs the exact
PNG bytes into the build directory before `vita_create_vpk` runs. It checks SHA-256,
dimensions, indexed PNG format, palette/transparency requirements and size limits, and
CMake fails closed if any check fails.

Do not resize, recompress or re-encode the approved assets during packaging.
`tools/validate_livearea_vpk.py` verifies the final VPK paths and exact PNG hashes.
