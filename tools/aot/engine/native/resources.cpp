#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
#include "engine_resources.hpp"
#include "resource_cache.hpp"
#include "installed_data.hpp"
#include "pac.hpp"
#include "image.hpp"
#if defined(__vita__)
#include <vitaGL.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <list>

namespace {
constexpr size_t kSaveSize = 12906;
std::unique_ptr<GameVfs> vfs;
std::vector<uint8_t> pending;
EngineResourceCache resource_cache(8u * 1024u * 1024u);
std::shared_ptr<CachedEngineResource> pending_resource;
struct ResourceStream { std::shared_ptr<CachedEngineResource> resource; size_t largest_read=0; };
std::unordered_map<int32_t,ResourceStream> resource_streams;
int32_t next_stream_handle=1;
int pending_encoding=0;
int text_encodings[2]={0,0};
std::string save_path;
std::vector<uint8_t> save_cache;
bool save_cache_known = false;
bool save_cache_exists = false;
struct Size { int w, h; };
std::unordered_map<unsigned, Size> textures;
std::unordered_map<std::string, bool> resource_exists_cache;
struct CachedTexture {
    uint32_t hash;
    bool linear;
    std::vector<uint8_t> source;
    GLuint id;
    size_t cost;
    unsigned users;
};
std::list<CachedTexture> texture_cache;
size_t texture_cache_bytes = 0;
constexpr size_t kTextureCacheBudget = 4u * 1024u * 1024u;
uint32_t textureHash(const uint8_t* bytes, size_t size) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}
bool trimTextures(size_t incoming) {
    for (auto it = texture_cache.end(); texture_cache_bytes + incoming > kTextureCacheBudget && it != texture_cache.begin();) {
        --it;
        if (it->users) continue;
        glDeleteTextures(1, &it->id);
        textures.erase(it->id);
        texture_cache_bytes -= it->cost;
        it = texture_cache.erase(it);
    }
    return texture_cache_bytes + incoming <= kTextureCacheBudget;
}

bool readFile(const std::string& path, std::vector<uint8_t>& out) {
    out.clear(); struct stat info{};
    if (lstat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0 || uint64_t(info.st_size) > kSaveSize)
        return false;
    FILE* f = std::fopen(path.c_str(), "rb"); if (!f) return false;
    out.resize(info.st_size);
    bool ok = out.empty() || std::fread(out.data(), 1, out.size(), f) == out.size();
    std::fclose(f); if (!ok) out.clear(); return ok;
}
bool publishSave(const std::vector<uint8_t>& out) {
    const std::string temporary = save_path + ".tmp";
    const int fd = open(temporary.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) return false;
    size_t written = 0;
    while (written < out.size()) {
        const ssize_t n = write(fd, out.data() + written, out.size() - written);
        if (n <= 0) break;
        written += static_cast<size_t>(n);
    }
    const bool synced = fsync(fd) == 0;
    const bool closed = close(fd) == 0;
    bool ok = written == out.size() && synced && closed;
    if (ok) ok = rename(temporary.c_str(), save_path.c_str()) == 0;
    if (!ok) unlink(temporary.c_str());
    return ok;
}
int upload(const RgbaImage& image, bool linear) {
    GLint old = 0; glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    GLuint id = 0; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());
    const GLenum error = glGetError(); glBindTexture(GL_TEXTURE_2D, old);
    if (!id || error != GL_NO_ERROR) { if (id) glDeleteTextures(1, &id); return -1; }
    textures[id] = {int(image.width), int(image.height)};
    return int(id);
}
}

