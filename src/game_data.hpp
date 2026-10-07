#pragma once
#include "pac.hpp"
#include "vfs.hpp"
#include <cstdint>
#include <string>
#include <vector>

// GameData.binCnv selected by InitGameData(cnvType=3). This is NOT raw
// animation DAC: its positions address bytes in the entire retained payload.
struct GameDataRecord {
    uint32_t position = 0;
    uint32_t width = 0, height = 0;
};

class GameDataTable {
public:
    bool decode(const std::vector<uint8_t>& payload, PacEncoding encoding);
    bool load(const GameVfs& vfs, const std::string& name);
    bool value(size_t record, size_t x, size_t y, uint8_t& out) const;
    const std::vector<GameDataRecord>& records() const { return records_; }
    const std::string& error() const { return error_; }
    const std::string& sourcePath() const { return source_path_; }
    PacEncoding encoding() const { return encoding_; }
private:
    void clear();
    std::vector<uint8_t> data_;
    std::vector<GameDataRecord> records_;
    std::string error_, source_path_;
    PacEncoding encoding_ = PacEncoding::Auto;
};

// The two data sets loaded by the original InitGameData; each PAC selects
// its own codec after resolution inside the active standalone profile.
struct GameDatabase {
    GameDataTable game, text;
    std::string error;
    bool load(const GameVfs& vfs);
};
