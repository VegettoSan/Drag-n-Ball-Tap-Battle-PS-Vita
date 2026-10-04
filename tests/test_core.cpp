#include "pac.hpp"
#include "vfs.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

#define CHECK(x) do { if (!(x)) { std::cerr << "FAIL " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
static void write(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary); f.write(reinterpret_cast<const char*>(data.data()), data.size());
}
int main(int argc, char** argv) {
    char temp[] = "/tmp/dbtb-test-XXXXXX";
    const char* root = mkdtemp(temp);
    CHECK(root);
    GameVfs vfs(root);
    CHECK(vfs.prepareDirectories());
    CHECK(!vfs.originalDataPresent());
    const std::string base(root), mod = base + "/mods/Mod ñ .. test";
    CHECK(mkdir(mod.c_str(), 0777) == 0);
    write(mod + "/common.pac", {1});
    CHECK(vfs.selectMod("Mod ñ .. test"));
    CHECK(!vfs.originalDataPresent());
    std::string resolved;
    CHECK(vfs.resolve("common.pac", resolved) && resolved == mod + "/common.pac");
    write(base + "/game/common.pac", {2});
    CHECK(vfs.originalDataPresent());
    write(base + "/game/other.dat", {3});
    CHECK(vfs.resolve("other.dat", resolved) && resolved == base + "/game/other.dat");
    CHECK(vfs.listMods().size() == 1);
    for (const std::string bad : {"../x", "x/../y", "ux0:x", "/x", "x\\y", "x//y", "./x", "x/./y", "x/", ""})
        CHECK(!vfs.resolve(bad, resolved) && resolved.empty());
    CHECK(!vfs.resolve(std::string("common.pac\0evil", 15), resolved));
    CHECK(!vfs.selectMod("../game"));
    CHECK(vfs.activeMod() == "Mod ñ .. test");
    CHECK(mkdir((mod + "/nested").c_str(), 0777) == 0);
    write(mod + "/nested/a", {4});
    CHECK(vfs.resolve("nested/a", resolved));
    CHECK(mkdir((mod + "/other.dat").c_str(), 0777) == 0);
    CHECK(!vfs.resolve("other.dat", resolved)); // invalid override cannot hide behind fallback
    CHECK(symlink((base + "/game/common.pac").c_str(), (mod + "/escape").c_str()) == 0);
    CHECK(!vfs.resolve("escape", resolved));
    for (int n=0;n<256;++n) {
        CHECK(mkdir((base + "/mods/Folder " + std::to_string(n)).c_str(),0777)==0);
    }
    CHECK(vfs.listMods().size()==257);
    const std::string long_name(255,'L');
    CHECK(mkdir((base + "/mods/" + long_name).c_str(),0777)==0);
    CHECK(vfs.selectMod(long_name));
    CHECK(vfs.listMods().size()==258);
    vfs.selectOriginal();
    CHECK(vfs.resolve("common.pac", resolved) && resolved == base + "/game/common.pac");

    const std::string file = base + "/test.pac";
    // Synthetic container with non-NUL-terminated tag; no commercial fixture.
    std::vector<uint8_t> valid = {1,0, 0,0,0,0, 3,0,0,0, 'T','E','S','T', 0,0,0,0, 1,2,3};
    write(file, valid);
    PacFile pac;
    CHECK(pac.open(file) && pac.dataBase() == 18 && pac.typeString(0) == "TEST");
    std::vector<uint8_t> out;
    CHECK(pac.readEntry(0,out) && out == std::vector<uint8_t>({1,2,3}));
    CHECK(!pac.readEntry(0,out,2) && out.empty() && !pac.error().empty());
    CHECK(!pac.readEntry(1,out));
    write(file,{0,0});
    CHECK(!pac.readEntry(0,out));
    CHECK(pac.open(file) && pac.entries().empty()); // valid empty container
    for (const auto& bad : {std::vector<uint8_t>{}, std::vector<uint8_t>{1}, std::vector<uint8_t>{255,255},
                           std::vector<uint8_t>{1,0,255,255,255,255,255,255,255,255,'p','n','g',0,0,0,0,0}}) {
        write(file,bad);
        CHECK(!pac.open(file) && !pac.isOpen() && pac.entries().empty());
    }
    // Failed opens must not expose entries parsed earlier in the table.
    auto partial=valid;partial[0]=2;partial.insert(partial.begin()+18,16,255);write(file,partial);
    CHECK(!pac.open(file) && pac.entries().empty());
    for (int i=1;i<argc;++i) {
        CHECK(pac.open(argv[i]));
        for (size_t n=0;n<pac.entries().size();++n) CHECK(pac.readEntry(n,out));
        std::cout << "PAC PASS " << argv[i] << " entries=" << pac.entries().size() << '\n';
    }
    std::cout << "CORE PASS: VFS safety/overlay and PAC corrupt/memory/reopen tests\n";
    return 0;
}
