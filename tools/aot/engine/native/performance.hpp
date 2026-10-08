#pragma once
#include <cstdint>
#if defined(__vita__)
#include <psp2/kernel/processmgr.h>
#else
#include <chrono>
#endif

// Main-thread counters only. Audio publishes its own atomic summary, so these
// counters add no synchronization or file I/O to individual draws/loads.
struct DbtbPerformance {
    uint64_t resource_us = 0, texture_us = 0, texture_decode_us = 0, texture_upload_us = 0,
             text_us = 0, audio_decode_us = 0;
    uint64_t draws = 0, client_bytes = 0;
    uint64_t resource_bytes = 0;
    uint32_t resource_cache_hits = 0;
    uint32_t voice_cache_hits = 0;
    uint32_t texture_cache_hits = 0;
    uint32_t resources = 0, textures = 0;
};
inline DbtbPerformance& dbtb_performance() {
    static DbtbPerformance counters;
    return counters;
}
inline uint64_t dbtb_timeUs() {
#if defined(__vita__)
    return sceKernelGetProcessTimeWide();
#else
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
}
struct DbtbTimedScope {
    uint64_t& counter;
    uint64_t start;
    explicit DbtbTimedScope(uint64_t& target) : counter(target), start(dbtb_timeUs()) {}
    ~DbtbTimedScope() { counter += dbtb_timeUs() - start; }
};
