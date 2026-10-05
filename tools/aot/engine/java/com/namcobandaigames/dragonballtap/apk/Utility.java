package com.namcobandaigames.dragonballtap.apk;

import org.teavm.interop.Address;
import android.content.Context;
import java.io.IOException;
public final class Utility {
    private static GlobalWork global;
    public static void SetGlobalWork(GlobalWork gw){global=gw;} public static GlobalWork GetGlobalWork(){return global;}
    public static String InttoString(int digits,int value){String s="0000000000000000"+value;return s.substring(s.length()-digits);}
    public static byte[] readDataRaw(Context c,String name)throws Exception {byte[] out=NativePlatform.read(name,false);if(out==null)throw new IOException("Missing raw resource: "+name);return out;}
    public static byte[] readDataFile(Context c,String name){return NativePlatform.read(name,name.equals("save.bin"));}
    public static byte[] readDataFile(Context c,String name,int off,int size){byte[] b=readDataFile(c,name);if(b==null||off<0||size<0||off>b.length-size)return null;byte[] out=new byte[size];System.arraycopy(b,off,out,0,size);return out;}
    public static boolean checkFile(Context c,String name){return readDataFile(c,name)!=null;}
    public static boolean writeDataFile(Context c,String name,byte[] b,boolean external){return writeDataFile(name,b,0,b.length,0,external,true);}
    public static boolean writeDataFile(String name,byte[] b,int off,int size,int pos,boolean external){return writeDataFile(name,b,off,size,pos,external,false);}
    private static boolean writeDataFile(String name,byte[] b,int off,int size,int pos,boolean external,boolean truncate){if(!name.equals("save.bin")||off<0||size<0||off>b.length-size||pos<0)return false;byte[] n=NativePlatform.cstr(name);return NativePlatform.writeSave(Address.ofData(n),Address.ofData(b).add(off),size,pos,truncate?1:0)!=0;}
    public static boolean deleteFile(Context c,String name){if(!name.equals("save.bin"))return false;byte[] n=NativePlatform.cstr(name);return NativePlatform.deleteSave(Address.ofData(n))!=0;}
    public static byte[] http2data(String url)throws Exception{throw new IOException("Offline HTTP: "+url);}
    public static void startBrowser(Context c,String url){NativePlatform.unavailable("Browser: "+url);}
    public static void SetVibrator(Context c,int milliseconds){if(milliseconds>0)System.err.println("Vita has no vibration motor");}
    public static String loadSharedPreferences(Context c,String name,String key)throws Exception{throw new IOException("Android account preferences unavailable");}
    public static void saveSharedPreferences(Context c,String name,String key,String value)throws Exception{throw new IOException("Android account preferences unavailable");}
    private static int u16(byte[] b,int p){if(p<0||p>b.length-2)throw new IndexOutOfBoundsException();return (b[p]&255)|((b[p+1]&255)<<8);}
    private static int u32(byte[] b,int p){return u16(b,p)|(u16(b,p+2)<<16);}
    public static int getPackCount(byte[] b,int base){return u16(b,base);}
    public static int getPackPos(byte[] b,int index,int base){int count=u16(b,base);if(index<0||index>=count)throw new IndexOutOfBoundsException();return base+2+count*8+u32(b,base+2+index*8);}
    public static int getPackSize(byte[] b,int index,int base){int count=u16(b,base);if(index<0||index>=count)throw new IndexOutOfBoundsException();return u32(b,base+6+index*8);}
    public static byte[] getPackBinary(byte[] b,int index){if(b==null)return null;int p=getPackPos(b,index,0),n=getPackSize(b,index,0);if(p<0||n<0||p>b.length-n)throw new IndexOutOfBoundsException();byte[] out=new byte[n];System.arraycopy(b,p,out,0,n);return out;}
}

