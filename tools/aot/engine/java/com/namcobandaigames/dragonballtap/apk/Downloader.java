package com.namcobandaigames.dragonballtap.apk;

// Explicit offline failure, never a successful fabricated download.
public final class Downloader {
    private static final Downloader instance=new Downloader();
    public static Downloader getInstance(){return instance;}
    public void Clear(){} public byte[] GetData(){return null;} public int GetSize(){return -1;}
    public void SetAPI(int api){} public boolean SetURL(String url){System.err.println("Offline download rejected: "+url);return false;} public boolean isDownload(){return false;}
}

