#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <set>
#include <string>
#include <vector>

// Sparse, same-length view of an already-normalized effect.pac. Shared cache
// bytes and installed files remain immutable. Unknown layouts fail atomically.
namespace VitaPadVisibility {
inline uint16_t u16(const uint8_t* p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
inline uint32_t u32(const uint8_t* p) { return uint32_t(u16(p)) | uint32_t(u16(p+2)) << 16; }
struct Action { bool present=false; size_t start=0,end=0; std::vector<unsigned> links; std::vector<size_t> fields; };
inline bool build(const std::vector<uint8_t>& pac, std::vector<size_t>& result, std::string& error) {
    result.clear(); error="unsupported effect animation layout";
    if(pac.size()<2)return false;
    const size_t count=u16(pac.data()), base=2+count*16;
    if(base>pac.size())return false;
    size_t dacBase=0,dacSize=0,cnvSize=0; bool foundDac=false,foundCnv=false;
    for(size_t i=0;i<count;i++) {
        const auto* e=pac.data()+2+i*16; size_t start=base+u32(e),size=u32(e+4);
        if(start>pac.size() || size>pac.size()-start)return false;
        if(!std::memcmp(e+8,"dac\0",4)) { if(foundDac)return false; foundDac=true;dacBase=start;dacSize=size; }
        if(!std::memcmp(e+8,"cnv\0",4)) { if(foundCnv)return false; foundCnv=true;cnvSize=size; }
    }
    if(!foundDac || !foundCnv || cnvSize%9 || dacSize<8)return false;
    const uint8_t* d=pac.data()+dacBase;
    size_t n=u16(d+2),index=u16(d+4),data=u16(d+6);
    if(u16(d)!=0x1100 || n>4096 || index<8 || index+n*2>data || data>dacSize)return false;
    std::vector<Action> records(n);std::set<size_t> starts{dacSize};
    for(size_t i=0;i<n;i++) {
        int offset=int16_t(u16(d+index+i*2)); if(offset==-1)continue;
        if(offset<0)return false;
        auto& r=records[i];r.present=true;r.start=data+size_t(offset)*4;
        if(r.start>=dacSize)return false;
        starts.insert(r.start);
    }
    for(auto& r:records)if(r.present) {
        r.end=*starts.upper_bound(r.start);size_t p=r.start;
        auto fits=[&](size_t pos,size_t size){return pos>=r.start && pos<=r.end && size<=r.end-pos;};
        if(!fits(p,2))return false;
        unsigned flags=d[p],frames=d[p+1];p+=2;
        if(flags&4){if(!fits(p,1))return false;++p;}
        if(flags&2) {
            if(!fits(p,1))return false;
            unsigned links=d[p++];
            if(!fits(p,links*6))return false;
            for(unsigned j=0;j<links;j++,p+=6)r.links.push_back(u16(d+p+4)&4095);
        }
        if(!fits(p,frames*2))return false;
        for(unsigned j=0;j<frames;j++) {
            size_t f=p+frames*2+u16(d+p+j*2);
            if(!fits(f,1))return false;
            size_t field=f+1+((d[f]&32)?1:0);
            if(!fits(field,2))return false;
            r.fields.push_back(field);
        }
    }
    std::set<unsigned> closure;
    for(unsigned a=200;a<=210;a++)closure.insert(a);
    for(unsigned a:{330u,331u,333u,334u,336u,337u,339u,340u})closure.insert(a);
    for(size_t round=0;round<n;round++) {
        auto next=closure;
        for(unsigned a:closure) {
            if(a>=n || !records[a].present)return false;
            next.insert(records[a].links.begin(),records[a].links.end());
        }
        if(next==closure)break;
        closure.swap(next);
    }
    auto expected=std::set<unsigned>{200,201,202,203,204,205,206,207,208,209,210,211,
        330,331,333,334,336,337,339,340,341,342};
    if(closure!=expected)return false;
    std::set<size_t> fields,bytes;
    for(unsigned a:closure)for(size_t field:records[a].fields) {
        int image=int16_t(u16(d+field)); if(image<-1 || (image>=0 && size_t(image)>=cnvSize/9))return false;
        fields.insert(field);bytes.insert(field);bytes.insert(field+1);
    }
    if(fields.size()!=25)return false;
    for(unsigned a=0;a<n;a++)if(records[a].present && !closure.count(a)) {
        const auto& r=records[a];
        for(unsigned link:r.links)if(closure.count(link))return false;
        for(size_t b:bytes)if(b>=r.start && b<r.end)return false;
        for(size_t f:r.fields)if(bytes.count(f) || bytes.count(f+1))return false;
    }
    for(size_t f:fields){result.push_back(dacBase+f);result.push_back(dacBase+f+1);}
    std::sort(result.begin(),result.end());error.clear();return true;
}
inline void apply(const std::vector<size_t>& offsets, size_t position, uint8_t* target, size_t size) {
    auto it=std::lower_bound(offsets.begin(),offsets.end(),position);
    while(it!=offsets.end() && *it-position<size) { target[*it-position]=255; ++it; }
}
}
