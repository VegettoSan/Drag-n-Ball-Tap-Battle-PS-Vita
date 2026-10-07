"""Exercise the actual Windows launcher/PowerShell tool using synthetic ZIPs.

No commercial assets are uploaded to CI. DBTB_POWERSHELL may point to pwsh on
Linux for extra real-APK checks; Windows CI uses built-in Windows PowerShell 5.1.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import community14
TOOL = ROOT / 'tools/windows/Extraer_APK_para_Vita.ps1'
BAT = ROOT / 'tools/windows/Extraer_APK_para_Vita.bat'
PS = os.environ.get('DBTB_POWERSHELL') or shutil.which('powershell') or shutil.which('pwsh')


def encoded_pac():
    payload = b'synthetic-payload'
    tag = ((-1893528307) & 0xffffffff) ^ ((-982916625) & 0xffffffff)
    return (struct.pack('<HII', 1 ^ 42802, 996678763, len(payload) ^ 47633006)
            + struct.pack('>I', tag) + b'\x00' * 4 + payload)


def encoded_pac_profile(profile):
    payload = b'synthetic-payload'
    raw_bin = next(key for key, kind in profile.type_keys.items() if kind == 'bin')
    return (struct.pack('<HII', 1 ^ profile.count_xor, profile.offset_xor,
                        len(payload) ^ profile.size_xor)
            + struct.pack('>I', raw_bin) + b'\x00' * 4 + payload)


def make_apk(path, files):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_STORED) as z:
        for name, data in files:
            if isinstance(name, zipfile.ZipInfo):
                z.writestr(name, data)
            else:
                z.writestr(name, data)
    return path


@unittest.skipUnless(PS, 'PowerShell is required')
class WindowsExtractorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='dbtb windows & ! ')
        self.root = Path(self.temp.name)
        self.output = self.root / 'salida con espacios [1]'

    def tearDown(self):
        self.temp.cleanup()

    def run_tool(self, *paths, expect=0, bat=False):
        env = dict(os.environ, DBTB_ARG_COUNT=str(len(paths)), DBTB_OUTPUT=str(self.output),
                   DBTB_NO_OPEN='1', DBTB_NO_PAUSE='1')
        for index, path in enumerate(paths):
            env[f'DBTB_APK_{index}'] = str(path)
        if bat:
            # Test cmd.exe argument transport, including spaces, &, ! and %.
            # BAT writes beside itself. Use an isolated copy to protect source.
            local = self.root / 'tool & ! % con espacios'
            local.mkdir()
            shutil.copy2(BAT, local / BAT.name)
            shutil.copy2(TOOL, local / TOOL.name)
            # Pass a raw CreateProcess command line: list2cmdline follows CRT
            # escaping rules, which do not match cmd.exe's nested /c quoting.
            # Expand the quoted env paths once, preserving literal % and !.
            env['DBTB_TEST_LAUNCHER'] = str(local / BAT.name)
            for index, path in enumerate(paths):
                env[f'DBTB_TEST_APK_{index}'] = str(path)
            args = ('cmd.exe /d /s /v:off /c ""%DBTB_TEST_LAUNCHER%" '
                    + ' '.join(f'"%DBTB_TEST_APK_{i}%"' for i in range(len(paths))) + '"')
            self.output = local / 'Listo_para_Vita'
        else:
            args = [PS, '-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                    '-File', str(TOOL), '-FromLauncher', '-NoOpen']
        result = subprocess.run(args, env=env, capture_output=True, timeout=90)
        self.assertEqual(result.returncode, expect, result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace'))
        self.assertEqual(list(self.output.glob('.extrayendo_*')), [])
        return sorted(self.output.glob('Paquete_*'))

    def apk(self, name='original', files=None):
        return make_apk(self.root / (name + '.apk'), files or [('res/raw/common.pac', b'original')])

    def manifest(self, package, profile='mods/original'):
        return json.loads((package / 'data/DBTapBattle' / profile / 'dbtb_manifest.json').read_text('utf-8'))

    def test_original_bytes_unknown_nested_save_and_checksums(self):
        source = self.apk(files=[('res/raw/common.pac', b'original'), ('res/raw/nested/new.xyz', b'unknown'),
                                 ('res/raw/save.bin', b'progress'), ('classes.dex', b'NOT EXTRACTED')])
        package, = self.run_tool(source)
        m = self.manifest(package)
        self.assertEqual(m['file_count'], 2)
        self.assertEqual(m['unknown_files'], ['nested/new.xyz'])
        self.assertTrue(m['bundled_save'])
        self.assertFalse((package / 'data/DBTapBattle/mods/original/save.bin').exists())
        self.assertFalse(m['profile_save_installed'])
        self.assertEqual(m['save_policy'], 'global-vpk-seed-ux0-root')
        self.assertFalse(list(package.rglob('classes.dex')))
        for line in (package / 'SHA256SUMS.txt').read_text().splitlines():
            digest, name = line.split('  ', 1)
            self.assertEqual(hashlib.sha256((package / name).read_bytes()).hexdigest(), digest)
        with zipfile.ZipFile(source) as z:
            for file in m['files']:
                self.assertEqual((package / 'data/DBTapBattle/mods/original' / file['name']).read_bytes(), z.read(file['apk_path']))

    def test_android14_aliases_preserve_encoded_pac(self):
        data = encoded_pac()
        source = self.apk('android14', [('assets/2752.pac', data), ('assets/E03B00.pac', data), ('assets/FAFD0000.pac', data)])
        package, = self.run_tool(source)
        m = self.manifest(package, 'mods/Android14')
        self.assertEqual(m['source_layout'], 'community14')
        self.assertEqual(m['pac_codec'], 'community14-a210795b')
        self.assertEqual(len(m['renamed_files']), 3)
        self.assertEqual((package / 'data/DBTapBattle/mods/Android14/char00.pac').read_bytes(), data)

    def test_spanish_and_invasion_alias_profiles(self):
        cases = [
            (community14.SPANISH, 'spanish', '4D7F.pac', 'F29821.pac'),
            (community14.INVASION, 'invasion', '9036.pac', '095321.pac'),
        ]
        for profile, stem, common_alias, char_alias in cases:
            with self.subTest(profile=profile.name):
                data = encoded_pac_profile(profile)
                source = self.apk(stem, [('assets/' + common_alias, data),
                                         ('assets/' + char_alias, data)])
                packages = self.run_tool(source)
                package = packages[-1]
                report = json.loads((package / 'RESULTADO.json').read_text('utf-8'))
                selected = report['profiles'][0]['profile']
                m = self.manifest(package, selected)
                self.assertEqual(m['source_layout'], 'community14')
                self.assertEqual(m['pac_codec'], profile.name)
                root = package / 'data/DBTapBattle' / selected
                self.assertEqual((root / 'common.pac').read_bytes(), data)
                self.assertEqual((root / 'char21.pac').read_bytes(), data)

    def test_assets_with_empty_raw_stubs(self):
        source = self.apk('assets', [('res/raw/common.pac', b''), ('assets/common.pac', b'original')])
        package, = self.run_tool(source)
        m = self.manifest(package, 'mods/assets')
        self.assertEqual(m['source_layout'], 'assets')

    def test_multi_apk_profile_collisions_and_duplicate_input(self):
        first = self.apk('first')
        second = self.apk('second')
        mod1 = self.apk('mod.one', [('assets/common.pac', b'a')])
        mod2 = self.apk('mod!one', [('assets/common.pac', b'b')])
        package, = self.run_tool(first, second, mod1, mod2, first)
        profiles = json.loads((package / 'RESULTADO.json').read_text())['profiles']
        self.assertEqual([p['profile'] for p in profiles], ['mods/first', 'mods/second', 'mods/mod_one', 'mods/mod_one_2'])

    def test_repeated_import_preserves_previous_package(self):
        source = self.apk()
        previous, = self.run_tool(source)
        sentinel = previous / 'mi_archivo.txt'
        sentinel.write_text('retain')
        self.assertEqual(len(self.run_tool(source)), 2)
        self.assertEqual(sentinel.read_text(), 'retain')

    def test_dynamic_gen_style_roster_70_is_preserved_and_counted(self):
        files = [('assets/common.pac', b'common')]
        for i in range(70):
            files += [
                (f'assets/char{i:02d}.pac', b'\x00\x00'),
                (f'assets/chardemo{i:02d}.pac', b'\x00\x00'),
                (f'assets/charf{i:04d}.pac', b'\x00\x00'),
            ]
        source = self.apk('gen70', files)
        package, = self.run_tool(source)
        m = self.manifest(package, 'mods/gen70')
        self.assertEqual(m['format'], 4)
        self.assertEqual(m['character_count'], 70)
        self.assertEqual(m['character_indices'], '00..69')
        self.assertTrue(m['character_runtime_compatible'])
        self.assertTrue(m['standalone_profile'])
        self.assertFalse(m['requires_game_directory'])
        self.assertTrue((package / 'data/DBTapBattle/mods/gen70/char69.pac').is_file())
        self.assertFalse((package / 'data/DBTapBattle/game').exists())

    def test_raw_mod_is_standalone_not_game(self):
        source = self.apk('rawmod', [('res/raw/common.pac', b'raw')])
        package, = self.run_tool(source)
        m = self.manifest(package, 'mods/rawmod')
        self.assertTrue(m['standalone_profile'])
        self.assertEqual(m['vita_profile'], 'mods/rawmod')
        self.assertFalse((package / 'data/DBTapBattle/game').exists())

    def rejected(self, files):
        source = self.apk(files=files)
        self.assertEqual(self.run_tool(source, expect=2), [])

    def test_traversal_device_name_and_file_directory_collision(self):
        for name in ('../escape.bin', 'CON.pac', 'nested/NUL.txt', 'bad:.pac', 'tail./file', 'dbtb_manifest.json'):
            with self.subTest(name=name):
                self.rejected([('res/raw/common.pac', b'a'), ('res/raw/' + name, b'b')])
        self.rejected([('res/raw/common.pac', b'a'), ('res/raw/a', b'b'), ('res/raw/A/child', b'c')])
        self.assertFalse((self.root / 'escape.bin').exists())

    def test_case_and_alias_duplicates(self):
        self.rejected([('res/raw/common.pac', b'a'), ('res/raw/Common.pac', b'b')])
        self.rejected([('assets/2752.pac', encoded_pac()), ('assets/common.pac', encoded_pac())])

    def test_symlink_entry(self):
        info = zipfile.ZipInfo('res/raw/link')
        info.create_system = 3
        info.external_attr = 0o120777 << 16
        self.rejected([('res/raw/common.pac', b'a'), (info, b'../../escape')])

    def test_ambiguous_missing_common_and_unsupported_codec(self):
        self.rejected([('res/raw/common.pac', b'a'), ('assets/common.pac', b'b')])
        self.rejected([('assets/unrelated.bin', b'b')])
        self.rejected([('assets/2752.pac', b'unsupported')])

    def test_bad_crc_leaves_no_partial_package(self):
        source = self.apk(files=[('res/raw/common.pac', b'first-good'), ('res/raw/last.bin', b'last-data')])
        with zipfile.ZipFile(source) as z:
            offset = z.getinfo('res/raw/last.bin').header_offset
        data = bytearray(source.read_bytes())
        name_len, extra_len = struct.unpack_from('<HH', data, offset + 26)
        data[offset + 30 + name_len + extra_len] ^= 1
        source.write_bytes(data)
        self.assertEqual(self.run_tool(source, expect=2), [])

    def test_failed_second_apk_rolls_back_whole_batch(self):
        good = self.apk('good')
        bad = self.apk('bad', [('assets/2752.pac', b'unsupported')])
        self.assertEqual(self.run_tool(good, bad, expect=2), [])

    def test_oversized_entry(self):
        source = self.apk()
        data = bytearray(source.read_bytes())
        pos = data.index(b'PK\x01\x02')
        struct.pack_into('<I', data, pos+24, 64 * 1024 * 1024 + 1)
        source.write_bytes(data)
        self.assertEqual(self.run_tool(source, expect=2), [])

    @unittest.skipUnless(os.name == 'nt', 'cmd.exe transport requires Windows')
    def test_actual_bat_launcher_special_character_paths(self):
        source = self.apk('original & ! % [1]')
        package, = self.run_tool(source, bat=True)
        profiles = json.loads((package / 'RESULTADO.json').read_text())['profiles']
        self.assertEqual(len(profiles), 1)
        self.assertEqual(self.manifest(package, profiles[0]['profile'])['file_count'], 1)


if __name__ == '__main__':
    unittest.main()
