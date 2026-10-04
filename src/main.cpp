#include "pac.hpp"
#include "image.hpp"
#include "log.hpp"
#include "ui.hpp"
#include "vfs.hpp"
#include "game_data.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <vitaGL.h>

extern "C" { unsigned int _newlib_heap_size_user = 64 * 1024 * 1024; }

int main() {
    GameVfs vfs;
    if (!vfs.prepareDirectories()) {
        std::fprintf(stderr, "%s\n", vfs.error().c_str());
        return 1;
    }

    runtimeLog(std::string("--- DB Tap Battle Vita " DBTB_VERSION " commit " DBTB_COMMIT " boot ---"));
    runtimeLog("State: boot selector; original data indicator is common.pac only");

    const std::vector<std::string> mods = vfs.listMods();
    runtimeLog("Detected mods: " + std::to_string(mods.size()));

    if (!vfs.error().empty()) runtimeLog("Mod scan: " + vfs.error());
    if (!vglInitExtended(0, 960, 544, 8 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE)) {
        runtimeLog("FATAL: vitaGL initialization failed");
        return 5;
    }
    runtimeLog("Renderer initialized: 960x544; no gameplay/audio yet");

    BootChoice choice;
    if (!runBootSelector(mods, vfs.originalDataPresent(), choice)) {
        runtimeLog("Boot selector cancelled by user");
        return 0;
    }

    if (choice.original) {
        vfs.selectOriginal();
        runtimeLog("Selected data set: Original");
    } else {
        if (!vfs.selectMod(choice.mod_directory)) {
            runtimeLog("Failed to activate mod: " + choice.mod_directory);
            showPacResult(false, "COULD NOT ACTIVATE MOD: " + choice.mod_directory);
            return 2;
        }
        runtimeLog("Selected mod: " + choice.mod_directory);
    }

    // Start the same two converted-table loads as original InitGameData.
    // Keep both tables alive for the engine that will replace the preview.
    GameDatabase database;
    if (!database.load(vfs)) {
        runtimeLog("Initial game data failed: " + database.error);
        showPacResult(false, "GAME DATA: " + database.error);
        return 8;
    }
    runtimeLog("Initial game tables loaded: game=" + std::to_string(database.game.records().size()) +
               " text=" + std::to_string(database.text.records().size()));
    runtimeLog("Game table source: " + database.game.sourcePath());
    runtimeLog("Text table source: " + database.text.sourcePath());

    std::string common_path;
    if (!vfs.resolve("common.pac", common_path)) {
        runtimeLog("common.pac not found through VFS");
        showPacResult(false, vfs.error());
        return 3;
    }

    runtimeLog("Resolved common.pac: " + common_path);

    PacFile pac;
    if (!pac.open(common_path)) {
        runtimeLog("PAC parse failed: " + pac.error());
        showPacResult(false, "COMMON.PAC: " + pac.error());
        return 4;
    }

    const std::string detail = "COMMON.PAC ENTRIES: " + std::to_string(pac.entries().size()) +
                               "  SOURCE: " + (choice.original ? std::string("ORIGINAL") : choice.mod_directory);
    runtimeLog(std::string("PAC codec: ") +
               (pac.encoding() == PacEncoding::Community14 ? "community14-a210795b" : "original"));
    runtimeLog("PAC parse OK. Entries: " + std::to_string(pac.entries().size()));
    RgbaImage image;
    bool found_image = false;
    for (size_t i = 0; i < pac.entries().size(); ++i) {
        const bool community_image = pac.typeString(i) == "rgba";
        if (!community_image && pac.typeString(i) != "png") continue;
        std::vector<uint8_t> payload;
        std::string error;
        if (!pac.readEntry(i, payload) ||
            !(community_image ? decodeCommunityImage(payload, i, image, error) : decodePng(payload, image, error))) {
            runtimeLog("Image error at entry " + std::to_string(i) + ": " +
                       (pac.error().empty() ? error : pac.error()));
            showPacResult(false, "IMAGE ERROR: " + (pac.error().empty() ? error : pac.error()));
            return 6;
        }
        runtimeLog("Image decoded: entry " + std::to_string(i) + " " + std::to_string(image.width) +
                   "x" + std::to_string(image.height) + " bytes=" + std::to_string(payload.size()));
        found_image = true;
        break;
    }
    if (!found_image) {
        runtimeLog("PAC valid but no supported image entry: unsupported bootstrap dataset");
        showPacResult(false, "PAC VALID, NO IMAGE FOR PREVIEW");
        return 7;
    }
    runtimeLog("State: original resource preview (diagnostic atlas, not game menu)");
    showPacResult(true, detail, &image);

    return 0;
}
