import java.nio.file.*;
import java.security.MessageDigest;
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
    private static final String HASH = "6088ebbe8e714ce3d056c9bf029f8ce27307cb3276c6b39550425b94b4d50ad6";

    private static String digest(byte[] b) throws Exception {
        StringBuilder s = new StringBuilder();
        for (byte v : MessageDigest.getInstance("SHA-256").digest(b)) s.append(String.format("%02x", v & 255));
        return s.toString();
    }

    static byte[] adapt(byte[] bytes) throws Exception {
        if (!digest(bytes).equals(HASH)) throw new IOException("Unsupported GameData.class; use the pinned original APK/dex2jar 2.4");
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
        entries.put(CLASS + ".class", adapt(original));
        try (JarOutputStream jar = new JarOutputStream(Files.newOutputStream(out, StandardOpenOption.CREATE_NEW))) {
            for (Map.Entry<String, byte[]> e : entries.entrySet()) {
                JarEntry entry = new JarEntry(e.getKey()); entry.setTime(0);
                jar.putNextEntry(entry); jar.write(e.getValue()); jar.closeEntry();
            }
        }
        System.out.println("Adapted one resource overload; all other class payloads retained");
    }
}
