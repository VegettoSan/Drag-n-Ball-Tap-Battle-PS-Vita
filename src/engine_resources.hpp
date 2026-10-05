#pragma once
#include "vfs.hpp"
#include <cstdint>
#include <string>
#include <vector>

// In-memory bridge to the original Java byte-array GameData decoder. The
// installation stays unchanged; community headers become ordinary PAC headers.
// Community image entries carry C14R + LE entry index before their exact payload.
bool normaliseEnginePac(const std::vector<uint8_t>& input, const std::string& logical_name,
                        std::vector<uint8_t>& output, std::string& error,
                        int* container_encoding = nullptr, int game_data_filter = 0);
bool readEngineResource(const GameVfs& vfs, const std::string& name,
                        std::vector<uint8_t>& output, std::string& path, std::string& error,
                        int* container_encoding = nullptr, int game_data_filter = 0,
                        size_t* bytes_read = nullptr);

// Returns 1 for UTF-8 and 0 for Shift_JIS. The PAC supplied here must already
// be in the ordinary/normalised form returned by readEngineResource(). The
// fallback preserves the known container profile when no strong text signal is
// present. This is intentionally content-based because some community-derived
// APKs use ordinary PAC headers while keeping UTF-8 GameData string payloads.
int detectEngineTextEncoding(const std::vector<uint8_t>& normalised_pac, int fallback_encoding);
