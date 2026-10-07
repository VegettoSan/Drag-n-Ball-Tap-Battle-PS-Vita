package com.namcobandaigames.dragonballtap.apk;

// Vita is deliberately offline. Preserve the original Downloader state-machine
// meaning: isDownload()==true means the asynchronous request is STILL running.
// Complete obsolete catalog requests immediately as failed/no-data requests so
// the preserved TCB state can take its normal offline fallback path.
public final class Downloader {
    private static final Downloader instance=new Downloader();
    public static Downloader getInstance(){return instance;}
    public void Clear(){}
    public byte[] GetData(){return null;}
    public int GetSize(){return 0;}
    public void SetAPI(int api){}
    public boolean SetURL(String url){
        System.err.println("Offline catalog skipped: "+url);
        return false;
    }
    public boolean isDownload(){return false;}
}
