#pragma once

#include <string>
#include <vector>

struct BootChoice {
    bool original = true;
    std::string mod_directory;
};

bool runBootSelector(const std::vector<std::string>& mods, bool original_data_present, BootChoice& choice);
void showPacResult(bool success, const std::string& detail);
