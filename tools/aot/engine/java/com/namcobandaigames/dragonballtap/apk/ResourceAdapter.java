package com.namcobandaigames.dragonballtap.apk;

/** The Android raw-ID/stream boundary, not a replacement game-data decoder. */
public final class ResourceAdapter {
    private static final java.util.Map<GameData,String> encodings=new java.util.WeakHashMap<GameData,String>();
    public static String textCharset(int source) { return NativePlatform.textEncoding(source)==1?"UTF-8":"Shift_JIS"; }
    public static String stringCharset(TCBManajer engine,int source,int slot) {
        if(source==0)return textCharset(0);
        if(source==2)return textCharset(1);
        GameData data=engine.ChrGameData[slot+(source==1?6:3)];
        String charset=encodings.get(data);return charset==null?"Shift_JIS":charset;
    }
    public static boolean load(GameData data, GlobalWork gw, String name, int conversion, int filter) {
        byte[] bytes;
        try {
            bytes = NativePlatform.readGameData(name, filter);
            if(bytes==null)bytes=NativePlatform.readGameData(name + ".pac", filter);
        } catch (Exception missingRaw) {
            bytes = NativePlatform.readGameData(name + ".pac", filter);
        }
        if(bytes==null)return false;
        encodings.put(data,NativePlatform.resourceEncoding()==1?"UTF-8":"Shift_JIS");
        return data.Init(gw, bytes, conversion, filter);
    }
}
