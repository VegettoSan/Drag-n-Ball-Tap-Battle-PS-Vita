#include "engine_resources.hpp"
#include "game_data.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

namespace {
constexpr size_t kBudget = 32u * 1024u * 1024u;
uint16_t le16(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
uint32_t le32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
}
void put16(std::vector<uint8_t>& out, size_t p, uint32_t n) { out[p] = n; out[p + 1] = n >> 8; }
void put32(std::vector<uint8_t>& out, size_t p, uint32_t n) {
    for (size_t i = 0; i < 4; ++i) out[p + i] = n >> (8 * i);
}
const char* tag(uint32_t type) {
    switch (type) {
        case 0x5d93757fu: return "act";
        case 0x8f230d0du: return "bin";
        case 0x86ffa7f3u: return "cnv";
        case 0x84dff882u: return "dac";
        case 0x8728c48au: return "rgba";
        case 0x83f2e69au: return "spr";
        case 0x425206e2u: return "wav";
        default: return nullptr;
    }
}

bool normaliseConvertedTable(std::vector<uint8_t>& payload, std::string& error) {
    GameDataTable table;
    if (!table.decode(payload, PacEncoding::Community14)) { error = table.error(); return false; }
    put16(payload, 0, table.records().size());
    for (size_t j = 0; j < table.records().size(); ++j) {
        const auto& record = table.records()[j];
        put32(payload, 2 + j * 8, record.position);
        put16(payload, 6 + j * 8, record.width);
        put16(payload, 8 + j * 8, record.height);
    }
    return true;
}

bool normaliseCommunityWav(std::vector<uint8_t>& payload, uint32_t entry_index, std::string& error) {
    if (payload.size() < 5) { error = "Community14 WAV wrapper is truncated"; return false; }
    const uint32_t decoded_size = le32(payload.data()) ^ 42802u ^ entry_index;
    if (!decoded_size || decoded_size > kBudget || (decoded_size & 1u)) {
        error = "Community14 WAV decoded size is invalid"; return false;
    }
    const uint8_t compression = payload[4];
    if (!compression) {
        if (payload.size() - 5 < decoded_size) { error = "Community14 PCM WAV payload is truncated"; return false; }
        std::vector<uint8_t> pcm(payload.begin() + 5, payload.begin() + 5 + decoded_size);
        payload.swap(pcm);
        return true;
    }

    // The supplied Android14 loader's native ext.a.w routine is the standard
    // PlayStation ADPCM predictor: one 16-byte frame carries 28 mono 16-bit
    // samples, with predictor/shift in byte 0 and the terminator flag in byte 1.
    // Reconstruct the PCM contract expected by the original AudioTrack path;
    // do not expose compressed community bytes to SoundEffect.
    const size_t compressed_size = payload.size() - 5;
    if (!compressed_size || (compressed_size & 15u)) { error = "Community14 ADPCM is not frame aligned"; return false; }
    static const double kCoef[5][2] = {
        {0.0, 0.0},
        {60.0 / 64.0, 0.0},
        {115.0 / 64.0, -52.0 / 64.0},
        {98.0 / 64.0, -55.0 / 64.0},
        {122.0 / 64.0, -60.0 / 64.0}
    };
    std::vector<uint8_t> pcm;
    pcm.reserve(decoded_size);
    double previous1 = 0.0, previous2 = 0.0;
    const uint8_t* src = payload.data() + 5;
    for (size_t frame = 0; frame < compressed_size; frame += 16) {
        const uint8_t header = src[frame];
        const uint8_t flags = src[frame + 1];
        if (flags == 7) break;
        const unsigned predictor = header >> 4;
        const unsigned shift = header & 15u;
        if (predictor >= 5 || shift > 12) { error = "Community14 ADPCM frame header is invalid"; return false; }
        for (size_t j = 2; j < 16 && pcm.size() < decoded_size; ++j) {
            const uint8_t packed = src[frame + j];
            for (unsigned half = 0; half < 2 && pcm.size() < decoded_size; ++half) {
                int nibble = half ? (packed >> 4) : (packed & 15);
                if (nibble & 8) nibble -= 16;
                const int base = nibble * (1 << (12 - shift));
                const double sample = double(base) + previous1 * kCoef[predictor][0] + previous2 * kCoef[predictor][1];
                long rounded = std::lrint(sample);
                if (rounded < -32768) rounded = -32768;
                if (rounded > 32767) rounded = 32767;
                previous2 = previous1;
                previous1 = sample;
                const int16_t value = static_cast<int16_t>(rounded);
                pcm.push_back(static_cast<uint8_t>(value & 0xff));
                pcm.push_back(static_cast<uint8_t>((uint16_t(value) >> 8) & 0xff));
            }
        }
        if (pcm.size() >= decoded_size || flags == 1) break;
    }
    if (pcm.size() != decoded_size) { error = "Community14 ADPCM decoded length mismatch"; return false; }
    payload.swap(pcm);
    return true;
}

bool normalise(const std::vector<uint8_t>& input, const std::string& name,
               std::vector<uint8_t>& output, std::string& error, unsigned depth) {
    if (input.size() < 2 || input.size() > kBudget || depth > 8) {
        error = "invalid PAC size or nesting depth"; return false;
    }
    const uint16_t plain_count = le16(input.data());
    const uint16_t private_count = plain_count ^ 42802u;
    bool encoded = false;
    if ((plain_count & 0x8000u) && private_count && 2ull + private_count * 16ull <= input.size()) {
        for (size_t i = 0; i < private_count; ++i) {
            if (tag(be32(input.data() + 10 + i * 16) ^ 0xc569e1efu ^ uint32_t(i))) { encoded = true; break; }
        }
    }
    const size_t count = encoded ? private_count : plain_count;
    const size_t base = 2 + count * 16;
    if (base > input.size()) { error = "PAC directory exceeds input"; return false; }
    std::vector<uint8_t> out(base, 0);
    put16(out, 0, count);
    bool changed = encoded;
    for (size_t i = 0; i < count; ++i) {
        const uint8_t* row = input.data() + 2 + i * 16;
        const uint32_t offset = le32(row) ^ (encoded ? (996678763u ^ uint32_t(i)) : 0u);
        const uint32_t size = le32(row + 4) ^ (encoded ? (47633006u ^ uint32_t(i)) : 0u);
        if (uint64_t(base) + offset + size > input.size()) { error = "PAC entry outside input"; return false; }
        std::string type;
        if (encoded) {
            const char* decoded = tag(be32(row + 8) ^ 0xc569e1efu ^ uint32_t(i));
            type = decoded ? decoded : "unk";
        } else {
            size_t length = 0; while (length < 4 && row[8 + length]) ++length;
            type.assign(reinterpret_cast<const char*>(row + 8), length);
        }
        std::vector<uint8_t> payload(input.begin() + base + offset, input.begin() + base + offset + size);
        if (encoded && type == "rgba") {
            if (payload.size() > kBudget - 8) { error = "image bridge exceeds budget"; return false; }
            std::vector<uint8_t> marked(8 + payload.size());
            std::memcpy(marked.data(), "C14R", 4); put32(marked, 4, i);
            std::memcpy(marked.data() + 8, payload.data(), payload.size());
            payload.swap(marked); type = "png";
        } else if (type == "spr") {
            std::vector<uint8_t> nested;
            if (!normalise(payload, "", nested, error, depth + 1)) return false;
            if (nested != payload) changed = true;
            payload.swap(nested);
        } else if (encoded && depth == 0 && type == "wav") {
            if (!normaliseCommunityWav(payload, static_cast<uint32_t>(i), error)) return false;
        } else if (encoded && depth == 0 && type == "bin") {
            // Community14 protects the converted GameData directory inside
            // top-level BIN payloads as well as the outer PAC directory. Nested
            // SPR BIN entries use different schemas and must remain untouched.
            // The original Java GameData.Init(..., conversion=2, ...) expects
            // the ordinary table header, so restore only that verified metadata
            // and preserve all record payload bytes unchanged. Character
            // selection relies on these tables for ChrGameData[*] fields.
            if (!normaliseConvertedTable(payload, error)) return false;
        } else if (encoded && type == "dac" && (name == "gamedata.pac" || name == "text00.pac")) {
            if (!normaliseConvertedTable(payload, error)) return false;
        }
        if (payload.size() > kBudget - out.size()) { error = "normalised PAC exceeds budget"; return false; }
        put32(out, 2 + i * 16, out.size() - base);
        put32(out, 6 + i * 16, payload.size());
        if (encoded) std::memcpy(out.data() + 10 + i * 16, type.data(), type.size());
        else std::memcpy(out.data() + 10 + i * 16, row + 8, 4);
        put32(out, 14 + i * 16, le32(row + 12));
        out.insert(out.end(), payload.begin(), payload.end());
    }
    // Preserve original gaps, ordering, reserved metadata and bytes exactly
    // whenever neither this directory nor any nested directory was adapted.
    output = changed ? std::move(out) : input;
    return true;
}
}

