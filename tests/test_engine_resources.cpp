#include "engine_resources.hpp"
#include "game_data.hpp"
#include "image.hpp"
#include "pac.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <dirent.h>
#include <zlib.h>

namespace {
uint16_t u16(const std::vector<uint8_t>& b, size_t p) { return b[p] | (uint16_t(b[p+1]) << 8); }
uint32_t u32(const std::vector<uint8_t>& b, size_t p) {
    return b[p] | (uint32_t(b[p+1]) << 8) | (uint32_t(b[p+2]) << 16) | (uint32_t(b[p+3]) << 24);
}
void p16(std::vector<uint8_t>& b,size_t p,uint16_t v){b[p]=uint8_t(v);b[p+1]=uint8_t(v>>8);}
void p32(std::vector<uint8_t>& b,size_t p,uint32_t v){for(size_t i=0;i<4;++i)b[p+i]=uint8_t(v>>(8*i));}
void pbe32(std::vector<uint8_t>& b,size_t p,uint32_t v){for(size_t i=0;i<4;++i)b[p+i]=uint8_t(v>>(24-8*i));}
std::vector<uint8_t> fileBytes(const std::string& path) {
    FILE* f=std::fopen(path.c_str(),"rb"); assert(f); assert(!std::fseek(f,0,SEEK_END));
    long n=std::ftell(f); assert(n>=0); std::rewind(f); std::vector<uint8_t> b(static_cast<size_t>(n));
    assert(std::fread(b.data(),1,b.size(),f)==b.size()); std::fclose(f); return b;
}
size_t textures=0,containers=0;
void validate(const std::vector<uint8_t>& b) {
    ++containers; assert(b.size()>=2);const size_t count=u16(b,0),base=2+count*16;assert(base<=b.size());
    for(size_t i=0;i<count;++i){size_t p=2+i*16;const size_t start=base+u32(b,p),n=u32(b,p+4);assert(start<=b.size()&&n<=b.size()-start);
        std::vector<uint8_t> payload(b.begin()+start,b.begin()+start+n);
        if(!std::memcmp(b.data()+p+8,"spr",3))validate(payload);
        if(std::memcmp(b.data()+p+8,"png",3))continue;
        RgbaImage image;std::string error;
        PacEncoding marked=PacEncoding::Auto;
        if(payload.size()>=8&&!std::memcmp(payload.data(),"C14R",4))marked=PacEncoding::Community14;
        else if(payload.size()>=8&&!std::memcmp(payload.data(),"C14S",4))marked=PacEncoding::Community14Spanish;
        else if(payload.size()>=8&&!std::memcmp(payload.data(),"C14I",4))marked=PacEncoding::Community14Invasion;
        if(marked!=PacEncoding::Auto){
            size_t original_index=u32(payload,4);assert(original_index==i);
            payload.erase(payload.begin(),payload.begin()+8);
            assert(decodeCommunityImageProfile(payload,original_index,marked,image,error));assert(image.premultiplied_alpha);
        }else assert(decodePng(payload,image,error));
        ++textures;
    }
}
std::vector<uint8_t> entry(const std::vector<uint8_t>& b,size_t wanted){
    const size_t count=u16(b,0),base=2+count*16;assert(wanted<count);const size_t p=2+wanted*16;
    const size_t start=base+u32(b,p),size=u32(b,p+4);assert(start<=b.size()&&size<=b.size()-start);
    return {b.begin()+start,b.begin()+start+size};
}
std::vector<uint8_t> dac(const std::vector<uint8_t>& b){size_t count=u16(b,0),base=2+count*16;
    for(size_t i=0;i<count;++i){size_t p=2+i*16;if(!std::memcmp(b.data()+p+8,"dac",3)){size_t start=base+u32(b,p),size=u32(b,p+4);return {b.begin()+start,b.begin()+start+size};}}
    assert(false);return {};
}
void sameTable(const GameDataTable& a,const GameDataTable& b){
    assert(a.records().size()==b.records().size());
    for(size_t r=0;r<a.records().size();++r){
        assert(a.records()[r].position==b.records()[r].position);
        assert(a.records()[r].width==b.records()[r].width&&a.records()[r].height==b.records()[r].height);
        for(size_t y=0;y<a.records()[r].height;++y)for(size_t x=0;x<a.records()[r].width;++x){
            uint8_t v,w;assert(a.value(r,x,y,v)&&b.value(r,x,y,w)&&v==w);
        }
    }
}
std::vector<uint8_t> encodingPac(const std::vector<uint8_t>& payload){
    std::vector<uint8_t> pac(18+payload.size(),0);pac[0]=1;p32(pac,6,payload.size());
    std::memcpy(pac.data()+10,"bin",3);std::memcpy(pac.data()+18,payload.data(),payload.size());return pac;
}
std::vector<uint8_t> encodedImage(const CommunityPacProfile& profile){
    std::vector<uint8_t> payload(4);
    const uint16_t w=uint16_t(1u^profile.image_width_xor),h=uint16_t(1u^profile.image_height_xor);
    payload[0]=uint8_t(w>>8);payload[1]=uint8_t(w);payload[2]=uint8_t(h>>8);payload[3]=uint8_t(h);
    const uint8_t pixel[4]={10,20,30,40};std::vector<uint8_t> compressed(64);
    z_stream stream{};assert(deflateInit2(&stream,6,Z_DEFLATED,-15,8,Z_DEFAULT_STRATEGY)==Z_OK);
    stream.next_in=const_cast<Bytef*>(pixel);stream.avail_in=sizeof(pixel);
    stream.next_out=compressed.data();stream.avail_out=compressed.size();
    assert(deflate(&stream,Z_FINISH)==Z_STREAM_END);const size_t n=stream.total_out;deflateEnd(&stream);
    payload.insert(payload.end(),compressed.begin(),compressed.begin()+n);return payload;
}
std::vector<uint8_t> protectedFixture(PacEncoding encoding){
    const CommunityPacProfile* profile=communityProfile(encoding);assert(profile);
    const auto image=encodedImage(*profile);
    std::vector<uint8_t> table(11,0);p16(table,0,uint16_t(1u^profile->table_count_xor));
    p32(table,2,10u^profile->table_position_xor);p16(table,6,uint16_t(1u^profile->table_width_xor));
    p16(table,8,uint16_t(1u^profile->table_height_xor));table[10]=0x7f;
    std::vector<uint8_t> wav(9,0);p32(wav,0,4u^profile->wav_size_xor^2u);wav[4]=0;
    wav[5]=1;wav[6]=2;wav[7]=3;wav[8]=4;
    const std::vector<std::vector<uint8_t>> payloads{image,table,wav};
    const uint32_t types[3]={profile->type_rgba,profile->type_bin,profile->type_wav};
    const size_t base=2+payloads.size()*16;std::vector<uint8_t> pac(base,0);
    p16(pac,0,uint16_t(payloads.size()^profile->count_xor));size_t cursor=0;
    for(size_t i=0;i<payloads.size();++i){const size_t p=2+i*16;
        p32(pac,p,uint32_t(cursor)^profile->offset_xor^uint32_t(i));
        p32(pac,p+4,uint32_t(payloads[i].size())^profile->size_xor^uint32_t(i));
        pbe32(pac,p+8,types[i]^uint32_t(i));p32(pac,p+12,0);
        pac.insert(pac.end(),payloads[i].begin(),payloads[i].end());cursor+=payloads[i].size();
    }
    return pac;
}
void profileNormalisation(){
    for(PacEncoding encoding:{PacEncoding::Community14,PacEncoding::Community14Spanish,PacEncoding::Community14Invasion}){
        const auto input=protectedFixture(encoding);std::vector<uint8_t> output;std::string error;int protected_encoding=0;
        assert(normaliseEnginePac(input,"common.pac",output,error,&protected_encoding)&&protected_encoding==1);
        assert(u16(output,0)==3&&!std::memcmp(output.data()+10,"png",3)&&!std::memcmp(output.data()+26,"bin",3)&&!std::memcmp(output.data()+42,"wav",3));
        auto image=entry(output,0);const char* expected=encoding==PacEncoding::Community14?"C14R":encoding==PacEncoding::Community14Spanish?"C14S":"C14I";
        assert(image.size()>=8&&!std::memcmp(image.data(),expected,4)&&u32(image,4)==0);
        image.erase(image.begin(),image.begin()+8);RgbaImage decoded;assert(decodeCommunityImageProfile(image,0,encoding,decoded,error));
        assert(decoded.width==1&&decoded.height==1&&decoded.pixels==std::vector<uint8_t>({10,20,30,40}));
        GameDataTable table;assert(table.decode(entry(output,1),PacEncoding::Original));uint8_t value=0;
        assert(table.records().size()==1&&table.value(0,0,0,value)&&value==0x7f);
        assert(entry(output,2)==std::vector<uint8_t>({1,2,3,4}));
    }
}
}

