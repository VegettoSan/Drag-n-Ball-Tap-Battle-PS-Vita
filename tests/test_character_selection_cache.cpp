// Regression for the Vita 1.0 character-selection ~125 ms PAC metadata reload.
// Uses a large *synthetic* original-format PAC; no commercial data required.
// Build:
// g++ -std=c++14 -O2 -Isrc tests/test_character_selection_cache.cpp \
//     src/engine_resources.cpp src/pac.cpp src/game_data.cpp src/vfs.cpp \
//     -o /tmp/test-character-selection-cache
// /tmp/test-character-selection-cache

#include "resource_cache.hpp"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

int main() {
    char root[] = "/tmp/dbtb-character-cache-XXXXXX";
    assert(mkdtemp(root));
    GameVfs vfs(root);
    assert(vfs.prepareDirectories());
    const std::string profile = std::string(root) + "/profiles/Synthetic";
    assert(mkdir(profile.c_str(), 0700) == 0);
    assert(vfs.selectProfile("Synthetic"));
    const std::string filename = profile + "/char00.pac";

    // Two PAC entries: an 8-byte BIN kept by 187/251, and an artificial
    // >2 MiB PNG-like payload excluded by both filters. Only the source size
    // matters here; the native resource bridge does not decode this PNG.
    constexpr size_t png_size = 3u * 1024u * 1024u;
    std::vector<uint8_t> pac(34u + 8u + png_size, 0);
    pac[0] = 2; // Little-endian entry count
    const auto put32 = [&](size_t p, uint32_t n) {
        for (int i = 0; i < 4; ++i) pac[p + i] = uint8_t(n >> (8 * i));
    };
    put32(2, 0); put32(6, 8);
    std::memcpy(pac.data() + 10, "bin", 3);
    put32(18, 8); put32(22, static_cast<uint32_t>(png_size));
    std::memcpy(pac.data() + 26, "png", 3);
    std::memcpy(pac.data() + 34, "FIRSTBIN", 8);
    const auto writePac = [&]() {
        std::ofstream out(filename, std::ios::binary | std::ios::trunc);
        assert(out.good());
        out.write(reinterpret_cast<const char*>(pac.data()), pac.size());
        out.close();
        assert(out.good());
    };
    writePac();

    EngineResourceCache cache(8u * 1024u * 1024u);
    std::shared_ptr<CachedEngineResource> first, second, filtered_voices, full;
    bool hit = false;
    std::string error;
    assert(cache.read(vfs, "char00", 251, first, hit, error) && !hit);
    assert(first->bytes.size() == 42u);
    assert(cache.used() <= 8u * 1024u * 1024u);

    // Importing the same large source through filter 187 used to flush the
    // *entire* resource cache, causing a second 251 read during selection.
    assert(cache.read(vfs, "char00", 187, filtered_voices, hit, error) && !hit);
    assert(cache.read(vfs, "char00.pac", 251, second, hit, error) && hit);
    assert(second == first && second->bytes == first->bytes);
    assert(cache.used() <= 8u * 1024u * 1024u);

    // Replacing a mod's PAC must never return a stale metadata cache entry.
    std::memcpy(pac.data() + 34, "NEW__BIN", 8);
    writePac();
    struct stat info{};
    assert(stat(filename.c_str(), &info) == 0);
    struct utimbuf timestamp{};
    timestamp.actime = info.st_atime;
    timestamp.modtime = info.st_mtime + 120;
    assert(utime(filename.c_str(), &timestamp) == 0);
    assert(cache.read(vfs, "char00", 251, second, hit, error) && !hit);
    assert(second != first && second->bytes != first->bytes);

    // Full large combat import must still drop metadata BEFORE allocating.
    assert(cache.read(vfs, "char00", 0, full, hit, error) && !hit);
    assert(full->bytes.size() == pac.size());
    assert(cache.used() == 0u);
    assert(cache.read(vfs, "char00", 251, first, hit, error) && !hit);
    cache.clear();
    assert(cache.used() == 0u);

    assert(unlink(filename.c_str()) == 0);
    assert(rmdir(profile.c_str()) == 0);
    assert(rmdir((std::string(root) + "/profiles").c_str()) == 0);
    assert(rmdir((std::string(root) + "/logs").c_str()) == 0);
    assert(rmdir((std::string(root) + "/config").c_str()) == 0);
    assert(rmdir(root) == 0);
    std::puts("CHARACTER SELECTION CACHE PASS: bounded metadata reuse, replacement invalidation, full-combat eviction");
}
