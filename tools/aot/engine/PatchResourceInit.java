import java.nio.file.*;
import java.util.*;
import java.util.jar.*;
import java.io.*;
import org.objectweb.asm.*;

/** Adapt one Android file-loading overload, keeping the original byte loader.
 * Input and output contain user-owned APK code and must remain outside Git.
 */
public final class PatchResourceInit implements Opcodes {
    private static final String PKG = "com/namcobandaigames/dragonballtap/apk/";
    private static final String CLASS = PKG + "GameData";
    private static final String GW = "L" + PKG + "GlobalWork;";
    private static final String DESC = "(" + GW + "Ljava/lang/String;II)Z";
    private static final String BYTE_DESC = "(" + GW + "[BII)Z";

    // dex2jar 2.4 is not byte-for-byte deterministic for this large class: its
    // generated constant-pool/label order can vary between otherwise identical
    // conversions. Verify the exact field boundary we rely on instead of a
    // brittle whole-class hash.
    static void verifyByteDefault(byte[] bytes) throws Exception {
        if (bytes == null) throw new IOException("TCBManajer.class absent");
        ClassReader cr = new ClassReader(bytes);
        if (!cr.getClassName().equals(PKG + "TCBManajer")) throw new IOException("Unexpected TCBManajer class name");
        final int[] fields = {0};
        cr.accept(new ClassVisitor(ASM9) {
            @Override public FieldVisitor visitField(int access, String name, String descriptor, String signature, Object value) {
                if (name.equals("bEventFlagBuf")) {
                    fields[0]++;
                    if ((access & ACC_STATIC) == 0 || !descriptor.equals("B") || value != null)
                        throw new IllegalStateException("Unexpected bEventFlagBuf shape");
                }
                return null;
            }
        }, ClassReader.SKIP_CODE | ClassReader.SKIP_DEBUG | ClassReader.SKIP_FRAMES);
        if (fields[0] != 1) throw new IOException("Expected exactly one static byte bEventFlagBuf with no ConstantValue");
    }

