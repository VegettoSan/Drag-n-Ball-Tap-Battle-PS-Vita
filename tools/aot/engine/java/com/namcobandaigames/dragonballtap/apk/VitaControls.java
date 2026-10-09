package com.namcobandaigames.dragonballtap.apk;

/** Vita-only pointer adapter. Never writes commands, tasks or controller keys. */
final class VitaControls {
    private final int mode;
    private final boolean[] wanted = new boolean[17];
    private final boolean[] real = new boolean[5];
    private final boolean[] realBegin = new boolean[5];
    private final int[] x = new int[17], y = new int[17];
    private final int[] owner = {-1,-1,-1,-1,-1};
    private int scene = -1, sceneMd = -1, blocked = 2047, previousHeld;
    private boolean pausePending;
    private boolean backPending;
    private TCB sceneTask, contextTask, backTask, sceneBackTask;
    private int sceneBackMd = -1;
    private static final int[] CONFIRMATION_MODES={704,851,264,286,62,74,112};

    VitaControls(int mode) { this.mode = mode; }

    private static Controller.ButtonObject pad(TCBManajer engine, int role) {
        int id = engine.padID[role];
        if (id < 0 || id >= engine.controller.gamePad.buttonCount) return null;
        return engine.controller.gamePad.buttonObj[id];
    }
    private static boolean matches(Controller.ButtonObject p, int type, int px, int py, int code) {
        return p != null && p.Type == type && p.Pos[0] == px && p.Pos[1] == py && p.KeyCode[0] == code;
    }
    private static TCB findTask(int md) {
        TCB head = TCBManajer.tcbHead;
        TCB current = head == null ? null : head.next;
        for (int n = 0; current != null && n < 1170; ++n, current = current.next)
            if (current.act && current.md == md && (md != 38 || current._work[1] == TCBManajer.iPlayerNo)) return current;
        return null;
    }
    // Only audited callers that pass the actual pointer coordinates to CheckBack.
    // Confirmation dialogs with sentinel coordinates are deliberately excluded.
    private static boolean backConsumer(int md) {
        switch(md) {
        case 688: case 692: case 694: case 695: case 696: case 699:
        case 847: case 855: case 246: case 283:
        case 127: case 131: case 141: case 143: case 146: case 149: case 158: case 179:
        case 353: case 360: case 368: case 375: case 385: case 409: case 416: case 421:
        case 1086: case 1104: case 1108: case 1117: case 761:
        case 1014: case 1024: case 1036: case 1048: case 1051: case 1057: case 1063:
            return true;
        default: return false;
        }
    }
    private static boolean confirmation() {
        // These original tasks pass sentinel coordinates to CheckBack. Keep
        // Yes/No choices tactile even when a parent back consumer is still live.
        for(int i=0;i<CONFIRMATION_MODES.length;i++)if(findTask(CONFIRMATION_MODES[i])!=null)return true;
        return false;
    }
    private int use(TCB t,int value) { contextTask=t; return value; }
    private int context(TCBManajer engine) {
        contextTask=null; backTask=null;
        if (TCBManajer.bDrawLoading || TCBManajer.bResume || confirmation()) return 0;
        if(TCBManajer.bPause && TCBManajer.iPlayMode==8)return 0;
        TCB head=TCBManajer.tcbHead;
        TCB t=head==null?null:head.next;
        for(int n=0;t!=null && n<1170;n++,t=t.next)
            if(t.act && backConsumer(t.md)) { backTask=t; break; }
        TCB pause=findTask(847);
        if(TCBManajer.bPause && pause!=null)return use(pause,3);
        if(!TCBManajer.bPause) {
            TCB dialog=findTask(811);
            // Dialogues remain tactile. Keep scripts ahead of lingering combat
            // tasks so X cannot become an attack behind a text screen.
            if(dialog!=null) {
                if(dialog._work[0]!=9)return use(dialog,4);
                // An automatic script ignores X, but cannot disable a separate
                // original menu's coordinate-back listener or expose combat.
                return backTask!=null?use(backTask,5):use(dialog,0);
            }
            TCB selection=findTask(1014);
            if(selection!=null && matches(pad(engine,0),3,100,0,0) &&
                matches(pad(engine,1),5,-50,20,0x4100) && matches(pad(engine,2),5,430,20,0x4100))return use(selection,1);
        }
        // A live audited coordinate consumer defines the back hit area. Menu
        // graphics vary; requiring the pause panel excluded all other layouts.
        // Run resets bTaskSkip after reading touches, so its previous-frame
        // value cannot gate fresh menu/script input in this platform adapter.
        if(backTask!=null)return use(backTask,5);
        TCB battle=findTask(38);
        if (!TCBManajer.bPause && !TCBManajer.bTaskSkip && TCBManajer.bGameStart && battle!=null &&
            matches(pad(engine,0),1,90,237,0) && matches(pad(engine,1),4,430,268,0x4100) &&
            matches(pad(engine,2),4,340,268,0x100000) && matches(pad(engine,3),4,380,218,0x200000) &&
            matches(pad(engine,4),4,440,188,0x400000) && matches(pad(engine,5),4,440,128,0x800000))return use(battle,2);
        return 0;
    }
    private void want(int action, int px, int py) { wanted[action] = true; x[action] = px; y[action] = py; }
    private int slot(int action) { for (int i=0;i<5;i++) if(owner[i]==action)return i; return -1; }
    private void clear(KeyData keys, int id) { keys.Clear(id); owner[id] = -1; }

    void update(GlobalWork gw, TCBManajer engine, int[] events, int count) {
        gw.bBackKey = false;
        for (int i=0;i<17;i++) wanted[i] = false;
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
        int currentMd=contextTask==null?-1:contextTask.md;
        int backMd=backTask==null?-1:backTask.md;
        blocked &= held;
        if(current!=scene || contextTask!=sceneTask || currentMd!=sceneMd || backTask!=sceneBackTask || backMd!=sceneBackMd) {
            blocked|=held; pausePending=false; backPending=false;
            scene=current; sceneTask=contextTask; sceneMd=currentMd;
            sceneBackTask=backTask; sceneBackMd=backMd;
        }
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
        if(backTask!=null && (current==1 || current==3 || current==4 || current==5)) {
            if((pressed&128)!=0 || (current==3 && (pressed&1024)!=0))backPending=true;
            if(backPending) {
                for(int i=5;i<17;i++)wanted[i]=false;
                blocked|=held;
                // CheckBack consumes ID 0. Never steal an active real finger.
                if(!real[0]) { want(16,40,24); backPending=false; }
            }
        }
        for(int i=0;i<5;i++)wanted[i]=real[i];
        for(int i=0;i<5;i++)if(owner[i]>=0 && (!wanted[owner[i]] || (real[i] && owner[i]!=i)))clear(gw.keyData,i);
        for(int i=0;i<5;i++)if(real[i]) {
            boolean begin=owner[i]!=i && realBegin[i]; owner[i]=i;
            gw.keyData.Set(x[i],y[i],begin?1:0,i);
        }
        for(int action=5;action<17;action++)if(wanted[action]) {
            int id=slot(action); boolean begin=id<0;
            if((action==12 || action==16) && id<0) { if(owner[0]<0)id=0; }
            else if(id<0) {
                // Keep ID 0 available for real menu touches and Start when possible.
                for(int n=1;n<=5;n++){int candidate=n%5;if(owner[candidate]<0){id=candidate;break;}}
            }
            if(id<0)continue; // Original core has five pointers; excess waits.
            owner[id]=action; gw.keyData.Set(x[action],y[action],begin?1:0,id);
        }
    }
}
