"""Publication ownership/provenance regressions, no GitHub writes or private APK."""
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools/releases'))
import publish_vita as release
import download_original_apk as source


def stage(directory, prerelease=False):
    tag = release.tag_for('00.24', prerelease, '123')
    stem = 'DBTapBattle-Vita-' + tag[1:]
    vpk = directory / (stem + '.vpk')
    vpk.write_bytes(b'validated VPK fixture')
    symbols = directory / (stem + '.symbols.zip')
    with zipfile.ZipFile(symbols, 'w') as z:
        z.writestr('dbtb_original_engine.elf', b'compiled ELF fixture')
        z.writestr('eboot.velf', b'compiled VELF fixture')
    report = {'version': '00.24', 'tag': tag, 'prerelease': prerelease,
              'source_commit': 'a' * 40, 'workflow_run_id': '123',
              'engine': 'full-original-TeaVM', 'original_apk_sha256': source.ORIGINAL_APK_SHA256,
              'artifacts': {p.name: {'sha256': release.digest(p), 'size_bytes': p.stat().st_size} for p in (vpk, symbols)}}
    (directory / 'build.json').write_text(json.dumps(report))
    (directory / 'release-notes.md').write_text('fixture notes')
    (directory / 'SHA256SUMS.txt').write_text(''.join(f'{release.digest(p)}  {p.name}\n' for p in (vpk, symbols, directory / 'build.json')))
    return report


