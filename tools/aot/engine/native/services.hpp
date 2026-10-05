#pragma once
#include "vfs.hpp"
#include <string>

bool dbtb_initResources(const std::string& base, const std::string& mod);
const GameVfs& dbtb_vfs();
void dbtb_forgetTexture(unsigned id);
void dbtb_mixAudio(short* interleaved, int frames);
bool dbtb_initFonts();
