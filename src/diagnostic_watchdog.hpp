#pragma once
#include <atomic>
#include <cstdint>
#include "performance.hpp"

// Diagnostic only. The native audio worker observes stage changes from the
// game thread. No user/game input and no state-machine changes.
struct DbtbDiagnosticStage {
    // 0: outside AOT update, 1: Java/AOT frame, 2: texture decode,
    // 3: texture GL upload, 4: resource read.
    std::atomic<uint32_t> phase{0};
    std::atomic<uint32_t> stamp_seconds{0};
    std::atomic<uint32_t> detail_bytes{0};
    std::atomic<uint32_t> detail_index{0};
};
inline DbtbDiagnosticStage& dbtb_diagnosticStage() {
    static DbtbDiagnosticStage state;
    return state;
}
inline void dbtb_setDiagnosticStage(uint32_t phase, uint32_t bytes=0, uint32_t index=0) {
    auto& state=dbtb_diagnosticStage();
    state.detail_bytes.store(bytes,std::memory_order_relaxed);
    state.detail_index.store(index,std::memory_order_relaxed);
    state.stamp_seconds.store(uint32_t(dbtb_timeUs()/1000000u),std::memory_order_relaxed);
    state.phase.store(phase,std::memory_order_release);
}
