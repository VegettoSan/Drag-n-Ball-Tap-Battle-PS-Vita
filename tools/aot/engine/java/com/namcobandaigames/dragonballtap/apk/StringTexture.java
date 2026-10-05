package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import javax.microedition.khronos.opengles.GL10;
import android.graphics.RectF;
public final class StringTexture {
    private int id,width,height;
    public StringTexture(){if(!CreateBitmap(512,512))throw new IllegalStateException("Text surface unavailable");}
    public boolean CreateBitmap(int w,int h){if(id!=0)NativePlatform.disposeText(id);id=NativePlatform.createText(w,h);width=w;height=h;return id>0;}
    public void Dispose(GL10 gl){if(id!=0)NativePlatform.disposeText(id);id=0;}
    public int GetImage(){return NativePlatform.textTexture(id);} public int GetWidth(){return width;} public int GetHeight(){return height;}
    protected void clear(){NativePlatform.clearText(id);}
    protected RectF drawString(String text,int size,int r,int g,int b,int a){char[] chars=text.toCharArray();int[] bounds=new int[4];if(NativePlatform.drawText(id,Address.ofData(chars),chars.length,size,r,g,b,a,Address.ofData(bounds))==0)return null;RectF out=new RectF();out.left=bounds[0];out.top=bounds[1];out.right=bounds[2];out.bottom=bounds[3];return out;}
    protected void flash(GL10 gl){NativePlatform.textTexture(id);}
}

