import java.nio.*;import java.lang.reflect.*;import org.teavm.interop.Address;import com.namcobandaigames.dragonballtap.apk.VitaGles;
public class GlesBufferProbe{
 static Method address,bytes;static VitaGles gl=new VitaGles();
 static Address get(int slot,Buffer b)throws Exception{int p=b.position(),l=b.limit();Address a=(Address)address.invoke(gl,slot,b);if(p!=b.position()||l!=b.limit())throw new Error("position/limit changed");return a;}
 public static void main(String[] args)throws Exception{
 address=VitaGles.class.getDeclaredMethod("clientAddress",int.class,Buffer.class);address.setAccessible(true);bytes=VitaGles.class.getDeclaredMethod("clientBytes",Buffer.class);bytes.setAccessible(true);
 FloatBuffer f=ByteBuffer.allocateDirect(64).order(ByteOrder.nativeOrder()).asFloatBuffer();f.put(new float[]{9,1.5f,-2.5f,3,8});f.position(1);f.limit(4);Address a=get(2,f);float[] fs=(float[])a.array;if(fs[0]!=1.5f||fs[1]!=-2.5f||fs[2]!=3||(Integer)bytes.invoke(gl,f)!=12)throw new Error("direct float range");if(a.array!=get(2,f).array)throw new Error("steady allocation");
 FloatBuffer heap=FloatBuffer.wrap(new float[]{9,8,7,6,5});heap.position(1);FloatBuffer slice=heap.slice();slice.position(1);slice.limit(3);a=get(2,slice);if(a.array!=heap.array()||a.offset!=8||(Integer)bytes.invoke(gl,slice)!=8)throw new Error("slice offset");
 ShortBuffer sh=ByteBuffer.allocateDirect(20).order(ByteOrder.nativeOrder()).asShortBuffer();sh.put(new short[]{9,0,2,1,3});sh.position(1);sh.limit(5);a=get(3,sh.asReadOnlyBuffer());if(((short[])a.array)[2]!=1||(Integer)bytes.invoke(gl,sh)!=8)throw new Error("read-only indices");
 ByteBuffer bb=ByteBuffer.allocateDirect(12);bb.put(new byte[]{9,1,2,3,4});bb.position(1);bb.limit(5);a=get(0,bb);if(((byte[])a.array)[3]!=4||(Integer)bytes.invoke(gl,bb)!=4)throw new Error("direct bytes");
 f.limit(2);a=get(2,f);if(a.array!=fs||(Integer)bytes.invoke(gl,f)!=4)throw new Error("shrinking active range");
 try{bytes.invoke(gl,FloatBuffer.allocate(4097));throw new Error("oversized range accepted");}catch(InvocationTargetException e){if(!(e.getCause() instanceof IllegalArgumentException))throw e;}
 System.out.println("GLES BUFFER PASS: direct/read-only arrays, heap slices, active offsets/limits, reuse, bounds; native imports mocked");
 }}