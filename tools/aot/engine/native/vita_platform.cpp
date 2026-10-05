#include "dbtb_bridge.h"
#include "services.hpp"
#include "input.hpp"
#include "log.hpp"
#include "ui.hpp"
#include "vfs.hpp"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <vitaGL.h>

#ifndef DBTB_VERSION
#define DBTB_VERSION "dev"
#endif

namespace {
std::unique_ptr<VitaInput> input;
bool renderer_ready = false;

void clearEvents(int32_t* events) {
    if (!events) return;
    for (int i = 0; i < 42; ++i) events[i] = 0;
}

void attachRuntimeStreams() {
    // Preserve TeaVM System.out/System.err and native diagnostics for device
    // testing. Append rather than truncate so an earlier crash remains visible.
    FILE* out = std::freopen("ux0:data/DBTapBattle/logs/runtime.log", "ab", stdout);
    FILE* err = std::freopen("ux0:data/DBTapBattle/logs/runtime.log", "ab", stderr);
    if (out) std::setvbuf(stdout, nullptr, _IOLBF, 0);
    if (err) std::setvbuf(stderr, nullptr, _IOLBF, 0);
}
}

extern "C" {
int32_t dbtb_start(void) {
    if (renderer_ready) return 0;

    GameVfs selector_vfs(GameVfs::kBasePath);
    if (!selector_vfs.prepareDirectories()) {
        std::fprintf(stderr, "Vita platform: %s\n", selector_vfs.error().c_str());
        return 0;
    }
    attachRuntimeStreams();
    runtimeLog(std::string("--- full original engine Vita ") + DBTB_VERSION + " boot ---");

    runtimeLog("Initializing vitaGL: 960x544, RAM threshold 16 MiB");
    // vitaGL's return value is NOT a success flag. GL_TRUE means the requested
    // resolution was too large and vitaGL fell back to the maximum supported
    // framebuffer size; GL_FALSE is the normal result for native 960x544.
    const GLboolean resolution_fallback =
        vglInitExtended(0, 960, 544, 16 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);
    renderer_ready = true;
    runtimeLog(resolution_fallback
        ? "vitaGL initialized with framebuffer resolution fallback"
        : "vitaGL initialized: 960x544 (no resolution fallback)");

    const std::vector<std::string> mods = selector_vfs.listMods();
    runtimeLog("Detected data/mod profiles: " + std::to_string(mods.size()));
    if (!selector_vfs.error().empty())
        runtimeLog("Mod scan: " + selector_vfs.error());

    BootChoice choice;
    if (!runBootSelector(mods, selector_vfs.originalDataPresent(), choice)) {
        runtimeLog("Data selection cancelled");
        return 0;
    }

    const std::string mod = choice.original ? std::string() : choice.mod_directory;
    runtimeLog(std::string("Selected profile: ") + (choice.original ? "Original" : choice.mod_directory));
    if (!dbtb_initResources(GameVfs::kBasePath, mod)) {
        runtimeLog("FATAL: resource VFS initialization failed");
        return 0;
    }
    runtimeLog("Resource VFS initialized; entering original engine");

    // Construct after the selector so held touches are primed and cannot leak
    // into the original title/menu as a new Begin event.
    input.reset(new VitaInput());
    std::printf("Vita platform ready: %s\n", choice.original ? "Original" : choice.mod_directory.c_str());
    return 1;
}

int32_t dbtb_frame(void* raw_events) {
    if (!renderer_ready || !input || !raw_events) return -1;
    auto* events = static_cast<int32_t*>(raw_events);
    clearEvents(events);

    const InputFrame frame = input->poll();
    const size_t count = std::min<size_t>(frame.pointer_count, 10);
    for (size_t i = 0; i < count; ++i) {
        const PointerEvent& event = frame.pointers[i];
        const size_t p = i * 4;
        events[p + 0] = event.id;
        events[p + 1] = static_cast<int32_t>(event.x);
        events[p + 2] = static_cast<int32_t>(event.y);
        events[p + 3] = static_cast<int32_t>(event.phase);
    }

    // The selector deliberately uses Vita face buttons, but once the original
    // touch game is running we must not translate Circle/Triangle into Android
    // Back. The original engine treats Back as an application/menu exit request
    // in several states, which made ordinary Vita button presses close the game.
    // Keep physical gameplay controls neutral until their gesture mappings are
    // implemented intentionally; front-touch remains the authoritative input.
    events[40] = 0;
    events[41] = 0;
    if (frame.back || frame.pause)
        runtimeLog("Gameplay physical back/pause button ignored; use front touch");
    return static_cast<int32_t>(count);
}

void dbtb_present(void) {
    if (renderer_ready) vglSwapBuffers(GL_FALSE);
}
}
