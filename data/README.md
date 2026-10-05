# Local game data — current profile contract

No Dragon Ball Tap Battle asset dataset is stored here. Prepare user-owned data
outside tracked source, then copy it to the Vita. See
[DATA_LAYOUT](../docs/DATA_LAYOUT.md), [BUILD](../docs/BUILD.md) and
[CURRENT_STATUS](../docs/CURRENT_STATUS.md).

```sh
# Original APK: 57 resources, but no downloaded character triplets.
python3 tools/extract_apk_data.py /private/DBTapBattle.apk /private/install/game
# Gen APK: populated ordinary assets, empty res/raw stubs, includes characters.
python3 tools/extract_apk_data.py /private/gen.apk /private/install-gen/game
# Community14: encoded assets in an isolated profile, base fallback retained.
python3 tools/extract_apk_data.py /private/community.apk /private/install --mod Android14
```

Run these commands from the repository root. The selector's Original slot reads
`ux0:data/DBTapBattle/game/`; mods read `mods/<Profile>/` with file-level fallback.
Only the selected dataset's `save.bin` is writable; there is no shared-save
fallback. Extraction preserves bundled saves, so back up existing progress before
copying a dataset. VPK updates do not require overwriting data/saves.

The supplied Android14 and Gen datasets each have 13 indexed character triplets;
that count does not certify arbitrary mod mechanics. Never commit extracted
resources, APKs, user saves or generated commercial core artifacts. A readme or
manifest in this directory is documentation, not a downloadable game installation.
