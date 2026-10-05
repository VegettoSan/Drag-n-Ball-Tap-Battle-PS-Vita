package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import org.teavm.interop.Import;
import org.teavm.interop.c.Include;
import java.nio.*;
import javax.microedition.khronos.opengles.*;
@Include(value="dbtb_bridge.h",isSystem=false)
public final class VitaGles implements GL10,GL11ExtensionPack {
    @Import(name="dbtb_glBindTexture") public native void glBindTexture(int target,int texture);
    @Import(name="dbtb_glBlendFunc") public native void glBlendFunc(int src,int dst);
    @Import(name="dbtb_glClear") public native void glClear(int mask);
    @Import(name="dbtb_glClearColor") public native void glClearColor(float r,float g,float b,float a);
    @Import(name="dbtb_glColor4f") public native void glColor4f(float r,float g,float b,float a);
    @Import(name="dbtb_glDisable") public native void glDisable(int cap);
    @Import(name="dbtb_glDisableClientState") public native void glDisableClientState(int cap);
    @Import(name="dbtb_glEnable") public native void glEnable(int cap);
    @Import(name="dbtb_glEnableClientState") public native void glEnableClientState(int cap);
    @Import(name="dbtb_glHint") public native void glHint(int target,int mode);
    @Import(name="dbtb_glLoadIdentity") public native void glLoadIdentity();
    @Import(name="dbtb_glMatrixMode") public native void glMatrixMode(int mode);
    @Import(name="dbtb_glOrthof") public native void glOrthof(float l,float r,float b,float t,float near,float far);
    @Import(name="dbtb_glPopMatrix") public native void glPopMatrix();
    @Import(name="dbtb_glPushMatrix") public native void glPushMatrix();
    @Import(name="dbtb_glScalef") public native void glScalef(float x,float y,float z);
    @Import(name="dbtb_glShadeModel") public native void glShadeModel(int mode);
    @Import(name="dbtb_glTexEnvf") public native void glTexEnvf(int target,int name,float value);
    @Import(name="dbtb_glTexParameterf") public native void glTexParameterf(int target,int name,float value);
    @Import(name="dbtb_glTranslatef") public native void glTranslatef(float x,float y,float z);
    @Import(name="dbtb_glViewport") public native void glViewport(int x,int y,int w,int h);
    @Import(name="dbtb_glBindFramebuffer") public native void glBindFramebufferOES(int target,int id);
    @Import(name="dbtb_glCheckFramebufferStatus") public native int glCheckFramebufferStatusOES(int target);
    @Import(name="dbtb_glFramebufferTexture2D") public native void glFramebufferTexture2DOES(int target,int attachment,int textarget,int texture,int level);
    @Import(name="dbtb_glArray") private static native void array(int operation,int n,Address data);
    private void arrayCall(int op,int n,int[] values,int offset) { if(n<0||offset<0||offset>values.length-n)throw new IndexOutOfBoundsException();array(op,n,Address.ofData(values).add(offset*4)); }
    public void glDeleteTextures(int n,int[] values,int offset){arrayCall(0,n,values,offset);}
    public void glGenTextures(int n,int[] values,int offset){arrayCall(1,n,values,offset);}
    public void glDeleteFramebuffersOES(int n,int[] values,int offset){arrayCall(2,n,values,offset);}
    public void glDeleteRenderbuffersOES(int n,int[] values,int offset){arrayCall(3,n,values,offset);}
    public void glGenFramebuffersOES(int n,int[] values,int offset){arrayCall(4,n,values,offset);}
    public void glGenRenderbuffersOES(int n,int[] values,int offset){arrayCall(5,n,values,offset);}
    @Import(name="dbtb_glPointer") private static native void pointer(int kind,int size,int type,int stride,Address data,int bytes);
    @Import(name="dbtb_glDraw") private static native void draw(int mode,int count,int type,Address data,int bytes);
    // Native pointer() copies client attributes before returning, so reusable
    // primitive arrays cannot be moved by GC while vitaGL still uses them.
    // Keep each active position/limit and use absolute reads: the original
    // Byte/Short/FloatBuffer is never advanced, duplicated or serialized.
    private final byte[][] byteScratch=new byte[4][];
    private final short[][] shortScratch=new short[4][];
    private final float[][] floatScratch=new float[4][];
    private Address clientAddress(int slot,Buffer data){
        int n=data.remaining();
        if(data instanceof ByteBuffer){
            ByteBuffer src=(ByteBuffer)data;
            if(src.hasArray())return Address.ofData(src.array()).add(src.arrayOffset()+src.position());
            if(byteScratch[slot]==null||byteScratch[slot].length<n)byteScratch[slot]=new byte[n];
            byte[] out=byteScratch[slot];for(int i=0;i<n;i++)out[i]=src.get(src.position()+i);
            return Address.ofData(out);
        }
        if(data instanceof ShortBuffer){
            ShortBuffer src=(ShortBuffer)data;
            if(src.hasArray())return Address.ofData(src.array()).add((src.arrayOffset()+src.position())*2);
            if(shortScratch[slot]==null||shortScratch[slot].length<n)shortScratch[slot]=new short[n];
            short[] out=shortScratch[slot];for(int i=0;i<n;i++)out[i]=src.get(src.position()+i);
            return Address.ofData(out);
        }
        if(data instanceof FloatBuffer){
            FloatBuffer src=(FloatBuffer)data;
            if(src.hasArray())return Address.ofData(src.array()).add((src.arrayOffset()+src.position())*4);
            if(floatScratch[slot]==null||floatScratch[slot].length<n)floatScratch[slot]=new float[n];
            float[] out=floatScratch[slot];for(int i=0;i<n;i++)out[i]=src.get(src.position()+i);
            return Address.ofData(out);
        }
        throw new IllegalArgumentException("Unsupported GL client buffer");
    }
    private int clientBytes(Buffer data){
        int width=data instanceof ByteBuffer?1:data instanceof ShortBuffer?2:data instanceof FloatBuffer?4:0;
        if(width==0||data.remaining()>16384/width)throw new IllegalArgumentException("Invalid GL client range");
        return data.remaining()*width;
    }
    public void glColorPointer(int size,int type,int stride,Buffer data){int n=clientBytes(data);pointer(0,size,type,stride,clientAddress(0,data),n);}
    public void glTexCoordPointer(int size,int type,int stride,Buffer data){int n=clientBytes(data);pointer(1,size,type,stride,clientAddress(1,data),n);}
    public void glVertexPointer(int size,int type,int stride,Buffer data){int n=clientBytes(data);pointer(2,size,type,stride,clientAddress(2,data),n);}
    public void glDrawElements(int mode,int count,int type,Buffer data){int n=clientBytes(data);draw(mode,count,type,clientAddress(3,data),n);}
    public String glGetString(int name){if(name==7939)return "GL_OES_framebuffer_object";return "DBTB Vita GLES bridge";}
}

