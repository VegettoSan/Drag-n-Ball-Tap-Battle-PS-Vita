package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import javax.microedition.khronos.opengles.GL10;
public final class AndroidGLTexture {
    private int id,width,height;
    public void Dispose(GL10 gl){if(id!=0){gl.glDeleteTextures(1,new int[]{id},0);}id=width=height=0;}
    public int GetImage(){return id;} public int GetWidth(){return width;} public int GetHeight(){return height;}
    protected boolean loadTexture(GL10 gl,byte[] b){return loadTexture(gl,b,0,b.length,true);}
    protected boolean loadTexture(GL10 gl,byte[] b,int off,int size,boolean linear){
        if(b==null||off<0||size<0||off>b.length-size)return false;
        Dispose(gl);id=NativePlatform.loadTexture(Address.ofData(b).add(off),size,linear?1:0);
        if(id<=0){id=0;return false;}width=NativePlatform.textureWidth(id);height=NativePlatform.textureHeight(id);return width>0&&height>0;
    }
}

