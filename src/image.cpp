#include "image.hpp"
#include <png.h>

bool decodePng(const std::vector<uint8_t>& data, RgbaImage& image, std::string& error) {
    image = RgbaImage{};
    error.clear();
    if (data.size() < 8 || png_sig_cmp(data.data(), 0, 8)) {
        error = "not a PNG payload";
        return false;
    }
    png_image png{};
    png.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_memory(&png, data.data(), data.size())) {
        error = png.message;
        png_image_free(&png);
        return false;
    }
    // Limit decoded allocation independently of compressed PAC entry size.
    const uint64_t bytes = static_cast<uint64_t>(png.width) * png.height * 4;
    if (!png.width || !png.height || png.width > 4096 || png.height > 4096 || bytes > 16 * 1024 * 1024) {
        error = "PNG dimensions exceed bootstrap texture budget";
        png_image_free(&png);
        return false;
    }
    png.format = PNG_FORMAT_RGBA;
    image.pixels.resize(static_cast<size_t>(bytes));
    if (!png_image_finish_read(&png, nullptr, image.pixels.data(), 0, nullptr)) {
        error = png.message;
        png_image_free(&png);
        image = RgbaImage{};
        return false;
    }
    image.width = png.width;
    image.height = png.height;
    png_image_free(&png);
    return true;
}
