#include "pac.hpp"
#include "image.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <zlib.h>

#define CHECK(x) do { if (!(x)) { std::cerr << "FAIL community " << __LINE__ << ": " << #x << '\n'; return false; } } while (0)
static void le16(std::vector<uint8_t>& v, uint16_t x) { v.push_back(x); v.push_back(x >> 8); }
static void le32(std::vector<uint8_t>& v, uint32_t x) { for(int i=0;i<4;++i) v.push_back(x>>(8*i)); }
static void be32(std::vector<uint8_t>& v, uint32_t x) { for(int i=3;i>=0;--i) v.push_back(x>>(8*i)); }
static void write(const std::string& path, const std::vector<uint8_t>& v) {
    std::ofstream f(path, std::ios::binary); f.write(reinterpret_cast<const char*>(v.data()), v.size());
}
static std::vector<uint8_t> imagePayload(const std::vector<uint8_t>& rgba, uint16_t width, uint16_t height,
                                         size_t index, PacEncoding encoding=PacEncoding::Community14) {
    const CommunityPacProfile* profile=communityProfile(encoding);
    if(!profile) return {};
    std::vector<uint8_t> v;
    width ^= profile->image_width_xor ^ index; height ^= profile->image_height_xor ^ index;
    v.push_back(width>>8); v.push_back(width); v.push_back(height>>8); v.push_back(height);
    std::vector<uint8_t> compressed(128);
    z_stream s{};
    if (deflateInit2(&s, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) return {};
    s.next_in = const_cast<Bytef*>(rgba.data()); s.avail_in = rgba.size();
    s.next_out = compressed.data(); s.avail_out = compressed.size();
    const int status = deflate(&s, Z_FINISH);
    const size_t size = s.total_out;
    deflateEnd(&s);
    if (status != Z_STREAM_END) return {};
    v.insert(v.end(), compressed.begin(), compressed.begin()+size);
    return v;
}
static bool negativeAndPixelTests(const std::string& path) {
    const std::vector<uint8_t> expected{128,0,0,128,0,0,255,255};
    auto payload = imagePayload(expected, 2, 1, 1); // index dependence matters
    RgbaImage img; std::string error;
    CHECK(decodeCommunityImage(payload, 1, img, error));
    CHECK(img.width == 2 && img.height == 1 && img.premultiplied_alpha && img.pixels == expected);
    CHECK(!decodeCommunityImage(payload, 0, img, error) && img.pixels.empty());
    auto bad = payload; bad.push_back(1);
    CHECK(!decodeCommunityImage(bad, 1, img, error) && img.pixels.empty());
    bad = payload; bad.pop_back();
    CHECK(!decodeCommunityImage(bad, 1, img, error));
    CHECK(!decodeCommunityImage(imagePayload(expected,4096,4096,0),0,img,error));
    CHECK(!decodeCommunityImage(imagePayload({1,2,3},2,1,0),0,img,error));
    CHECK(!decodeCommunityImage({1,2,3},0,img,error));
    CHECK(!decodeCommunityImage(payload,65536,img,error));

    // Metadata-first container: recognized entry 1 selects encoded profile,
    // unknown record 0 stays unknown and does not hide an out-of-range record.
    std::vector<uint8_t> pac; le16(pac,2^42802u);
    le32(pac,996678763u); le32(pac,1^47633006u); be32(pac,0x12345678u ^ 0xc569e1efu); le32(pac,17);
    le32(pac,1^996678763u^1u); le32(pac,payload.size()^47633006u^1u);
    be32(pac,0x8728c48au ^ 0xc569e1efu ^ 1u); le32(pac,0);
    pac.push_back(99); pac.insert(pac.end(),payload.begin(),payload.end());
    write(path,pac);
    PacFile file; CHECK(file.open(path) && file.encoding()==PacEncoding::Community14);
    CHECK(file.entries().size()==2 && file.typeString(0)=="unk" && file.entries()[0].reserved==17);
    CHECK(file.typeString(1)=="rgba");
    std::vector<uint8_t> out;
    CHECK(file.readEntry(1,out) && out==payload);
    CHECK(!file.readEntry(1,out,1) && out.empty());
    pac[2] ^= 0x80; write(path,pac);
    CHECK(!file.open(path) && !file.isOpen() && file.entries().empty());
    return true;
}
static bool profileTests(const std::string& path) {
    const std::vector<uint8_t> expected{128,0,0,128,0,0,255,255};
    for(PacEncoding encoding : {PacEncoding::Community14, PacEncoding::Community14Spanish,
                                PacEncoding::Community14Invasion}) {
        const CommunityPacProfile* profile=communityProfile(encoding);
        CHECK(profile);
        auto payload=imagePayload(expected,2,1,0,encoding);
        CHECK(!payload.empty());
        std::vector<uint8_t> pac;
        le16(pac,uint16_t(1u^profile->count_xor));
        le32(pac,profile->offset_xor);
        le32(pac,uint32_t(payload.size())^profile->size_xor);
        be32(pac,profile->type_rgba);
        le32(pac,0);
        pac.insert(pac.end(),payload.begin(),payload.end());
        write(path,pac);
        PacFile file;
        CHECK(file.open(path) && file.encoding()==encoding);
        CHECK(file.entries().size()==1 && file.typeString(0)=="rgba");
        std::vector<uint8_t> out;
        CHECK(file.readEntry(0,out) && out==payload);
        RgbaImage img; std::string error;
        CHECK(decodeCommunityImageProfile(out,0,encoding,img,error));
        CHECK(img.width==2 && img.height==1 && img.premultiplied_alpha && img.pixels==expected);
    }
    return true;
}
static bool corpus(const std::string& path, const std::string& scratch, size_t& textures, size_t& packs, size_t depth=0) {
    CHECK(depth <= 2);
    PacFile file;
    CHECK(file.open(path)); ++packs;
    for(size_t i=0;i<file.entries().size();++i) {
        std::vector<uint8_t> payload; CHECK(file.readEntry(i,payload));
        const std::string type=file.typeString(i);
        if (type=="rgba" || type=="png") {
            RgbaImage img; std::string error;
            CHECK(type=="rgba" ? decodeCommunityImageProfile(payload,i,file.encoding(),img,error) : decodePng(payload,img,error));
            CHECK(img.pixels.size()==static_cast<size_t>(img.width)*img.height*4);
            CHECK(img.premultiplied_alpha == (type=="rgba")); ++textures;
        }
        if (type=="spr") {
            const std::string nested=scratch+std::to_string(depth);
            write(nested,payload);
            CHECK(corpus(nested,scratch,textures,packs,depth+1));
            unlink(nested.c_str());
        }
    }
    return true;
}
int main(int argc,char** argv) {
    char temp[]="/tmp/dbtb-community-XXXXXX";
    const int fd=mkstemp(temp); if(fd<0) return 2; close(fd);
    const std::string path(temp);
    // XOR constant independently read from the DEX: -982916625.
    if(!negativeAndPixelTests(path) || !profileTests(path)) { unlink(temp); return 1; }
    size_t textures=0,packs=0;
    for(int i=1;i<argc;++i) if(!corpus(argv[i],path+"-nested-",textures,packs)) { unlink(temp); return 1; }
    unlink(temp);
    std::cout << "COMMUNITY PASS packs=" << packs << " textures=" << textures
              << "; corrupt table/DEFLATE/budget/index tests passed\n";
    return 0;
}