int main(int argc,char** argv){
    profileNormalisation();
    if(argc==1){std::puts("ENGINE RESOURCE PROFILE PASS: Android14 + Spanish + Invasion normalisation");return 0;}
    assert(argc==2);GameVfs vfs(argv[1]);std::string path,error;std::vector<uint8_t> output;
    size_t files=0,converted_bins=0,decoded_wavs=0; long long wav_energy=0;
    for(const std::string& folder:{std::string("game"),std::string("mods/Android14")}){
        if(folder=="game")vfs.selectOriginal();else assert(vfs.selectMod("Android14"));
        DIR* dir=opendir((std::string(argv[1])+"/"+folder).c_str());assert(dir);
        while(dirent* e=readdir(dir)){std::string name=e->d_name;if(name.size()<4||name.substr(name.size()-4)!=".pac")continue;
            assert(readEngineResource(vfs,name,output,path,error));validate(output);++files;
            const auto original=fileBytes(path);if(folder=="game")assert(output==original);
            PacFile source;assert(source.open(path));assert(source.entries().size()==u16(output,0));
            for(size_t i=0;i<source.entries().size();++i){
                assert(source.entries()[i].reserved==u32(output,14+i*16));
                if(folder=="mods/Android14"&&source.typeString(i)=="bin"){
                    std::vector<uint8_t> raw;assert(source.readEntry(i,raw));
                    GameDataTable a,b;assert(a.decode(raw,PacEncoding::Community14));
                    assert(b.decode(entry(output,i),PacEncoding::Original));sameTable(a,b);++converted_bins;
                }
                if(folder=="mods/Android14"&&source.typeString(i)=="wav"){
                    std::vector<uint8_t> raw;assert(source.readEntry(i,raw));assert(raw.size()>=5);
                    const uint32_t decoded_size=u32(raw,0)^42802u^uint32_t(i);
                    const std::vector<uint8_t> pcm=entry(output,i);
                    assert(decoded_size>0&&!(decoded_size&1u)&&pcm.size()==decoded_size);
                    if(raw[4])assert(raw.size()>5&&((raw.size()-5)&15u)==0);
                    for(size_t q=0;q+1<pcm.size();q+=64){
                        const int16_t sample=static_cast<int16_t>(uint16_t(pcm[q])|(uint16_t(pcm[q+1])<<8));
                        wav_energy+=sample<0?-int(sample):int(sample);
                    }
                    ++decoded_wavs;
                }
            }
            if(name=="gamedata.pac"||name=="text00.pac"){
                std::vector<uint8_t> raw;for(size_t i=0;i<source.entries().size();++i)if(source.typeString(i)=="dac")assert(source.readEntry(i,raw));
                GameDataTable a,b;assert(a.decode(raw,source.encoding()));assert(b.decode(dac(output),PacEncoding::Original));sameTable(a,b);
            }
        }closedir(dir);
    }
    assert(files==125&&containers==137&&textures==470&&converted_bins==68&&decoded_wavs==198&&wav_energy>0);
    std::vector<uint8_t> utf8;for(int i=0;i<20;++i){utf8.push_back(0xe3);utf8.push_back(0x81);utf8.push_back(0x82);}utf8.push_back(0);
    std::vector<uint8_t> sjis;for(int i=0;i<20;++i){sjis.push_back(0x82);sjis.push_back(0xa0);}sjis.push_back(0);
    assert(detectEngineTextEncoding(encodingPac(utf8),0)==1);
    assert(detectEngineTextEncoding(encodingPac(sjis),0)==0);
    assert(detectEngineTextEncoding(encodingPac(sjis),1)==1);
    for(size_t n=0;n<18;++n){std::vector<uint8_t> b(n,0xFF);assert(!normaliseEnginePac(b,"common.pac",output,error));assert(output.empty());}
    std::vector<uint8_t> bad(18,0);bad[0]=1;bad[2]=0xFF;bad[3]=0xFF;bad[4]=0xFF;bad[5]=0xFF;
    assert(!normaliseEnginePac(bad,"common.pac",output,error));assert(output.empty());
    assert(!readEngineResource(vfs,"../game/common.pac",output,path,error));assert(output.empty());
    assert(readEngineResource(vfs,"loading",output,path,error));assert(output==fileBytes(path));
    assert(readEngineResource(vfs,"mk",output,path,error));assert(output==fileBytes(path));
    assert(readEngineResource(vfs,"se_00",output,path,error));assert(output==fileBytes(path));
    std::printf("ENGINE RESOURCE PASS: %zu files, %zu containers, %zu textures, %zu converted BIN tables, %zu decoded WAV streams\n",files,containers,textures,converted_bins,decoded_wavs);
}
