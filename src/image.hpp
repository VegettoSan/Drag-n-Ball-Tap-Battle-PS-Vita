#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct RgbaImage {
    uint32_t width = 0, height = 0;
    std::vector<uint8_t> pixels;
};
// Decode user-provided PNG in memory. No asset conversion or persistent output.
bool decodePng(const std::vector<uint8_t>& data, RgbaImage& image, std::string& error);
