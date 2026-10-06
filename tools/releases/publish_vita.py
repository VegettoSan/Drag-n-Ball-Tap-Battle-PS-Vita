#!/usr/bin/env python3
"""Manual full-engine release: validate provenance, stage public binaries, publish."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request
import zipfile

from download_original_apk import ORIGINAL_APK_SHA256, validate_apk

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
from validate_livearea_vpk import EXPECTED, TEMPLATE_PATH
from decode_livearea_assets import ASSETS, validate_png

NOTICES = {'notices/THIRD_PARTY.md', 'notices/vitaGL-GPL-3.0.txt', 'notices/vitaGL-LGPL-3.0.txt'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def cmake_version():
    source = (REPO / 'tools/aot/engine/vita/CMakeLists.txt').read_text()
    versions = re.findall(r'set\(VITA_VERSION "(\d{2}\.\d{2})"\)', source)
    if len(versions) != 1:
        raise ValueError('Expected one Vita APP_VER in the full-engine CMake target')
    return versions[0]


def tag_for(version, prerelease, run_id):
    if not re.fullmatch(r'\d{2}\.\d{2}', version) or not re.fullmatch(r'[1-9]\d*', str(run_id)):
        raise ValueError('Invalid version or workflow run ID')
    return f'v{version}-pre.{run_id}' if prerelease else f'v{version}'


def make_plan():
    channel = os.environ.get('PRERELEASE', '')
    if channel not in ('true', 'false'):
        raise ValueError('PRERELEASE must be true or false')
    source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    if source != os.environ.get('GITHUB_SHA'):
        raise ValueError('Checkout does not match the workflow source SHA')
    version, run_id, prerelease = cmake_version(), os.environ.get('GITHUB_RUN_ID', ''), channel == 'true'
    return {'version': version, 'tag': tag_for(version, prerelease, run_id),
            'prerelease': prerelease, 'source_commit': source,
            'source_marker': subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], cwd=REPO, text=True).strip(),
            'workflow_run_id': run_id}


def github_get(endpoint, missing_ok=False):
    repo = os.environ.get('GITHUB_REPOSITORY', '')
    token = os.environ.get('GH_TOKEN', '')
    if not re.fullmatch(r'[\w.-]+/[\w.-]+', repo) or not token:
        raise ValueError('GitHub repository/token context missing')
    request = urllib.request.Request('https://api.github.com/repos/' + repo + '/' + endpoint,
                                     headers={'Authorization': 'Bearer ' + token,
                                              'Accept': 'application/vnd.github+json',
                                              'User-Agent': 'DBTB-full-engine-release'})
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            return json.load(response)
    except urllib.error.HTTPError as exc:
        if exc.code == 404 and missing_ok:
            return None
        raise ValueError(f'GitHub API failed ({exc.code}); no publication permitted') from None


def check_target(plan):
    tag = urllib.parse.quote(plan['tag'], safe='')
    if github_get('releases/tags/' + tag, missing_ok=True) is not None:
        raise ValueError('Release/tag already exists; preserve it. Bump VITA_VERSION or use a new prerelease run')
    if github_get('git/ref/tags/' + tag, missing_ok=True) is not None:
        raise ValueError('Git tag already exists; refusing to attach a different build to it')


def parse_sfo(data):
    if len(data) < 20 or data[:4] != b'\0PSF':
        raise ValueError('Invalid param.sfo header')
    _, _, keys, values, count = struct.unpack_from('<5I', data)
    if not 20 + count * 16 <= keys <= values <= len(data):
        raise ValueError('Invalid SFO index extents')
    result = {}
    for index in range(count):
        key_offset, fmt, size, capacity, value_offset = struct.unpack_from('<HHIII', data, 20 + index * 16)
        start = keys + key_offset
        if not keys <= start < values or size > capacity or value_offset + size > len(data) - values:
            raise ValueError('Invalid SFO entry bounds')
        end = data.find(b'\0', start, values)
        if end < 0:
            raise ValueError('Unterminated SFO key')
        key = data[start:end].decode('ascii')
        if key in result:
            raise ValueError('Duplicate SFO key')
        result[key] = data[values + value_offset:values + value_offset + size].rstrip(b'\0').decode('utf-8') if fmt == 0x204 else None
    return result


def validate_engine(elf, generation_log, plan):
    binary = elf.read_bytes()
    if len(binary) < 52 or binary[:6] != b'\x7fELF\x01\x01' or struct.unpack_from('<H', binary, 18)[0] != 40:
        raise ValueError('Expected complete ARM ELF32 engine')
    for name in (b'meth_cnda_TCBManajer_Game3', b'meth_cnda_GameData_Init', b'meth_cnda_VitaEngine_main'):
        if name not in binary:
            raise ValueError('Full original-engine symbol missing; native link probes are not playable')
    if (plan['source_marker'] + '\0').encode() not in binary or (plan['version'] + '\0').encode() not in binary:
        raise ValueError('ELF version/source marker mismatch')
    log = generation_log.read_text()
    classes = re.findall(r'Classes compiled:\s*(\d+)', log)
    methods = re.findall(r'Methods compiled:\s*(\d+)', log)
    if len(classes) != 1 or len(methods) != 1 or int(classes[0]) < 400 or int(methods[0]) < 3000:
        raise ValueError('Missing full TeaVM generation evidence; refusing a stub engine')
    return {'classes': int(classes[0]), 'methods': int(methods[0])}


def validate_vpk(vpk_path, eboot_path, version):
    with zipfile.ZipFile(vpk_path) as vpk:
        names = vpk.namelist()
        allowed = set(EXPECTED) | {TEMPLATE_PATH, 'eboot.bin', 'sce_sys/param.sfo'} | NOTICES
        files = {n for n in names if not n.endswith('/')}
        if len(names) != len(set(names)) or files != allowed or vpk.testzip():
            raise ValueError('Invalid release VPK layout, CRC, duplicate entries or unexpected private/game-data files')
        eboot = vpk.read('eboot.bin')
        if not eboot.startswith(b'SCE\0') or eboot != eboot_path.read_bytes():
            raise ValueError('Packaged eboot differs from the just-built full engine')
        sfo = parse_sfo(vpk.read('sce_sys/param.sfo'))
        if sfo.get('APP_VER') != version or sfo.get('TITLE_ID') != 'DBTB00001':
            raise ValueError('VPK APP_VER/title ID mismatch')
        for path, expected in EXPECTED.items():
            name = path.rsplit('/', 1)[-1]
            if validate_png(vpk.read(path), name, ASSETS[name]) != expected:
                raise ValueError('Approved LiveArea image changed')
        if vpk.read(TEMPLATE_PATH) != (REPO / 'assets/livearea/template.xml').read_bytes():
            raise ValueError('LiveArea template differs from the source being released')
    return hashlib.sha256(eboot).hexdigest()


def package(plan, private_root, output):
    validate_apk(private_root / 'original.apk')
    build = private_root / 'vita-build/vita'
    elf = build / 'dbtb_original_engine'
    generation = validate_engine(elf, private_root / 'generated/generation.log', plan)
    vpk = build / f'DBTapBattle-Vita-{plan["version"]}.vpk'
    eboot_sha = validate_vpk(vpk, build / 'eboot.bin', plan['version'])
    if output.exists():
        raise ValueError('Use a fresh release staging directory')
    output.mkdir(parents=True)
    stem = 'DBTapBattle-Vita-' + plan['tag'][1:]
    target = output / (stem + '.vpk')
    shutil.copyfile(vpk, target)
    symbols = output / (stem + '.symbols.zip')
    with zipfile.ZipFile(symbols, 'w', zipfile.ZIP_DEFLATED) as archive:
        archive.write(elf, 'dbtb_original_engine.elf')
        archive.write(build / 'dbtb_original_engine.velf', 'eboot.velf')
    report = {**plan, 'engine': 'full-original-TeaVM', 'original_apk_sha256': ORIGINAL_APK_SHA256,
              'eboot_sha256': eboot_sha, 'elf_sha256': digest(elf), 'generation': generation,
              'toolchain': {'vitasdk': '2026.08', 'teavm': '0.12.3', 'java': '17',
                            'dex2jar': '2.4', 'ecj': '3.37.0', 'generated_c': '-O1', 'native': '-O2',
                            'docker_image': (private_root / 'sdk-image.txt').read_text().strip()},
              'artifacts': {path.name: {'sha256': digest(path), 'size_bytes': path.stat().st_size} for path in (target, symbols)},
              'hardware_scope': '00.24 checkpoint user-confirmed; this fresh CI artifact has not itself been device-tested'}
    (output / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
    (output / 'SHA256SUMS.txt').write_text(''.join(f'{digest(path)}  {path.name}\n' for path in (target, symbols, output / 'build.json')))
    notes = (f'Dragon Ball Tap Battle Vita {plan["version"]}\n\n'
             f'Full original engine, source `{plan["source_commit"]}`.\n'
             f'TeaVM generation: {generation["classes"]} classes / {generation["methods"]} methods.\n'
             'Includes the approved LiveArea and both battle-memory repairs: native PAC streaming and exact Ogg PCM allocation.\n\n'
             'Install the VPK as an update; preserve `ux0:data/DBTapBattle/` and saves. Original game data is supplied separately.\n'
             '00.24 was confirmed working on a physical Vita; this newly compiled artifact needs its own device retest.\n'
             'The symbols ZIP contains compiled ELF/VELF for crash analysis, never APK/JAR/classes/generated C or game data.\n')
    extra = os.environ.get('RELEASE_NOTES', '').strip()
    (output / 'release-notes.md').write_text(notes + ('\n' + extra + '\n' if extra else ''))
    print('Full-engine release package validated: ' + plan['tag'])


def verify_staged(directory):
    report = json.loads((directory / 'build.json').read_text())
    if report.get('engine') != 'full-original-TeaVM' or report.get('original_apk_sha256') != ORIGINAL_APK_SHA256:
        raise ValueError('Release input is not the pinned full engine')
    if not re.fullmatch(r'[0-9a-f]{40}', report.get('source_commit', '')) or type(report.get('prerelease')) is not bool:
        raise ValueError('Invalid release provenance')
    if report['tag'] != tag_for(report['version'], report['prerelease'], report['workflow_run_id']):
        raise ValueError('Release tag/channel does not match the build')
    stem = 'DBTapBattle-Vita-' + report['tag'][1:]
    expected = {stem + '.vpk', stem + '.symbols.zip', 'build.json', 'SHA256SUMS.txt', 'release-notes.md'}
    if {p.name for p in directory.iterdir()} != expected:
        raise ValueError('Unexpected file in publication directory')
    rows = (directory / 'SHA256SUMS.txt').read_text().splitlines()
    checksums = {}
    for row in rows:
        match = re.fullmatch(r'([0-9a-f]{64})  ([A-Za-z0-9_.-]+)', row)
        if not match or match[2] in checksums:
            raise ValueError('Invalid checksum manifest')
        checksums[match[2]] = match[1]
    if set(checksums) != {stem + '.vpk', stem + '.symbols.zip', 'build.json'}:
        raise ValueError('Missing release checksums')
    for name, expected_sha in checksums.items():
        if digest(directory / name) != expected_sha:
            raise ValueError('Release asset checksum mismatch: ' + name)
    if set(report['artifacts']) != {stem + '.vpk', stem + '.symbols.zip'}:
        raise ValueError('Release artifact list mismatch')
    for name, info in report['artifacts'].items():
        if info['sha256'] != checksums[name] or info['size_bytes'] != (directory / name).stat().st_size:
            raise ValueError('Release asset identity mismatch')
    with zipfile.ZipFile(directory / (stem + '.symbols.zip')) as symbols:
        if symbols.namelist() != ['dbtb_original_engine.elf', 'eboot.velf'] or symbols.testzip():
            raise ValueError('Symbols archive contains private sources or invalid entries')
    return report


def publish(directory):
    report = verify_staged(directory)
    if report['source_commit'] != os.environ.get('GITHUB_SHA') or report['workflow_run_id'] != os.environ.get('GITHUB_RUN_ID'):
        raise ValueError('Release provenance differs from this workflow run')
    if str(report['prerelease']).lower() != os.environ.get('PRERELEASE'):
        raise ValueError('Release/prerelease caller mismatch')
    check_target(report)
    tag, repo = report['tag'], os.environ['GITHUB_REPOSITORY']
    assets = [directory / n for n in report['artifacts']] + [directory / 'build.json', directory / 'SHA256SUMS.txt']
    command = ['gh', 'release', 'create', tag, '--repo', repo, '--target', report['source_commit'],
               '--title', 'Dragon Ball Tap Battle Vita ' + tag[1:], '--notes-file', str(directory / 'release-notes.md'),
               '--draft', '--latest=false']
    if report['prerelease']:
        command.append('--prerelease')
    subprocess.run(command + [str(p) for p in assets], check=True)
    # Do not make a partially uploaded release public.
    release = github_get('releases/tags/' + urllib.parse.quote(tag, safe=''))
    if not release['draft'] or release['prerelease'] != report['prerelease'] or release['target_commitish'] != report['source_commit']:
        raise ValueError('Draft identity mismatch; leaving it unpublished')
    observed = {a['name']: a['size'] for a in release['assets']}
    if observed != {p.name: p.stat().st_size for p in assets}:
        raise ValueError('Draft upload incomplete; leaving it unpublished')
    subprocess.run(['gh', 'release', 'edit', tag, '--repo', repo, '--draft=false',
                    '--prerelease=' + str(report['prerelease']).lower(),
                    '--latest=' + str(not report['prerelease']).lower()], check=True)
    released = github_get('releases/tags/' + urllib.parse.quote(tag, safe=''))
    if released['draft'] or released['prerelease'] != report['prerelease']:
        raise ValueError('Publication state mismatch; inspect the release')
    print(released['html_url'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='action', required=True)
    commands.add_parser('plan').add_argument('--output', type=Path, required=True)
    commands.add_parser('check-target').add_argument('--plan', type=Path, required=True)
    p = commands.add_parser('package')
    p.add_argument('--plan', type=Path, required=True)
    p.add_argument('--private-root', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    commands.add_parser('publish').add_argument('--directory', type=Path, required=True)
    args = parser.parse_args()
    try:
        if args.action == 'plan':
            args.output.write_text(json.dumps(make_plan(), indent=2) + '\n')
        elif args.action == 'check-target':
            check_target(json.loads(args.plan.read_text()))
        elif args.action == 'package':
            package(json.loads(args.plan.read_text()), args.private_root, args.output)
        else:
            publish(args.directory)
    except (ValueError, OSError, zipfile.BadZipFile, subprocess.CalledProcessError) as exc:
        parser.exit(1, f'::error::{exc}\n')


if __name__ == '__main__':
    main()
