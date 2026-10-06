import hashlib
import importlib.util
import sys
import struct
import subprocess
from pathlib import Path
import tempfile
import unittest
import zipfile

sys.path.insert(0, str(Path(__file__).parents[1] / 'tools'))
import community14

spec = importlib.util.spec_from_file_location('extractor', Path(__file__).parents[1] / 'tools/extract_apk_data.py')
e = importlib.util.module_from_spec(spec)
spec.loader.exec_module(e)


class ExtractorTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.apk = self.root / 'data.apk'
        self.output = self.root / 'game'

    def tearDown(self):
        self.temp.cleanup()

    def archive(self, entries):
        with zipfile.ZipFile(self.apk, 'w') as archive:
            for name, data in entries:
                archive.writestr(name, data)

    def test_nested_unknown_and_exact_hash(self):
        self.archive([('res/raw/carpeta/Mod ñ.custom', b'\0\xfforiginal')])
        m = e.extract(self.apk, self.output)
        self.assertEqual(m['unknown_raw_files'], ['carpeta/Mod ñ.custom'])
        self.assertEqual(m['files'][0]['sha256'], hashlib.sha256(b'\0\xfforiginal').hexdigest())
        self.assertEqual((self.output / 'carpeta/Mod ñ.custom').read_bytes(), b'\0\xfforiginal')

    def test_unsafe_collision_and_duplicates_publish_nothing(self):
        for names in [['good.pac', '../escape'], ['A.pac', 'a.pac'], ['a', 'a/b'], ['C:evil'],
                      ['bad\\x'], ['dbtb_manifest.json'], ['x/./bad'], ['x//bad']]:
            with self.subTest(names=names):
                self.archive([('res/raw/' + n, b'data') for n in names])
                with self.assertRaises(ValueError):
                    e.extract(self.apk, self.output)
                self.assertFalse(self.output.exists())

    def test_manifest_and_late_conflicts_preserve_output(self):
        self.output.mkdir()
        (self.output / e.MANIFEST).write_bytes(b'old manifest')
        self.archive([('res/raw/a.pac', b'new')])
        with self.assertRaises(ValueError):
            e.extract(self.apk, self.output)
        self.assertFalse((self.output / 'a.pac').exists())
        self.assertEqual((self.output / e.MANIFEST).read_bytes(), b'old manifest')
        e.extract(self.apk, self.output, True)
        self.assertEqual((self.output / 'a.pac').read_bytes(), b'new')

    def test_symlink_rejected(self):
        self.output.mkdir()
        elsewhere = self.root / 'outside'
        elsewhere.mkdir()
        (self.output / 'nested').symlink_to(elsewhere, target_is_directory=True)
        self.archive([('res/raw/nested/a.pac', b'new')])
        with self.assertRaises(ValueError):
            e.extract(self.apk, self.output, True)
        self.assertFalse((elsewhere / 'a.pac').exists())

    def test_crc_failure_does_not_publish_earlier_files(self):
        self.archive([('res/raw/a.pac', b'good'), ('res/raw/z.pac', b'unique-bad-content')])
        data = self.apk.read_bytes().replace(b'unique-bad-content', b'corruptbad-content')
        self.apk.write_bytes(data)
        with self.assertRaises(zipfile.BadZipFile):
            e.extract(self.apk, self.output)
        self.assertFalse(self.output.exists())


    def encoded_pac(self):
        c = community14
        # One real encoded BIN-type table entry; payload stays byte-identical.
        return (struct.pack('<HII', 1 ^ c.COUNT_XOR, c.OFFSET_XOR, 3 ^ c.SIZE_XOR)
                + struct.pack('>I', (0x8f230d0d ^ c.TYPE_XOR)) + bytes(4) + b'abc')

    def encoded_pac_profile(self, profile):
        raw_bin = next(key for key, kind in profile.type_keys.items() if kind == 'bin')
        return (struct.pack('<HII', 1 ^ profile.count_xor,
                            profile.offset_xor, 3 ^ profile.size_xor)
                + struct.pack('>I', raw_bin) + bytes(4) + b'abc')

    def test_spanish_and_invasion_profiles_auto_detect_and_canonicalize(self):
        cases = [
            (community14.SPANISH, '4D7F.pac', 'common.pac',
             'F29821.pac', 'char21.pac'),
            (community14.INVASION, '9036.pac', 'common.pac',
             '095321.pac', 'char21.pac'),
        ]
        for profile, common_alias, common_name, char_alias, char_name in cases:
            with self.subTest(profile=profile.name):
                apk = self.root / (profile.name + '.apk')
                output = self.root / ('out-' + profile.name)
                data = self.encoded_pac_profile(profile)
                with zipfile.ZipFile(apk, 'w') as archive:
                    archive.writestr('assets/' + common_alias, data)
                    archive.writestr('assets/' + char_alias, data)
                manifest = e.extract(apk, output)
                self.assertEqual(manifest['source_layout'], 'community14')
                self.assertEqual(manifest['pac_codec'], profile.name)
                self.assertEqual((output / common_name).read_bytes(), data)
                self.assertEqual((output / char_name).read_bytes(), data)
                self.assertEqual(community14.detect_profile(data), profile)

    def test_encoded_import_preserves_payload_and_maps_all_name_families(self):
        names = {'2752.pac': 'common.pac', '1BC2.pac': 'select0.pac',
                 '0B4903.pac': 'back03.pac', 'BDC701.pac': 'bobj01.pac',
                 'E03B12.pac': 'char12.pac', '8AC112.pac': 'chardemo12.pac',
                 'FAFD0012.pac': 'charf0012.pac', '47DD050.pac': 'card050.pac'}
        data = self.encoded_pac()
        self.archive([('assets/' + n, data) for n in names])
        m = e.extract(self.apk, self.output)
        self.assertEqual(m['source_layout'], 'community14')
        self.assertEqual(len(m['renamed_files']), len(names))
        for logical in names.values():
            self.assertEqual((self.output / logical).read_bytes(), data)
        self.assertEqual(m['file_count'], len(names))

    def test_alias_collision_bad_codec_and_unsafe_assets_publish_nothing(self):
        data = self.encoded_pac()
        for entries in [[('assets/2752.pac', data), ('assets/common.pac', data)],
                        [('assets/2752.pac', data), ('assets/47DD050.pac', b'wrong profile')],
                        [('assets/../outside.pac', data)],
                        [('assets/dbtb_manifest.json/x', b'data')],
                        [('assets/A.pac', data), ('assets/a.pac', data)]]:
            with self.subTest(entries=[n for n, _ in entries]):
                self.archive(entries)
                with self.assertRaises(ValueError):
                    e.extract(self.apk, self.output)
                self.assertFalse(self.output.exists())

    def test_mixed_layout_requires_choice_and_plain_assets_remain_untouched(self):
        self.archive([('res/raw/common.pac', b'raw'), ('assets/common.pac', b'asset')])
        with self.assertRaisesRegex(ValueError, 'ambiguous'):
            e.extract(self.apk, self.output)
        m = e.extract(self.apk, self.output, layout='assets')
        self.assertEqual(m['source_layout'], 'assets')
        self.assertEqual((self.output / 'common.pac').read_bytes(), b'asset')
        self.assertEqual(m['renamed_files'], [])

    def test_empty_raw_stubs_auto_select_real_assets(self):
        self.archive([('res/raw/common.pac', b''), ('res/raw/text00.pac', b''),
                      ('assets/common.pac', b'asset common'), ('assets/text00.pac', b'asset text')])
        m = e.extract(self.apk, self.output)
        self.assertEqual(m['source_layout'], 'assets')
        self.assertEqual(m['file_count'], 2)
        self.assertEqual((self.output / 'common.pac').read_bytes(), b'asset common')
        self.assertEqual((self.output / 'text00.pac').read_bytes(), b'asset text')
        self.assertIn('res/raw/common.pac', m['not_extracted'])
        self.assertIn('res/raw/text00.pac', m['not_extracted'])

    def test_cli_mod_never_replaces_original(self):
        self.output.mkdir()
        original = self.output / 'game'
        original.mkdir()
        (original / 'common.pac').write_bytes(b'original marker')
        self.archive([('assets/2752.pac', self.encoded_pac())])
        tool = Path(__file__).parents[1] / 'tools/extract_apk_data.py'
        result = subprocess.run([sys.executable, str(tool), str(self.apk), str(self.output),
                                 '--mod', 'Android14', '--overwrite'], capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((original / 'common.pac').read_bytes(), b'original marker')
        self.assertEqual((self.output / 'mods/Android14/common.pac').read_bytes(), self.encoded_pac())
        result = subprocess.run([sys.executable, str(tool), str(self.apk), str(self.output),
                                 '--mod', '../game'], capture_output=True)
        self.assertEqual(result.returncode, 2)
        self.assertEqual((original / 'common.pac').read_bytes(), b'original marker')


if __name__ == '__main__':
    unittest.main()
