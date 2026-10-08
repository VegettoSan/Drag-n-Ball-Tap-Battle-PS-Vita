package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import org.teavm.runtime.EventQueue;
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
        VitaControls controls=new VitaControls(NativePlatform.controlMode());
        int[] events=new int[45];int frames=0;
        while(gw.bThreadActive){int count=NativePlatform.frame(Address.ofData(events));if(count<0)break;if(count>10)throw new IllegalStateException("Input overflow");
            controls.update(gw,engine,events,count);
            engine.Run(gw);NativePlatform.present();
            // TeaVM's C backend maps java.lang.Thread to cooperative fibers
            // scheduled through EventQueue. Android normally pumps its scheduler,
            // but this Vita entry point owns the main loop. Without this call the
            // original AutoCardTask is queued forever, leaving the ability-card
            // screen stuck on its processing overlay even though rendering/input
            // keep running. Run at most one ready event per frame so background
            // work progresses without draining unrelated delayed events at once.
            EventQueue.processSingle();frames++;
        }
        System.out.println("ORIGINAL ENGINE RUN FRAMES="+frames);engine.Dispose(gw);
    }
}
