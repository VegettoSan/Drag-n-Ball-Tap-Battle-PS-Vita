package com.namcobandaigames.dragonballtap.apk;

// Deliberately attempts the original startup/update, without Android adapters.
// A failed compilation is a boundary inventory, not a playable engine build.
public final class EngineProbe {
    public static void main(String[] args) {
        GlobalWork gw = new GlobalWork();
        TCBManajer engine = new TCBManajer();
        if (!engine.Init(gw)) throw new IllegalStateException("Init failed");
        engine.Run(gw);
    }
}
