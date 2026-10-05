package com.namcobandaigames.dragonballtap.apk;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.*;
import java.util.Map;

/** Platform charset boundary. Mapping/error lengths are generated from the JDK
 * Shift_JIS decoder, never from game data or a guessed Windows code page. */
public final class VitaCharsets {
    public static void register(Map<String,Charset> map){Charset sjis=new Sjis();map.put("SHIFT_JIS",sjis);map.put("SJIS",sjis);}
    private static final class Sjis extends Charset {
        Sjis(){super("Shift_JIS",new String[]{"SJIS"});}
        public boolean contains(Charset other){return other.name().equals("US-ASCII")||other instanceof Sjis;}
        public CharsetDecoder newDecoder(){return new Decoder(this);}
        public CharsetEncoder newEncoder(){return new Encoder(this);}
    }
    private static final class Decoder extends CharsetDecoder {
        Decoder(Charset charset){super(charset,1,1);}
        protected CoderResult decodeLoop(ByteBuffer in,CharBuffer out){
            while(in.hasRemaining()){
                int at=in.position(),first=in.get(at)&255;
                char value=SjisMapping.single[first];int count=1;
                if(value==0xffff){
                    if(in.remaining()<2)return CoderResult.UNDERFLOW;
                    value=SjisMapping.pair[first*256+(in.get(at+1)&255)];count=2;
                }
                if(value>=0xfff8){int kind=value-0xfff8,length=(kind&3)+1;
                    return (kind&4)==0?CoderResult.malformedForLength(length):CoderResult.unmappableForLength(length);}
                if(!out.hasRemaining())return CoderResult.OVERFLOW;
                out.put(value);in.position(at+count);
            }return CoderResult.UNDERFLOW;
        }
    }
    private static final class Encoder extends CharsetEncoder {
        private final int[] reverse=new int[65536];
        Encoder(Charset charset){super(charset,1.5f,2);
            for(int i=0;i<reverse.length;i++)reverse[i]=-1;
            for(int i=0;i<SjisMapping.pair.length;i++)if(SjisMapping.pair[i]<0xfff8)reverse[SjisMapping.pair[i]]=i;
            for(int i=0;i<256;i++)if(SjisMapping.single[i]<0xfff8)reverse[SjisMapping.single[i]]=i;
            reverse[0x00a5]=0x5c;reverse[0x203e]=0x7e;
        }
        protected CoderResult encodeLoop(CharBuffer in,ByteBuffer out){while(in.hasRemaining()){
            char ch=in.get(in.position());int code=reverse[ch];
            if(code<0){if(Character.isHighSurrogate(ch)&&in.remaining()<2)return CoderResult.UNDERFLOW;
                if(Character.isSurrogate(ch))return CoderResult.malformedForLength(1);return CoderResult.unmappableForLength(1);}
            int n=code>255?2:1;if(out.remaining()<n)return CoderResult.OVERFLOW;
            if(n==2)out.put((byte)(code>>8));out.put((byte)code);in.get();
        }return CoderResult.UNDERFLOW;}
    }
}
