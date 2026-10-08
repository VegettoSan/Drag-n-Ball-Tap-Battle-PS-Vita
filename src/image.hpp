#pragma once
#include "community_profiles.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct RgbaImage {
    uint32_t width = 0, height = 0;
    bool premultiplied_alpha = false;
    std::vector<uint8_t> pixels;
};
// Decode user-provided PNG in memory. No asset conversion or persistent output.
bool decodePng(const std::vector<uint8_t>& data, RgbaImage& image, std::string& error);
bool decodePng(const uint8_t* data, size_t size, RgbaImage& image, std::string& error);

// Encoded dimensions + raw DEFLATE RGBA from an audited Community14 profile.
bool decodeCommunityImageProfile(const std::vector<uint8_t>& data, size_t entry_index,
                                 PacEncoding encoding, RgbaImage& image, std::string& error);
bool decodeCommunityImageProfile(const uint8_t* data, size_t size, size_t entry_index,
                                 PacEncoding encoding, RgbaImage& image, std::string& error);
// Backwards-compatible entry point for the hardware-confirmed a210795b profile.
bool decodeCommunityImage(const std::vector<uint8_t>& data, size_t entry_index,
                          RgbaImage& image, std::string& error);
