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
    private byte[] bytes(Buffer data){
        if(data instanceof ByteBuffer){ByteBuffer b=((ByteBuffer)data).duplicate();byte[] out=new byte[b.remaining()];b.get(out);return out;}
        ByteBuffer b;
        if(data instanceof ShortBuffer){ShortBuffer s=((ShortBuffer)data).duplicate();b=ByteBuffer.allocate(s.remaining()*2).order(ByteOrder.nativeOrder());while(s.hasRemaining())b.putShort(s.get());}
        else if(data instanceof FloatBuffer){FloatBuffer f=((FloatBuffer)data).duplicate();b=ByteBuffer.allocate(f.remaining()*4).order(ByteOrder.nativeOrder());while(f.hasRemaining())b.putFloat(f.get());}
        else throw new IllegalArgumentException("Unsupported GL client buffer");
        return b.array();
    }
    public void glColorPointer(int size,int type,int stride,java.nio.Buffer data){byte[] b=bytes(data);pointer(0,size,type,stride,Address.ofData(b),b.length);}
    public void glTexCoordPointer(int size,int type,int stride,java.nio.Buffer data){byte[] b=bytes(data);pointer(1,size,type,stride,Address.ofData(b),b.length);}
    public void glVertexPointer(int size,int type,int stride,java.nio.Buffer data){byte[] b=bytes(data);pointer(2,size,type,stride,Address.ofData(b),b.length);}
    public void glDrawElements(int mode,int count,int type,Buffer data){byte[] b=bytes(data);draw(mode,count,type,Address.ofData(b),b.length);}
    public String glGetString(int name){if(name==7939)return "GL_OES_framebuffer_object";return "DBTB Vita GLES bridge";}
}

