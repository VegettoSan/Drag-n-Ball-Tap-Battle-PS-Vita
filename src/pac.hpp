#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct PacEntry {
    uint32_t offset = 0;
    uint32_t size = 0;
    char type[4] = {0, 0, 0, 0};
    uint32_t reserved = 0;
};

class PacFile {
public:
    bool open(const std::string& path);
    void close();

    bool isOpen() const { return open_; }
    const std::string& path() const { return path_; }
    const std::string& error() const { return error_; }
    uint32_t dataBase() const { return data_base_; }
    const std::vector<PacEntry>& entries() const { return entries_; }

    std::string typeString(size_t index) const;
    // Budget is an explicit per-read limit, not an inferred original-format rule.
    bool readEntry(size_t index, std::vector<uint8_t>& out, size_t max_bytes = 16 * 1024 * 1024);

private:
    std::string path_;
    std::string error_;
    std::vector<PacEntry> entries_;
    uint32_t data_base_ = 0;
    uint64_t file_size_ = 0;
    bool open_ = false;
};
