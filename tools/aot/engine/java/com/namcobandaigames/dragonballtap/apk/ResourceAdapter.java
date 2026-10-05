package com.namcobandaigames.dragonballtap.apk;

/** The Android raw-ID/stream boundary, not a replacement game-data decoder. */
public final class ResourceAdapter {
    private static String gameCharset="Shift_JIS", textCharset="Shift_JIS";
    public static String textCharset(int source) { return source==0?gameCharset:textCharset; }
    public static boolean load(GameData data, GlobalWork gw, String name, int conversion, int filter) {
        byte[] bytes;
        try {
            bytes = Utility.readDataRaw(gw.context, name);
        } catch (Exception missingRaw) {
            bytes = Utility.readDataFile(gw.context, name + ".pac");
        }
        if(bytes==null)return false;
        String charset=NativePlatform.resourceEncoding()==1?"UTF-8":"Shift_JIS";
        if(name.equals("gamedata"))gameCharset=charset;
        if(name.equals("text00"))textCharset=charset;
        return data.Init(gw, bytes, conversion, filter);
    }
}
