#pragma once

#include <string>
#include <vector>

class GameVfs {
public:
    static const char* kBasePath;
    static const char* kGamePath;
    static const char* kModsPath;

    bool prepareDirectories();
    std::vector<std::string> listMods() const;

    void selectOriginal();
    bool selectMod(const std::string& directory_name);

    bool usingMod() const { return !active_mod_.empty(); }
    const std::string& activeMod() const { return active_mod_; }

    bool resolve(const std::string& relative_path, std::string& resolved_path) const;
    bool originalDataPresent() const;

private:
    static bool safeRelativePath(const std::string& path);
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);

    std::string active_mod_;
};
