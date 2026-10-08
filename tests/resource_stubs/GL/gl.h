#pragma once
#include <vector>
#include <unordered_map>
#include <cassert>
using GLuint=unsigned;using GLint=int;using GLenum=unsigned;
enum {GL_NO_ERROR=0,GL_TEXTURE_BINDING_2D=1,GL_TEXTURE_2D,GL_RGBA,GL_UNSIGNED_BYTE,GL_TEXTURE_MIN_FILTER,GL_TEXTURE_MAG_FILTER,GL_LINEAR,GL_NEAREST,GL_TEXTURE_WRAP_S,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE,GL_UNSIGNED_SHORT_4_4_4_4};
static GLuint mock_next_texture=1;
static GLint mock_bound_texture=0;
static unsigned mock_uploads=0,mock_deletes=0;
static std::unordered_map<GLuint,std::vector<unsigned char>> mock_pixels;
inline void glGetIntegerv(GLenum,GLint* n){*n=mock_bound_texture;}
inline void glBindTexture(GLenum,GLuint id){mock_bound_texture=id;}
inline void glGenTextures(int n,GLuint* ids){for(int i=0;i<n;++i)ids[i]=mock_next_texture++;}
inline void glTexParameteri(GLenum,GLenum,GLint){}
inline void glTexImage2D(GLenum,int,GLenum,int w,int h,int,GLenum,GLenum type,const void* data){
 ++mock_uploads;const auto* b=static_cast<const unsigned char*>(data);mock_pixels[mock_bound_texture]=std::vector<unsigned char>(b,b+size_t(w)*h*(type==GL_UNSIGNED_SHORT_4_4_4_4?2:4));
}
inline void glDeleteTextures(int n,const GLuint* ids){for(int i=0;i<n;++i){++mock_deletes;mock_pixels.erase(ids[i]);}}
inline GLenum glGetError(){return GL_NO_ERROR;}
