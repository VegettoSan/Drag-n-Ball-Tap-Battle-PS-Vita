#include "game_data.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#define CHECK(x) do { if (!(x)) { std::cerr << "FAIL game data " << __LINE__ << ": " << #x << '\n'; return false; } } while (0)
static void le16(std::vector<uint8_t>& v, uint16_t n) { v.push_back(n); v.push_back(n >> 8); }
static void le32(std::vector<uint8_t>& v, uint32_t n) { for (int i=0; i<4; ++i) v.push_back(n >> (i*8)); }
static std::vector<uint8_t> fixture(PacEncoding encoding) {
    const CommunityPacProfile* profile=communityProfile(encoding);
    std::vector<uint8_t> v;
    le16(v, uint16_t(2u ^ (profile ? profile->table_count_xor : 0u)));
    for (uint32_t i=0; i<2; ++i) {
        le32(v, uint32_t(18+i*4) ^ (profile ? (profile->table_position_xor^i) : 0u));
        le16(v, uint16_t(2u ^ (profile ? (profile->table_width_xor^i) : 0u)));
        le16(v, uint16_t(2u ^ (profile ? (profile->table_height_xor^i) : 0u)));
    }
    v.insert(v.end(), {0,128,254,255,7,8,9,10});
    return v;
}
static bool bounds() {
    GameDataTable t; uint8_t value;
    for (auto codec : {PacEncoding::Original, PacEncoding::Community14,
                       PacEncoding::Community14Spanish, PacEncoding::Community14Invasion}) {
        const auto data = fixture(codec);
        CHECK(t.decode(data, codec));
        CHECK(t.records().size() == 2 && t.encoding() == codec);
        CHECK(t.value(0,1,1,value) && value==255);
        CHECK(t.value(0,1,0,value) && value==128);
        CHECK(t.value(1,0,1,value) && value==9); // record-index XOR, row stride
        CHECK(!t.value(2,0,0,value) && value==0);
        CHECK(!t.value(0,2,0,value));
        CHECK(!t.value(0,0,2,value));
        for (size_t n=0; n<data.size(); ++n) {
            CHECK(!t.decode({data.begin(),data.begin()+n},codec));
            CHECK(t.records().empty() && !t.value(0,0,0,value));
        }
    }
    auto bad = fixture(PacEncoding::Original);
    bad[2]=0; CHECK(!t.decode(bad,PacEncoding::Original)); // header overlap
    bad=fixture(PacEncoding::Original);
    for(size_t i=2;i<6;++i) bad[i]=255;
    CHECK(!t.decode(bad,PacEncoding::Original)); // u32 position, no overflow
    bad=fixture(PacEncoding::Original); bad[6]=255; bad[7]=255; bad[8]=255; bad[9]=255;
    CHECK(!t.decode(bad,PacEncoding::Original)); // huge row product
    CHECK(!t.decode(fixture(PacEncoding::Original),PacEncoding::Auto));
    for (auto codec : {PacEncoding::Community14, PacEncoding::Community14Spanish,
                       PacEncoding::Community14Invasion}) {
        CHECK(!t.decode(fixture(codec),PacEncoding::Original));
        CHECK(!t.decode(fixture(PacEncoding::Original),codec));
    }
    return true;
}
static bool copy(const std::string& from, const std::string& to) {
    std::ifstream src(from,std::ios::binary); std::ofstream dst(to,std::ios::binary);
    if (!src || !dst) return false;
    dst << src.rdbuf(); return bool(dst);
}
static bool corpus(const std::string& installation, const std::string& temp) {
    GameVfs real(installation); GameDatabase original, community, mixed;
    CHECK(original.load(real));
    CHECK(original.game.records().size()==271 && original.text.records().size()==1);
    CHECK(real.selectMod("Android14") && community.load(real));
    CHECK(community.game.records().size()==271 && community.text.records().size()==1);
    CHECK(original.game.encoding()==PacEncoding::Original);
    CHECK(community.game.encoding()==PacEncoding::Community14);
    uint8_t a,b; size_t changed=0, compared=0;
    for(size_t i=0;i<271;++i) {
        const auto& r=original.game.records()[i]; const auto& s=community.game.records()[i];
        if (r.width!=s.width || r.height!=s.height) continue;
        for(size_t y=0;y<r.height;++y) for(size_t x=0;x<r.width;++x) {
            CHECK(original.game.value(i,x,y,a) && community.game.value(i,x,y,b));
            ++compared; changed += a!=b;
        }
    }
    // Reproduce the actual mixed-codec route: converted override + ordinary fallback.
    GameVfs overlay(temp); CHECK(overlay.prepareDirectories());
    CHECK(copy(installation+"/game/gamedata.pac",temp+"/game/gamedata.pac"));
    CHECK(copy(installation+"/game/text00.pac",temp+"/game/text00.pac"));
    CHECK(mkdir((temp+"/mods/Mixed").c_str(),0700)==0);
    CHECK(copy(installation+"/mods/Android14/gamedata.pac",temp+"/mods/Mixed/gamedata.pac"));
    CHECK(overlay.selectMod("Mixed") && mixed.load(overlay));
    CHECK(mixed.game.encoding()==PacEncoding::Community14 && mixed.text.encoding()==PacEncoding::Original);
    CHECK(mixed.game.sourcePath()==temp+"/mods/Mixed/gamedata.pac");
    CHECK(mixed.text.sourcePath()==temp+"/game/text00.pac");
    // Broken override fails visibly instead of quietly reading the base file.
    { std::ofstream broken(temp+"/mods/Mixed/gamedata.pac"); broken << "broken"; }
    CHECK(!mixed.load(overlay) && !mixed.error.empty());
    CHECK(mixed.game.records().empty() && mixed.text.records().empty());
    std::cout << "GAME TABLE CORPUS PASS: 271+1 records per APK; " << changed << '/' << compared
              << " comparable game cells differ; mixed codec fallback PASS\n";
    return true;
}
int main(int argc, char** argv) {
    if(!bounds()) return 1;
    std::cout << "GAME TABLE BOUNDS PASS\n";
    if(argc==3 && !corpus(argv[1],argv[2])) return 1;
    return 0;
}
