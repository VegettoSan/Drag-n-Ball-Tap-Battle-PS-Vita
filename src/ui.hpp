#pragma once

#include "image.hpp"
#include <string>
#include <vector>

struct BootChoice {
    std::string profile_directory;
};

bool runBootSelector(const std::vector<std::string>& profiles, BootChoice& choice);
void showPacResult(bool success, const std::string& detail, const RgbaImage* image = nullptr);
