#pragma once
#include <cstdint>
using ScePvfLibId=void*;using ScePvfFontId=void*;using ScePvfError=int;using ScePvfFontIndex=int;using ScePvfLanguageCode=int;
enum {SCE_PVF_DEFAULT_FAMILY_CODE=0,SCE_PVF_DEFAULT_STYLE_CODE=0,SCE_PVF_DEFAULT_LANGUAGE_CODE=0,SCE_PVF_LANGUAGE_J=1,SCE_PVF_LANGUAGE_CJK=5,SCE_PVF_MEMORYBASEDSTREAM=1,SCE_PVF_FILEBASEDSTREAM=0,SCE_PVF_USERIMAGE_DIRECT8=2};
struct ScePvfFontStyleInfo {int languageCode,familyCode,style;};
struct ScePvfInitRec {unsigned maxNumFonts;void*(*allocFunc)(void*,unsigned);void*(*reallocFunc)(void*,void*,unsigned);void(*freeFunc)(void*,void*);};
struct ScePvfCharInfo {unsigned bitmapWidth,bitmapHeight;struct {int horizontalAdvance64,horizontalBearingX64,horizontalBearingY64;} glyphMetrics;};
struct ScePvfIrect {uint16_t width,height;};
struct ScePvfUserImageBufferRec {unsigned pixelFormat;int xPos64,yPos64;ScePvfIrect rect;uint16_t bytesPerLine;uint8_t* buffer;};
ScePvfLibId scePvfNewLib(ScePvfInitRec*,ScePvfError*);
int scePvfDoneLib(ScePvfLibId);
int scePvfFindOptimumFont(ScePvfLibId,ScePvfFontStyleInfo*,ScePvfError*);
ScePvfFontId scePvfOpen(ScePvfLibId,int,int,ScePvfError*);
int scePvfSetResolution(ScePvfLibId,float,float);
int scePvfSetCharSize(ScePvfFontId,float,float);
int scePvfGetCharInfo(ScePvfFontId,uint16_t,ScePvfCharInfo*);
int scePvfGetCharImageRect(ScePvfFontId,uint16_t,ScePvfIrect*);
int scePvfGetCharGlyphImage(ScePvfFontId,uint16_t,ScePvfUserImageBufferRec*);
