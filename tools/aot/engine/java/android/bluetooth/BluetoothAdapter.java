package android.bluetooth;

// The entry point explicitly disables Bluetooth after constructing GlobalWork.
public final class BluetoothAdapter { private static final BluetoothAdapter disabled=new BluetoothAdapter(); public static BluetoothAdapter getDefaultAdapter(){return disabled;} }
