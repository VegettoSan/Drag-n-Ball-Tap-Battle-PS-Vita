#!/usr/bin/env python3
"""Run client-buffer tests on the JVM, with mocked TeaVM addresses/native imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--ecj', required=True, type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
with tempfile.TemporaryDirectory(prefix='dbtb-gles-probe-') as directory:
    work = Path(directory)
    mocks = {
        'org/teavm/interop/Import.java': 'package org.teavm.interop; public @interface Import {String name();}',
        'org/teavm/interop/c/Include.java': 'package org.teavm.interop.c; public @interface Include {String value();boolean isSystem();}',
        'org/teavm/interop/Address.java': """package org.teavm.interop;
public class Address {
 public Object array; public int offset;
 public Address(Object a,int o){array=a;offset=o;}
 public static Address ofData(Object a){return new Address(a,0);}
 public Address add(int o){return new Address(array,offset+o);}
}""",
    }
    for name, content in mocks.items():
        path = work / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
    sources = [work / name for name in mocks]
    sources += [root / 'java/javax/microedition/khronos/opengles' / name
                for name in ['GL10.java', 'GL11ExtensionPack.java']]
    sources += [root / 'java/com/namcobandaigames/dragonballtap/apk/VitaGles.java',
                root / 'tests/GlesBufferProbe.java']
    subprocess.run(['java', '-jar', str(args.ecj.resolve()), '-8', '-d', str(work / 'classes'),
                    *map(str, sources)], check=True)
    subprocess.run(['java', '-cp', str(work / 'classes'), 'GlesBufferProbe'], check=True)
