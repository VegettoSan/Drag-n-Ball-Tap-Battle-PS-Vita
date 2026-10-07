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

    const std::string profile = std::string(base) + "/profiles/Invasion";
    assert(mkdir(profile.c_str(), 0700) == 0);

    for (int i = 0; i < 22; ++i) writeTriplet(profile, i);
    for (const char* name : {"select0.pac", "effect.pac", "back00.pac"})
        writePac(profile + "/" + name);
    assert(vfs.selectProfile("Invasion"));

    auto audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 22 && audit.error.empty());

    // A selected profile is autonomous. Missing files are never borrowed from
    // another dataset because every dataset lives under profiles/.
    assert(unlink((profile + "/charf0000.pac").c_str()) == 0);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 0 &&
           audit.error == "character triplet is incomplete");
    writePac(profile + "/charf0000.pac");

    // One missing member of a triplet is rejected.
    assert(unlink((profile + "/charf0017.pac").c_str()) == 0);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 17 &&
           audit.error == "character triplet is incomplete");
    writePac(profile + "/charf0017.pac");

    // A full gap followed by later characters is rejected.
    removeTriplet(profile, 17);
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 17 &&
           audit.error == "character data is not contiguous");
    writeTriplet(profile, 17);

    // An invalid PAC is a structural failure, not a missing-file fallback.
    {
        std::ofstream broken(profile + "/char20.pac", std::ios::binary | std::ios::trunc);
        broken << "x";
    }
    audit = auditInstalledData(vfs);
    assert(!audit.ready && audit.complete_characters == 20 &&
           audit.error.find("char20.pac:") == 0);
    writePac(profile + "/char20.pac");

    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 22);

    // Gen-derived community datasets can extend the same two-digit resource
    // namespace far beyond Invasion. SamuGamerYT is observed through index 91.
    for (int i = 22; i < 92; ++i) writeTriplet(profile, i);
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 92 && audit.error.empty());

    // The Vita-side gate accepts the complete two-digit namespace 00..99.
    // This validates capacity only; it does not invent data beyond a supplied mod.
    for (int i = 92; i < 100; ++i) writeTriplet(profile, i);
    audit = auditInstalledData(vfs);
    assert(audit.ready && audit.complete_characters == 100 && audit.error.empty());
    audit = auditInstalledData(vfs, 13, 101);
    assert(!audit.ready && audit.error == "invalid character audit bounds");

    std::puts("INSTALLED DATA PASS: standalone unified selected profile, no cross-profile fallback, protected bobj00 omission, 13 baseline, 22 Invasion, 92 Samu, 00..99 namespace");
    return 0;
}
