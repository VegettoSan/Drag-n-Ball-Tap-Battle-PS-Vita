// Exercise the real text adapter with independent PVF metrics/image responses.
#include "../tools/aot/engine/native/vita_text.cpp"
#include <cassert>
#include <cstdio>
int image_rect_calls=0,glyph_image_calls=0,info_calls=0;
bool fail_image=false;
void runtimeLog(const std::string&){}
ScePvfLibId scePvfNewLib(ScePvfInitRec*,ScePvfError* e){*e=0;return reinterpret_cast<void*>(1);}
int scePvfDoneLib(ScePvfLibId){return 0;}
int scePvfFindOptimumFont(ScePvfLibId,ScePvfFontStyleInfo*,ScePvfError* e){*e=0;return 0;}
ScePvfFontId scePvfOpen(ScePvfLibId,int,int,ScePvfError* e){*e=0;return reinterpret_cast<void*>(2);}
int scePvfSetResolution(ScePvfLibId,float,float){return 0;}
int scePvfSetCharSize(ScePvfFontId,float,float){return 0;}
int scePvfGetCharInfo(ScePvfFontId,uint16_t,ScePvfCharInfo* i){
 ++info_calls;*i={};i->glyphMetrics.horizontalAdvance64=6*64;
 i->glyphMetrics.horizontalBearingX64=64;i->glyphMetrics.horizontalBearingY64=7*64;
 // Valid advance/bearings, no bitmap dimensions: this must still render.
 return 0;
}
int scePvfGetCharImageRect(ScePvfFontId,uint16_t c,ScePvfIrect* r){++image_rect_calls;*r=c==' '?ScePvfIrect{0,0}:ScePvfIrect{5,7};return 0;}
int scePvfGetCharGlyphImage(ScePvfFontId,uint16_t,ScePvfUserImageBufferRec* i){
 ++glyph_image_calls;if(fail_image)return -1;
 assert(i->xPos64==64&&i->yPos64==9*64);
 assert(i->rect.width==9&&i->rect.height==11);
 for(int y=2;y<9;++y)for(int x=2;x<7;++x)i->buffer[y*i->bytesPerLine+x]=255;
 return 0;
}
int main(){
 int id=dbtb_createText(64,64);assert(id>0);
 uint16_t text[]={'A',0x3042,' '};int32_t bounds[4]={};
 assert(dbtb_drawText(id,text,3,12,255,255,255,255,bounds)==1);
 auto* s=surface(id);size_t opaque=0;for(size_t p=3;p<s->rgba.size();p+=4)opaque+=s->rgba[p]!=0;
 assert(opaque==70&&bounds[2]==18&&bounds[3]==15);
 assert(image_rect_calls==3&&glyph_image_calls==2);
 int calls=info_calls;dbtb_clearText(id);assert(dbtb_drawText(id,text,3,12,255,255,255,255,bounds)==1);
 assert(info_calls==calls&&image_rect_calls==3&&glyph_image_calls==2);
 bool miss=false;auto* offscreen=glyphFor(12,'Z',false,miss);assert(offscreen&&!offscreen->rasterized&&!miss&&image_rect_calls==3);
 glyphFor(12,'Z',true,miss);assert(miss&&image_rect_calls==4);
 fail_image=true;miss=false;auto* failed=glyphFor(12,'Q',true,miss);assert(failed&&!failed->drawable&&failed->mask.empty());
 dbtb_disposeText(id);puts("TEXT PASS: independent bitmap rectangle, ASCII/CJK coverage, bounds, warm cache, deferred raster, image failure");
}
