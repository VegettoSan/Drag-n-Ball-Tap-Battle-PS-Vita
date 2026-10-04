#include "pac.hpp"
#include "ui.hpp"
#include "vfs.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <vitaGL.h>

namespace {

void appendRuntimeLog(const std::string& line) {
    FILE* fp = std::fopen("ux0:data/DBTapBattle/runtime.log", "ab");
    if (!fp) return;
    std::fwrite(line.data(), 1, line.size(), fp);
    std::fwrite("\n", 1, 1, fp);
    std::fclose(fp);
}

} // namespace

int main() {
    GameVfs vfs;
    if (!vfs.prepareDirectories()) {
        return 1;
    }

    appendRuntimeLog("--- DB Tap Battle Vita boot ---");

    const std::vector<std::string> mods = vfs.listMods();
    appendRuntimeLog("Detected mods: " + std::to_string(mods.size()));

    vglInit(0x800000);

    BootChoice choice;
    if (!runBootSelector(mods, vfs.originalDataPresent(), choice)) {
        appendRuntimeLog("Boot selector cancelled by user");
        return 0;
    }

    if (choice.original) {
        vfs.selectOriginal();
        appendRuntimeLog("Selected data set: Original");
    } else {
        if (!vfs.selectMod(choice.mod_directory)) {
            appendRuntimeLog("Failed to activate mod: " + choice.mod_directory);
            showPacResult(false, "COULD NOT ACTIVATE MOD: " + choice.mod_directory);
            return 2;
        }
        appendRuntimeLog("Selected mod: " + choice.mod_directory);
    }

    std::string common_path;
    if (!vfs.resolve("common.pac", common_path)) {
        appendRuntimeLog("common.pac not found through VFS");
        showPacResult(false, "COMMON.PAC NOT FOUND. COPY ORIGINAL DATA TO UX0:DATA/DBTAPBATTLE/GAME");
        return 3;
    }

    appendRuntimeLog("Resolved common.pac: " + common_path);

    PacFile pac;
    if (!pac.open(common_path)) {
        appendRuntimeLog("PAC parse failed: " + pac.error());
        showPacResult(false, "COMMON.PAC: " + pac.error());
        return 4;
    }

    const std::string detail = "COMMON.PAC ENTRIES: " + std::to_string(pac.entries().size()) +
                               "  SOURCE: " + (choice.original ? std::string("ORIGINAL") : choice.mod_directory);
    appendRuntimeLog("PAC parse OK. Entries: " + std::to_string(pac.entries().size()));
    showPacResult(true, detail);

    return 0;
}
