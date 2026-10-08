#pragma once
#include "engine_resources.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <sys/stat.h>

struct CachedEngineResource {
    std::vector<uint8_t> bytes;
    std::string path;
    int encoding = 0;
    size_t io_bytes = 0;
};

// Main-thread, profile-scoped LRU. Physical paths isolate independent datasets;
// file size/time changes invalidate a hit. Saves never enter this cache.
//
// Character-selection BIN-only PACs (original Game3 filter 251) are tiny even
// when their backing charNN.pac is multi-MiB. A small, separately bounded LRU
// survives the character's large filtered/voice import so revisiting fighters
// does not repeat the ~125 ms source read shown by Vita v1.0 logs. The two LRU
// budgets add up to the *existing* total budget, never more. Full large PAC
// loads discard both caches before allocation (00.30/00.32 battle memory guard).
class EngineResourceCache {
    struct Entry {
        std::string path;
        int filter;
        int64_t size, modified, changed;
        std::shared_ptr<CachedEngineResource> resource;
    };
    static size_t metadataBudget(size_t total) {
        return std::min(total / 16u, size_t(256u * 1024u));
    }
    static bool characterPac(const std::string& logical) {
        return logical.size() == 10 &&
            logical.compare(0, 4, "char") == 0 &&
            logical[4] >= '0' && logical[4] <= '9' &&
            logical[5] >= '0' && logical[5] <= '9' &&
            logical.compare(6, 4, ".pac") == 0;
    }
    static bool isCharacterMetadata(const std::string& logical, int filter) {
        return filter == 251 && characterPac(logical);
    }

    size_t budget_, used_ = 0;
    std::list<Entry> entries_;
    size_t metadata_budget_, metadata_used_ = 0;
    std::list<Entry> metadata_;

    void clearNormal() { entries_.clear(); used_ = 0; }
    void clearMetadata() { metadata_.clear(); metadata_used_ = 0; }

    static bool find(std::list<Entry>& entries, size_t& used, const std::string& path,
                     int filter, const struct stat& info,
                     std::shared_ptr<CachedEngineResource>& result, bool& hit) {
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            if (it->path != path || it->filter != filter) continue;
            if (it->size != info.st_size || it->modified != info.st_mtime ||
                it->changed != info.st_ctime) {
                used -= it->resource->bytes.capacity();
                entries.erase(it);
                return false;
            }
            result = it->resource;
            hit = true;
            entries.splice(entries.begin(), entries, it);
            return true;
        }
        return false;
    }

public:
    explicit EngineResourceCache(size_t budget)
        : budget_(budget - metadataBudget(budget)),
          metadata_budget_(metadataBudget(budget)) {}
    void clear() { clearNormal(); clearMetadata(); }
    size_t used() const { return used_ + metadata_used_; }

    bool read(const GameVfs& vfs, const std::string& name, int filter,
              std::shared_ptr<CachedEngineResource>& result, bool& hit, std::string& error) {
        result.reset(); hit = false;
        std::string logical = name;
        if (name.find('.') == std::string::npos) {
            if (name == "loading") logical += ".png";
            else if (name == "mk") logical += ".bin";
            else if (name.compare(0, 4, "bgm_") == 0 || name.compare(0, 3, "se_") == 0) logical += ".ogg";
            else logical += ".pac";
        }
        std::string path;
        if (!vfs.resolve(logical, path)) { error = vfs.error(); return false; }
        struct stat info{};
        if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode)) {
            error = "resource metadata unavailable"; return false;
        }
        const bool selection_metadata = isCharacterMetadata(logical, filter);
        if (selection_metadata &&
            find(metadata_, metadata_used_, path, filter, info, result, hit)) return true;
        if (find(entries_, used_, path, filter, info, result, hit)) return true;

        // Large full character PACs can expand several MiB while the engine is
        // entering a fight. Drop cached owners *before* materializing them.
        // Preserve only bounded, BIN-only selection metadata during filtered
        // character loads (187 / 251); drop it for large unfiltered combat PACs.
        constexpr uint64_t kLargeResourceThreshold = 2u * 1024u * 1024u;
        const bool large_source = uint64_t(info.st_size) > kLargeResourceThreshold;
        if (large_source) {
            clearNormal();
            if (!characterPac(logical) || (filter != 187 && filter != 251))
                clearMetadata();
        }

        auto resource = std::make_shared<CachedEngineResource>();
        int container_encoding = 0;
        if (!readEngineResource(vfs, name, resource->bytes, resource->path, error,
                                &container_encoding, filter, &resource->io_bytes)) return false;
        resource->encoding = detectEngineTextEncoding(resource->bytes, container_encoding);
        const size_t cost = resource->bytes.capacity();

        // Metadata is strictly 24 KiB per entry and 256 KiB total (or less on
        // small test budgets). Larger mods fall back to the original policy.
        if (selection_metadata && cost <= 24u * 1024u && cost <= metadata_budget_) {
            while (metadata_used_ + cost > metadata_budget_) {
                metadata_used_ -= metadata_.back().resource->bytes.capacity();
                metadata_.pop_back();
            }
            metadata_.push_front({path, filter, info.st_size, info.st_mtime, info.st_ctime, resource});
            metadata_used_ += cost;
        } else if (!large_source && cost <= budget_) {
            while (used_ + cost > budget_) {
                used_ -= entries_.back().resource->bytes.capacity();
                entries_.pop_back();
            }
            entries_.push_front({path, filter, info.st_size, info.st_mtime, info.st_ctime, resource});
            used_ += cost;
        }
        result = std::move(resource);
        return true;
    }
};