bool dbtb_initResources(const std::string& base, const std::string& mod) {
    vfs.reset(new GameVfs(base));
    if (!vfs->prepareDirectories() || (!mod.empty() && !vfs->selectMod(mod))) return false;

    // Save progress is intentionally global across every APK/data profile.
    // app0: is read-only, so the VPK-bundled save.bin is only a seed: on the
    // first boot copy it byte-for-byte to ux0:data/DBTapBattle/save.bin. Never
    // overwrite an existing mutable save when switching Original/mod profiles.
    save_path = base + "/save.bin";
    save_cache.clear();
    resource_exists_cache.clear();
    resource_streams.clear();
    resource_cache.clear();
    pending_resource.reset();
    save_cache_exists = readFile(save_path, save_cache);
    if (!save_cache_exists) {
        std::vector<uint8_t> seed;
        if (readFile("app0:/save.bin", seed) && seed.size() == kSaveSize) {
            if (publishSave(seed)) {
                save_cache.swap(seed);
                save_cache_exists = true;
                std::printf("Shared save seeded from app0:/save.bin: %s (%zu bytes)\n",
                            save_path.c_str(), save_cache.size());
            } else {
                std::fprintf(stderr, "Shared save seed publication failed: %s\n", save_path.c_str());
            }
        } else {
            std::fprintf(stderr, "Shared save seed missing or invalid: app0:/save.bin\n");
        }
    }
    save_cache_known = true;
    std::printf("Shared save: %s (%s, %zu bytes)\n", save_path.c_str(),
                save_cache_exists ? "cached" : "not present", save_cache.size());
    return true;
}
const GameVfs& dbtb_vfs() { if (!vfs) std::abort(); return *vfs; }
void dbtb_reclaimIdleResources() {
    // Cache ownership only: active streams retain shared PAC owners and live
    // texture references survive trimTextures. Called on the game thread.
    resource_cache.clear();
    trimTextures(kTextureCacheBudget);
}
bool dbtb_releaseTexture(unsigned id) {
    for (auto& cached : texture_cache) if (cached.id == id) {
        if (cached.users) --cached.users;
        return false; // immutable imported texture retained until idle eviction
    }
    textures.erase(id);
    return true;
}

