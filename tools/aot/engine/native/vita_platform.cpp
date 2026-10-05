#include "dbtb_bridge.h"
#include "services.hpp"
#include "input.hpp"
#include "ui.hpp"
#include "vfs.hpp"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <vitaGL.h>

namespace {
std::unique_ptr<VitaInput> input;
bool renderer_ready = false;

void clearEvents(int32_t* events) {
    if (!events) return;
    for (int i = 0; i < 42; ++i) events[i] = 0;
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

    if (!vglInitExtended(0, 960, 544, 16 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE)) {
        std::fprintf(stderr, "Vita platform: vitaGL initialization failed\n");
        return 0;
    }
    renderer_ready = true;

    const std::vector<std::string> mods = selector_vfs.listMods();
    if (!selector_vfs.error().empty())
        std::fprintf(stderr, "Vita platform: mod scan: %s\n", selector_vfs.error().c_str());

    BootChoice choice;
    if (!runBootSelector(mods, selector_vfs.originalDataPresent(), choice)) {
        std::fprintf(stderr, "Vita platform: data selection cancelled\n");
        return 0;
    }

    const std::string mod = choice.original ? std::string() : choice.mod_directory;
    if (!dbtb_initResources(GameVfs::kBasePath, mod)) {
        std::fprintf(stderr, "Vita platform: resource VFS initialization failed\n");
        return 0;
    }

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
    // Android's Back key is the only physical-key field consumed by the
    // original loop today. Gameplay itself receives the front-touch pointers.
    events[40] = frame.back ? 1 : 0;
    events[41] = frame.pause ? 1 : 0; // reserved for the later lifecycle adapter
    return static_cast<int32_t>(count);
}

void dbtb_present(void) {
    if (renderer_ready) vglSwapBuffers(GL_FALSE);
}
}
