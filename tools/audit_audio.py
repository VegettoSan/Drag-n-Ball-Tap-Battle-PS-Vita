#!/usr/bin/env python3
"""Probe user-owned APK Ogg streams with ffprobe; save metadata only."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import zipfile


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('apk', type=Path)
    p.add_argument('report', type=Path)
    args = p.parse_args()
    report = {}
    with zipfile.ZipFile(args.apk) as archive:
        for name in sorted(archive.namelist()):
            if not name.endswith('.ogg'):
                continue
            with tempfile.NamedTemporaryFile(suffix='.ogg') as file:
                file.write(archive.read(name));file.flush()
                data = json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams',
                    '-show_format','-of','json',file.name],text=True))
                stream = data['streams'][0]
                report[name] = {k: stream[k] for k in ('codec_name','sample_rate','channels')}
                report[name]['duration'] = data['format']['duration']
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f'{len(report)} audio streams probed')


if __name__ == '__main__':
    main()
