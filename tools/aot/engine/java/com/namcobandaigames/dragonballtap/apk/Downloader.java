package com.namcobandaigames.dragonballtap.apk;

// Match the original Gen offline stub semantics exactly. The Android class
// reports an already-finished zero-byte download even when SetURL rejects the
// obsolete URL. Returning "not downloading" here made the preserved state
// machine wait roughly 25 seconds before continuing on Vita.
public final class Downloader {
    private static final Downloader instance=new Downloader();
    public static Downloader getInstance(){return instance;}
    public void Clear(){}
    public byte[] GetData(){return new byte[0];}
    public int GetSize(){return 0;}
    public void SetAPI(int api){}
    public boolean SetURL(String url){System.err.println("Offline catalog skipped: "+url);return false;}
    public boolean isDownload(){return true;}
}
