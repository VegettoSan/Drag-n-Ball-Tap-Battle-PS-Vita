package com.namcobandaigames.dragonballtap.apk;

/** The Android raw-ID/stream boundary, not a replacement game-data decoder. */
public final class ResourceAdapter {
    public static boolean load(GameData data, GlobalWork gw, String name, int conversion, int filter) {
        byte[] bytes;
        try {
            bytes = Utility.readDataRaw(gw.context, name);
        } catch (Exception missingRaw) {
            bytes = Utility.readDataFile(gw.context, name + ".pac");
        }
        return data.Init(gw, bytes, conversion, filter);
    }
}
