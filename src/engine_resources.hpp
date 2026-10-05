#pragma once
#include "vfs.hpp"
#include <cstdint>
#include <string>
#include <vector>

// In-memory bridge to the original Java byte-array GameData decoder. The
// installation stays unchanged; community headers become ordinary PAC headers.
// Community image entries carry C14R + LE entry index before their exact payload.
bool normaliseEnginePac(const std::vector<uint8_t>& input, const std::string& logical_name,
                        std::vector<uint8_t>& output, std::string& error);
bool readEngineResource(const GameVfs& vfs, const std::string& name,
                        std::vector<uint8_t>& output, std::string& path, std::string& error);
