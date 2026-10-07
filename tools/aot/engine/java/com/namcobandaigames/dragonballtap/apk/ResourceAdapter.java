package com.namcobandaigames.dragonballtap.apk;

/** The Android raw-ID/stream boundary, not a replacement game-data decoder. */
public final class ResourceAdapter {
    private static final java.util.Map<GameData,String> encodings=new java.util.WeakHashMap<GameData,String>();
    private static String characterProfileCharset;
    private static boolean characterFallbackReported;

    public static String textCharset(int source) { return NativePlatform.textEncoding(source)==1?"UTF-8":"Shift_JIS"; }

    private static boolean isCharacterPac(String name) {
        if(name==null || name.length()<6 || !name.startsWith("char"))return false;
        if(name.startsWith("chardemo") || name.startsWith("charf"))return false;
        return Character.isDigit(name.charAt(4)) && Character.isDigit(name.charAt(5)) &&
            (name.length()==6 || (name.length()==10 && name.endsWith(".pac")));
    }

    public static String stringCharset(TCBManajer engine,int source,int slot) {
        if(source==0)return textCharset(0);
        if(source==2)return textCharset(1);

        // The original Gen SetString indexes ChrGameData with its own slot
        // convention. Android14/Invasion DEX variants can use a different
        // SetString implementation while keeping the same 43-record character
        // BIN schema. Prefer the exact GameData mapping when available, but do
        // not silently fall back to Shift_JIS if that original slot expression
        // misses: every loaded character PAC has already been content-sniffed
        // by the native resource bridge, so use the active character-profile
        // charset as the evidence-backed fallback.
        int index=slot+(source==1?6:3);
        if(index>=0 && index<engine.ChrGameData.length) {
            String charset=encodings.get(engine.ChrGameData[index]);
            if(charset!=null)return charset;
        }
        if(characterProfileCharset!=null) {
            if(!characterFallbackReported) {
                System.err.println("Character text charset fallback: source="+source+
                    " slot="+slot+" charset="+characterProfileCharset);
                characterFallbackReported=true;
            }
            return characterProfileCharset;
        }
        return "Shift_JIS";
    }

    public static java.io.InputStream open(GameData data, String name, int filter, boolean fallback) {
        java.io.InputStream stream = NativePlatform.openGameData(fallback ? name + ".pac" : name, filter);
        if (stream != null) {
            String charset=NativePlatform.resourceEncoding()==1 ? "UTF-8" : "Shift_JIS";
            encodings.put(data, charset);
            if(isCharacterPac(name))characterProfileCharset=charset;
        }
        return stream;
    }
}
