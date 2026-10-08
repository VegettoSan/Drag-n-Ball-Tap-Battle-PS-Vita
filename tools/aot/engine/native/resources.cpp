#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
#include "diagnostic_watchdog.hpp"
#include "engine_resources.hpp"
#include "resource_cache.hpp"
#include "installed_data.hpp"
#include "pac.hpp"
#include "dynamic_codec.hpp"
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
#ifdef __vita__
#include <malloc.h>
#endif

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
InstalledDataAudit installed_audit;
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

bool synchronizeInstalledCharacters(std::vector<uint8_t>& save, int characters) {
    if (save.size() != kSaveSize || characters < 1) return false;
    characters = std::min(characters, 100);
    bool changed = false;
    // The pinned original core stores each character in a 100-byte ConfigData
    // record beginning at offset 30. Hardware testing with the approved seed
    // showed 00..12 have these three bytes set and 13+ clear:
    // +1 GetCharctorBuy/SetCharVisible, +2 GetCharDL, +85 CharOpen.
    // A complete APK-derived profile already contains the character resources,
    // so expose every audited contiguous triplet as locally installed/unlocked.
    for (int i = 0; i < characters; ++i) {
        const size_t record = size_t(i) * 100u + 30u;
        for (size_t offset : {size_t(1), size_t(2), size_t(85)}) {
            const size_t pos = record + offset;
            if (pos >= save.size()) break;
            if (save[pos] != 1) {
                save[pos] = 1;
                changed = true;
            }
        }
    }
    return changed;
}
struct CompactTexturePixels {
    // The source pixels remain at full fidelity while being decompressed.
    // For Vita's very large PRIVATE mods, pack to a single RGBA4444 upload;
    // inputs >=384px use 2:1 downsampling (resource-specific, not engine logic).
    uint32_t source_w = 0, source_h = 0;
    uint32_t gpu_w = 0, gpu_h = 0, divisor = 1;
    uint32_t rows_seen = 0;
    std::vector<uint16_t> packed;
};