bool normaliseEnginePac(const std::vector<uint8_t>& input, const std::string& logical_name,
                        std::vector<uint8_t>& output, std::string& error) {
    output.clear(); error.clear();
    std::vector<uint8_t> pending;
    if (!normalise(input, logical_name, pending, error, 0)) return false;
    output.swap(pending); return true;
}

bool readEngineResource(const GameVfs& vfs, const std::string& name,
                        std::vector<uint8_t>& output, std::string& path, std::string& error) {
    output.clear(); path.clear(); error.clear();
    if (!GameVfs::safeRelativePath(name)) { error = "unsafe engine resource name"; return false; }
    std::string logical = name;
    if (name.find('.') == std::string::npos) {
        if (name == "loading") logical += ".png";
        else if (name == "mk") logical += ".bin";
        else if (name.compare(0, 4, "bgm_") == 0 || name.compare(0, 3, "se_") == 0) logical += ".ogg";
        else logical += ".pac";
    }
    if (!vfs.resolve(logical, path)) { error = vfs.error(); return false; }
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) { error = "cannot open engine resource"; return false; }
    if (std::fseek(file, 0, SEEK_END) != 0) { std::fclose(file); error = "cannot seek resource"; return false; }
    const long size = std::ftell(file);
    if (size < 0 || uint64_t(size) > kBudget || std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file); error = "resource exceeds memory budget"; return false;
    }
    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    const bool ok = bytes.empty() || std::fread(bytes.data(), 1, bytes.size(), file) == bytes.size();
    std::fclose(file);
    if (!ok) { error = "resource read truncated"; return false; }
    if (logical.size() >= 4 && logical.compare(logical.size() - 4, 4, ".pac") == 0) {
        return normaliseEnginePac(bytes, logical, output, error);
    }
    output.swap(bytes); return true;
}
