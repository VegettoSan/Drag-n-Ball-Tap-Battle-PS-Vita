package com.namcobandaigames.dragonballtap.apk;

import javax.microedition.khronos.opengles.*;
public final class offscreen {
    private int texture,fbo,width,height,tw;
    public boolean Init(GL10 gl,int w,int h){Dispose(gl);tw=w<=512?512:1024;texture=NativePlatform.emptyTexture(tw,512);if(texture<=0)return false;int[] ids=new int[1];GL11ExtensionPack f=(GL11ExtensionPack)gl;f.glGenFramebuffersOES(1,ids,0);fbo=ids[0];f.glBindFramebufferOES(36160,fbo);f.glFramebufferTexture2DOES(36160,36064,3553,texture,0);boolean ok=f.glCheckFramebufferStatusOES(36160)==36053;f.glBindFramebufferOES(36160,0);if(!ok){Dispose(gl);return false;}width=w;height=h;return true;}
    public void Dispose(GL10 gl){if(fbo!=0)((GL11ExtensionPack)gl).glDeleteFramebuffersOES(1,new int[]{fbo},0);if(texture!=0)gl.glDeleteTextures(1,new int[]{texture},0);texture=fbo=0;}
    public void bind(GL10 gl){if(fbo==0)throw new IllegalStateException("FBO absent");((GL11ExtensionPack)gl).glBindFramebufferOES(36160,fbo);}
    public void bindClear(GL10 gl){((GL11ExtensionPack)gl).glBindFramebufferOES(36160,0);}
    public void draw(GL10 gl,int x,int y,int w,int h){int[] v={x,y,x,y+h,x+w,y+h,0,0,x+w,y,0,0};float u=(float)width/tw,t=(float)height/512;float[] uv={0,0,0,t,u,t,0,0,u,0,0,0};Graphics2D g=Graphics2D.getInstance();g.drawMode(gl,3);g.drawTexture(gl,texture,v,uv);g.clearMatrix(gl);g.drawMode(gl,0);}
}

