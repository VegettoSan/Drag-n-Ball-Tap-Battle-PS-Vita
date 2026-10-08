#!/usr/bin/env python3
"""Test Vita pointers against privately supplied original Controller/KeyData."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--original-jar', required=True, type=Path)
parser.add_argument('--ecj', required=True, type=Path)
parser.add_argument('--adapter-classes', required=True, type=Path,
                    help='Java classes produced by engine/generate.py')
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
with tempfile.TemporaryDirectory(prefix='dbtb-controls-probe-') as directory:
    subprocess.run(['java', '-jar', str(args.ecj.resolve()), '-8', '-cp',
                    str(args.original_jar.resolve()), '-d', directory,
                    str(root / 'java/com/namcobandaigames/dragonballtap/apk/VitaControls.java'),
                    str(root / 'tests/VitaControlsProbe.java')], check=True)
    subprocess.run(['java', '-cp', ':'.join([directory, str(args.adapter_classes.resolve()),
                    str(args.original_jar.resolve())]),
                    'com.namcobandaigames.dragonballtap.apk.VitaControlsProbe'], check=True)