extern "C" {
int32_t dbtb_resourceFiltered(void* name, int32_t filter) {
    DbtbTimedScope timer(dbtb_performance().resource_us);
    ++dbtb_performance().resources;
    std::string error; pending.clear(); pending_resource.reset(); pending_encoding=0;
    bool hit = false;
    const uint64_t start = dbtb_timeUs();
    if (!name || !resource_cache.read(dbtb_vfs(), static_cast<const char*>(name), filter, pending_resource, hit, error)) {
        std::fprintf(stderr, "Resource %s: %s\n", name ? static_cast<const char*>(name) : "(null)", error.c_str()); return -1;
    }
    const std::string& path = pending_resource->path;
    pending_encoding = pending_resource->encoding;
    dbtb_performance().resource_cache_hits += hit;
    if (!hit) dbtb_performance().resource_bytes += pending_resource->io_bytes;
    const std::string logical=path.substr(path.find_last_of('/')+1);
    if(logical=="gamedata.pac")text_encodings[0]=pending_encoding;
    if(logical=="text00.pac")text_encodings[1]=pending_encoding;
    if(logical=="gamedata.pac"||logical=="text00.pac")std::fprintf(stderr,"Text codec %s: %d\n",logical.c_str(),pending_encoding);
    // Vita's nano printf does not implement the z length modifier.
    std::fprintf(stderr, "Resource: %s filter=%d cache=%s io_bytes=%llu bridge_bytes=%llu us=%llu\n",
        logical.c_str(), filter, hit ? "hit" : "miss",
        static_cast<unsigned long long>(hit ? 0 : pending_resource->io_bytes),
        static_cast<unsigned long long>(pending_resource->bytes.size()),
        static_cast<unsigned long long>(dbtb_timeUs() - start));
    return int32_t(pending_resource->bytes.size());
}
int32_t dbtb_resource(void* name) { return dbtb_resourceFiltered(name, 0); }
int32_t dbtb_resourceEncoding() { return pending_encoding; }
int32_t dbtb_openResourceStream(void* name, int32_t filter) {
    if (resource_streams.size() >= 8) return -1;
    const int32_t size=dbtb_resourceFiltered(name, filter);
    if (size < 0) return -1;
    if (size > 32 * 1024 * 1024) { pending_resource.reset(); return -1; }
    const int32_t handle=next_stream_handle;
    next_stream_handle=handle==INT32_MAX ? 1 : handle+1;
    if (resource_streams.count(handle)) { pending_resource.reset(); return -1; }
    resource_streams.emplace(handle, ResourceStream{pending_resource,0});
    pending_resource.reset();
    return handle;
}
int32_t dbtb_resourceStreamSize(int32_t handle) {
    const auto stream=resource_streams.find(handle);
    return stream==resource_streams.end() ? -1 : int32_t(stream->second.resource->bytes.size());
}
int32_t dbtb_readResourceStream(int32_t handle, int32_t position, void* target, int32_t size) {
    auto stream=resource_streams.find(handle);
    if (stream==resource_streams.end() || position<0 || size<0 || (size && !target)) return -1;
    const auto& bytes=stream->second.resource->bytes;
    if (size_t(position)>bytes.size() || size_t(size)>bytes.size()-size_t(position)) return -1;
    if (size) std::memcpy(target,bytes.data()+position,size);
    stream->second.largest_read=std::max(stream->second.largest_read,size_t(size));
    return size;
}
void dbtb_closeResourceStream(int32_t handle) {
    const auto stream=resource_streams.find(handle);
    if (stream==resource_streams.end()) return;
    std::fprintf(stderr,"Resource stream closed: bytes=%llu largest_read=%llu active=%u\n",
        static_cast<unsigned long long>(stream->second.resource->bytes.size()),
        static_cast<unsigned long long>(stream->second.largest_read),
        unsigned(resource_streams.size()-1));
    resource_streams.erase(stream);
}
int32_t dbtb_installedData() {
    const InstalledDataAudit audit = auditInstalledData(dbtb_vfs());
    if (!audit.ready) {
        std::fprintf(stderr, "Offline data audit: %s\n", audit.error.c_str());
        return 0;
    }
    std::fprintf(stderr, "Offline character triplets: %d\n", audit.complete_characters);
    return 1;
}
int32_t dbtb_textEncoding(int32_t source) { return source>=0 && source<2?text_encodings[source]:-1; }
void dbtb_copyResource(void* data, int32_t size) {
    const auto& bytes = pending_resource ? pending_resource->bytes : pending;
    if (size < 0 || size_t(size) != bytes.size() || (size && !data)) std::abort();
    if (size) std::memcpy(data, bytes.data(), size);
    pending_resource.reset();
    std::vector<uint8_t>().swap(pending);
}
int32_t dbtb_exists(void* name) {
    if (!name) return 0;
    const std::string logical(static_cast<const char*>(name));
    if (!GameVfs::safeRelativePath(logical)) return 0;
    if (logical == "save.bin") {
        if (save_cache_known) return save_cache_exists ? 1 : 0;
        save_cache_exists = readFile(save_path, save_cache);
        save_cache_known = true;
        return save_cache_exists ? 1 : 0;
    }
    const auto cached = resource_exists_cache.find(logical);
    if (cached != resource_exists_cache.end()) return cached->second ? 1 : 0;
    std::string resolved;
    const bool found = dbtb_vfs().resolve(logical, resolved);
    resource_exists_cache.emplace(logical, found);
    return found ? 1 : 0;
}
int32_t dbtb_readSave(void* name) {
    pending.clear();
    pending_resource.reset();
    if (!name || std::strcmp(static_cast<const char*>(name), "save.bin")) return -1;
    if (!save_cache_known) {
        save_cache_exists = readFile(save_path, save_cache);
        save_cache_known = true;
    }
    if (!save_cache_exists) return -1;
    pending = save_cache;
    return int32_t(pending.size());
}
int32_t dbtb_writeSave(void* name, void* data, int32_t size, int32_t position, int32_t truncate) {
    if (!name || std::strcmp(static_cast<const char*>(name), "save.bin") || size < 0 || position < 0 ||
        uint64_t(position) + size > kSaveSize || (size && !data)) return 0;
    if (!save_cache_known) {
        save_cache_exists = readFile(save_path, save_cache);
        save_cache_known = true;
    }
    std::vector<uint8_t> out;
    if (!truncate && save_cache_exists) out = save_cache;
    if (truncate || size_t(position) + size > out.size()) out.resize(size_t(position) + size);
    if (size) std::memcpy(out.data() + position, data, size);
    const bool ok = publishSave(out);
    if (!ok) {
        std::fprintf(stderr, "Save publication failed: %s\n", save_path.c_str());
        return 0;
    }
    save_cache.swap(out);
    save_cache_exists = true;
    save_cache_known = true;
    return 1;
}
int32_t dbtb_deleteSave(void* name) {
    if (!name || std::strcmp(static_cast<const char*>(name), "save.bin")) return 0;
    if (unlink(save_path.c_str()) != 0) return 0;
    save_cache.clear();
    save_cache_exists = false;
    save_cache_known = true;
    return 1;
}
int32_t dbtb_loadTexture(void* data, int32_t size, int32_t linear) {
    DbtbTimedScope timer(dbtb_performance().texture_us);
    ++dbtb_performance().textures;
    if (!data || size < 0 || size > 16 * 1024 * 1024) return -1;
    const auto* b = static_cast<const uint8_t*>(data);
    const uint32_t hash = textureHash(b, size_t(size));
    for (auto it = texture_cache.begin(); it != texture_cache.end(); ++it) {
        if (it->hash != hash || it->linear != (linear != 0) || it->source.size() != size_t(size) ||
            std::memcmp(it->source.data(), b, size_t(size)) != 0) continue;
        ++it->users;
        const int id = int(it->id);
        texture_cache.splice(texture_cache.begin(), texture_cache, it);
        ++dbtb_performance().texture_cache_hits;
        return id;
    }
    RgbaImage image; std::string error; bool ok;
    PacEncoding image_encoding = PacEncoding::Auto;
    if (size >= 8 && !std::memcmp(b, "C14R", 4)) image_encoding = PacEncoding::Community14;
    else if (size >= 8 && !std::memcmp(b, "C14S", 4)) image_encoding = PacEncoding::Community14Spanish;
    else if (size >= 8 && !std::memcmp(b, "C14I", 4)) image_encoding = PacEncoding::Community14Invasion;
    if (image_encoding != PacEncoding::Auto) {
        const uint32_t index = b[4] | (uint32_t(b[5]) << 8) | (uint32_t(b[6]) << 16) | (uint32_t(b[7]) << 24);
        ok = decodeCommunityImageProfile(std::vector<uint8_t>(b + 8, b + size), index, image_encoding, image, error);
    } else {
        ok = decodePng(std::vector<uint8_t>(b, b + size), image, error);
        if (ok) {
            // Match Android Bitmap/GLUtils premultiplication, without touching
            // the original PNG bytes or applying it twice to community pixels.
            for (size_t i = 0; i < image.pixels.size(); i += 4)
                for (size_t c = 0; c < 3; ++c)
                    image.pixels[i+c] = (uint16_t(image.pixels[i+c]) * image.pixels[i+3] + 127) / 255;
            image.premultiplied_alpha = true;
        }
    }
    if (!ok) { std::fprintf(stderr, "Engine texture: %s\n", error.c_str()); return -1; }
    const size_t cost = size_t(size) + image.pixels.size();
    // Discard idle selection atlases before allocating a large combat texture.
    const bool retain = trimTextures(cost);
    const int id = upload(image, linear != 0);
    if (id > 0 && retain) {
        texture_cache.push_front({hash, linear != 0, std::vector<uint8_t>(b, b + size), GLuint(id), cost, 1});
        texture_cache_bytes += cost;
    }
    return id;
}
int32_t dbtb_textureWidth(int32_t id) { auto i = textures.find(id); return i == textures.end() ? 0 : i->second.w; }
int32_t dbtb_textureHeight(int32_t id) { auto i = textures.find(id); return i == textures.end() ? 0 : i->second.h; }
int32_t dbtb_emptyTexture(int32_t w, int32_t h) {
    if (w <= 0 || h <= 0 || w > 2048 || h > 2048) return -1;
    RgbaImage image; image.width = w; image.height = h; image.pixels.resize(size_t(w) * h * 4);
    return upload(image, true);
}
void dbtb_unsupported(void* message) { std::fprintf(stderr, "Unsupported service: %s\n", static_cast<const char*>(message)); }
}
