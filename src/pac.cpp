#include "pac.hpp"

#include <cstdio>
#include <cstring>

namespace {

bool readExact(FILE* fp, void* dst, size_t size) {
    return size == 0 || std::fread(dst, 1, size, fp) == size;
}

uint16_t readLe16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) |
           static_cast<uint16_t>(p[1] << 8);
}

uint32_t readLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

uint32_t readBe32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | p[3];
}

} // namespace

bool PacFile::open(const std::string& path, PacEncoding encoding) {
    close();
    path_ = path;

    FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        error_ = "could not open PAC";
        return false;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        error_ = "could not seek to PAC end";
        std::fclose(fp);
        return false;
    }

    const long end = std::ftell(fp);
    if (end < 2 || static_cast<uint64_t>(end) > 0x7FFFFFFFull) {
        error_ = "PAC size is outside supported 32-bit seek range";
        std::fclose(fp);
        return false;
    }
    file_size_ = static_cast<uint64_t>(end);
    std::rewind(fp);

    uint8_t count_bytes[2];
    if (!readExact(fp, count_bytes, sizeof(count_bytes))) {
        error_ = "could not read PAC entry count";
        std::fclose(fp);
        return false;
    }

    const uint16_t raw_count = readLe16(count_bytes);
    const CommunityPacProfile* profile = nullptr;
    encoding_ = PacEncoding::Original;

    if (encoding == PacEncoding::Auto) {
        // Read only the directory-sized prefix needed by the detector. The
        // detector validates every decoded extent against the full file size,
        // so a wrong/private codec cannot silently fall back to another one.
        size_t profile_count = 0;
        const CommunityPacProfile* profiles = communityProfiles(profile_count);
        size_t largest_table = 2;
        for (size_t p = 0; p < profile_count; ++p) {
            const uint16_t count = raw_count ^ profiles[p].count_xor;
            const uint64_t table = 2ull + uint64_t(count) * 16ull;
            if (count && table <= file_size_ && table > largest_table) largest_table = size_t(table);
        }
        std::vector<uint8_t> header(largest_table);
        std::rewind(fp);
        if (!readExact(fp, header.data(), header.size())) {
            error_ = "could not read PAC table for profile detection";
            std::fclose(fp);
            return false;
        }
        encoding_ = detectCommunityEncoding(header.data(), file_size_);
        profile = communityProfile(encoding_);
        if (std::fseek(fp, 2, SEEK_SET) != 0) {
            error_ = "could not seek to PAC table";
            std::fclose(fp);
            return false;
        }
    } else if (encoding == PacEncoding::Original) {
        encoding_ = PacEncoding::Original;
    } else {
        profile = communityProfile(encoding);
        if (!profile) {
            error_ = "unknown protected PAC profile";
            std::fclose(fp);
            return false;
        }
        encoding_ = encoding;
    }

    const uint16_t count = profile ? (raw_count ^ profile->count_xor) : raw_count;
    const uint64_t data_base64 = 2ull + static_cast<uint64_t>(count) * 16ull;
    if (data_base64 > file_size_ || data_base64 > 0xFFFFFFFFull) {
        error_ = "PAC table exceeds file size";
        std::fclose(fp);
        return false;
    }
    data_base_ = static_cast<uint32_t>(data_base64);

    std::vector<PacEntry> parsed;
    parsed.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        uint8_t raw[16];
        if (!readExact(fp, raw, sizeof(raw))) {
            error_ = "PAC table is truncated";
            entries_.clear();
            std::fclose(fp);
            return false;
        }

        PacEntry entry;
        entry.offset = readLe32(raw + 0);
        entry.size = readLe32(raw + 4);
        std::memcpy(entry.type, raw + 8, 4);
        entry.reserved = readLe32(raw + 12);
        if (profile) {
            entry.offset ^= profile->offset_xor ^ i;
            entry.size ^= profile->size_xor ^ i;
            entry.encoded_type = readBe32(raw + 8) ^ i;
            const char* type = communityType(*profile, entry.encoded_type);
            std::memset(entry.type, 0, 4);
            std::memcpy(entry.type, type ? type : "unk", type ? std::strlen(type) : 3);
            if (encoding_ == PacEncoding::Community14Dbfz && !std::memcmp(raw + 8, "plt", 4))
                std::memcpy(entry.type, "plt", 4);
        }

        const uint64_t start = data_base64 + static_cast<uint64_t>(entry.offset);
        const uint64_t finish = start + static_cast<uint64_t>(entry.size);
        if (finish < start || start > file_size_ || finish > file_size_) {
            error_ = "PAC entry points outside file";
            entries_.clear();
            std::fclose(fp);
            return false;
        }

        parsed.push_back(entry);
    }

    std::fclose(fp);
    entries_.swap(parsed);
    error_.clear();
    open_ = true;
    return true;
}

void PacFile::close() {
    path_.clear();
    error_.clear();
    entries_.clear();
    data_base_ = 0;
    file_size_ = 0;
    open_ = false;
    encoding_ = PacEncoding::Original;
}

std::string PacFile::typeString(size_t index) const {
    if (index >= entries_.size()) {
        return std::string();
    }

    const PacEntry& entry = entries_[index];
    size_t length = 0;
    while (length < 4 && entry.type[length] != '\0') {
        ++length;
    }
    return std::string(entry.type, entry.type + length);
}

bool PacFile::readEntry(size_t index, std::vector<uint8_t>& out, size_t max_bytes) {
    out.clear();
    error_.clear();
    if (!open_ || index >= entries_.size()) {
        error_ = "PAC is closed or entry index is invalid";
        return false;
    }

    const PacEntry& entry = entries_[index];
    if (entry.size > max_bytes) {
        error_ = "PAC entry exceeds caller memory budget";
        return false;
    }
    FILE* fp = std::fopen(path_.c_str(), "rb");
    if (!fp) {
        error_ = "could not reopen PAC entry";
        return false;
    }

    // Mods can be replaced after selection. Do not allocate from a stale table
    // when the backing file has changed length or become truncated.
    if (std::fseek(fp, 0, SEEK_END) != 0 || std::ftell(fp) != static_cast<long>(file_size_)) {
        error_ = "PAC backing file size changed since open";
        std::fclose(fp);
        return false;
    }
    const uint64_t absolute = static_cast<uint64_t>(data_base_) + entry.offset;
    if (absolute > 0x7FFFFFFFull || std::fseek(fp, static_cast<long>(absolute), SEEK_SET) != 0) {
        std::fclose(fp);
        error_ = "could not seek to PAC entry";
        return false;
    }

    out.resize(entry.size);
    const bool ok = readExact(fp, out.data(), out.size());
    std::fclose(fp);
    if (!ok) {
        out.clear();
        error_ = "PAC entry read was truncated";
    }
    return ok;
}
