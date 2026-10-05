#include "dbtb_bridge.h"
#include "services.hpp"
#include "engine_resources.hpp"
#include "pac.hpp"
#include "image.hpp"
#if defined(__vita__)
#include <vitaGL.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

namespace {
constexpr size_t kSaveSize = 12906;
std::unique_ptr<GameVfs> vfs;
std::vector<uint8_t> pending;
int pending_encoding=0;
std::string save_path;
struct Size { int w, h; };
std::unordered_map<unsigned, Size> textures;

bool directory(const std::string& path) {
    if (mkdir(path.c_str(), 0777) != 0 && errno != EEXIST) return false;
    struct stat info{};
    return lstat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
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
    save_path = base + "/saves/" + (mod.empty() ? std::string("original") : mod);
    if (!directory(save_path)) return false;
    save_path += "/save.bin";
    return true;
}
const GameVfs& dbtb_vfs() { if (!vfs) std::abort(); return *vfs; }
void dbtb_forgetTexture(unsigned id) { textures.erase(id); }

extern "C" {
int32_t dbtb_resource(void* name) {
    std::string path, error; pending.clear(); pending_encoding=0;
    if (!name || !readEngineResource(dbtb_vfs(), static_cast<const char*>(name), pending, path, error)) {
        std::fprintf(stderr, "Resource %s: %s\n", name ? static_cast<const char*>(name) : "(null)", error.c_str()); return -1;
    }
    PacFile source;
    if (source.open(path) && source.encoding()==PacEncoding::Community14) pending_encoding=1;
    std::printf("Resource: %s (%zu bridge bytes)\n", path.c_str(), pending.size());
    return int32_t(pending.size());
}
int32_t dbtb_resourceEncoding() { return pending_encoding; }
void dbtb_copyResource(void* data, int32_t size) {
    if (size < 0 || size_t(size) != pending.size() || (size && !data)) std::abort();
    if (size) std::memcpy(data, pending.data(), size);
    std::vector<uint8_t>().swap(pending);
}
int32_t dbtb_readSave(void* name) {
    pending.clear();
    if (!name || std::strcmp(static_cast<const char*>(name), "save.bin") || !readFile(save_path, pending)) return -1;
    return int32_t(pending.size());
}
int32_t dbtb_writeSave(void* name, void* data, int32_t size, int32_t position, int32_t truncate) {
    if (!name || std::strcmp(static_cast<const char*>(name), "save.bin") || size < 0 || position < 0 ||
        uint64_t(position) + size > kSaveSize || (size && !data)) return 0;
    std::vector<uint8_t> out;
    if (!truncate && !readFile(save_path, out)) {
        struct stat info{};
        if (lstat(save_path.c_str(), &info) == 0 || errno != ENOENT) return 0;
    }
    if (truncate || size_t(position) + size > out.size()) out.resize(size_t(position) + size);
    if (size) std::memcpy(out.data() + position, data, size);
    const bool ok = publishSave(out);
    if (!ok) std::fprintf(stderr, "Save publication failed: %s\n", save_path.c_str());
    return ok;
}
int32_t dbtb_deleteSave(void* name) {
    return name && !std::strcmp(static_cast<const char*>(name), "save.bin") && unlink(save_path.c_str()) == 0;
}
int32_t dbtb_loadTexture(void* data, int32_t size, int32_t linear) {
    if (!data || size < 0 || size > 16 * 1024 * 1024) return -1;
    const auto* b = static_cast<const uint8_t*>(data);
    RgbaImage image; std::string error; bool ok;
    if (size >= 8 && !std::memcmp(b, "C14R", 4)) {
        const uint32_t index = b[4] | (uint32_t(b[5]) << 8) | (uint32_t(b[6]) << 16) | (uint32_t(b[7]) << 24);
        ok = decodeCommunityImage(std::vector<uint8_t>(b + 8, b + size), index, image, error);
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
    return upload(image, linear != 0);
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