    // Verify the Android-only overload before replacing it. The byte-array
    // overload is the original parser and must remain present and untouched.
    static void verifyGameData(byte[] bytes) throws Exception {
        if (bytes == null) throw new IOException("GameData.class absent");
        ClassReader cr = new ClassReader(bytes);
        if (!cr.getClassName().equals(CLASS)) throw new IOException("Unexpected GameData class name");
        final int[] android = {0}, bytesInit = {0};
        final int[] miner = {0}, field = {0}, resources = {0}, raw = {0}, reads = {0}, closes = {0};
        cr.accept(new ClassVisitor(ASM9) {
            @Override public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
                if (name.equals("Init") && descriptor.equals(BYTE_DESC)) bytesInit[0]++;
                if (!name.equals("Init") || !descriptor.equals(DESC)) return null;
                android[0]++;
                return new MethodVisitor(ASM9) {
                    @Override public void visitMethodInsn(int opcode, String owner, String name, String descriptor, boolean isInterface) {
                        if (owner.equals(PKG + "ResourceMiner") && name.equals("getInstance")) miner[0]++;
                        if (owner.equals(PKG + "ResourceMiner") && name.equals("getFieldValue")) field[0]++;
                        if (owner.equals("android/content/Context") && name.equals("getResources")) resources[0]++;
                        if (owner.equals("android/content/res/Resources") && name.equals("openRawResource")) raw[0]++;
                        if (owner.equals("java/io/InputStream") && name.equals("read")) reads[0]++;
                        if (owner.equals("java/io/InputStream") && name.equals("close")) closes[0]++;
                    }
                };
            }
        }, ClassReader.SKIP_DEBUG | ClassReader.SKIP_FRAMES);
        if (android[0] != 1 || bytesInit[0] != 1 || miner[0] != 1 || field[0] != 1 ||
            resources[0] != 1 || raw[0] != 1 || reads[0] < 1 || closes[0] < 1)
            throw new IOException("Unsupported GameData Android loader shape: init=" + android[0] + "/" + bytesInit[0] +
                " miner=" + miner[0] + "/" + field[0] + " resources=" + resources[0] + "/" + raw[0] +
                " read/close=" + reads[0] + "/" + closes[0]);
    }

    // Retain the original GetString table traversal and String constructor;
    // replace only its platform encoding token, selected per resolved table.
    static byte[] adaptText(byte[] bytes) throws Exception {
        verifyByteDefault(bytes);
        ClassReader cr=new ClassReader(bytes);
        ClassWriter cw=new ClassWriter(cr,ClassWriter.COMPUTE_MAXS);
        final int[] found={0}, formatted={0};
        cr.accept(new ClassVisitor(ASM9,cw) {
            @Override public MethodVisitor visitMethod(int access,String name,String desc,String signature,String[] exceptions) {
                MethodVisitor mv=super.visitMethod(access,name,desc,signature,exceptions);
                boolean get=name.equals("GetString") && desc.equals("("+GW+"III)Ljava/lang/String;");
                boolean format=name.equals("SetString") && desc.equals("("+GW+"IIIIIIIIIIII)V");
                if(!get && !format)return mv;
                return new MethodVisitor(ASM9,mv) {
                    @Override public void visitLdcInsn(Object value) {
                        if("Shift_JIS".equals(value)) {
                            if(get){found[0]++;super.visitVarInsn(ILOAD,2);
                                super.visitMethodInsn(INVOKESTATIC,PKG+"ResourceAdapter","textCharset","(I)Ljava/lang/String;",false);
                            }else{formatted[0]++;super.visitVarInsn(ALOAD,0);super.visitVarInsn(ILOAD,5);super.visitVarInsn(ILOAD,2);
                                super.visitMethodInsn(INVOKESTATIC,PKG+"ResourceAdapter","stringCharset","(L"+PKG+"TCBManajer;II)Ljava/lang/String;",false);
                            }
                        } else super.visitLdcInsn(value);
                    }
                };
            }
        },0);
        if(found[0]!=1||formatted[0]!=4)throw new IOException("Expected one GetString and four SetString charset boundaries: "+found[0]+"/"+formatted[0]+"");
        return cw.toByteArray();
    }

    static byte[] adapt(byte[] bytes) throws Exception {
        verifyGameData(bytes);
        ClassReader cr = new ClassReader(bytes);
        ClassWriter cw = new ClassWriter(cr, ClassWriter.COMPUTE_MAXS);
        final int[] found = {0};
        cr.accept(new ClassVisitor(ASM9, cw) {
            @Override public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
                if (!name.equals("Init") || !descriptor.equals(DESC)) return super.visitMethod(access, name, descriptor, signature, exceptions);
                found[0]++;
                MethodVisitor mv = super.visitMethod(access, name, descriptor, signature, exceptions);
                mv.visitCode();
                mv.visitVarInsn(ALOAD, 0);
                mv.visitVarInsn(ALOAD, 1);
                mv.visitVarInsn(ALOAD, 2);
                mv.visitVarInsn(ILOAD, 3);
                mv.visitVarInsn(ILOAD, 4);
                mv.visitMethodInsn(INVOKESTATIC, PKG + "ResourceAdapter", "load",
                    "(L" + CLASS + ";" + GW + "Ljava/lang/String;II)Z", false);
                mv.visitInsn(IRETURN);
                mv.visitMaxs(0, 0);
                mv.visitEnd();
                return null;
            }
        }, 0);
        if (found[0] != 1) throw new IOException("Expected exactly one Android resource overload");
        return cw.toByteArray();
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 2) throw new IllegalArgumentException("input-original.jar output-private.jar");
        Path out = Paths.get(args[1]);
        if (Files.exists(out)) throw new IOException("Refusing to overwrite " + out);
        // Preflight and keep all other class payloads byte-for-byte identical.
        Map<String, byte[]> entries = new LinkedHashMap<>();
        try (JarFile jar = new JarFile(args[0])) {
            Enumeration<JarEntry> e = jar.entries();
            while (e.hasMoreElements()) {
                JarEntry entry = e.nextElement();
                if (entries.containsKey(entry.getName())) throw new IOException("Duplicate JAR entry");
                try (InputStream in = jar.getInputStream(entry); ByteArrayOutputStream b = new ByteArrayOutputStream()) {
                    byte[] buffer = new byte[8192];
                    for (int n; (n = in.read(buffer)) != -1;) b.write(buffer, 0, n);
                    entries.put(entry.getName(), b.toByteArray());
                }
            }
        }
        byte[] original = entries.get(CLASS + ".class");
        if (original == null) throw new IOException("Original GameData absent");
        verifyByteDefault(entries.get(PKG + "TCBManajer.class"));
        entries.put(CLASS + ".class", adapt(original));
        entries.put(PKG+"TCBManajer.class",adaptText(entries.get(PKG+"TCBManajer.class")));
        try (JarOutputStream jar = new JarOutputStream(Files.newOutputStream(out, StandardOpenOption.CREATE_NEW))) {
            for (Map.Entry<String, byte[]> e : entries.entrySet()) {
                JarEntry entry = new JarEntry(e.getKey()); entry.setTime(0);
                jar.putNextEntry(entry); jar.write(e.getValue()); jar.closeEntry();
            }
        }
        System.out.println("Adapted verified Android resource I/O and text encoding boundaries; all other class payloads retained");
    }
}
