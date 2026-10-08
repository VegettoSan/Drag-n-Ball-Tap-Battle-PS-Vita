package com.namcobandaigames.dragonballtap.apk;

// Research harness only. Executes the user's APK-derived classes on the JVM.
// No original class or gameplay implementation is included here.
public final class VirtualPadProbe {
    private static void check(boolean ok, String reason) {
        if (!ok) throw new IllegalStateException(reason);
    }
    private static Controller pad() {
        Controller c = new Controller();
        c.Init();
        c.AddPad(90, 237, 230, 0, 1, 0);
        c.AddPad(430, 268, 48, 0, 4, 16640);
        c.AddPad(340, 268, 48, 0, 4, 1048576);
        c.AddPad(380, 218, 48, 0, 4, 2097152);
        c.AddPad(440, 188, 48, 0, 4, 4194304);
        c.AddPad(440, 128, 48, 0, 4, 8388608);
        c.AddPad(60, 40, 108, 0, 4, 553648128);
        return c;
    }
    public static void main(String[] args) {
        int[][] directions = {
            {100,183,1},{164,183,9},{164,247,8},{164,311,10},
            {100,311,2},{36,311,6},{36,247,4},{36,183,5}
        };
        for (int[] d : directions) {
            Controller c = pad(); KeyData k = new KeyData();
            k.Set(d[0],d[1],1,0); c.SetKey(k);
            check(c.GetKey(0,0)==d[2] && c.GetKey(0,2)==d[2], "direction press");
            k.ClearBegin(); k.Set(d[0],d[1],0,0); c.SetKey(k);
            check(c.GetKey(0,0)==0 && c.GetKey(0,2)==d[2], "direction hold");
            k.Clear(0); c.SetKey(k);
            check(c.GetKey(0,1)==d[2] && c.GetKey(0,2)==0, "direction release");
        }
        System.out.println("DIRECTIONS: eight directions, press/hold/release PASS");
        int[][] buttons = {
            {430,268,16640},{340,268,1048576},{380,218,2097152},
            {440,188,4194304},{440,128,8388608},{60,8,553648128}
        };
        for (int i=0;i<buttons.length;i++) {
            Controller c=pad(); KeyData k=new KeyData(); int[] b=buttons[i];
            k.Set(b[0],b[1],1,0); c.SetKey(k);
            check(c.GetKey(i+1,0)==b[2] && c.GetKey(i+1,2)==b[2], "button press");
            check(c.GetKey(0,2)==0, "button also moves virtual stick");
            k.ClearBegin(); k.Set(b[0],b[1],0,0); c.SetKey(k);
            check(c.GetKey(i+1,0)==0 && c.GetKey(i+1,2)==b[2], "button hold");
            k.Clear(0); c.SetKey(k);
            check(c.GetKey(i+1,1)==b[2] && c.GetKey(i+1,2)==0, "button release");
        }
        System.out.println("BUTTONS: six safe points, masks and press/hold/release PASS");
        Controller c=pad(); KeyData k=new KeyData();
        k.Set(60,40,1,0); c.SetKey(k);
        check(c.GetKey(0,2)==0 && c.GetKey(6,2)==553648128, "rage center isolation");
        check(c.GetRange(0,0)==115 && c.GetRange(1,0)==24 && c.GetRange(6,0)==54,
            "AddPad halves type-1/type-4 ranges");
        System.out.println("RANGES: actual half-widths 115/24/54; rage center isolation PASS");
        c=pad(); k=new KeyData();
        k.Set(164,247,1,0); k.Set(430,268,1,1); k.Set(340,268,1,2);
        k.Set(380,218,1,3); k.Set(60,8,1,4); c.SetKey(k);
        check(c.GetKey(0,2)==8 && c.GetKey(1,2)==16640 &&
            c.GetKey(2,2)==1048576 && c.GetKey(3,2)==2097152 &&
            c.GetKey(6,2)==553648128, "five simultaneous stable IDs");
        for(int id=0;id<5;id++) k.Clear(id);
        c.SetKey(k);
        for(int i=0;i<7;i++) check(c.GetKey(i,2)==0, "stuck key after clear");
        System.out.println("MULTITOUCH: IDs 0..4, simultaneous input and full release PASS");
        c=pad(); k=new KeyData();
        k.Set(164,247,1,0); c.SetKey(k); k.ClearBegin();
        k.Set(36,247,0,0); c.SetKey(k);
        check(c.GetKey(0,2)==4 && c.GetKey(0,0)==0, "continuous direction edge");
        System.out.println("DIRECTION CHANGE: held mask changes; type-1 press edge stays zero PASS");
        System.out.println("ORIGINAL VIRTUAL PAD JVM PASS (not a Vita/AOT/gameplay test)");
    }
}
