#pragma once
#include <cstdint>
#include <string>

struct DbtbCompressedBgm;

// Open an MP3 or AAC/M4A BGM exactly as stored in the selected dataset.
// Returns nullptr for non-supported/non-compressed input; error is diagnostic.
DbtbCompressedBgm* dbtb_openCompressedBgm(const std::string& relative,
                                          float gain, bool loop,
                                          std::string& error);
void dbtb_closeCompressedBgm(DbtbCompressedBgm* stream);
void dbtb_mixCompressedBgm(DbtbCompressedBgm* stream, int32_t& left, int32_t& right);
const char* dbtb_compressedBgmCodec(const DbtbCompressedBgm* stream);
