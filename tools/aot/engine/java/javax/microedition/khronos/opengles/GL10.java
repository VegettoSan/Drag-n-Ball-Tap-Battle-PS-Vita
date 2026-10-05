package javax.microedition.khronos.opengles;

public interface GL10 {
    void glBindTexture(int target,int texture);
    void glBlendFunc(int src,int dst);
    void glClear(int mask);
    void glClearColor(float r,float g,float b,float a);
    void glColor4f(float r,float g,float b,float a);
    void glDisable(int cap);
    void glDisableClientState(int cap);
    void glEnable(int cap);
    void glEnableClientState(int cap);
    void glHint(int target,int mode);
    void glLoadIdentity();
    void glMatrixMode(int mode);
    void glOrthof(float l,float r,float b,float t,float near,float far);
    void glPopMatrix();
    void glPushMatrix();
    void glScalef(float x,float y,float z);
    void glShadeModel(int mode);
    void glTexEnvf(int target,int name,float value);
    void glTexParameterf(int target,int name,float value);
    void glTranslatef(float x,float y,float z);
    void glViewport(int x,int y,int w,int h);
    void glDeleteTextures(int n,int[] values,int offset);
    void glGenTextures(int n,int[] values,int offset);
    void glColorPointer(int size,int type,int stride,java.nio.Buffer data);
    void glTexCoordPointer(int size,int type,int stride,java.nio.Buffer data);
    void glVertexPointer(int size,int type,int stride,java.nio.Buffer data);
    void glDrawElements(int mode,int count,int type,java.nio.Buffer data);
    String glGetString(int name);
}
