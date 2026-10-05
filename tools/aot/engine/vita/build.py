#!/usr/bin/env python3
"""Build the complete Vita engine from a private TeaVM C output directory.

The generated commercial C is copied to the private build directory, patched
there, and never written into the repository.
"""
import argparse
from pathlib import Path
import os
import shutil
import subprocess
import sys


def run(command):
    print('+', ' '.join(str(x) for x in command), flush=True)
    subprocess.run([str(x) for x in command], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generated-c', type=Path, required=True,
                        help='raw TeaVM 0.12.3 C directory containing all.c')
    parser.add_argument('--build-directory', type=Path, required=True,
                        help='fresh private directory outside the repository')
    parser.add_argument('--jobs', type=int, default=max(1, min(4, os.cpu_count() or 1)))
    args = parser.parse_args()

    here = Path(__file__).resolve().parent
    repo = here.parents[3]
    source = args.generated_c.resolve()
    build = args.build_directory.resolve()
    if not (source / 'all.c').is_file():
        parser.error('--generated-c must contain TeaVM all.c')
    if build == repo or repo in build.parents:
        parser.error('Keep generated commercial sources/build products outside the repository')
    if build.exists():
        parser.error('Use a fresh --build-directory; refusing to overwrite')
    if args.jobs < 1 or args.jobs > 32:
        parser.error('--jobs must be between 1 and 32')
    if not (os.environ.get('VITASDK') or shutil.which('arm-vita-eabi-gcc')):
        parser.error('VitaSDK is not selected; set VITASDK and PATH first')

    private_c = build / 'private-teavm-c'
    cmake_build = build / 'vita'
    build.mkdir(parents=True)
    shutil.copytree(source, private_c)
    run([sys.executable, here / 'patch_runtime.py', private_c])
    run(['cmake', '-S', here, '-B', cmake_build, '-DCMAKE_BUILD_TYPE=Release',
         f'-DTEAVM_C_DIR={private_c}'])
    run(['cmake', '--build', cmake_build, f'-j{args.jobs}'])

    vpk = cmake_build / 'DBTapBattle-Vita-00.03.vpk'
    if not vpk.is_file() or vpk.stat().st_size == 0:
        raise RuntimeError('build completed without the expected VPK')
    print('VPK:', vpk)
    print('Private TeaVM copy retained only in:', private_c)


if __name__ == '__main__':
    main()
