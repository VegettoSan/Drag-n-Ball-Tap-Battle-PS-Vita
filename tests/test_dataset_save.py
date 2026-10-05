import importlib.util
import sys
from pathlib import Path
import tempfile
import unittest
import zipfile

sys.path.insert(0, str(Path(__file__).parents[1] / 'tools'))
spec = importlib.util.spec_from_file_location('extractor', Path(__file__).parents[1] / 'tools/extract_apk_data.py')
extractor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(extractor)


class DatasetSaveTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.apk = self.root / 'profile.apk'
        self.output = self.root / 'profile'

    def tearDown(self):
        self.temp.cleanup()

    def write_apk(self, entries):
        with zipfile.ZipFile(self.apk, 'w') as archive:
            for name, data in entries:
                archive.writestr(name, data)

    def test_assets_save_is_preserved_byte_for_byte(self):
        save = bytes((i * 37 + 11) & 0xff for i in range(12906))
        self.write_apk([
            ('res/raw/common.pac', b''),
            ('assets/common.pac', b'profile data'),
            ('assets/save.bin', save),
        ])
        manifest = extractor.extract(self.apk, self.output)
        self.assertEqual(manifest['source_layout'], 'assets')
        self.assertEqual((self.output / 'save.bin').read_bytes(), save)
        self.assertIn('save.bin', [entry['name'] for entry in manifest['files']])

    def test_profile_without_save_does_not_invent_one(self):
        self.write_apk([('assets/common.pac', b'profile data')])
        extractor.extract(self.apk, self.output, layout='assets')
        self.assertFalse((self.output / 'save.bin').exists())


if __name__ == '__main__':
    unittest.main()
