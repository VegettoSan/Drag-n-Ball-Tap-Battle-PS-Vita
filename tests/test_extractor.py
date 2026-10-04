import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile

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


if __name__ == '__main__':
    unittest.main()
