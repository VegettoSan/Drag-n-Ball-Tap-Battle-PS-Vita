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

} // namespace

bool PacFile::open(const std::string& path) {
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
    if (end < 2) {
        error_ = "PAC is too small";
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

    const uint16_t count = readLe16(count_bytes);
    const uint64_t data_base64 = 2ull + static_cast<uint64_t>(count) * 16ull;
    if (data_base64 > file_size_ || data_base64 > 0xFFFFFFFFull) {
        error_ = "PAC table exceeds file size";
        std::fclose(fp);
        return false;
    }
    data_base_ = static_cast<uint32_t>(data_base64);

    entries_.reserve(count);
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

        const uint64_t start = data_base64 + static_cast<uint64_t>(entry.offset);
        const uint64_t finish = start + static_cast<uint64_t>(entry.size);
        if (finish < start || start > file_size_ || finish > file_size_) {
            error_ = "PAC entry points outside file";
            entries_.clear();
            std::fclose(fp);
            return false;
        }

        entries_.push_back(entry);
    }

    std::fclose(fp);
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

bool PacFile::readEntry(size_t index, std::vector<uint8_t>& out) const {
    out.clear();
    if (!open_ || index >= entries_.size()) {
        return false;
    }

    const PacEntry& entry = entries_[index];
    FILE* fp = std::fopen(path_.c_str(), "rb");
    if (!fp) {
        return false;
    }

    const uint64_t absolute = static_cast<uint64_t>(data_base_) + entry.offset;
    if (absolute > 0x7FFFFFFFull || std::fseek(fp, static_cast<long>(absolute), SEEK_SET) != 0) {
        std::fclose(fp);
        return false;
    }

    out.resize(entry.size);
    const bool ok = readExact(fp, out.data(), out.size());
    std::fclose(fp);
    if (!ok) {
        out.clear();
    }
    return ok;
}
