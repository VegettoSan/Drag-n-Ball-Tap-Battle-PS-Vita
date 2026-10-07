#include "installed_data.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace {
void writePac(const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    assert(out);
    const char empty_pac[2] = {0, 0};
    out.write(empty_pac, sizeof(empty_pac));
    assert(out);
}

void writeTriplet(const std::string& root, int index) {
    char name[64];
    for (const char* format : {"char%02d.pac", "chardemo%02d.pac", "charf00%02d.pac"}) {
        std::snprintf(name, sizeof(name), format, index);
        writePac(root + "/" + name);
    }
}

void removeTriplet(const std::string& root, int index) {
    char name[64];
    for (const char* format : {"char%02d.pac", "chardemo%02d.pac", "charf00%02d.pac"}) {
        std::snprintf(name, sizeof(name), format, index);
        assert(unlink((root + "/" + name).c_str()) == 0);
    }
}
}

int main() {
    char temp[] = "/tmp/dbtb-installed-data-XXXXXX";
    char* base = mkdtemp(temp);
    assert(base);
    GameVfs vfs(base);
    assert(vfs.prepareDirectories());

    const std::string game = std::string(base) + "/game";
    const std::string mods = std::string(base) + "/mods/Invasion";
    assert(mkdir(mods.c_str(), 0700) == 0);

    for (int i = 0; i < 13; ++i) writeTriplet(game, i);
    for (const char* name : {"select0.pac", "effect.pac", "back00.pac", "bobj00.pac"})
        writePac(game + "/" + name);

    auto audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 13 && audit.error.empty());

    // Android14/Spanish/Invasion are valid standalone datasets even though
    // their bobj family starts at 01. The Vita gate must not require bobj00.
    assert(unlink((game + "/bobj00.pac").c_str()) == 0);
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 13 && audit.error.empty());

    // A selected APK profile is a standalone dataset. It carries its own
    // complete roster/shared resources and must never borrow missing files from
    // game/, even when a matching base file happens to exist there.
    for (int i = 0; i < 22; ++i) writeTriplet(mods, i);
    for (const char* name : {"select0.pac", "effect.pac", "back00.pac"})
        writePac(mods + "/" + name);
    assert(vfs.selectMod("Invasion"));
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 22 && audit.error.empty());

    // Prove no hidden game/ fallback: game/ still has charf0000.pac, but a
    // missing selected-profile copy must fail the selected dataset audit.
    assert(unlink((mods + "/charf0000.pac").c_str()) == 0);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 0 &&
           audit.error == "character triplet is incomplete");
    writePac(mods + "/charf0000.pac");

    // One missing member of a triplet is rejected.
    assert(unlink((mods + "/charf0017.pac").c_str()) == 0);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 17 &&
           audit.error == "character triplet is incomplete");
    writePac(mods + "/charf0017.pac");

    // A full gap followed by later characters is rejected.
    removeTriplet(mods, 17);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 17 &&
           audit.error == "character data is not contiguous");
    writeTriplet(mods, 17);

    // An invalid PAC is a structural failure, not a missing-file fallback.
    {
        std::ofstream broken(mods + "/char20.pac", std::ios::binary | std::ios::trunc);
        broken << "x";
    }
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 20 &&
           audit.error.find("char20.pac:") == 0);
    writePac(mods + "/char20.pac");

    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 22);

    // Gen-derived community datasets can extend the same two-digit resource
    // namespace far beyond Invasion. SamuGamerYT is observed through index 91.
    for (int i = 22; i < 92; ++i) writeTriplet(mods, i);
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 92 && audit.error.empty());

    // The Vita-side gate accepts the complete two-digit namespace 00..99.
    // This validates capacity only; it does not invent data beyond a supplied mod.
    for (int i = 92; i < 100; ++i) writeTriplet(mods, i);
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 100 && audit.error.empty());
    audit = auditInstalledData(vfs, 13, 101);
    assert(!audit.ready && audit.error == "invalid character audit bounds");

    std::puts("INSTALLED DATA PASS: standalone selected profile, no game fallback, protected bobj00 omission, 13 baseline, 22 Invasion, 92 Samu, 00..99 namespace");
    return 0;
}
