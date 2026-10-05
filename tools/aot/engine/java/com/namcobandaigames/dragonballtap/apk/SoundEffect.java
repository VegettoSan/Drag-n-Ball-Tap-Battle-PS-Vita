package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import android.content.Context;
public final class SoundEffect {
    private static final SoundEffect instance=new SoundEffect();
    public static SoundEffect getInstance(){return instance;}
    public void dispose(){NativePlatform.audioDispose();}
    public int load(Context c,String s){byte[] n=NativePlatform.cstr(s);return NativePlatform.effectLoad(Address.ofData(n));}
    public int loadAudioTrack(Context c,byte[] b){return NativePlatform.voiceLoad(Address.ofData(b),b.length);}
    public void playAudio(int id,float gain){NativePlatform.voicePlay(id,gain);}
    public void playSE(int id,float gain){NativePlatform.effectPlay(id,gain);}
    public void playBgm(Context c,String s,float gain,boolean loop){byte[] n=NativePlatform.cstr(s);if(NativePlatform.bgmPlay(Address.ofData(n),gain,loop?1:0)<0)throw new IllegalStateException("BGM load failed: "+s);}
    public void releaseAudio(){NativePlatform.voiceRelease();} public void stopAudio(){NativePlatform.voiceStop();}
    public void stopBgm(){NativePlatform.bgmStop();} public void stopSE(){NativePlatform.effectStop();}
}

