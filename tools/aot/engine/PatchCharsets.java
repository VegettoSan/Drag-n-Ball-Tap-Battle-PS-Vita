import java.nio.file.*;
import java.io.*;
import java.util.jar.*;
import org.objectweb.asm.*;
public final class PatchCharsets implements Opcodes {
    public static void main(String[] args)throws Exception{
        byte[] bytes;String owner="org/teavm/classlib/java/nio/charset/TCharset$Charsets";
        try(JarFile jar=new JarFile(args[0]);InputStream in=jar.getInputStream(jar.getJarEntry(owner+".class"));ByteArrayOutputStream out=new ByteArrayOutputStream()){
            byte[] buffer=new byte[4096];for(int n;(n=in.read(buffer))!=-1;)out.write(buffer,0,n);bytes=out.toByteArray();}
        ClassWriter writer=new ClassWriter(ClassWriter.COMPUTE_MAXS);final int[] found={0};
        new ClassReader(bytes).accept(new ClassVisitor(ASM9,writer){
            public MethodVisitor visitMethod(int access,String name,String descriptor,String signature,String[] exceptions){
                MethodVisitor original=super.visitMethod(access,name,descriptor,signature,exceptions);
                if(!name.equals("<clinit>"))return original;
                return new MethodVisitor(ASM9,original){public void visitInsn(int opcode){if(opcode==RETURN){found[0]++;
                    super.visitFieldInsn(GETSTATIC,owner,"value","Ljava/util/Map;");
                    super.visitMethodInsn(INVOKESTATIC,"com/namcobandaigames/dragonballtap/apk/VitaCharsets","register","(Ljava/util/Map;)V",false);}
                    super.visitInsn(opcode);}};
            }
        },0);
        if(found[0]!=1)throw new IOException("Unexpected TeaVM registry shape");
        byte[] buffers;String buffer="org/teavm/classlib/java/nio/TByteBuffer";
        try(JarFile jar=new JarFile(args[0]);InputStream in=jar.getInputStream(jar.getJarEntry(buffer+".class"));ByteArrayOutputStream out=new ByteArrayOutputStream()){
            byte[] block=new byte[4096];for(int n;(n=in.read(block))!=-1;)out.write(block,0,n);buffers=out.toByteArray();}
        ClassReader reader=new ClassReader(buffers);ClassWriter bufferWriter=new ClassWriter(reader,ClassWriter.COMPUTE_MAXS);final int[] adapted={0};
        reader.accept(new ClassVisitor(ASM9,bufferWriter){
            public MethodVisitor visitMethod(int access,String name,String descriptor,String signature,String[] exceptions){
                MethodVisitor mv=super.visitMethod(access,name,descriptor,signature,exceptions);
                if(!name.equals("allocateDirect")||!descriptor.equals("(I)L"+buffer+";"))return mv;
                adapted[0]++;mv.visitCode();mv.visitVarInsn(ILOAD,0);
                mv.visitMethodInsn(INVOKESTATIC,buffer,"allocate","(I)L"+buffer+";",false);
                mv.visitInsn(ARETURN);mv.visitMaxs(0,0);mv.visitEnd();return null;
            }
        },0);
        if(adapted[0]!=1)throw new IOException("Unexpected TeaVM direct buffer shape");
        // Native GL owns copied client arrays, so direct JNI storage is neither
        // needed nor retained. Heap-backed views keep order/position/data while
        // avoiding TeaVM 0.12.3's broken weak direct-buffer GC list.
        try(JarOutputStream jar=new JarOutputStream(Files.newOutputStream(Paths.get(args[1]),StandardOpenOption.CREATE_NEW))){
            jar.putNextEntry(new JarEntry(owner+".class"));jar.write(writer.toByteArray());jar.closeEntry();
            jar.putNextEntry(new JarEntry(buffer+".class"));jar.write(bufferWriter.toByteArray());jar.closeEntry();}
    }
}
