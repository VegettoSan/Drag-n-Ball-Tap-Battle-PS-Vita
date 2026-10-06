#!/usr/bin/env python3
"""Fetch the pinned private input without putting its URL or bytes in logs/Git."""
import argparse
import hashlib
import os
from pathlib import Path
import urllib.error
import urllib.parse
import urllib.request
import zipfile

ORIGINAL_APK_SHA256 = 'b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b'
MAX_BYTES = 32 * 1024 * 1024


def validate_apk(path):
    if not path.is_file() or path.stat().st_size > MAX_BYTES:
        raise ValueError('Original APK missing or exceeds 32 MiB')
    if hashlib.sha256(path.read_bytes()).hexdigest() != ORIGINAL_APK_SHA256:
        raise ValueError('APK SHA-256 mismatch: use the pinned original DBTapBattle.apk, not Android14/Gen')
    with zipfile.ZipFile(path) as apk:
        if apk.testzip() or len(apk.namelist()) != len(set(apk.namelist())):
            raise ValueError('Original APK ZIP integrity failure')
        if not {'classes.dex', 'AndroidManifest.xml'} <= set(apk.namelist()):
            raise ValueError('Original APK is missing its core/manifest')


class HttpsRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        if urllib.parse.urlsplit(newurl).scheme != 'https':
            raise ValueError('Private APK redirects must remain HTTPS')
        return super().redirect_request(req, fp, code, msg, headers, newurl)


def download(url, destination):
    if urllib.parse.urlsplit(url).scheme != 'https':
        raise ValueError('DBTB_ORIGINAL_APK_URL must be a direct HTTPS download URL')
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        raise ValueError('Refusing to overwrite an existing original APK')
    partial = destination.with_suffix('.partial')
    partial_owned = False
    try:
        request = urllib.request.Request(url, headers={'User-Agent': 'DBTB-private-build'})
        with urllib.request.build_opener(HttpsRedirect()).open(request, timeout=60) as response:
            with partial.open('xb') as output:
                partial_owned = True
                count = 0
                while True:
                    block = response.read(1024 * 1024)
                    if not block:
                        break
                    count += len(block)
                    if count > MAX_BYTES:
                        raise ValueError('Private APK exceeds 32 MiB')
                    output.write(block)
        validate_apk(partial)
        partial.rename(destination)
    except (urllib.error.URLError, TimeoutError, OSError):
        # urllib exceptions can contain the entire signed/secret URL.
        raise ValueError('Private APK download failed; check access/expiry of DBTB_ORIGINAL_APK_URL') from None
    finally:
        if partial_owned:
            partial.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        download(os.environ.get('ORIGINAL_APK_URL', ''), args.output)
    except (ValueError, zipfile.BadZipFile) as exc:
        parser.exit(1, f'::error::{exc}\n')
    print('Private original APK verified: ' + ORIGINAL_APK_SHA256)


if __name__ == '__main__':
    main()
