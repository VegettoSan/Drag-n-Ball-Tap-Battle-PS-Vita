# Dragon Ball Tap Battle LiveArea

Approved LiveArea art extracted from the supplied `gen.apk` and adapted for PS Vita.

Packaging contract:

- `icon0.png`: 128x128, indexed PNG (P), opaque, <= 128 KiB.
- `bg0.png`: 840x500, indexed PNG (P), opaque, <= 128 KiB.
- `startup.png`: 280x158, indexed PNG (P), transparency allowed, <= 128 KiB.
- `pic0.png`: 960x544, indexed PNG (P), opaque, <= 1024 KiB.
- `template.xml`: verified minimal `style="a1"` gate layout.

The gate uses the original Dragon Ball Tap Battle logo as `startup.png` over the Shenlong background, matching the visual language of the game's original startup screen. Do not resize or re-encode these assets during VPK packaging; package the validated bytes directly.
