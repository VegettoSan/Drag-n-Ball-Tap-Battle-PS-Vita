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
    static void battle() throws Exception {
        gw=(GlobalWork)allocate(GlobalWork.class);gw.keyData=new KeyData();gw.fScreenScale=320f/544;gw.iScreenOffsetX=42;
        engine=(TCBManajer)allocate(TCBManajer.class);engine.controller=new Controller();engine.padID=new int[10];
        int[][] pads={{90,237,230,1,0},{430,268,48,4,0x4100},{340,268,48,4,0x100000},
            {380,218,48,4,0x200000},{440,188,48,4,0x400000},{440,128,48,4,0x800000},{60,40,108,4,0x21000000}};
        for(int i=0;i<7;i++){int[]p=pads[i];engine.padID[i]=engine.controller.AddPad(p[0],p[1],p[2],0,p[3],p[4]);}
        TCBManajer.tcbHead=new TCB();active=new TCB();active.act=true;active.md=38;active._work[1]=0;
        TCBManajer.tcbHead.next=active;TCBManajer.iPlayerNo=0;TCBManajer.bGameStart=true;
        TCBManajer.bPause=false;TCBManajer.bDrawLoading=false;TCBManajer.bTaskSkip=false;TCBManajer.bResume=false;TCBManajer.iPlayMode=0;
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
        System.out.println("VITA CONTROLS JVM PASS: directions/buttons/holds/releases/five pointers/touch priority/pause/rearming/swapped L-R/character arrows/X confirmation and transition rearming/menu neutrality/analog");
    }
}
