#include "vfs.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#ifdef __vita__
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

const char* GameVfs::kBasePath = "ux0:data/DBTapBattle";

GameVfs::GameVfs(const std::string& base) : base_(base), game_(base + "/game"), mods_(base + "/mods") {}

bool GameVfs::prepareDirectories() {
    error_.clear();
#ifdef __vita__
    sceIoMkdir("ux0:data", 0777);
#endif
    for (const auto& path : {base_, game_, mods_, base_ + "/config", base_ + "/logs", base_ + "/saves"}) {
#ifdef __vita__
        sceIoMkdir(path.c_str(), 0777);
#else
        mkdir(path.c_str(), 0777);
#endif
        if (!isDirectory(path)) {
            error_ = "could not create directory: " + path;
            return false;
        }
    }
    return true;
}

std::vector<std::string> GameVfs::listMods() const {
    std::vector<std::string> mods;
    error_.clear();
#ifdef __vita__
    const SceUID dfd = sceIoDopen(mods_.c_str());
    if (dfd < 0) { error_ = "could not scan mods directory"; return mods; }
    for (;;) {
        SceIoDirent entry{};
        const int result = sceIoDread(dfd, &entry);
        if (result <= 0) {
            if (result < 0) error_ = "mods directory read failed";
            break;
        }
        const std::string name(entry.d_name);
#else
    DIR* dir = opendir(mods_.c_str());
    if (!dir) { error_ = "could not scan mods directory"; return mods; }
    for (;;) {
        errno = 0;
        dirent* entry = readdir(dir);
        if (!entry) {
            if (errno) error_ = "mods directory read failed";
            break;
        }
        const std::string name(entry->d_name);
#endif
        if (safeRelativePath(name) && name.find('/') == std::string::npos && isDirectory(mods_ + "/" + name))
            mods.push_back(name);
    }
#ifdef __vita__
    sceIoDclose(dfd);
#else
    closedir(dir);
#endif
    std::sort(mods.begin(), mods.end());
    return mods;
}

void GameVfs::selectOriginal() { active_mod_.clear(); error_.clear(); }

bool GameVfs::selectMod(const std::string& name) {
    error_.clear();
    if (!safeRelativePath(name) || name.find('/') != std::string::npos || !isDirectory(mods_ + "/" + name)) {
        error_ = "invalid or missing mod directory";
        return false;
    }
    active_mod_ = name;
    return true;
}

bool GameVfs::resolve(const std::string& relative, std::string& resolved) const {
    resolved.clear();
    error_.clear();
    if (!safeRelativePath(relative)) { error_ = "unsafe resource path"; return false; }
    if (!active_mod_.empty()) {
        const std::string candidate = mods_ + "/" + active_mod_ + "/" + relative;
        if (isRegularFile(candidate)) { resolved = candidate; return true; }
        // An existing but invalid override is an error, not a silent fallback.
        if (exists(candidate)) { error_ = "mod resource is not a regular file: " + candidate; return false; }
    }
    const std::string candidate = game_ + "/" + relative;
    if (isRegularFile(candidate)) { resolved = candidate; return true; }
    error_ = "missing original resource: " + relative;
    return false;
}

bool GameVfs::originalDataPresent() const {
    // Bootstrap indicator only. Does not prove a complete playable installation.
    return isRegularFile(game_ + "/common.pac");
}

bool GameVfs::safeRelativePath(const std::string& path) {
    if (path.empty() || path.size() > 900) return false;
    for (unsigned char c : path) if (c < 32 || c == 127 || c == ':' || c == '\\') return false;
    size_t start = 0;
    while (start <= path.size()) {
        const size_t slash = path.find('/', start);
        const std::string part = path.substr(start, slash == std::string::npos ? slash : slash - start);
        if (part.empty() || part == "." || part == ".." || part.size() > 255) return false;
        if (slash == std::string::npos) return true;
        start = slash + 1;
    }
    return false;
}

// Host tests reject symlink components. Vita's ux0 filesystem has no symlinks.
#ifndef __vita__
static bool noSymlinks(const std::string& path) {
    for (size_t end = 1; end <= path.size(); ++end) {
        if (end != path.size() && path[end] != '/') continue;
        struct stat s{};
        if (lstat(path.substr(0, end).c_str(), &s) != 0) return false;
        if (S_ISLNK(s.st_mode)) return false;
    }
    return true;
}
#endif

bool GameVfs::exists(const std::string& path) {
#ifdef __vita__
    SceIoStat s{};
    return sceIoGetstat(path.c_str(), &s) >= 0;
#else
    struct stat s{};
    return lstat(path.c_str(), &s) == 0;
#endif
}

bool GameVfs::isDirectory(const std::string& path) {
#ifdef __vita__
    SceIoStat s{};
    return sceIoGetstat(path.c_str(), &s) >= 0 && (s.st_mode & SCE_S_IFMT) == SCE_S_IFDIR;
#else
    struct stat s{};
    return noSymlinks(path) && lstat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
#endif
}

bool GameVfs::isRegularFile(const std::string& path) {
#ifdef __vita__
    SceIoStat s{};
    return sceIoGetstat(path.c_str(), &s) >= 0 && (s.st_mode & SCE_S_IFMT) == SCE_S_IFREG;
#else
    struct stat s{};
    return noSymlinks(path) && lstat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
#endif
}
