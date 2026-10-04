#include "game_data.hpp"
#include <utility>

namespace {
uint16_t le16(const std::vector<uint8_t>& data, size_t pos) {
    return uint16_t(data[pos]) | (uint16_t(data[pos + 1]) << 8);
}
uint32_t le32(const std::vector<uint8_t>& data, size_t pos) {
    return uint32_t(data[pos]) | (uint32_t(data[pos + 1]) << 8) |
           (uint32_t(data[pos + 2]) << 16) | (uint32_t(data[pos + 3]) << 24);
}
}

void GameDataTable::clear() {
    data_.clear(); records_.clear(); error_.clear(); source_path_.clear();
    encoding_ = PacEncoding::Auto;
}

bool GameDataTable::decode(const std::vector<uint8_t>& payload, PacEncoding encoding) {
    clear();
    if (encoding != PacEncoding::Original && encoding != PacEncoding::Community14) {
        error_ = "game table needs the resolved PAC codec"; return false;
    }
    if (payload.size() < 2 || payload.size() > 16u * 1024u * 1024u) {
        error_ = "invalid game table size"; return false;
    }
    const bool encoded = encoding == PacEncoding::Community14;
    const size_t count = le16(payload, 0) ^ (encoded ? 34594u : 0u);
    const size_t header = 2 + count * 8;
    if (header > payload.size()) {
        error_ = "truncated game table directory"; return false;
    }
    std::vector<GameDataRecord> records;
    records.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const size_t p = 2 + i * 8;
        GameDataRecord record;
        record.position = le32(payload, p) ^ (encoded ? (uint32_t(-1887452470) ^ uint32_t(i)) : 0u);
        record.width = le16(payload, p + 4) ^ (encoded ? (23261u ^ uint32_t(i)) : 0u);
        record.height = le16(payload, p + 6) ^ (encoded ? (47592u ^ uint32_t(i)) : 0u);
        const uint64_t end = uint64_t(record.position) + uint64_t(record.width) * record.height;
        if (record.position < header || end > payload.size()) {
            error_ = "game table record out of payload bounds: " + std::to_string(i); return false;
        }
        records.push_back(record);
    }
    data_ = payload;
    records_ = std::move(records);
    encoding_ = encoding;
    return true;
}

bool GameDataTable::load(const GameVfs& vfs, const std::string& name) {
    clear();
    std::string path;
    if (!vfs.resolve(name, path)) { error_ = vfs.error(); return false; }
    PacFile pac;
    if (!pac.open(path)) { error_ = pac.error(); return false; }
    // Limit this entry point to a single converted table. Other PACs can carry
    // several unrelated/raw DAC commands and must use their own loader mode.
    size_t index = pac.entries().size();
    for (size_t i = 0; i < pac.entries().size(); ++i) {
        if (pac.typeString(i) != "dac") continue;
        if (index != pac.entries().size()) { error_ = "ambiguous converted DAC table"; return false; }
        index = i;
    }
    if (index == pac.entries().size()) { error_ = "converted DAC table absent"; return false; }
    std::vector<uint8_t> payload;
    if (!pac.readEntry(index, payload)) { error_ = pac.error(); return false; }
    if (!decode(payload, pac.encoding())) return false;
    source_path_ = path;
    return true;
}

bool GameDataTable::value(size_t record, size_t x, size_t y, uint8_t& out) const {
    out = 0;
    if (record >= records_.size()) return false;
    const auto& r = records_[record];
    if (x >= r.width || y >= r.height) return false;
    out = data_[r.position + size_t(r.width) * y + x];
    return true;
}

bool GameDatabase::load(const GameVfs& vfs) {
    *this = GameDatabase{};
    GameDatabase pending;
    if (!pending.game.load(vfs, "gamedata.pac")) {
        error = "gamedata.pac: " + pending.game.error(); return false;
    }
    if (!pending.text.load(vfs, "text00.pac")) {
        error = "text00.pac: " + pending.text.error(); return false;
    }
    *this = std::move(pending);
    return true;
}
