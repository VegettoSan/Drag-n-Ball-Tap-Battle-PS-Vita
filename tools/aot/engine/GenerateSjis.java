import java.nio.*;
import java.nio.charset.*;
import java.nio.file.*;
import java.io.*;
/** Produce the platform's JDK Shift_JIS decode table and patch only TeaVM's
 * private charset registry. No original game class is changed. */
public final class GenerateSjis {
    static final Charset CHARSET=Charset.forName("Shift_JIS");
    static int entry(byte[] bytes){
        CharsetDecoder decoder=CHARSET.newDecoder();ByteBuffer in=ByteBuffer.wrap(bytes);CharBuffer out=CharBuffer.allocate(2);
        CoderResult result=decoder.decode(in,out,true);
        if(result.isError())return 0xfff8+(result.isMalformed()?0:4)+result.length()-1;
        if(out.position()!=1||in.position()!=bytes.length)throw new IllegalStateException("Unexpected decoder cardinality");
        return out.get(0);
    }
    static boolean lead(int b){CharsetDecoder d=CHARSET.newDecoder();return d.decode(ByteBuffer.wrap(new byte[]{(byte)b}),CharBuffer.allocate(2),false).isUnderflow() && b>=128 && d.decode(ByteBuffer.wrap(new byte[]{(byte)b}),CharBuffer.allocate(2),true).isMalformed();}
    public static void main(String[] args)throws Exception{
        Path out=Paths.get(args[0]);if(Files.exists(out))throw new IOException("Refusing to overwrite");
        try(PrintWriter p=new PrintWriter(Files.newBufferedWriter(out,StandardOpenOption.CREATE_NEW))){
            p.println("package com.namcobandaigames.dragonballtap.apk; public final class SjisMapping {");
            p.println("public static final char[] single=new char[256],pair=new char[65536]; static {");
            for(int b=0;b<256;b++){boolean isLead=(b>=0x81&&b<=0x9f)||(b>=0xe0&&b<=0xfc);p.println("single["+b+"]="+(isLead?65535:entry(new byte[]{(byte)b}))+";");}
            for(int start=0;start<65536;start+=256){StringBuilder hex=new StringBuilder();
                for(int i=start;i<start+256;i++){int b=i>>8;int value=((b>=0x81&&b<=0x9f)||(b>=0xe0&&b<=0xfc))?entry(new byte[]{(byte)b,(byte)i}):0xfff8;hex.append(String.format("%04x",value));}
                p.println("fill("+start+",\""+hex+"\");");}
            p.println("} private static void fill(int at,String s){for(int i=0;i<s.length();i+=4)pair[at++]=(char)Integer.parseInt(s.substring(i,i+4),16);} }");
        }
        if(args.length>1){Path nativeOut=Paths.get(args[1]);
            try(PrintWriter p=new PrintWriter(Files.newBufferedWriter(nativeOut,StandardOpenOption.CREATE_NEW))){
                p.println("#include <stdint.h>\nconst uint16_t dbtb_sjis_encoding[65536]={");
                CharsetEncoder encoder=CHARSET.newEncoder();
                for(int c=0;c<65536;c++){int value=65535;
                    try{ByteBuffer encoded=encoder.encode(CharBuffer.wrap(new char[]{(char)c}));
                        if(encoded.remaining()==1)value=encoded.get()&255;
                        else if(encoded.remaining()==2)value=((encoded.get()&255)<<8)|(encoded.get()&255);
                    }catch(CharacterCodingException ignored){}
                    p.print(value+",");if((c&31)==31)p.println();
                }p.println("};");
            }
        }
    }
}
