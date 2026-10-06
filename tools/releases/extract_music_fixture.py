#!/usr/bin/env python3
"""Extract only the 17 private original BGM fixtures for PCM regression tests."""
import argparse
from pathlib import Path
import zipfile
from download_original_apk import validate_apk


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    validate_apk(args.apk)
    game = args.destination / 'game'
    game.mkdir(parents=True, exist_ok=False)
    with zipfile.ZipFile(args.apk) as apk:
        for index in range(17):
            name = f'bgm_{index:02d}.ogg'
            data = apk.read('res/raw/' + name)
            if not data or len(data) > 1024 * 1024:
                raise ValueError('Private music fixture size exceeds the tested corpus budget')
            (game / name).write_bytes(data)
    print('Private music fixtures ready: 17 tracks; no data enters release assets')


if __name__ == '__main__':
    main()
