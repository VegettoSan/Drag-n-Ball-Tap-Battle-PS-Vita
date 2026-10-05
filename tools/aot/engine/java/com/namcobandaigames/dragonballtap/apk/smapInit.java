package com.namcobandaigames.dragonballtap.apk;

// The obsolete remote catalog must not block a complete local installation.
// Keep remote/catalog entries empty, but report success only after the native VFS
// proves the complete offline character/shared data required by the original core.
public final class smapInit {
    private boolean localDataReady;
    public void Init(){localDataReady=NativePlatform.installedData()!=0;}
    public boolean isEnd(){return true;}
    public int getError(){return localDataReady?0:-1;}
    public int getSize(int kind){return 0;}
    public int getDataNo(int kind,int index){return -1;}
    public String getDataURL(int kind,int index){return null;}
    public int getDataVersion(int kind,int index){return -1;}
}
