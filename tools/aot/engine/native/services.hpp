#pragma once
#include "vfs.hpp"
#include <string>

bool dbtb_initResources(const std::string& base, const std::string& mod);
const GameVfs& dbtb_vfs();
void dbtb_forgetTexture(unsigned id);
void dbtb_mixAudio(short* interleaved, int frames);
bool dbtb_initFonts();

struct DbtbAudioStats {
    uint32_t clipped_samples = 0;
    uint32_t late_mix_blocks = 0;
    uint32_t max_mix_us = 0;
};
DbtbAudioStats dbtb_takeAudioStats();
