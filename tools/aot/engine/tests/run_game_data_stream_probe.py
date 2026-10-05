#!/usr/bin/env python3
"""Compare both original parsers on private normalized PACs. Native I/O and GL mocked."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--ecj', type=Path, required=True)
parser.add_argument('--patched-jar', type=Path, required=True)
parser.add_argument('--fixtures', type=Path, nargs='+', required=True)
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
pkg = 'com/namcobandaigames/dragonballtap/apk/'
mocks = {
    'org/teavm/interop/Address.java': '''package org.teavm.interop;
public final class Address {
 public byte[] bytes;public int offset;
 Address(byte[] b,int o){bytes=b;offset=o;}
 public static Address ofData(byte[] b){return new Address(b,0);}
 public Address add(int o){return new Address(bytes,offset+o);}
}''',
    'javax/microedition/khronos/opengles/GL10.java': 'package javax.microedition.khronos.opengles;public interface GL10 {}',
    pkg+'GlobalWork.java': '''package com.namcobandaigames.dragonballtap.apk;
public final class GlobalWork {public javax.microedition.khronos.opengles.GL10 gl;}''',
    pkg+'AndroidGLTexture.java': '''package com.namcobandaigames.dragonballtap.apk;
import javax.microedition.khronos.opengles.GL10;
public final class AndroidGLTexture {
 private int hash;
 public void Dispose(GL10 gl){hash=0;}
 public int GetImage(){return hash;}public int GetWidth(){return 512;}public int GetHeight(){return 512;}
 protected boolean loadTexture(GL10 gl,byte[] b){return loadTexture(gl,b,0,b.length,true);}
 protected boolean loadTexture(GL10 gl,byte[] b,int off,int n,boolean linear){
  hash=1;for(int i=off;i<off+n;i++)hash=31*hash+b[i];return true;
 }
}''',
    pkg+'NativePlatform.java': '''package com.namcobandaigames.dragonballtap.apk;
import java.io.*;import java.util.*;import org.teavm.interop.Address;
public final class NativePlatform {
 public static File root;public static int next=1,largestRead;
 public static Map<Integer,RandomAccessFile> streams=new HashMap<Integer,RandomAccessFile>();
 public static int textEncoding(int s){return 0;}public static int resourceEncoding(){return 0;}
 public static InputStream openGameData(String name,int filter){
  File file=new File(root,name.endsWith(".pac")?name:name+".pac");
  try {RandomAccessFile f=new RandomAccessFile(file,"r");int h=next++;streams.put(h,f);
   return new NativeResourceStream(h,(int)f.length());}catch(IOException e){return null;}
 }
 public static int readResourceStream(int h,int p,Address target,int n){
  try {RandomAccessFile f=streams.get(h);f.seek(p);f.readFully(target.bytes,target.offset,n);
   largestRead=Math.max(largestRead,n);return n;}catch(Exception e){return -1;}
 }
 public static void closeResourceStream(int h){try{streams.remove(h).close();}catch(IOException e){throw new RuntimeException(e);}}
}''',
    'GameDataStreamProbe.java': '''import com.namcobandaigames.dragonballtap.apk.*;
import java.io.*;import java.nio.file.*;import java.lang.reflect.*;import java.util.*;
public final class GameDataStreamProbe {
 static void require(boolean ok){if(!ok)throw new AssertionError();}
 static String snapshot(Object value)throws Exception{
  if(value==null)return "null";
  if(value instanceof byte[])return java.util.Base64.getEncoder().encodeToString(java.security.MessageDigest.getInstance("SHA-256").digest((byte[])value));
  Class<?> type=value.getClass();
  if(type.isArray()){StringBuilder s=new StringBuilder("[");for(int i=0;i<Array.getLength(value);i++)s.append(snapshot(Array.get(value,i))).append(',');return s.append(']').toString();}
  if(value instanceof Number||value instanceof Boolean||value instanceof String)return value.toString();
  if(value instanceof List){StringBuilder s=new StringBuilder();for(Object item:(List<?>)value)s.append(snapshot(item));return s.toString();}
  StringBuilder s=new StringBuilder(type.getName());
  for(Field f:type.getDeclaredFields())if(!Modifier.isStatic(f.getModifiers())&&!f.isSynthetic()){
   f.setAccessible(true);s.append(f.getName()).append('=').append(snapshot(f.get(value)));}
  return s.toString();
 }
 public static void main(String[] args)throws Exception {
  int loads=0;long largestPac=0;
  GlobalWork gw=new GlobalWork();GameData stream=new GameData(),array=new GameData();
  for(String root:args){NativePlatform.root=new File(root);
   for(int i=0;i<13;i++)for(int mask:new int[]{0,33,64,187,251,255,256,-1,Integer.MIN_VALUE}){
    String name=String.format("char%02d",i);byte[] bytes=Files.readAllBytes(new File(root,name+".pac").toPath());
    largestPac=Math.max(largestPac,bytes.length);
    require(array.Init(gw,bytes,0,mask));require(stream.Init(gw,name,0,mask));
    if(!snapshot(array).equals(snapshot(stream)))throw new AssertionError(name+" mask="+mask);
    require(NativePlatform.streams.isEmpty());loads++;
   }
  }
  // Exercise the real Java stream's bounds, EOF, skip, and close contract.
  NativePlatform.root=new File(args[0]);InputStream s=NativePlatform.openGameData("char00",0);
  require(s.read(new byte[1],0,0)==0);require(s.skip(-1)==0);require(s.read()>=0);
  boolean bad=false;try{s.read(new byte[3],2,2);}catch(IndexOutOfBoundsException expected){bad=true;}require(bad);
  s.skip(Long.MAX_VALUE);require(s.available()==0&&s.read()==-1&&s.read(new byte[0])==0);
  s.close();s.close();bad=false;try{s.read();}catch(IOException expected){bad=true;}require(bad&&NativePlatform.streams.isEmpty());
  stream.Dispose(gw);array.Dispose(gw);
  System.out.println("ORIGINAL PARSER PASS loads="+loads+" largest_pac="+largestPac+" largest_stream_read="+NativePlatform.largestRead+" no_open_handles=true GL/native_IO=mocked");
 }
}''',
}
with tempfile.TemporaryDirectory(prefix='dbtb-original-stream-') as directory:
    work = Path(directory)
    sources = []
    for name, content in mocks.items():
        path = work / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
        sources.append(path)
    sources += [root / 'java' / pkg / name for name in ['ResourceAdapter.java', 'NativeResourceStream.java']]
    subprocess.run(['java', '-jar', str(args.ecj.resolve()), '-8', '-cp', str(args.patched_jar.resolve()),
                    '-d', str(work / 'classes'), *map(str, sources)], check=True)
    subprocess.run(['java', '-Xmx256m', '-cp', f'{work / "classes"}:{args.patched_jar.resolve()}',
                    'GameDataStreamProbe', *[str(p.resolve()) for p in args.fixtures]], check=True)
