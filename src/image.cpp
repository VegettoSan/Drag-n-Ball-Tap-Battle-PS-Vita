#include "image.hpp"
#include <png.h>
#include <zlib.h>
#include <limits>
#include <algorithm>

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

bool communityImageDimensions(const uint8_t* data, size_t size, size_t entry_index,
                              PacEncoding encoding, uint32_t& width, uint32_t& height,
                              std::string& error) {
    width = height = 0;
    const CommunityPacProfile* profile = communityProfile(encoding);
    if (!profile || !data || size < 5 || entry_index > 65535) {
        error = "invalid community image header or profile";
        return false;
    }
    width = ((uint32_t(data[0]) << 8) | data[1]) ^ profile->image_width_xor ^ uint32_t(entry_index);
    height = ((uint32_t(data[2]) << 8) | data[3]) ^ profile->image_height_xor ^ uint32_t(entry_index);
    const uint64_t bytes = uint64_t(width) * height * 4;
    if (!width || !height || width > 4096 || height > 4096 || bytes > 16u*1024u*1024u ||
        size - 4 > std::numeric_limits<uInt>::max()) {
        error = "community image dimensions exceed texture limit";
        width = height = 0;
        return false;
    }
    return true;
}

bool decodeCommunityImageProfileRows(const uint8_t* data, size_t size, size_t entry_index,
                                     PacEncoding encoding, void* opaque, CommunityImageRowSink sink,
                                     std::string& error) {
    error.clear();
    uint32_t width = 0, height = 0;
    if (!sink || !communityImageDimensions(data, size, entry_index, encoding, width, height, error))
        return false;
    constexpr size_t kTileBudget = 256u * 1024u;
    const size_t row_bytes = size_t(width) * 4;
    const uint32_t tile_rows = uint32_t(std::max<size_t>(1, kTileBudget / row_bytes));
    const size_t allocated = std::min<size_t>(tile_rows, height) * row_bytes;
    std::vector<uint8_t> tile(allocated);
    z_stream z{};
    z.next_in = const_cast<Bytef*>(data + 4);
    z.avail_in = uInt(size - 4);
    if (inflateInit2(&z, -15) != Z_OK) {
        error = "community image raw DEFLATE init failed"; return false;
    }
    int status = Z_OK;
    bool valid = true;
    for (uint32_t y = 0; y < height && valid; y += tile_rows) {
        const uint32_t rows = std::min(tile_rows, height - y);
        z.next_out = tile.data();
        z.avail_out = uInt(size_t(rows) * row_bytes);
        while (z.avail_out && valid) {
            const uLong before_in = z.total_in, before_out = z.total_out;
            status = inflate(&z, Z_NO_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END) valid = false;
            else if (z.avail_out && status == Z_STREAM_END) valid = false;
            else if (z.total_in == before_in && z.total_out == before_out) valid = false;
        }
        if (valid && !sink(opaque, tile.data(), y, rows, width)) {
            error = "community texture row upload failed"; valid = false;
        }
        if (status == Z_STREAM_END && y + rows < height) valid = false;
    }
    if (valid && status != Z_STREAM_END) {
        // A stream can fill the last scanline before consuming its end marker.
        uint8_t overflow = 0;
        z.next_out = &overflow;
        z.avail_out = 1;
        while (status == Z_OK) {
            const uLong before_in = z.total_in;
            status = inflate(&z, Z_FINISH);
            if (status != Z_STREAM_END && (status != Z_OK || before_in == z.total_in)) break;
        }
    }
    if (!valid || status != Z_STREAM_END || z.total_out != uint64_t(width)*height*4 || z.avail_in != 0) {
        if (error.empty()) error = "invalid community DEFLATE image stream or dimensions";
        valid = false;
    }
    inflateEnd(&z);
    return valid;
}

bool decodeCommunityImageProfile(const std::vector<uint8_t>& data, size_t entry_index,
                                 PacEncoding encoding, RgbaImage& image, std::string& error) {
    return decodeCommunityImageProfile(data.data(), data.size(), entry_index, encoding, image, error);
}

bool decodeCommunityImage(const std::vector<uint8_t>& data, size_t entry_index,
                          RgbaImage& image, std::string& error) {
    return decodeCommunityImageProfile(data, entry_index, PacEncoding::Community14, image, error);
}
