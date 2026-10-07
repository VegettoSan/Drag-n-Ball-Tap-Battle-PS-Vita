#include "installed_data.hpp"
#include "pac.hpp"
#include <cstdio>

namespace {
bool openPac(const GameVfs& vfs, const char* logical, bool& present, std::string& error) {
    std::string path;
    if (!vfs.resolve(logical, path)) {
        present = false;
        // Missing data is handled by the caller as a gap. Other VFS failures
        // (for example a non-regular override) are structural errors.
        const std::string& vfs_error = vfs.error();
        const bool missing_original = vfs_error.compare(0, 26, "missing original resource:") == 0;
        const bool missing_profile = vfs_error.compare(0, 34, "missing selected profile resource:") == 0;
        if (!missing_original && !missing_profile) {
            error = vfs_error;
            return false;
        }
        return true;
    }
    present = true;
    PacFile pac;
    if (!pac.open(path)) {
        error = std::string(logical) + ": " + pac.error();
        return false;
    }
    return true;
}
}

InstalledDataAudit auditInstalledData(const GameVfs& vfs, int min_characters,
                                      int max_characters) {
    InstalledDataAudit result;
    if (min_characters < 1 || max_characters < min_characters || max_characters > 100) {
        result.error = "invalid character audit bounds";
        return result;
    }

    bool gap = false;
    for (int i = 0; i < max_characters; ++i) {
        bool any = false, all = true;
        for (const char* format : {"char%02d.pac", "chardemo%02d.pac", "charf00%02d.pac"}) {
            char logical[48];
            std::snprintf(logical, sizeof(logical), format, i);
            bool present = false;
            if (!openPac(vfs, logical, present, result.error)) return result;
            any |= present;
            all &= present;
        }
        if (!any) {
            gap = true;
            continue;
        }
        if (gap) {
            result.error = "character data is not contiguous";
            return result;
        }
        if (!all) {
            result.error = "character triplet is incomplete";
            return result;
        }
        ++result.complete_characters;
    }

    if (result.complete_characters < min_characters) {
        result.error = "not enough complete character triplets";
        return result;
    }

    // Protected Android14-derived APKs (including Spanish and Invasion)
    // are standalone even though their bobj family begins at 01 and they do
    // not bundle bobj00.pac. Do not impose the original dataset's bobj00
    // inventory as a Vita installation requirement. If the preserved original
    // core later requests an omitted resource, that is a profile-adaptation
    // issue to diagnose; selected profiles never borrow it from game/.
    for (const char* logical : {"select0.pac", "effect.pac", "back00.pac"}) {
        bool present = false;
        if (!openPac(vfs, logical, present, result.error)) return result;
        if (!present) {
            result.error = std::string("missing shared combat resource: ") + logical;
            return result;
        }
    }

    result.ready = true;
    return result;
}

InstalledDataAudit scanInstalledData(const GameVfs& vfs, int min_characters,
                                     int max_characters) {
    InstalledDataAudit result;
    if (min_characters < 1 || max_characters < min_characters || max_characters > 100) {
        result.error = "invalid character scan bounds";
        return result;
    }

    auto present = [&](const std::string& logical, bool& exists) -> bool {
        std::string path;
        if (vfs.resolve(logical, path)) { exists = true; return true; }
        exists = false;
        const std::string& e = vfs.error();
        const bool missing_original = e.compare(0, 26, "missing original resource:") == 0;
        const bool missing_profile = e.compare(0, 34, "missing selected profile resource:") == 0;
        if (!missing_original && !missing_profile) { result.error = e; return false; }
        return true;
    };

    bool gap = false;
    for (int i = 0; i < max_characters; ++i) {
        bool any = false, all = true;
        for (const char* format : {"char%02d.pac", "chardemo%02d.pac", "charf00%02d.pac"}) {
            char logical[48];
            std::snprintf(logical, sizeof(logical), format, i);
            bool exists = false;
            if (!present(logical, exists)) return result;
            any |= exists;
            all &= exists;
        }
        if (!any) { gap = true; continue; }
        if (gap) { result.error = "character data is not contiguous"; return result; }
        if (!all) { result.error = "character triplet is incomplete"; return result; }
        ++result.complete_characters;
    }

    if (result.complete_characters < min_characters) {
        result.error = "not enough complete character triplets";
        return result;
    }

    for (const char* logical : {"select0.pac", "effect.pac", "back00.pac"}) {
        bool exists = false;
        if (!present(logical, exists)) return result;
        if (!exists) { result.error = std::string("missing shared combat resource: ") + logical; return result; }
    }

    result.ready = true;
    return result;
}
