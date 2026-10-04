#include "vfs.hpp"

#include <algorithm>
#include <cstring>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>

const char* GameVfs::kBasePath = "ux0:data/DBTapBattle";
const char* GameVfs::kGamePath = "ux0:data/DBTapBattle/game";
const char* GameVfs::kModsPath = "ux0:data/DBTapBattle/mods";

bool GameVfs::prepareDirectories() {
    sceIoMkdir("ux0:data", 0777);
    sceIoMkdir(kBasePath, 0777);
    sceIoMkdir(kGamePath, 0777);
    sceIoMkdir(kModsPath, 0777);
    return isDirectory(kBasePath) && isDirectory(kGamePath) && isDirectory(kModsPath);
}

std::vector<std::string> GameVfs::listMods() const {
    std::vector<std::string> mods;

    SceUID dfd = sceIoDopen(kModsPath);
    if (dfd < 0) {
        return mods;
    }

    for (;;) {
        SceIoDirent entry;
        std::memset(&entry, 0, sizeof(entry));
        const int result = sceIoDread(dfd, &entry);
        if (result <= 0) {
            break;
        }

        if (entry.d_name[0] == '\0' || std::strcmp(entry.d_name, ".") == 0 || std::strcmp(entry.d_name, "..") == 0) {
            continue;
        }

        if ((entry.d_stat.st_mode & SCE_S_IFMT) != SCE_S_IFDIR) {
            continue;
        }

        std::string name(entry.d_name);
        if (name.find("..") != std::string::npos || name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
            continue;
        }

        mods.push_back(name);
    }

    sceIoDclose(dfd);
    std::sort(mods.begin(), mods.end());
    return mods;
}

void GameVfs::selectOriginal() {
    active_mod_.clear();
}

bool GameVfs::selectMod(const std::string& directory_name) {
    if (directory_name.empty() || !safeRelativePath(directory_name) || directory_name.find('/') != std::string::npos) {
        return false;
    }

    const std::string path = std::string(kModsPath) + "/" + directory_name;
    if (!isDirectory(path)) {
        return false;
    }

    active_mod_ = directory_name;
    return true;
}

bool GameVfs::resolve(const std::string& relative_path, std::string& resolved_path) const {
    resolved_path.clear();
    if (!safeRelativePath(relative_path)) {
        return false;
    }

    if (!active_mod_.empty()) {
        const std::string mod_path = std::string(kModsPath) + "/" + active_mod_ + "/" + relative_path;
        if (exists(mod_path) && !isDirectory(mod_path)) {
            resolved_path = mod_path;
            return true;
        }
    }

    const std::string original_path = std::string(kGamePath) + "/" + relative_path;
    if (exists(original_path) && !isDirectory(original_path)) {
        resolved_path = original_path;
        return true;
    }

    return false;
}

bool GameVfs::originalDataPresent() const {
    std::string path;
    return resolve("common.pac", path) || exists(std::string(kGamePath) + "/common.pac");
}

bool GameVfs::safeRelativePath(const std::string& path) {
    if (path.empty() || path[0] == '/' || path.find(':') != std::string::npos) {
        return false;
    }
    if (path == ".." || path.find("../") != std::string::npos || path.find("/..") != std::string::npos || path.find("\\") != std::string::npos) {
        return false;
    }
    return true;
}

bool GameVfs::exists(const std::string& path) {
    SceIoStat stat;
    std::memset(&stat, 0, sizeof(stat));
    return sceIoGetstat(path.c_str(), &stat) >= 0;
}

bool GameVfs::isDirectory(const std::string& path) {
    SceIoStat stat;
    std::memset(&stat, 0, sizeof(stat));
    if (sceIoGetstat(path.c_str(), &stat) < 0) {
        return false;
    }
    return (stat.st_mode & SCE_S_IFMT) == SCE_S_IFDIR;
}
