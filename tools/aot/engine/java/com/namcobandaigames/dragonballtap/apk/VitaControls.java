package com.namcobandaigames.dragonballtap.apk;

/** Vita-only pointer adapter. Never writes commands, tasks or controller keys. */
final class VitaControls {
    private final int mode;
    private final boolean[] wanted = new boolean[16];
    private final boolean[] real = new boolean[5];
    private final boolean[] realBegin = new boolean[5];
    private final int[] x = new int[16], y = new int[16];
    private final int[] owner = {-1,-1,-1,-1,-1};
    private int scene = -1, blocked = 2047, previousHeld;
    private boolean pausePending;

    VitaControls(int mode) { this.mode = mode; }

    private static Controller.ButtonObject pad(TCBManajer engine, int role) {
        int id = engine.padID[role];
        if (id < 0 || id >= engine.controller.gamePad.buttonCount) return null;
        return engine.controller.gamePad.buttonObj[id];
    }
    private static boolean matches(Controller.ButtonObject p, int type, int px, int py, int code) {
        return p != null && p.Type == type && p.Pos[0] == px && p.Pos[1] == py && p.KeyCode[0] == code;
    }
    private static boolean task(int md) {
        TCB head = TCBManajer.tcbHead;
        TCB current = head == null ? null : head.next;
        for (int n = 0; current != null && n < 1170; ++n, current = current.next)
            if (current.act && current.md == md && (md != 38 || current._work[1] == TCBManajer.iPlayerNo)) return true;
        return false;
    }
    private static int context(TCBManajer engine) {
        if (TCBManajer.bPause || TCBManajer.bDrawLoading || TCBManajer.bTaskSkip || TCBManajer.bResume) return 0;
        if (TCBManajer.bGameStart && task(38) &&
            matches(pad(engine,0),1,90,237,0) && matches(pad(engine,1),4,430,268,0x4100) &&
            matches(pad(engine,2),4,340,268,0x100000) && matches(pad(engine,3),4,380,218,0x200000) &&
            matches(pad(engine,4),4,440,188,0x400000) && matches(pad(engine,5),4,440,128,0x800000)) return 2;
        if (task(1014) && matches(pad(engine,0),3,100,0,0) &&
            matches(pad(engine,1),5,-50,20,0x4100) && matches(pad(engine,2),5,430,20,0x4100)) return 1;
        return 0;
    }
    private void want(int action, int px, int py) { wanted[action] = true; x[action] = px; y[action] = py; }
    private int slot(int action) { for (int i=0;i<5;i++) if(owner[i]==action)return i; return -1; }
    private void clear(KeyData keys, int id) { keys.Clear(id); owner[id] = -1; }

    void update(GlobalWork gw, TCBManajer engine, int[] events, int count) {
        gw.bBackKey = false;
        for (int i=0;i<16;i++) wanted[i] = false;
        for (int i=0;i<5;i++) realBegin[i] = false;
        // End before Begin; Vita raw IDs are already compact and stable. Real
        // fingers retain those IDs, and preempt only synthetic owners of a slot.
        for (int i=0;i<count;i++) {
            int p=i*4, id=events[p]; if(id<0 || id>=5)continue;
            if(events[p+3]==2)real[id]=false;
            else { real[id]=true; realBegin[id]=events[p+3]==0;
                x[id]=(int)(events[p+1]*gw.fScreenScale)-gw.iScreenOffsetX;
                y[id]=(int)(events[p+2]*gw.fScreenScale)-gw.iScreenOffsetY; }
        }
        int held=events[41]&2047;
        int dx=((held&8)!=0?1:0)-((held&4)!=0?1:0);
        int dy=((held&2)!=0?1:0)-((held&1)!=0?1:0);
        boolean analog=events[42]<80 || events[42]>176 || events[43]<80 || events[43]>176;
        // Treat analog movement as a held direction for transition rearming.
        if(analog)held|=2048;
        int current=mode==0?0:context(engine);
        blocked &= held;
        if(current!=scene) { blocked|=held; pausePending=false; scene=current; }
        int active=held & ~blocked, pressed=active & ~previousHeld;
        previousHeld=held;
        if(current==2) {
            if((active&15)!=0 || (active&2048)!=0) {
                if(dx==0 && (active&2048)!=0)dx=events[42]<80?-1:events[42]>176?1:0;
                if(dy==0 && (active&2048)!=0)dy=events[43]<80?-1:events[43]>176?1:0;
                if(dx!=0 || dy!=0)want(5,100+dx*64,247+dy*64);
            }
            if((active&16)!=0)want(6,430,268);
            if((active&32)!=0)want(7,340,268);
            if((active&64)!=0)want(8,380,218);
            if((active&128)!=0)want(9,440,188);
            if((active&512)!=0)want(10,440,128);
            Controller.ButtonObject rage=pad(engine,6);
            if((active&256)!=0 && rage!=null && rage.Type==4 && rage.KeyCode[0]==0x21000000)
                want(11,rage.Pos[0],rage.Pos[1]);
            if((pressed&1024)!=0 && TCBManajer.iPlayMode!=8)pausePending=true;
            if(pausePending) {
                for(int i=5;i<12;i++)wanted[i]=false;
                blocked|=held;
                // Original pause accepts only pointer ID 0. Never evict a finger.
                if(!real[0]) { want(12,240,30); pausePending=false; }
            }
        } else if(current==1) {
            // One step per press; character animations/loading keep their timing.
            if((pressed&12)==4)want(13,16,128);
            else if((pressed&12)==8)want(14,464,128);
            else if((pressed&16)!=0 && TCBManajer.iChrSelectMode==0 &&
                matches(pad(engine,4),4,240,140,0x4100))want(15,240,140);
        }
        for(int i=0;i<5;i++)wanted[i]=real[i];
        for(int i=0;i<5;i++)if(owner[i]>=0 && (!wanted[owner[i]] || (real[i] && owner[i]!=i)))clear(gw.keyData,i);
        for(int i=0;i<5;i++)if(real[i]) {
            boolean begin=owner[i]!=i && realBegin[i]; owner[i]=i;
            gw.keyData.Set(x[i],y[i],begin?1:0,i);
        }
        for(int action=5;action<16;action++)if(wanted[action]) {
            int id=slot(action); boolean begin=id<0;
            if(action==12 && id<0) { if(owner[0]<0)id=0; }
            else if(id<0) {
                // Keep ID 0 available for real menu touches and Start when possible.
                for(int n=1;n<=5;n++){int candidate=n%5;if(owner[candidate]<0){id=candidate;break;}}
            }
            if(id<0)continue; // Original core has five pointers; excess waits.
            owner[id]=action; gw.keyData.Set(x[action],y[action],begin?1:0,id);
        }
    }
}