bool packCompactRows(void* context, const uint8_t* rgba, uint32_t y,
                     uint32_t rows, uint32_t width) {
    auto& dst = *static_cast<CompactTexturePixels*>(context);
    if (!rgba || width != dst.source_w || y != dst.rows_seen ||
        y > dst.source_h || rows > dst.source_h-y) return false;
    for (uint32_t sy = 0; sy < rows; ++sy) {
        const uint32_t source_y = y + sy;
        if (source_y % dst.divisor) continue;
        const uint32_t ty = source_y / dst.divisor;
        if (ty >= dst.gpu_h) return false;
        const uint8_t* line = rgba + size_t(sy) * width * 4;
        uint16_t* output = dst.packed.data() + size_t(ty) * dst.gpu_w;
        for (uint32_t tx = 0; tx < dst.gpu_w; ++tx) {
            const uint8_t* px = line + size_t(tx * dst.divisor) * 4;
            // VitaGL read_rgba4444: AAAABBBBGGGGRRRR (bit0=R), premultiplied
            // original color and transparency are retained (4-bit precision).
            output[tx] = uint16_t((px[0] >> 4) |
                                  (px[1] & 0xF0u) |
                                  ((px[2] & 0xF0u) << 4) |
                                  ((px[3] & 0xF0u) << 8));
        }
    }
    dst.rows_seen = y + rows;
    return true;
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

bool dbtb_initResources(const std::string& base, const std::string& profile) {
    vfs.reset(new GameVfs(base));
    if (!vfs->prepareDirectories() || profile.empty() || !vfs->selectProfile(profile)) return false;
    // Profile isolation: do not retain a previous mod's keys on reinitialization.
    installDynamicCommunityProfile(nullptr);
    const std::string codec_path=base + "/profiles/" + profile + "/dbtb_codec.json";
    struct stat codec_stat{};
    if (lstat(codec_path.c_str(), &codec_stat)==0) {
        if (!S_ISREG(codec_stat.st_mode)) {
            std::fprintf(stderr, "Protected profile codec is not a regular file: %s\n", codec_path.c_str());
            return false;
        }
        std::vector<uint8_t> codec_bytes;
        CommunityPacProfile decoded{};
        std::string codec_error;
        if (!readFile(codec_path, codec_bytes) ||
            !parseDynamicCodecJson(codec_bytes, decoded, codec_error)) {
            std::fprintf(stderr, "Protected profile codec invalid: %s (%s)\n",
                         codec_path.c_str(), codec_error.c_str());
            return false;
        }
        installDynamicCommunityProfile(&decoded);
        std::printf("Dynamic protected PAC codec activated for profile: %s\n", profile.c_str());
    }
 

    installed_audit = scanInstalledData(*vfs);
    if (installed_audit.ready)
        std::printf("Profile character triplets scanned: %d\n", installed_audit.complete_characters);
    else
        std::fprintf(stderr, "Profile character audit: %s\n", installed_audit.error.c_str());

    // Each selected APK/data profile owns an independent mutable save, but every
    // profile starts from the same exact VPK-bundled seed. app0: is read-only:
    // copy app0:/save.bin only when this profile has no save yet. Never overwrite
    // existing profile progress on launch, profile switches or VPK updates.
    save_path = base + "/profiles/" + profile + "/save.bin";
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
                std::printf("Profile save seeded from app0:/save.bin: %s (%zu bytes)\n",
                            save_path.c_str(), save_cache.size());
            } else {
                std::fprintf(stderr, "Profile save seed publication failed: %s\n", save_path.c_str());
            }
        } else {
            std::fprintf(stderr, "Profile save seed missing or invalid: app0:/save.bin\n");
        }
    }
    if (save_cache_exists && installed_audit.ready &&
        synchronizeInstalledCharacters(save_cache, installed_audit.complete_characters)) {
        if (publishSave(save_cache))
            std::printf("Profile save character flags synchronized: %d\n", installed_audit.complete_characters);
        else
            std::fprintf(stderr, "Profile save character synchronization failed: %s\n", save_path.c_str());
    }
    save_cache_known = true;
    std::printf("Profile save: %s (%s, %zu bytes)\n", save_path.c_str(),
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
    const auto& backing=stream->second.resource->path;
    const size_t slash=backing.find_last_of('/');
    const std::string logical=backing.substr(slash==std::string::npos ? 0 : slash+1);
    const bool combat_object=logical.size()>=10 && logical.compare(0,4,"bobj")==0 &&
                             logical.compare(logical.size()-4,4,".pac")==0;
    std::fprintf(stderr,"Resource stream closed: bytes=%llu largest_read=%llu active=%u\n",
        static_cast<unsigned long long>(stream->second.resource->bytes.size()),
        static_cast<unsigned long long>(stream->second.largest_read),
        unsigned(resource_streams.size()-1));
    resource_streams.erase(stream);
    if (combat_object) {
        // Combat object PACs are loaded after fighters/effects/cards. Reclaim
        // only idle cache ownership before the AOT engine allocates combat state.
        // Open streams and active textures remain owned by their consumers.
        const size_t cached_before=resource_cache.used();
        dbtb_reclaimIdleResources();
#ifdef __vita__
        const struct mallinfo heap=mallinfo();
        std::fprintf(stderr,
            "Combat resource boundary: %s idle_cache_released=%llu newlib_used=%d newlib_free=%d\n",
            logical.c_str(),static_cast<unsigned long long>(cached_before),
            heap.uordblks,heap.fordblks);
#else
        std::fprintf(stderr,"Combat resource boundary: %s idle_cache_released=%llu\n",
            logical.c_str(),static_cast<unsigned long long>(cached_before));
#endif
    }
}
int32_t dbtb_installedData() {
    if (!installed_audit.ready) {
        std::fprintf(stderr, "Offline data audit: %s\n", installed_audit.error.c_str());
        return 0;
    }
    std::fprintf(stderr, "Offline character triplets: %d\n", installed_audit.complete_characters);
    return 1;
}
int32_t dbtb_installedCharacters() {
    return installed_audit.ready ? installed_audit.complete_characters : 0;
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
    dbtb_setDiagnosticStage(2,uint32_t(size),uint32_t(dbtb_performance().textures));
    const uint64_t decode_start=dbtb_timeUs();
    RgbaImage image; std::string error; bool ok;
    PacEncoding image_encoding = PacEncoding::Auto;
    if (size >= 8 && !std::memcmp(b, "C14R", 4)) image_encoding = PacEncoding::Community14;
    else if (size >= 8 && !std::memcmp(b, "C14S", 4)) image_encoding = PacEncoding::Community14Spanish;
    else if (size >= 8 && !std::memcmp(b, "C14I", 4)) image_encoding = PacEncoding::Community14Invasion;
    else if (size >= 8 && !std::memcmp(b, "C14D", 4)) image_encoding = PacEncoding::Community14Dbfz;
    else if (size >= 8 && !std::memcmp(b, "C14U", 4)) image_encoding = PacEncoding::Community14Dynamic;
    if (image_encoding == PacEncoding::Community14Dynamic) {
        // Hardware crash psp2core-1791436599: stack glTexSubImage2D ->
        // _malloc_r during original GameData.Init after other PACs load.
        // Single GL upload, compact GPU format, *no sub-image writes*.
        const uint32_t index = b[4] | (uint32_t(b[5]) << 8) |
                              (uint32_t(b[6]) << 16) | (uint32_t(b[7]) << 24);
        uint32_t width=0, height=0;
        if (!communityImageDimensions(b + 8, size_t(size) - 8, index,
                                      image_encoding, width, height, error)) {
            dbtb_setDiagnosticStage(1);
            std::fprintf(stderr,"[TextureCompact] invalid source: %s\n",error.c_str());
            return -1;
        }
        CompactTexturePixels compact;
        compact.source_w=width; compact.source_h=height;
        compact.divisor=(width >= 384 || height >= 384) ? 2u : 1u;
        compact.gpu_w=(width+compact.divisor-1)/compact.divisor;
        compact.gpu_h=(height+compact.divisor-1)/compact.divisor;
        const size_t rgba4_bytes=size_t(compact.gpu_w)*compact.gpu_h*2;
        // Strictly cap output to protect Newlib and GPU memory.
        if (!rgba4_bytes || rgba4_bytes>8u*1024u*1024u) {
            dbtb_setDiagnosticStage(1);
            std::fprintf(stderr,"[TextureCompact] unsupported dimensions: %ux%u\n",width,height);
            return -1;
        }
        compact.packed.resize(size_t(compact.gpu_w)*compact.gpu_h);
        if (!decodeCommunityImageProfileRows(b + 8,size_t(size)-8,index,
                image_encoding,&compact,&packCompactRows,error) ||
            compact.rows_seen != height) {
            dbtb_setDiagnosticStage(1);
            std::fprintf(stderr,"[TextureCompact] decode rejected: %s\n",error.c_str());
            return -1;
        }
        const uint64_t decode_end=dbtb_timeUs();
        dbtb_performance().texture_decode_us += decode_end-decode_start;
        const size_t cost=size_t(size)+rgba4_bytes;
        const bool retain=trimTextures(cost);
        GLint old=0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&old);
        GLuint id=0;
        glGenTextures(1,&id);
        if (!id) { dbtb_setDiagnosticStage(1); return -1; }
        glBindTexture(GL_TEXTURE_2D,id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        dbtb_setDiagnosticStage(3,uint32_t(rgba4_bytes),index);
        const uint64_t upload_start=dbtb_timeUs();
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,compact.gpu_w,compact.gpu_h,
                     0,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,compact.packed.data());
        const GLenum gl_error=glGetError();
        const uint64_t upload_us=dbtb_timeUs()-upload_start;
        dbtb_performance().texture_upload_us+=upload_us;
        glBindTexture(GL_TEXTURE_2D,old);
        dbtb_setDiagnosticStage(1);
        if (gl_error!=GL_NO_ERROR) {
            glDeleteTextures(1,&id);
            std::fprintf(stderr,"[TextureCompact] GL error 0x%x %ux%u\n",
                         unsigned(gl_error),compact.gpu_w,compact.gpu_h);
            return -1;
        }
        // Keep the *logical* dimensions used by the original Java engine,
        // even though the physical GPU image is smaller. UV math is unchanged.
        textures[id]={int(width),int(height)};
        if (retain) {
            texture_cache.push_front({hash,linear!=0,
                std::vector<uint8_t>(b,b+size),id,cost,1});
            texture_cache_bytes+=cost;
        }
        if (dbtb_performance().textures<=5 || upload_us>=200000) {
            std::fprintf(stderr,
                "[TextureCompact] logical=%ux%u gpu=%ux%u rgba4444_KiB=%u "
                "decode_ms=%llu GPU_ms=%llu\n",
                width,height,compact.gpu_w,compact.gpu_h,unsigned(rgba4_bytes/1024),
                static_cast<unsigned long long>((decode_end-decode_start)/1000),
                static_cast<unsigned long long>(upload_us/1000));
        }
        return int32_t(id);
    }
    if (image_encoding != PacEncoding::Auto) {
        const uint32_t index = b[4] | (uint32_t(b[5]) << 8) | (uint32_t(b[6]) << 16) | (uint32_t(b[7]) << 24);
        ok = decodeCommunityImageProfile(b + 8, size_t(size) - 8, index, image_encoding, image, error);
    } else {
        ok = decodePng(b, size_t(size), image, error);
        if (ok) {
            // Match Android Bitmap/GLUtils premultiplication, without touching
            // the original PNG bytes or applying it twice to community pixels.
            for (size_t i = 0; i < image.pixels.size(); i += 4)
                for (size_t c = 0; c < 3; ++c)
                    image.pixels[i+c] = (uint16_t(image.pixels[i+c]) * image.pixels[i+3] + 127) / 255;
            image.premultiplied_alpha = true;
        }
    }
    dbtb_performance().texture_decode_us += dbtb_timeUs() - decode_start;
    if (!ok) {
        dbtb_setDiagnosticStage(1);
        std::fprintf(stderr, "Engine texture: %s\n", error.c_str()); return -1;
    }
    const size_t cost = size_t(size) + image.pixels.size();
    // Discard idle selection atlases before allocating a large combat texture.
    const bool retain = trimTextures(cost);
    dbtb_setDiagnosticStage(3,uint32_t(image.pixels.size()),uint32_t(dbtb_performance().textures));
    const uint64_t upload_start=dbtb_timeUs();
    const int id = upload(image, linear != 0);
    const uint64_t upload_us=dbtb_timeUs()-upload_start;
    dbtb_performance().texture_upload_us += upload_us;
    if (upload_us >= 750000)
        std::fprintf(stderr,"[TextureSlow] upload_us=%llu pixels=%llu dimensions=%ux%u success=%d\n",
            static_cast<unsigned long long>(upload_us),static_cast<unsigned long long>(image.pixels.size()),
            unsigned(image.width),unsigned(image.height),id>0 ? 1 : 0);
    dbtb_setDiagnosticStage(1);
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
