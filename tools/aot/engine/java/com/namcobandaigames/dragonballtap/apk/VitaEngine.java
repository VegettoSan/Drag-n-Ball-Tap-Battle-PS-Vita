package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import android.content.Context;
public final class VitaEngine {
    public static void main(String[] args){
        if(NativePlatform.start()==0)throw new IllegalStateException("Native startup failed");
        GlobalWork gw=new GlobalWork();gw.bBluetoothEnebled=false;gw.context=new Context();gw.glview=new AndroidGLView();
        gw.gl=new VitaGles();gw.build_model="PS Vita";gw.iLocale=0;
        gw.iScreenBaseWidth=960;gw.iScreenBaseHeight=544;gw.fScreenScale=320f/544;
        gw.iScreenScalWidth=(int)(960*gw.fScreenScale);gw.iScreenScalHeight=320;gw.iScreenScalWidthMax=568;
        gw.iScreenOffsetX=(gw.iScreenScalWidth-480)/2;gw.iScreenDrawOffsetX=-(gw.iScreenScalWidth/2-gw.iScreenOffsetX);gw.iScreenDrawOffsetY=-160;gw.fScreenGameScale=gw.iScreenScalWidth/480f;
        gw.gl.glViewport(0,0,960,544);gw.gl.glMatrixMode(5889);gw.gl.glLoadIdentity();gw.gl.glOrthof(-gw.iScreenScalWidth/2,gw.iScreenScalWidth/2,-160,160,-100,100);gw.gl.glMatrixMode(5888);gw.gl.glLoadIdentity();
        gw.gl.glEnableClientState(32884);gw.gl.glEnableClientState(32886);gw.gl.glEnable(3042);gw.gl.glBlendFunc(770,771);
        Utility.SetGlobalWork(gw);TCBManajer engine=new TCBManajer();gw.setTCBM(engine);
        if(!engine.Init(gw))throw new IllegalStateException("Original engine Init failed");
        System.out.println("ORIGINAL ENGINE INIT PASS");
        // Android marks the first active frame as a resume transition. The
        // original Run() uses this edge to allocate StringTexture[0..1], reset
        // Graphics2D and restore BGM, then clears bResume itself. Without it the
        // Vita port entered DrawExec with the text surfaces still null.
        gw.bResume=true;
        int[] events=new int[42];int frames=0;
        while(gw.bThreadActive){int count=NativePlatform.frame(Address.ofData(events));if(count<0)break;if(count>10)throw new IllegalStateException("Input overflow");gw.bBackKey=events[40]!=0;
            for(int i=0;i<count;i++){int p=i*4,id=events[p],phase=events[p+3];int x=(int)(events[p+1]*gw.fScreenScale)-gw.iScreenOffsetX;int y=(int)(events[p+2]*gw.fScreenScale)-gw.iScreenOffsetY;
                if(phase==2)gw.keyData.Clear(id);else gw.keyData.Set(x,y,phase==0?1:0,id);}
            engine.Run(gw);NativePlatform.present();frames++;
        }
        System.out.println("ORIGINAL ENGINE RUN FRAMES="+frames);engine.Dispose(gw);
    }
}
