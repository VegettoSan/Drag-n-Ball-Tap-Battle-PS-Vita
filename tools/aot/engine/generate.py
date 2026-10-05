#!/usr/bin/env python3
"""Generate the original core privately; this does not link/run a Vita game."""
import argparse
from pathlib import Path
import subprocess
import shutil
import zipfile


def run(args, log):
    with log.open('w') as output:
        subprocess.run([str(a) for a in args], stdout=output, stderr=subprocess.STDOUT, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-jar', type=Path, required=True)
    parser.add_argument('--ecj', type=Path, required=True)
    parser.add_argument('--lib-directory', type=Path, required=True)
    parser.add_argument('--work-directory', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    work = args.work_directory.resolve()
    repo = root.parents[2]
    if work == repo or repo in work.parents:
        parser.error('Keep generated commercial sources outside the repository')
    if work.exists():
        parser.error('Use a fresh private work directory; refusing to overwrite')
    libs = args.lib_directory.resolve()
    for name in ['teavm-cli-0.12.3.jar', 'teavm-interop-0.12.3.jar', 'asm-9.7.1.jar']:
        if not (libs / name).is_file():
            parser.error(f'Missing pinned dependency {name}')
    original, ecj = args.original_jar.resolve(), args.ecj.resolve()
    if not original.is_file() or not ecj.is_file():
        parser.error('Original JAR and ECJ must exist')
    work.mkdir(parents=True)
    generators = work / 'generators'
    run(['java', '-jar', ecj, '-8', '-d', generators, '-cp', libs / 'asm-9.7.1.jar',
         root / 'GenerateSjis.java', root / 'PatchCharsets.java'], work / 'generators-build.log')
    mapping = work / 'SjisMapping.java'
    run(['java', '-cp', generators, 'GenerateSjis', mapping], work / 'sjis-generation.log')
    charset_registry = work / 'charset-registry.jar'
    run(['java', '-cp', f'{generators}:{libs / "asm-9.7.1.jar"}', 'PatchCharsets',
         libs / 'teavm-classlib-0.12.3.jar', charset_registry], work / 'charset-registry.log')
    cp = f'{original}:{libs / "teavm-interop-0.12.3.jar"}'
    classes = work / 'classes'
    run(['java', '-jar', ecj, '-8', '-d', classes, '-cp', cp,
         *sorted((root / 'java').rglob('*.java')), mapping], work / 'java-build.log')
    adapters = work / 'adapters.jar'
    with zipfile.ZipFile(adapters, 'x', zipfile.ZIP_DEFLATED) as jar:
        for path in sorted(classes.rglob('*.class')):
            entry = zipfile.ZipInfo(path.relative_to(classes).as_posix())
            jar.writestr(entry, path.read_bytes())
    patch_classes = work / 'patch-classes'
    run(['java', '-jar', ecj, '-8', '-d', patch_classes, '-cp', libs / 'asm-9.7.1.jar',
         root / 'PatchResourceInit.java'], work / 'patch-build.log')
    patched = work / 'original-vfs.jar'
    run(['java', '-cp', f'{patch_classes}:{libs / "asm-9.7.1.jar"}',
         'PatchResourceInit', original, patched], work / 'patch.log')
    run(['java', '-cp', f'{charset_registry}:{libs}/*:{adapters}:{patched}', 'org.teavm.cli.TeaVMRunner',
         '-t', 'c', '-d', work / 'c', '--min-heap', '8', '--max-heap', '48', '--strict',
         '--', 'com.namcobandaigames.dragonballtap.apk.VitaEngine'], work / 'generation.log')
    # The patch tool verified the pinned original field is a static byte with no
    # ConstantValue. JVM default is zero. Repair this C generator defect only;
    # all gameplay methods and the original TCBManajer.class stay unchanged.
    generated = work / 'c/classes/com/namcobandaigames/dragonballtap/apk/TCBManajer.c'
    source = generated.read_text()
    declaration = 'int8_t sfld_cnda_TCBManajer_bEventFlagBuf;'
    broken = 'sfld_cnda_TCBManajer_bEventFlagBuf = ;'
    if source.count(declaration) != 1 or source.count(broken) != 1:
        raise ValueError('Unexpected pinned TeaVM byte-default shape')
    generated.write_text(source.replace(broken, 'sfld_cnda_TCBManajer_bEventFlagBuf = 0;'))
    print(f'Generated privately: {work / "c"}; native bridge linking/execution is a separate step')


if __name__ == '__main__':
    main()
