#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
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
#include <psp2/power.h>

#ifndef DBTB_VERSION
#define DBTB_VERSION "dev"
#endif
#ifndef DBTB_COMMIT
#define DBTB_COMMIT "unknown"
#endif

namespace {
std::unique_ptr<VitaInput> input;
bool renderer_ready = false;
uint64_t frame_start = 0, previous_frame = 0, window_start = 0;
uint64_t run_total = 0, run_max = 0, swap_total = 0, interval_max = 0;
unsigned window_frames = 0;

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
    runtimeLog(std::string("--- full original engine Vita ") + DBTB_VERSION +
               " boot (" + DBTB_COMMIT + ") ---");

    // Public Vita clock settings; no overclock plugin is required. The original
    // frame loop, full-resolution renderer and audio worker share the CPU.
    const int cpu_result = scePowerSetArmClockFrequency(444);
    const int bus_result = scePowerSetBusClockFrequency(166);
    const int gpu_result = scePowerSetGpuClockFrequency(222);
    const int xbar_result = scePowerSetGpuXbarClockFrequency(166);
    runtimeLog("Clocks CPU=" + std::to_string(scePowerGetArmClockFrequency()) +
               " bus=" + std::to_string(scePowerGetBusClockFrequency()) +
               " GPU=" + std::to_string(scePowerGetGpuClockFrequency()) +
               " xbar=" + std::to_string(scePowerGetGpuXbarClockFrequency()) +
               " results=" + std::to_string(cpu_result) + "," +
               std::to_string(bus_result) + "," + std::to_string(gpu_result) +
               "," + std::to_string(xbar_result));

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
    frame_start = dbtb_timeUs();
    if (!window_start) window_start = frame_start;
    if (previous_frame)
        interval_max = std::max(interval_max, frame_start - previous_frame);
    previous_frame = frame_start;
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
    if (!renderer_ready) return;
    const uint64_t before_swap = dbtb_timeUs();
    const uint64_t run_us = frame_start ? before_swap - frame_start : 0;
    run_total += run_us;
    run_max = std::max(run_max, run_us);
    vglSwapBuffers(GL_FALSE);
    const uint64_t now = dbtb_timeUs();
    swap_total += now - before_swap;
    if (++window_frames < 120 || !window_start || now <= window_start) return;

    const auto& perf = dbtb_performance();
    const auto audio = dbtb_takeAudioStats();
    // Swap includes pacing/GPU waits. Run includes Java/GL/loads; neither is a
    // hardware CPU utilization counter. Interval also includes cooperative work
    // after present(), making it possible to spot an event-queue loading pause.
    std::fprintf(stderr,
        "[Perf] fps=%.1f run_ms=%.2f run_max_ms=%.2f swap_ms=%.2f interval_max_ms=%.2f "
        "draws_per_frame=%.1f client_KiB=%llu loads=%u load_ms=%.1f textures=%u "
        "texture_ms=%.1f text_ms=%.1f audio_decode_ms=%.1f "
        "resource_cache_hits=%u resource_io_KiB=%llu voice_cache_hits=%u texture_cache_hits=%u "
        "audio_clip_samples=%u audio_overload_samples=%u audio_late_mix=%u audio_mix_max_us=%u "
        "audio_submit_gaps=%u audio_submit_max_us=%u\n",
        double(window_frames) * 1000000.0 / double(now - window_start),
        double(run_total) / (window_frames * 1000.0), double(run_max) / 1000.0,
        double(swap_total) / (window_frames * 1000.0), double(interval_max) / 1000.0,
        double(perf.draws) / window_frames,
        static_cast<unsigned long long>(perf.client_bytes / 1024), perf.resources,
        double(perf.resource_us) / 1000.0, perf.textures, double(perf.texture_us) / 1000.0,
        double(perf.text_us) / 1000.0, double(perf.audio_decode_us) / 1000.0,
        perf.resource_cache_hits, static_cast<unsigned long long>(perf.resource_bytes / 1024), perf.voice_cache_hits, perf.texture_cache_hits,
        audio.clipped_samples, audio.overload_samples, audio.late_mix_blocks, audio.max_mix_us,
        audio.submission_gaps, audio.max_submission_gap_us);
    dbtb_performance() = DbtbPerformance{};
    window_start = now;
    window_frames = 0;
    run_total = run_max = swap_total = interval_max = 0;
}
}
