#include "control_settings.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <unistd.h>

// Run inside an owned temporary host directory: Vita's relative ux0:data path
// is a fixture here. Legacy three-mode files remain readable after the UI change.
int main() {
    char temporary[]="/tmp/dbtb-control-settings-XXXXXX";
    assert(mkdtemp(temporary) && chdir(temporary)==0);
    assert(mkdir("ux0:data",0700)==0);
    GameVfs vfs(GameVfs::kBasePath);assert(vfs.prepareDirectories());
    const std::string profile="Fixture";
    assert(mkdir((std::string(GameVfs::kBasePath)+"/profiles/"+profile).c_str(),0700)==0);
    assert(readControlPreference(profile)==0);
    for(int mode=0;mode<3;mode++) {
        assert(writeControlPreference(profile,mode));
        assert(readControlPreference(profile)==mode);
        const int row=controlSelectionFromPreference(readControlPreference(profile));
        assert(row==(mode?0:1));
        assert(controlModeFromSelection(row)==(mode?2:0));
    }
    assert(writeControlPreference(profile,1));
    assert(writeControlPreference(profile,controlModeFromSelection(controlSelectionFromPreference(readControlPreference(profile)))));
    assert(readControlPreference(profile)==2);
    const auto path=controlPreferencePath(profile);
    {std::ofstream f(path);f<<"DBTC1:9\n";}assert(readControlPreference(profile)==0);
    {std::ofstream f(path);f<<"DBTC1:2";}assert(readControlPreference(profile)==0);
    assert(!writeControlPreference(profile,3));
    assert(unlink(path.c_str())==0);
    assert(symlink("missing",path.c_str())==0 && readControlPreference(profile)==0);
    assert(unlink(path.c_str())==0);
    std::puts("CONTROL SETTINGS PASS: two choices; legacy visible/hidden migration; persistence; malformed/symlink fallback");
}
