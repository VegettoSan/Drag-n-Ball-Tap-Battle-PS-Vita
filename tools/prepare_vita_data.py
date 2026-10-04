#!/usr/bin/env python3
"""Package two user-owned APK data sets for copying to ux0:, without conversion."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile
import zipfile
from extract_apk_data import extract, check_destination

README = """DATOS DRAGON BALL TAP BATTLE PARA PS VITA

Extrae este ZIP en el PC y copia su carpeta data a la raiz ux0: usando VitaShell.
La estructura final debe ser:
  ux0:data/DBTapBattle/game/common.pac
  ux0:data/DBTapBattle/mods/Android14/common.pac

En el selector del VPK:
  Original: usa game/.
  Android14: usa mods/Android14/ y recurre a game/ si falta un archivo.

Los recursos se mantienen byte por byte; solo se normalizan los nombres
codificados del APK comunitario. Cada carpeta incluye dbtb_manifest.json con
el APK de origen, sus hashes y los nombres originales. SHA256SUMS.txt cubre
todos los archivos del paquete excepto el propio listado de hashes.

Este ZIP no contiene un VPK y no convierte el lector de recursos en un juego
jugable. La compatibilidad depende de la version del motor instalada.
El APK original suministrado no incluye los paquetes de personajes descargados.
El APK comunitario incluye 13 conjuntos de personajes; se mantienen en Android14.
El archivo text00.pac contiene tablas; sus cadenas/renderizado requieren el motor.

No se incluyen APK, clases Java/Dalvik, bibliotecas Android ni archivos de guardado.
Este paquete es una copia preparada a partir de tus dos APK; no se publica
en el repositorio ni en GitHub Releases.
"""


def package(original, community, output):
    output = Path(output)
    check_destination(output, False)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='dbtb-data-', dir=output.parent) as temporary:
        stage = Path(temporary)
        root = stage / 'data' / 'DBTapBattle'
        a = extract(Path(original), root / 'game', layout='raw')
        b = extract(Path(community), root / 'mods' / 'Android14', layout='community14')
        (stage / 'LEEME.txt').write_text(README, encoding='utf-8')
        files = sorted(p for p in stage.rglob('*') if p.is_file())
        checks = {p.relative_to(stage).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                  for p in files}
        sums = stage / 'SHA256SUMS.txt'
        sums.write_text(''.join(f'{digest}  {name}\n' for name, digest in checks.items()), encoding='utf-8')
        files.append(sums)
        # Assemble and validate privately before publishing a complete archive.
        pending = stage / 'ready.zip'
        with zipfile.ZipFile(pending, 'x', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
            for p in files:
                archive.write(p, p.relative_to(stage).as_posix())
        with zipfile.ZipFile(pending) as archive:
            if archive.testzip() is not None:
                raise ValueError('ZIP CRC validation failed')
            for name, digest in checks.items():
                if hashlib.sha256(archive.read(name)).hexdigest() != digest:
                    raise ValueError('ZIP hash mismatch: ' + name)
        # Same-filesystem hard link publishes the validated ZIP atomically and
        # refuses an existing destination, including one created concurrently.
        os.link(pending, output)
        return dict(original_files=a['file_count'], community_files=b['file_count'],
                    original_apk_sha256=a['source_apk_sha256'],
                    community_apk_sha256=b['source_apk_sha256'],
                    zip_size=output.stat().st_size,
                    zip_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
                    verified=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_apk', type=Path)
    parser.add_argument('community_apk', type=Path)
    parser.add_argument('output_zip', type=Path)
    args = parser.parse_args()
    try:
        report = package(args.original_apk, args.community_apk, args.output_zip)
    except (OSError, ValueError, RuntimeError, zipfile.BadZipFile) as exc:
        parser.exit(2, f'error: {exc}\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
