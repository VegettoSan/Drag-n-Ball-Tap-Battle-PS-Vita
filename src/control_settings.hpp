#pragma once
#include <cstdio>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "vfs.hpp"

// Retain the v1 file values: 1 was visible Vita, 2 is hidden Vita. Both select
// the sole Vita row now; confirming migrates the old visible preference to 2.
inline int controlSelectionFromPreference(int mode) { return mode == 0 ? 1 : 0; }
inline int controlModeFromSelection(int row) { return row == 0 ? 2 : 0; }

inline std::string controlPreferencePath(const std::string& profile) {
    return std::string(GameVfs::kBasePath) + "/profiles/" + profile + "/vita-controls.cfg";
}
inline int readControlPreference(const std::string& profile) {
    const std::string path = controlPreferencePath(profile);
    struct stat st{};
    if (lstat(path.c_str(), &st) || !S_ISREG(st.st_mode) || st.st_size != 8) return 0;
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) return 0;
    char bytes[8]; const bool ok = std::fread(bytes, 1, 8, file) == 8;
    std::fclose(file);
    return ok && std::string(bytes, 6) == "DBTC1:" && bytes[6] >= '0' &&
        bytes[6] <= '2' && bytes[7] == '\n' ? bytes[6] - '0' : 0;
}
inline bool writeControlPreference(const std::string& profile, int mode) {
    if (mode < 0 || mode > 2) return false;
    const std::string path = controlPreferencePath(profile), temporary = path + ".tmp";
    const int fd = open(temporary.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) return false;
    char bytes[] = "DBTC1:0\n"; bytes[6] += mode;
    const bool written = write(fd, bytes, 8) == 8;
    const bool synced = fsync(fd) == 0, closed = close(fd) == 0;
    const bool ok = written && synced && closed && rename(temporary.c_str(), path.c_str()) == 0;
    if (!ok) unlink(temporary.c_str());
    return ok;
}
