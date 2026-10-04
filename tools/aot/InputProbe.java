package com.namcobandaigames.dragonballtap.apk;

// Test harness only. KeyData/Controller are supplied by the user's APK/JAR,
// not copied into this repository. Compare JVM and native C stdout exactly.
public final class InputProbe {
    private static void check(boolean ok, String reason) {
        if (!ok) throw new IllegalStateException(reason);
    }
    private static void trace(Controller c, int pad, String label) {
        System.out.println(label + ":" + c.GetKey(pad, 0) + ":" +
            c.GetKey(pad, 1) + ":" + c.GetKey(pad, 2) + ":" + c.GetKeepFrame(pad));
    }
    public static void main(String[] args) {
        KeyData k = new KeyData();
        Controller c = new Controller();
        c.Init();
        int pad = c.AddPad(90, 110, 80, 80, 5, 16);
        check(c.GetCount() == 1 && c.GetKey(pad, 0) == 0, "pad initial state");
        k.Set(100, 120, 1, 7);
        check(k.GetIndex(7) >= 0, "touch absent");
        check(k.getX(7) == 100 && k.getY(7) == 120 && k.getMoveX(7) == 0 &&
            k.isBeginTouch(7), "begin state");
        c.SetKey(k);
        check(c.GetKey(pad, 0) == 16 && c.GetKey(pad, 2) == 16, "button press");
        trace(c, pad, "press");
        k.ClearBegin();
        k.Set(110, 125, 0, 7);
        check(k.getMoveX(7) == 10 && k.getMoveY(7) == 5 &&
            k.getSpeedX(7) == 10 && k.getSpeedY(7) == 5 &&
            !k.isBeginTouch(7), "move state");
        c.SetKey(k);
        check(c.GetKey(pad, 0) == 0 && c.GetKey(pad, 2) == 16, "button hold");
        trace(c, pad, "move");
        k.Set(400, 220, 1, 9);
        check(k.GetIndex(9) != k.GetIndex(7) && k.getX(9) == 400, "pointer identity");
        k.Clear(7);
        check(k.GetIndex(7) == -1 && k.getX(7) == 0 && k.getY(9) == 220, "release state");
        c.SetKey(k);
        check(c.GetKey(pad, 1) == 16 && c.GetKey(pad, 2) == 0, "button release");
        trace(c, pad, "release");
        k.Clear(9);
        for (int i = 0; i < 10; ++i) k.Set(20 + i, 40, 1, 100 + i);
        k.Set(99, 99, 1, 999);
        check(k.GetIndex(999) == -1, "slot budget");
        for (int i = 0; i < 10; ++i) k.Clear(100 + i);
        k.Set(42, 84, 1, 999);
        check(k.getX(999) == 42, "slot reuse");
        System.out.println("UTF16: \u00f1 \u65e5\u672c \ud83d\udc09");
        System.out.println("ORIGINAL INPUT AOT PASS");
    }
}
