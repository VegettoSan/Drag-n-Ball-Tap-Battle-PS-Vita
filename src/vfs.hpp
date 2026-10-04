#pragma once
#include <string>
#include <vector>

class GameVfs {
public:
    static const char* kBasePath;
    explicit GameVfs(const std::string& base = kBasePath);
    bool prepareDirectories();
    std::vector<std::string> listMods() const;
    void selectOriginal();
    bool selectMod(const std::string& directory_name);
    bool usingMod() const { return !active_mod_.empty(); }
    const std::string& activeMod() const { return active_mod_; }
    const std::string& error() const { return error_; }
    bool resolve(const std::string& relative_path, std::string& resolved_path) const;
    bool originalDataPresent() const;
    static bool safeRelativePath(const std::string& path);
private:
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isRegularFile(const std::string& path);
    std::string base_, game_, mods_, active_mod_;
    mutable std::string error_;
};
