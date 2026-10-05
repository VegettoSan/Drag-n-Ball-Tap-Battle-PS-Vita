#include "resource_cache.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <unistd.h>
uint16_t u16(const std::vector<uint8_t>& b,size_t p){return b[p]|uint16_t(b[p+1])<<8;}
uint32_t u32(const std::vector<uint8_t>& b,size_t p){return b[p]|uint32_t(b[p+1])<<8|uint32_t(b[p+2])<<16|uint32_t(b[p+3])<<24;}
int bit(const uint8_t* t){for(const auto& p:{std::pair<const char*,int>{"png",1},{"act",2},{"bin",4},{"cnv",8},{"dac",16},{"spr",32},{"wav",64}})if(!memcmp(t,p.first,3))return p.second;return 0;}
void sameSelection(const std::vector<uint8_t>& full,const std::vector<uint8_t>& sparse,int filter){
 assert(u16(full,0)==u16(sparse,0));size_t base=2+u16(full,0)*16;
 for(size_t i=0;i<u16(full,0);++i){size_t p=2+i*16,n=u32(sparse,p+4);assert(!memcmp(full.data()+p+8,sparse.data()+p+8,8));
  assert(base+u32(sparse,p)+n<=sparse.size());
  if(filter&bit(full.data()+p+8))assert(n==0);
  else {assert(n==u32(full,p+4));assert(!memcmp(full.data()+base+u32(full,p),sparse.data()+base+u32(sparse,p),n));}
 }
}
int main(int argc,char** argv){
 assert(argc==3);size_t packs=0,full_bytes=0,sparse_bytes=0;
 for(int dataset=0;dataset<2;++dataset){GameVfs vfs(argv[dataset+1]);if(!dataset)assert(vfs.selectMod("Android14"));
  EngineResourceCache cache(8*1024*1024);
  for(int i=0;i<13;++i){char name[32];snprintf(name,sizeof name,"char%02d",i);std::string path,error;std::vector<uint8_t> full,sparse;int codec=-1;size_t bytes;
   assert(readEngineResource(vfs,name,full,path,error,&codec,0,&bytes));assert(codec==(!dataset));full_bytes+=bytes;
   // Game3 actually requests 187 (BIN + WAV), and other original paths use
   // 251 (BIN only). Bit 128 is not a type exclusion, but remains legal.
   for(int filter:{1,33,64,127,187,251,128,255,256,-1,INT32_MIN,INT32_MIN|187}){int sparse_codec=-1;
    bool loaded=readEngineResource(vfs,name,sparse,path,error,&sparse_codec,filter,&bytes);
    if(!loaded)std::fprintf(stderr,"Resource %s filter=%d: %s\n",name,filter,error.c_str());
    assert(loaded);assert(codec==sparse_codec);sameSelection(full,sparse,filter);
    if(filter==33){sparse_bytes+=bytes;assert(bytes<full.size()/2);}
    if(filter==187||filter==251){size_t bins=0,waves=0;for(size_t j=0;j<u16(sparse,0);++j){size_t p=2+j*16,n=u32(sparse,p+4);
      if(!memcmp(sparse.data()+p+8,"bin",3)){assert(n>0);++bins;}
      else if(!memcmp(sparse.data()+p+8,"wav",3)){assert((n>0)==(filter==187));if(n)++waves;}
      else if(bit(sparse.data()+p+8))assert(n==0);
     }assert(bins>0);if(filter==187)assert(waves>0);
    }
   }
   std::shared_ptr<CachedEngineResource> a,b;bool hit;
   assert(cache.read(vfs,name,33,a,hit,error)&&!hit);assert(cache.read(vfs,std::string(name)+".pac",33,b,hit,error)&&hit&&a==b);
   assert(cache.read(vfs,name,64,b,hit,error)&&!hit&&a!=b);sameSelection(full,b->bytes,64);assert(cache.used()<=8*1024*1024);++packs;
  }
  std::shared_ptr<CachedEngineResource> ignored;bool hit;std::string error;assert(!cache.read(vfs,"../game/common.pac",0,ignored,hit,error));
 }
 // A bounded cache must release entries and detect replacement/truncation.
 char tmp[]="/tmp/dbtb-cache-XXXXXX";assert(mkdtemp(tmp));GameVfs vfs(tmp);assert(vfs.prepareDirectories());
 std::string f=std::string(tmp)+"/game/mk.bin";
 {std::ofstream out(f);out<<"abc";}
 EngineResourceCache cache(4);std::shared_ptr<CachedEngineResource>a,b;bool hit;std::string error;
 assert(cache.read(vfs,"mk",0,a,hit,error)&&!hit);assert(cache.read(vfs,"mk",0,b,hit,error)&&hit&&b==a);
 {std::ofstream out(f);out<<"abcdef";}
 assert(cache.read(vfs,"mk",0,b,hit,error)&&!hit&&b!=a&&b->bytes.size()==6&&cache.used()==0);
 cache.clear();assert(cache.used()==0);unlink(f.c_str());
 printf("RESOURCE STREAM PASS: %zu ordinary/community character PACs; source I/O %zu -> %zu bytes (filter 33), exact selected payloads/slots including original 187/251 and signed/high-bit masks, cache reuse/bounds/invalidation\n",packs,full_bytes,sparse_bytes);
}
