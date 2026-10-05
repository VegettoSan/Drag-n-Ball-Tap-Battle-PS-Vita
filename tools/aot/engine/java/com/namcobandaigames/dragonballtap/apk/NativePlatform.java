package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import org.teavm.interop.Import;
import org.teavm.interop.c.Include;

@Include(value="dbtb_bridge.h",isSystem=false)
public final class NativePlatform {
    @Import(name="dbtb_start") public static native int start();
    @Import(name="dbtb_frame") public static native int frame(Address events);
    @Import(name="dbtb_present") public static native void present();
    @Import(name="dbtb_resource") public static native int resource(Address name);
    @Import(name="dbtb_copyResource") public static native void copyResource(Address target,int size);
    @Import(name="dbtb_readSave") public static native int readSave(Address name);
    @Import(name="dbtb_writeSave") public static native int writeSave(Address name,Address bytes,int size,int position,int truncate);
    @Import(name="dbtb_deleteSave") public static native int deleteSave(Address name);
    @Import(name="dbtb_loadTexture") public static native int loadTexture(Address bytes,int size,int linear);
    @Import(name="dbtb_textureWidth") public static native int textureWidth(int id);
    @Import(name="dbtb_textureHeight") public static native int textureHeight(int id);
    @Import(name="dbtb_emptyTexture") public static native int emptyTexture(int w,int h);
    @Import(name="dbtb_createText") public static native int createText(int w,int h);
    @Import(name="dbtb_clearText") public static native void clearText(int id);
    @Import(name="dbtb_drawText") public static native int drawText(int id,Address text,int length,int size,int r,int g,int b,int a,Address bounds);
    @Import(name="dbtb_textTexture") public static native int textTexture(int id);
    @Import(name="dbtb_disposeText") public static native void disposeText(int id);
    @Import(name="dbtb_effectLoad") public static native int effectLoad(Address name);
    @Import(name="dbtb_effectPlay") public static native void effectPlay(int id,float gain);
    @Import(name="dbtb_effectStop") public static native void effectStop();
    @Import(name="dbtb_audioDispose") public static native void audioDispose();
    @Import(name="dbtb_voiceLoad") public static native int voiceLoad(Address bytes,int size);
    @Import(name="dbtb_voicePlay") public static native void voicePlay(int id,float gain);
    @Import(name="dbtb_voiceRelease") public static native void voiceRelease();
    @Import(name="dbtb_voiceStop") public static native void voiceStop();
    @Import(name="dbtb_bgmPlay") public static native int bgmPlay(Address name,float gain,int loop);
    @Import(name="dbtb_bgmStop") public static native void bgmStop();
    @Import(name="dbtb_unsupported") public static native void unsupported(Address message);
    public static byte[] cstr(String s) { byte[] b=s.getBytes(java.nio.charset.StandardCharsets.UTF_8);byte[] z=new byte[b.length+1];System.arraycopy(b,0,z,0,b.length);return z; }
    public static void unavailable(String s) { byte[] b=cstr(s);unsupported(Address.ofData(b));throw new UnsupportedOperationException(s); }
    public static byte[] read(String name,boolean save) { byte[] n=cstr(name);int size=save?readSave(Address.ofData(n)):resource(Address.ofData(n));if(size<0)return null;if(size>32*1024*1024)throw new IllegalStateException("Resource exceeds bridge budget");byte[] b=new byte[size];copyResource(Address.ofData(b),size);return b; }
}

