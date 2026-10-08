#include "image.hpp"
#include <png.h>
#include <zlib.h>
#include <limits>

bool decodePng(const uint8_t* data, size_t size, RgbaImage& image, std::string& error) {
    image = RgbaImage{};
    error.clear();
    if (size < 8 || !data || png_sig_cmp(data, 0, 8)) {
        error = "not a PNG payload";
        return false;
    }
    png_image png{};
    png.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_memory(&png, data, size)) {
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

bool decodePng(const std::vector<uint8_t>& data, RgbaImage& image, std::string& error) {
    return decodePng(data.data(), data.size(), image, error);
}

bool decodeCommunityImageProfile(const uint8_t* data, size_t size, size_t entry_index,
                                 PacEncoding encoding, RgbaImage& image, std::string& error) {
    image = RgbaImage{};
    error.clear();
    const CommunityPacProfile* profile = communityProfile(encoding);
    if (!profile) {
        error = "unknown community image profile";
        return false;
    }
    if (!data || size < 5 || entry_index > 65535) {
        error = "truncated community image / invalid entry index";
        return false;
    }
    const uint32_t width = ((data[0] << 8) | data[1]) ^ profile->image_width_xor ^ entry_index;
    const uint32_t height = ((data[2] << 8) | data[3]) ^ profile->image_height_xor ^ entry_index;
    const uint64_t bytes = static_cast<uint64_t>(width) * height * 4;
    if (!width || !height || width > 4096 || height > 4096 || bytes > 16 * 1024 * 1024 ||
        size - 4 > std::numeric_limits<uInt>::max()) {
        error = "community image dimensions exceed bootstrap texture budget";
        return false;
    }
    std::vector<uint8_t> pixels(static_cast<size_t>(bytes));
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(data + 4);
    stream.avail_in = static_cast<uInt>(size - 4);
    stream.next_out = pixels.data();
    stream.avail_out = static_cast<uInt>(pixels.size());
    if (inflateInit2(&stream, -15) != Z_OK) {
        error = "could not initialize community image DEFLATE";
        return false;
    }
    const int status = inflate(&stream, Z_FINISH);
    const bool valid = status == Z_STREAM_END && stream.total_out == bytes && stream.avail_in == 0;
    inflateEnd(&stream);
    if (!valid) {
        error = "invalid community image DEFLATE size or termination";
        return false;
    }
    image.width = width;
    image.height = height;
    image.premultiplied_alpha = true;
    image.pixels.swap(pixels);
    return true;
}

bool decodeCommunityImageProfile(const std::vector<uint8_t>& data, size_t entry_index,
                                 PacEncoding encoding, RgbaImage& image, std::string& error) {
    return decodeCommunityImageProfile(data.data(), data.size(), entry_index, encoding, image, error);
}

bool decodeCommunityImage(const std::vector<uint8_t>& data, size_t entry_index,
                          RgbaImage& image, std::string& error) {
    return decodeCommunityImageProfile(data, entry_index, PacEncoding::Community14, image, error);
}
