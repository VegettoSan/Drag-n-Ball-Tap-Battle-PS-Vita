#pragma once
#include "engine_resources.hpp"
#include <list>
#include <memory>
#include <sys/stat.h>

struct CachedEngineResource {
    std::vector<uint8_t> bytes;
    std::string path;
    int encoding = 0;
    size_t io_bytes = 0;
};

// Main-thread, profile-scoped LRU. Physical paths isolate overrides/fallback;
// file size/time changes invalidate a hit. Saves never enter this cache.
class EngineResourceCache {
    struct Entry {
        std::string path;
        int filter;
        int64_t size, modified, changed;
        std::shared_ptr<CachedEngineResource> resource;
    };
    size_t budget_, used_ = 0;
    std::list<Entry> entries_;
public:
    explicit EngineResourceCache(size_t budget) : budget_(budget) {}
    void clear() { entries_.clear(); used_ = 0; }
    size_t used() const { return used_; }
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
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->path != path || it->filter != filter) continue;
            if (it->size != info.st_size || it->modified != info.st_mtime || it->changed != info.st_ctime) {
                used_ -= it->resource->bytes.capacity(); entries_.erase(it); break;
            }
            result = it->resource; hit = true;
            entries_.splice(entries_.begin(), entries_, it);
            return true;
        }
        auto resource = std::make_shared<CachedEngineResource>();
        int container_encoding = 0;
        if (!readEngineResource(vfs, name, resource->bytes, resource->path, error,
                                &container_encoding, filter, &resource->io_bytes)) return false;
        resource->encoding = detectEngineTextEncoding(resource->bytes, container_encoding);
        const size_t cost = resource->bytes.capacity();
        if (cost <= budget_) {
            while (used_ + cost > budget_) {
                used_ -= entries_.back().resource->bytes.capacity(); entries_.pop_back();
            }
            entries_.push_front({path, filter, info.st_size, info.st_mtime, info.st_ctime, resource});
            used_ += cost;
        }
        result = std::move(resource);
        return true;
    }
};