class PublicationTests(unittest.TestCase):
    def test_release_and_prerelease_names_and_version_bounds(self):
        self.assertEqual(release.tag_for('00.24', False, '123'), 'v00.24')
        self.assertEqual(release.tag_for('00.24', True, '123'), 'v00.24-pre.123')
        for version, run in [('24', '123'), ('00.24;echo', '123'), ('00.24', '0'), ('00.24', '../x')]:
            with self.assertRaises(ValueError):
                release.tag_for(version, True, run)

    def test_wrong_apk_and_non_https_refused_without_network(self):
        with tempfile.TemporaryDirectory() as tmp:
            apk = Path(tmp) / 'original.apk'
            apk.write_bytes(b'other APK / HTML download page')
            with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
                source.validate_apk(apk)
            with patch.object(source.urllib.request, 'build_opener') as network:
                with self.assertRaises(ValueError):
                    source.download('http://private.example/file?secret=hidden', Path(tmp) / 'output.apk')
                network.assert_not_called()

    def test_invalid_native_probe_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            elf, log = Path(tmp) / 'probe.elf', Path(tmp) / 'generation.log'
            header = bytearray(52); header[:6] = b'\x7fELF\x01\x01';struct.pack_into('<H', header, 18, 40)
            elf.write_bytes(header + b'main\0native link probe\0')
            log.write_text('INFO: Classes compiled: 467\nINFO: Methods compiled: 4086\n')
            with self.assertRaisesRegex(ValueError, 'symbol missing'):
                release.validate_engine(elf, log, {'source_marker': 'abcdefg', 'version': '00.24'})

    def test_stub_generation_evidence_and_wrong_source_marker_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            elf, log = Path(tmp) / 'engine.elf', Path(tmp) / 'generation.log'
            header = bytearray(52); header[:6] = b'\x7fELF\x01\x01';struct.pack_into('<H', header, 18, 40)
            elf.write_bytes(header + b'meth_cnda_TCBManajer_Game3\0meth_cnda_GameData_Init\0meth_cnda_VitaEngine_main\0abcdefg\0' + b'00.24\0')
            log.write_text('INFO: Classes compiled: 1\nINFO: Methods compiled: 1\n')
            with self.assertRaisesRegex(ValueError, 'generation evidence'):
                release.validate_engine(elf, log, {'source_marker': 'abcdefg', 'version': '00.24'})
            with self.assertRaisesRegex(ValueError, 'source marker mismatch'):
                release.validate_engine(elf, log, {'source_marker': '1234567', 'version': '00.24'})

    def test_corrupt_sfo_and_private_vpk_entries_refused(self):
        with self.assertRaises(ValueError):
            release.parse_sfo(b'bad')
        with tempfile.TemporaryDirectory() as tmp:
            vpk = Path(tmp) / 'wrong.vpk'
            with zipfile.ZipFile(vpk, 'w') as z:
                z.writestr('original.apk', b'private')
            with self.assertRaisesRegex(ValueError, 'layout'):
                release.validate_vpk(vpk, Path(tmp) / 'eboot.bin', '00.24')

    def test_checksum_corruption_and_extra_private_file_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp); report = stage(directory)
            self.assertEqual(release.verify_staged(directory)['tag'], 'v00.24')
            vpk = directory / next(n for n in report['artifacts'] if n.endswith('.vpk'))
            vpk.write_bytes(b'corrupted after build')
            with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
                release.verify_staged(directory)
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp);stage(directory);(directory / 'original.jar').write_bytes(b'private')
            with self.assertRaisesRegex(ValueError, 'Unexpected file'):
                release.verify_staged(directory)

    def test_wrong_caller_or_workflow_sha_cannot_publish(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp);stage(directory, True)
            with patch.dict(os.environ, {'GITHUB_SHA': 'b' * 40, 'GITHUB_RUN_ID': '123', 'PRERELEASE': 'true'}):
                with patch.object(release, 'github_get') as network:
                    with self.assertRaisesRegex(ValueError, 'provenance differs'):
                        release.publish(directory)
                    network.assert_not_called()
            with patch.dict(os.environ, {'GITHUB_SHA': 'a' * 40, 'GITHUB_RUN_ID': '123', 'PRERELEASE': 'false'}):
                with self.assertRaisesRegex(ValueError, 'caller mismatch'):
                    release.publish(directory)

    def test_existing_tag_is_never_overwritten(self):
        plan = {'tag': 'v00.24'}
        with patch.object(release, 'github_get', return_value={'id': 1}):
            with self.assertRaisesRegex(ValueError, 'already exists'):
                release.check_target(plan)
        with patch.object(release, 'github_get', side_effect=[None, {'object': {'sha': 'b' * 40}}]):
            with self.assertRaisesRegex(ValueError, 'Git tag already'):
                release.check_target(plan)

    def test_draft_complete_then_publish_correct_channel_latest_flag(self):
        for prerelease in (False, True):
            with self.subTest(prerelease=prerelease), tempfile.TemporaryDirectory() as tmp:
                directory = Path(tmp);report = stage(directory, prerelease)
                names = list(report['artifacts']) + ['build.json', 'SHA256SUMS.txt']
                draft = {'draft': True, 'prerelease': prerelease, 'target_commitish': 'a' * 40,
                         'assets': [{'name': n, 'size': (directory / n).stat().st_size} for n in names]}
                published = {'draft': False, 'prerelease': prerelease, 'html_url': 'https://github.com/test/repo/releases/tag/' + report['tag']}
                with patch.dict(os.environ, {'GITHUB_SHA': 'a' * 40, 'GITHUB_RUN_ID': '123',
                                            'GITHUB_REPOSITORY': 'test/repo', 'PRERELEASE': str(prerelease).lower()}), \
                     patch.object(release, 'github_get', side_effect=[None, None, draft, published]), \
                     patch.object(release.subprocess, 'run') as command:
                    release.publish(directory)
                create, edit = [call.args[0] for call in command.call_args_list]
                self.assertIn('--draft', create)
                self.assertIn('--latest=false', create)
                self.assertEqual('--prerelease' in create, prerelease)
                self.assertIn('--prerelease=' + str(prerelease).lower(), edit)
                self.assertIn('--latest=' + str(not prerelease).lower(), edit)
                self.assertIn('--draft=false', edit)

    def test_partial_draft_upload_never_becomes_public(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp);stage(directory)
            draft = {'draft': True, 'prerelease': False, 'target_commitish': 'a' * 40, 'assets': []}
            with patch.dict(os.environ, {'GITHUB_SHA': 'a' * 40, 'GITHUB_RUN_ID': '123',
                                        'GITHUB_REPOSITORY': 'test/repo', 'PRERELEASE': 'false'}), \
                 patch.object(release, 'github_get', side_effect=[None, None, draft]), \
                 patch.object(release.subprocess, 'run') as command:
                with self.assertRaisesRegex(ValueError, 'upload incomplete'):
                    release.publish(directory)
                self.assertEqual(command.call_count, 1)


if __name__ == '__main__':
    unittest.main()
