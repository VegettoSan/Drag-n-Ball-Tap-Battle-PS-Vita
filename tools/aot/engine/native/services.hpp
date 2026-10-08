#pragma once
// Drop cache-only PAC owners and idle imported textures before large audio loads.
void dbtb_reclaimIdleResources();
#include "vfs.hpp"
#include <string>

bool dbtb_initResources(const std::string& base, const std::string& mod);
void dbtb_setControlMode(int mode);
const GameVfs& dbtb_vfs();
bool dbtb_releaseTexture(unsigned id);
void dbtb_mixAudio(short* interleaved, int frames);
bool dbtb_initFonts();

struct DbtbAudioStats {
    uint32_t clipped_samples = 0;
    uint32_t overload_samples = 0;
    uint32_t late_mix_blocks = 0;
    uint32_t max_mix_us = 0;
    uint32_t submission_gaps = 0;
    uint32_t max_submission_gap_us = 0;
};
DbtbAudioStats dbtb_takeAudioStats();
