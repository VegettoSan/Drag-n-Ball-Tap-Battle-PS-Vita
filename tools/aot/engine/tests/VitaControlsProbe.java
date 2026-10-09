package com.namcobandaigames.dragonballtap.apk;

// Own test harness: original Controller/KeyData/TCB classes come from the APK.
// Engine construction is skipped because its Android resource service is absent.
public final class VitaControlsProbe {
    static void check(boolean ok,String reason){if(!ok)throw new IllegalStateException(reason);}
    static Object allocate(Class<?> type)throws Exception {
        Class<?> u=Class.forName("sun.misc.Unsafe"); java.lang.reflect.Field f=u.getDeclaredField("theUnsafe");
        f.setAccessible(true);Object instance=f.get(null);return u.getMethod("allocateInstance",Class.class).invoke(instance,type);
    }
    static GlobalWork gw; static TCBManajer engine; static TCB active;
    static int[] events=new int[45];static VitaControls controls;
    static TCB addTask(int md) { TCB t=new TCB();t.act=true;t.md=md;t.next=TCBManajer.tcbHead.next;TCBManajer.tcbHead.next=t;return t; }
    static void consumeTouches() {
        TCBManajer.iTouchStatus=0;
        for(int id=0;id<5;id++) {
            TCBManajer.TouchesXPos[id]=gw.keyData.getX(id);
            TCBManajer.TouchesYPos[id]=gw.keyData.getY(id);
            TCBManajer.TouchesStatus[id]=gw.keyData.isBeginTouch(id)?1:0;
            if(TCBManajer.TouchesStatus[id]!=0)TCBManajer.iTouchStatus=1;
        }
    }
    static void originalScriptTap(String reason) {
        consumeTouches();TCBManajer.tcbNow=active;
        TCBManajer.bTaskNext=false;TCBManajer.bTaskRepeat=false;
        engine.Game4(gw,811);
        check(TCBManajer.bTaskNext && TCBManajer.bTaskRepeat,reason+" original script did not advance");
        gw.keyData.ClearBegin();
    }
    static void backTap(String reason) {
        check(gw.keyData.isBeginTouch(0) && gw.keyData.getX(0)==40 && gw.keyData.getY(0)==24,reason+" pointer");
        check(engine.CheckBack(gw,gw.keyData.getX(0),gw.keyData.getY(0),1),reason+" original CheckBack");
        check(!gw.bBackKey,reason+" Android Back injected");
    }
    static void battle() throws Exception {
        gw=(GlobalWork)allocate(GlobalWork.class);gw.keyData=new KeyData();gw.fScreenScale=320f/544;gw.iScreenOffsetX=42;
        engine=(TCBManajer)allocate(TCBManajer.class);engine.controller=new Controller();engine.padID=new int[10];
        int[][] pads={{90,237,230,1,0},{430,268,48,4,0x4100},{340,268,48,4,0x100000},
            {380,218,48,4,0x200000},{440,188,48,4,0x400000},{440,128,48,4,0x800000},{60,40,108,4,0x21000000}};
        for(int i=0;i<7;i++){int[]p=pads[i];engine.padID[i]=engine.controller.AddPad(p[0],p[1],p[2],0,p[3],p[4]);}
        TCBManajer.tcbHead=new TCB();active=new TCB();active.act=true;active.md=38;active._work[1]=0;
        TCBManajer.tcbHead.next=active;TCBManajer.iPlayerNo=0;TCBManajer.bGameStart=true;
        TCBManajer.bPause=false;TCBManajer.bDrawLoading=false;TCBManajer.bTaskSkip=false;TCBManajer.bResume=false;TCBManajer.iPlayMode=0;
        TCBManajer.iDemoPushXPos=-1;TCBManajer.iDemoPushYPos=-1;TCBManajer.iTextEnd=0;TCBManajer.wSystemFlag=0;
        events=new int[45];TCBManajer.bBackKeyPush=false;
        controls=new VitaControls(1);step(0);
    }
    static void step(int held) { events[41]=held;events[42]=events[43]=128;controls.update(gw,engine,events,0);engine.controller.SetKey(gw.keyData);gw.keyData.ClearBegin(); }
    static void neutral(){step(0);for(int i=0;i<7;i++)check(engine.controller.GetKey(i,2)==0,"stuck pad "+i);}
    public static void main(String[] args)throws Exception {
        int[] bits={1,9,8,10,2,6,4,5}, masks={1,9,8,10,2,6,4,5};
        for(int i=0;i<bits.length;i++){battle();step(bits[i]);check(engine.controller.GetKey(0,2)==masks[i],"direction "+i);neutral();}
        for(int i=0;i<6;i++){battle();int button=i<4?16<<i:i==4?512:256;step(button);int mask=engine.controller.GetKey(i+1,2);check(mask!=0,"button "+i);step(button);check(engine.controller.GetKey(i+1,0)==0 && engine.controller.GetKey(i+1,2)==mask,"hold "+i);step(0);check(engine.controller.GetKey(i+1,1)==mask,"release "+i);neutral();}
        battle();step(8|16|32|64|256);for(int i:new int[]{0,1,2,3,6})check(engine.controller.GetKey(i,2)!=0,"five simultaneous");
        // A real finger takes its native ID 0 even if a synthetic button used it.
        events[0]=0;events[1]=480;events[2]=272;events[3]=0;events[41]=8|16|32|64|256;
        controls.update(gw,engine,events,1);check(gw.keyData.getX(0)==240,"touch priority");
        events[3]=2;controls.update(gw,engine,events,1);neutral();
        battle();step(16);TCBManajer.bPause=true;step(16);check(engine.controller.GetKey(1,2)==0,"pause releases");
        TCBManajer.bPause=false;step(16);check(engine.controller.GetKey(1,2)==0,"held button rearmed early");step(0);step(16);check(engine.controller.GetKey(1,0)==0x4100,"release/repress after pause");
        battle();step(1024);check(gw.keyData.getX(0)==240 && gw.keyData.getY(0)==30,"Start contact 0");step(1024);check(gw.keyData.GetIndex(0)<0,"Start hold repeated");
        battle();active.md=1014;engine.controller.Init();engine.padID[0]=engine.controller.AddPad(100,0,280,320,3,0);
        engine.padID[1]=engine.controller.AddPad(-50,20,120,320,5,0x4100);engine.padID[2]=engine.controller.AddPad(430,20,120,320,5,0x4100);
        engine.padID[4]=engine.controller.AddPad(240,140,360,90,4,0x4100);TCBManajer.iChrSelectMode=0;
        step(0);step(4);check(engine.controller.GetKey(1,0)==0x4100,"character left");step(4);check(engine.controller.GetKey(1,2)==0,"character hold repeats");step(0);step(8);check(engine.controller.GetKey(2,0)==0x4100,"character right");
        step(0);step(16);check(engine.controller.GetKey(engine.padID[4],0)==0x4100,"character X confirm");
        step(16);check(engine.controller.GetKey(engine.padID[4],2)==0,"character X repeats while held");
        active.md=1004;step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"character X leaks to next screen");
        active.md=1014;step(16);check(engine.controller.GetKey(engine.padID[4],2)==0,"held X rearmed at selection entry");
        step(0);step(16);check(engine.controller.GetKey(engine.padID[4],0)==0x4100,"fresh X after transition");
        step(0);TCBManajer.iChrSelectMode=1;step(16);check(engine.controller.GetKey(engine.padID[4],2)==0,"X in character information");
        TCBManajer.iChrSelectMode=0;active.md=1004;step(0);step(16|32|64|128|256|512|1024);
        for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"menu button leaks");
        battle();controls=new VitaControls(0);step(0);step(2047);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"touch mode injects");
        battle();events[41]=0;events[42]=50;events[43]=50;controls.update(gw,engine,events,0);engine.controller.SetKey(gw.keyData);check(engine.controller.GetKey(0,2)==5,"analog diagonal");
        check(!gw.bBackKey,"Android Back injected");
        // Pointer 0 works through actual original back consumers, without
        // fabricated pause-panel sprites in every unrelated menu fixture.
        battle();step(1024);TCBManajer.bPause=true;TCBManajer.bTaskSkip=true;active.md=847;
        step(1024);check(gw.keyData.GetIndex(0)<0,"held Start resumed immediately");
        step(0);events[41]=1024;controls.update(gw,engine,events,0);backTap("Start resume");
        gw.keyData.ClearBegin();step(1024);check(gw.keyData.GetIndex(0)<0,"held resume repeated");
        step(0);events[41]=128;controls.update(gw,engine,events,0);backTap("Circle in pause");
        gw.keyData.ClearBegin();
        int[] menus={688,692,694,695,696,699,855,246,283,127,131,141,143,146,149,158,179,
            353,360,368,375,385,409,416,421,1086,1104,1108,1117,761,1014,1024,1036,1048,1051,1057,1063};
        for(int md:menus) {
            battle();active.md=md;TCBManajer.bTaskSkip=true;step(0);
            events[41]=128;controls.update(gw,engine,events,0);backTap("menu "+md);
            gw.keyData.ClearBegin();step(128);check(gw.keyData.GetIndex(0)<0,"held Circle repeats "+md);
        }
        // Execute a real original menu branch after the emitted touch. No
        // panel object or fake menu-side adapter logic participates.
        battle();active.md=692;step(0);events[41]=128;controls.update(gw,engine,events,0);
        consumeTouches();TCBManajer.tcbNow=active;engine.Game1(gw,692);
        check(active.md==693,"original menu did not consume Circle back");gw.keyData.ClearBegin();
        // Returning menus take precedence over stale battle state/pads.
        battle();TCB menu=addTask(692);step(0);events[41]=128;controls.update(gw,engine,events,0);backTap("menu over stale battle");
        gw.keyData.ClearBegin();menu.md=694;step(128);check(gw.keyData.GetIndex(0)<0,"held Circle crosses menu");
        step(0);events[41]=128;controls.update(gw,engine,events,0);backTap("fresh Circle next menu");gw.keyData.ClearBegin();
        for(int md:new int[]{704,851,264,286,62,74,112}) {
            battle();active.md=692;addTask(md);step(0);step(128|16|1024);
            for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"confirmation accepts shortcuts "+md);
        }
        battle();active.md=1004;step(0);step(128);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"unknown menu back injected");
        battle();active.md=692;TCBManajer.bDrawLoading=true;step(0);step(128);check(gw.keyData.GetIndex(0)<0,"Circle during loading");
        TCBManajer.bDrawLoading=false;step(128);check(gw.keyData.GetIndex(0)<0,"held Circle after loading");
        // ID 0 belongs to a real finger until End; pending back cancels when its
        // task identity or frozen mode changes, including simultaneous scripts.
        step(0);events[0]=0;events[1]=480;events[2]=272;events[3]=0;events[41]=128;
        controls.update(gw,engine,events,1);check(gw.keyData.getY(0)==160,"back stole finger 0");
        events[3]=2;events[41]=0;controls.update(gw,engine,events,1);backTap("queued Circle");gw.keyData.ClearBegin();
        step(0);events[0]=0;events[3]=0;events[41]=128;controls.update(gw,engine,events,1);
        active.md=694;events[3]=2;events[41]=0;controls.update(gw,engine,events,1);
        check(gw.keyData.GetIndex(0)<0,"pending back crossed consumer md change");
        // Execute Game4's real any-screen Begin consumer at the text-end branch,
        // and its interactive script branch with no demo marker or text object.
        battle();active.md=811;active._work[0]=8;TCBManajer.bTaskSkip=true;
        TCBManajer.iDemoPushXPos=30;TCBManajer.iDemoPushYPos=-1;TCBManajer.iTextEnd=1;
        step(0);events[41]=16;controls.update(gw,engine,events,0);
        check(TCBManajer.iTextEnd==1,"adapter changed text state");originalScriptTap("finished dialogue X");
        step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"held X repeats text");
        step(0);TCBManajer.iDemoPushXPos=-1;TCBManajer.iTextEnd=0;active._work[7]=0;
        events[41]=16;controls.update(gw,engine,events,0);check(TCBManajer.iTextEnd==0,"adapter rewrote markerless text");
        originalScriptTap("markerless any-screen dialogue X");
        step(0);active._work[0]=9;step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"noninteractive script skipped");
        active._work[0]=8;step(0);TCBManajer.bDrawLoading=true;step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"X text during loading");
        TCBManajer.bDrawLoading=false;step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"held X after loading");
        battle();TCB dialog=addTask(811);dialog._work[0]=8;step(0);events[41]=16;controls.update(gw,engine,events,0);
        boolean textTap=false;for(int id=0;id<5;id++)textTap|=gw.keyData.isBeginTouch(id) && gw.keyData.getY(id)==280;
        check(textTap,"dialog failed to override lingering combat task");gw.keyData.ClearBegin();
        dialog._work[0]=9;step(0);step(16);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"noninteractive script fell through to combat");
        // A script can coexist with a coordinate-based back consumer.
        battle();active.md=811;active._work[0]=8;menu=addTask(692);step(0);
        events[41]=128;controls.update(gw,engine,events,0);backTap("back alongside dialogue");gw.keyData.ClearBegin();
        step(0);events[0]=0;events[3]=0;events[41]=128;controls.update(gw,engine,events,1);
        menu.md=694;events[3]=2;events[41]=0;controls.update(gw,engine,events,1);
        check(gw.keyData.GetIndex(0)<0,"dialog pending back crossed consumer change");
        battle();TCBManajer.bPause=true;TCBManajer.bTaskSkip=true;active.md=847;TCBManajer.iPlayMode=8;step(0);step(1024);for(int id=0;id<5;id++)check(gw.keyData.GetIndex(id)<0,"Start Bluetooth pause");
        System.out.println("VITA CONTROLS JVM PASS: combat/selection regression; Start resume; 37 back consumers without pause sprites; original Game1 navigation; pointer-0 priority; original Game4 markerless/finished dialogue; holds/transitions/loading/confirmation exclusions");
    }
}
