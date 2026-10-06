#pragma once
#include "vfs.hpp"
#include <string>

struct InstalledDataAudit {
    bool ready = false;
    int complete_characters = 0;
    std::string error;
};

// Validate the offline dataset consumed by the original core. The baseline game
// needs at least 13 contiguous character triplets; audited mods may extend that
// sequence across the complete two-digit character namespace (00..99).
InstalledDataAudit auditInstalledData(const GameVfs& vfs, int min_characters = 13,
                                      int max_characters = 100);
