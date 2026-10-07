package com.namcobandaigames.dragonballtap.apk;

public final class AndroidGLView {
    public void BluetoothStart(){NativePlatform.unavailable("Bluetooth multiplayer is not implemented");}

    public void SmapStart(){
        // Android launches the bundled Smap/shop Activity and later SmapEnd()
        // restores rendering with iSmapEnd=1. Vita has no Android Activity or
        // marketplace service, so complete that lifecycle edge synchronously
        // instead of throwing out of the preserved TCB state machine.
        GlobalWork gw=Utility.GetGlobalWork();
        if(gw!=null){
            gw.iSmapEnd=1;
            gw.bRenderStop=false;
        }
        System.err.println("Vita shop bridge: Android marketplace unavailable; returning to game");
    }

    public void startBrowser(String url){NativePlatform.unavailable("External browser: "+url);}
}
