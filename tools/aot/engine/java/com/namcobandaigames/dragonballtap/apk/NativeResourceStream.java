package com.namcobandaigames.dragonballtap.apk;

import java.io.InputStream;
import java.io.IOException;
import org.teavm.interop.Address;

/** Native cached PAC owner: no complete PAC byte[] in the managed heap. */
public final class NativeResourceStream extends InputStream {
    private int handle, position;
    private final int size;
    NativeResourceStream(int handle, int size) { this.handle=handle; this.size=size; }
    private void checkOpen() throws IOException { if(handle==0)throw new IOException("Resource stream closed"); }
    @Override public int read() throws IOException {
        byte[] one=new byte[1];
        return read(one,0,1)<0 ? -1 : one[0]&255;
    }
    @Override public int read(byte[] target,int offset,int length) throws IOException {
        checkOpen();
        if(target==null)throw new NullPointerException();
        if(offset<0||length<0||offset>target.length-length)throw new IndexOutOfBoundsException();
        if(length==0)return 0;
        if(position==size)return -1;
        int count=Math.min(length,size-position);
        if(NativePlatform.readResourceStream(handle,position,Address.ofData(target).add(offset),count)!=count)
            throw new IOException("Invalid native resource stream read");
        position+=count;
        return count;
    }
    @Override public long skip(long count) throws IOException {
        checkOpen();
        int skipped=(int)Math.min(Math.max(count,0L),(long)size-position);
        position+=skipped;
        return skipped;
    }
    @Override public int available() throws IOException { checkOpen();return size-position; }
    @Override public void close() { if(handle!=0){NativePlatform.closeResourceStream(handle);handle=0;} }
}
