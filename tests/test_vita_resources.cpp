// Real resource/PNG/native ownership paths; only GL upload/delete is mocked.
#include "../tools/aot/engine/native/resources.cpp"
#include <cassert>
#include <cstdio>
uint16_t u16(const std::vector<uint8_t>& b,size_t p){return b[p]|uint16_t(b[p+1])<<8;}
uint32_t u32(const std::vector<uint8_t>& b,size_t p){return b[p]|uint32_t(b[p+1])<<8|uint32_t(b[p+2])<<16|uint32_t(b[p+3])<<24;}
std::vector<uint8_t> png(const std::string& base,int character){
 GameVfs vfs(base);std::string path,error;std::vector<uint8_t> pac;char name[32];snprintf(name,sizeof name,"chardemo%02d",character);
 assert(readEngineResource(vfs,name,pac,path,error));size_t n=u16(pac,0),begin=2+n*16;
 for(size_t i=0;i<n;++i)if(!memcmp(pac.data()+10+i*16,"png",3)){size_t p=2+i*16,start=begin+u32(pac,p);return {pac.begin()+start,pac.begin()+start+u32(pac,p+4)};}
 assert(false);return {};
}
void release(int id){if(dbtb_releaseTexture(id)){GLuint texture=id;glDeleteTextures(1,&texture);}}
int main(int argc,char** argv){
 assert(argc==2);assert(dbtb_initResources(argv[1],""));
 char name[]="char00";int size=dbtb_resourceFiltered(name,33);assert(size>0&&pending_resource);auto first=pending_resource;
 std::vector<uint8_t> bytes(size);dbtb_copyResource(bytes.data(),size);assert(bytes==first->bytes&&!pending_resource);
 assert(dbtb_resourceFiltered(name,33)==size&&pending_resource==first);
 dbtb_copyResource(bytes.data(),size);assert(dbtb_performance().resource_cache_hits==1);
 assert(dbtb_resource(name)>size);std::vector<uint8_t> full(pending_resource->bytes.size());dbtb_copyResource(full.data(),full.size());
 // Exercise the exact native import used by original Game3, not only direct
 // stream reads with seven-bit synthetic masks. Metadata/voice banks must load.
 for(int filter:{187,251}){
  int selected=dbtb_resourceFiltered(name,filter);assert(selected>0&&pending_resource);
  auto cached=pending_resource;std::vector<uint8_t> selected_bytes(selected);dbtb_copyResource(selected_bytes.data(),selected);
  assert(selected_bytes==cached->bytes&&!pending_resource);
  size_t bins=0,waves=0;for(size_t i=0;i<u16(selected_bytes,0);++i){size_t p=2+i*16,n=u32(selected_bytes,p+4);
   if(!memcmp(selected_bytes.data()+p+8,"bin",3)){assert(n>0);++bins;}
   if(!memcmp(selected_bytes.data()+p+8,"wav",3)){assert((n>0)==(filter==187));if(n)++waves;}
  }assert(bins>0);if(filter==187)assert(waves>0);
  assert(dbtb_resourceFiltered(name,filter)==selected&&pending_resource==cached);
  dbtb_copyResource(selected_bytes.data(),selected);
 }
 // A stream pins its own native owner across other imports and cache eviction.
 int stream=dbtb_openResourceStream(name,0);assert(stream>0&&!pending_resource);
 assert(dbtb_resourceStreamSize(stream)==int(full.size()));
 char second_name[]="char01";int second=dbtb_openResourceStream(second_name,187);assert(second>0&&second!=stream);
 assert(dbtb_readResourceStream(stream,-1,bytes.data(),1)==-1);
 assert(dbtb_readResourceStream(stream,0,nullptr,1)==-1);
 assert(dbtb_readResourceStream(stream,INT32_MAX,bytes.data(),INT32_MAX)==-1);
 assert(dbtb_readResourceStream(stream,int(full.size()),nullptr,0)==0);
 assert(dbtb_readResourceStream(stream,int(full.size()),bytes.data(),1)==-1);
 for(int i=1;i<13;++i){char other[32];snprintf(other,sizeof(other),"char%02d",i);assert(dbtb_resourceFiltered(other,0)>0);}
 assert(resource_cache.used()<=8u*1024u*1024u);
 std::vector<uint8_t> chunk(4096);size_t read=0;
 while(read<full.size()){
  int count=int(std::min(chunk.size(),full.size()-read));
  assert(dbtb_readResourceStream(stream,int(read),chunk.data(),count)==count);
  assert(!memcmp(chunk.data(),full.data()+read,count));read+=count;
 }
 assert(resource_streams.at(stream).largest_read==4096);
 dbtb_closeResourceStream(stream);dbtb_closeResourceStream(stream);
 assert(dbtb_resourceStreamSize(stream)==-1&&dbtb_readResourceStream(stream,0,bytes.data(),1)==-1);
 dbtb_closeResourceStream(second);assert(resource_streams.empty());
 int pinned[8];for(int& handle:pinned){handle=dbtb_openResourceStream(name,251);assert(handle>0);}
 assert(dbtb_openResourceStream(name,251)==-1);
 for(int handle:pinned)dbtb_closeResourceStream(handle);
 assert(resource_streams.empty());
 auto image=png(argv[1],0);int a=dbtb_loadTexture(image.data(),image.size(),1);assert(a>0&&mock_uploads==1&&dbtb_textureWidth(a)==512);
 auto pixels=mock_pixels.at(a);release(a);assert(mock_pixels.count(a)&&texture_cache.front().users==0);
 int b=dbtb_loadTexture(image.data(),image.size(),1);assert(b==a&&mock_uploads==1&&mock_pixels.at(b)==pixels);
 int shared=dbtb_loadTexture(image.data(),image.size(),1);assert(shared==a&&texture_cache.front().users==2);release(shared);assert(texture_cache.front().users==1);
 int nearest=dbtb_loadTexture(image.data(),image.size(),0);assert(nearest!=a&&mock_uploads==2);
 // Different bytes must never alias just because a hash matches.
 auto changed=image;changed[40]^=1;texture_cache.back().hash=textureHash(changed.data(),changed.size());
 int different=dbtb_loadTexture(changed.data(),changed.size(),1);assert(different!=a);if(different>0)release(different);
 release(nearest);
 // Hold a live texture while idle entries are evicted by later character loads.
 for(int i=1;i<8;++i){auto other=png(argv[1],i);int id=dbtb_loadTexture(other.data(),other.size(),1);assert(id>0);release(id);assert(mock_pixels.count(a)&&texture_cache_bytes<=kTextureCacheBudget);}
 assert(mock_deletes>0&&mock_pixels.at(a)==pixels);
 // Audio reclamation may drop cached PACs/idle textures, never active owners.
 int audio_stream=dbtb_openResourceStream(name,251);assert(audio_stream>0);
 dbtb_reclaimIdleResources();
 assert(resource_cache.used()==0&&mock_pixels.count(a));
 assert(dbtb_readResourceStream(audio_stream,0,chunk.data(),2)==2);
 dbtb_closeResourceStream(audio_stream);
 release(a);assert(trimTextures(kTextureCacheBudget)&&texture_cache.empty());
 int empty=dbtb_emptyTexture(16,16);assert(empty>0);release(empty);assert(!mock_pixels.count(empty));
 // Profile reinitialization discards resource hits, not pending/save aliases.
 assert(dbtb_openResourceStream(name,251)>0);
 assert(dbtb_initResources(argv[1],"")&&resource_cache.used()==0&&!pending_resource&&resource_streams.empty());
 puts("NATIVE RESOURCE PASS: original 187/251 bridge/copy/cache and metadata/voices, exact PNG reuse, filter modes, reference ownership, collision isolation, idle eviction, uncached render target, profile reset");
}
