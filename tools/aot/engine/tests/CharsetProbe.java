import java.nio.*;
import java.nio.charset.*;
import java.util.*;
import com.namcobandaigames.dragonballtap.apk.VitaCharsets;
/** Exhaustive two-byte decoding parity, including error replacement/lengths. */
public final class CharsetProbe {
    public static void main(String[] args){
        Map<String,Charset> map=new HashMap<>();VitaCharsets.register(map);Charset candidate=map.get("SHIFT_JIS");
        int cases=0;
        long hash=1469598103934665603L;
        for(int size=1;size<=2;size++){int count=size==1?256:65536;
            for(int n=0;n<count;n++){byte[] input=size==1?new byte[]{(byte)n}:new byte[]{(byte)(n>>8),(byte)n};
                String actual=candidate.decode(ByteBuffer.wrap(input)).toString();
                if(args.length==0){String expected=new String(input,Charset.forName("Shift_JIS"));
                    if(!expected.equals(actual))throw new AssertionError("SJIS mismatch "+size+":"+Integer.toHexString(n));}
                hash=(hash^actual.length())*1099511628211L;
                for(int i=0;i<actual.length();i++)hash=(hash^actual.charAt(i))*1099511628211L;
                cases++;
            }
        }
        System.out.println("SHIFT_JIS DECODE PARITY CASES="+cases+" HASH="+Long.toHexString(hash));
    }
}
