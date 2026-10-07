#pragma once
#include <string>
#include <vector>

class GameVfs {
public:
    static const char* kBasePath;
    explicit GameVfs(const std::string& base = kBasePath);
    bool prepareDirectories();
    std::vector<std::string> listProfiles() const;
    bool selectProfile(const std::string& directory_name);
    const std::string& activeProfile() const { return active_profile_; }
    const std::string& error() const { return error_; }
    bool resolve(const std::string& relative_path, std::string& resolved_path) const;
        static bool safeRelativePath(const std::string& path);
private:
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isRegularFile(const std::string& path);
    std::string base_, profiles_, active_profile_;
    mutable std::string error_;
};
