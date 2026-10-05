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

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
