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
    const std::string base(root), profile = base + "/profiles/Mod ñ .. test";
    CHECK(mkdir(profile.c_str(), 0777) == 0);
    write(profile + "/common.pac", {1});
    CHECK(vfs.selectProfile("Mod ñ .. test"));
    std::string resolved;
    CHECK(vfs.resolve("common.pac", resolved) && resolved == profile + "/common.pac");
    CHECK(vfs.listProfiles().size() == 1);
    for (const std::string bad : {"../x", "x/../y", "ux0:x", "/x", "x\\y", "x//y", "./x", "x/./y", "x/", ""})
        CHECK(!vfs.resolve(bad, resolved) && resolved.empty());
    CHECK(!vfs.resolve(std::string("common.pac\0evil", 15), resolved));
    CHECK(!vfs.selectProfile("../profiles"));
    CHECK(vfs.activeProfile() == "Mod ñ .. test");
    CHECK(mkdir((profile + "/nested").c_str(), 0777) == 0);
    write(profile + "/nested/a", {4});
    CHECK(vfs.resolve("nested/a", resolved));
    CHECK(mkdir((profile + "/other.dat").c_str(), 0777) == 0);
    CHECK(!vfs.resolve("other.dat", resolved));
    CHECK(symlink((profile + "/common.pac").c_str(), (profile + "/escape").c_str()) == 0);
    CHECK(!vfs.resolve("escape", resolved));
    for (int n=0;n<256;++n)
        CHECK(mkdir((base + "/profiles/Folder " + std::to_string(n)).c_str(),0777)==0);
    CHECK(vfs.listProfiles().size()==257);
    const std::string long_name(255,'L');
    CHECK(mkdir((base + "/profiles/" + long_name).c_str(),0777)==0);
    CHECK(vfs.selectProfile(long_name));
    CHECK(vfs.listProfiles().size()==258);

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
    std::cout << "CORE PASS: unified profile VFS safety and PAC corrupt/memory/reopen tests\n";
    return 0;
}
