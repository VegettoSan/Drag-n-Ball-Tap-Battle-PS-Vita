package com.namcobandaigames.dragonballtap.apk;

// An absent transport: queries report disconnected; sending is rejected.
public final class BluetoothManajer {
    private static final BluetoothManajer instance=new BluetoothManajer();
    public static BluetoothManajer getInstance(){return instance;}
    public static void clearInstance(){} public void ReadStop(boolean stop){} public void cancel(){} public void clearData(){} public void dispose(){} public void reset(){}
    public boolean isConnect(){return false;} public boolean isServer(){return false;} public boolean checkSocket(){return false;}
    public byte[] getData(){return null;} public byte[] getSendData(){return null;} public int getReadDataSize(){return 0;}
    public void setData(byte[] data){NativePlatform.unavailable("Bluetooth send requested without a transport");}
}

