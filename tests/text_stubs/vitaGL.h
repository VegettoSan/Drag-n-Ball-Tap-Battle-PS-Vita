#pragma once
using GLuint=unsigned;using GLint=int;using GLenum=unsigned;
enum {GL_TEXTURE_BINDING_2D,GL_TEXTURE_2D,GL_RGBA,GL_UNSIGNED_BYTE,GL_TEXTURE_MIN_FILTER,GL_TEXTURE_MAG_FILTER,GL_LINEAR,GL_TEXTURE_WRAP_S,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE,GL_NO_ERROR};
inline void glGetIntegerv(GLenum,GLint* n){*n=0;}
inline void glBindTexture(GLenum,GLuint){}
inline void glGenTextures(int,GLuint* t){*t=1;}
inline void glTexParameteri(GLenum,GLenum,GLint){}
inline void glTexImage2D(GLenum,int,GLenum,int,int,int,GLenum,GLenum,const void*){}
inline void glTexSubImage2D(GLenum,int,int,int,int,int,GLenum,GLenum,const void*){}
inline void glDeleteTextures(int,const GLuint*){}
inline GLenum glGetError(){return GL_NO_ERROR;}
