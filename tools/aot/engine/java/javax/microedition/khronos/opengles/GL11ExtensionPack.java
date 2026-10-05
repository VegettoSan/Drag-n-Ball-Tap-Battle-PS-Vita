package javax.microedition.khronos.opengles;

public interface GL11ExtensionPack {
    void glBindFramebufferOES(int target,int id);
    int glCheckFramebufferStatusOES(int target);
    void glFramebufferTexture2DOES(int target,int attachment,int textarget,int texture,int level);
    void glDeleteFramebuffersOES(int n,int[] values,int offset);
    void glDeleteRenderbuffersOES(int n,int[] values,int offset);
    void glGenFramebuffersOES(int n,int[] values,int offset);
    void glGenRenderbuffersOES(int n,int[] values,int offset);
}
