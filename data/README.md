# Local Game Data

No original Dragon Ball Tap Battle assets are stored in this repository.

Use:

```bash
python3 tools/extract_apk_data.py /path/to/your/DBTapBattle.apk ./original-data
```

Then copy the extracted contents to the Vita:

```text
ux0:data/DBTapBattle/game/
```

Mods belong in separate folders:

```text
ux0:data/DBTapBattle/mods/<ModName>/
```

Do not commit extracted original game data to this repository.
