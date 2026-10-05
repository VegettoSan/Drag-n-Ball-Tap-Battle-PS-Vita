#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
#if defined(__vita__)
#include <vitaGL.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
std::array<std::vector<uint8_t>, 3> client_data;
std::array<int, 3> client_stride{}, client_size{}, client_type{};
std::array<bool, 3> client_enabled{};
[[noreturn]] void bad(const char* reason) {
    std::fprintf(stderr, "GL bridge: %s\n", reason); std::abort();
}
int elementSize(int type) {
    if (type == GL_FLOAT) return 4;
    if (type == GL_SHORT) return 2;
    if (type == GL_UNSIGNED_BYTE) return 1;
    if (type == GL_UNSIGNED_SHORT) return 2;
    bad("unsupported client element type");
}
}

// Instance import receivers are deliberately unused: these methods bind the
// current native GL context, exactly like the original GLES interface.
extern "C" {
void dbtb_glBindTexture(void*, int32_t t, int32_t id) { glBindTexture(t, id); }
void dbtb_glBlendFunc(void*, int32_t s, int32_t d) { glBlendFunc(s, d); }
void dbtb_glClear(void*, int32_t mask) { glClear(mask); }
void dbtb_glClearColor(void*, float r, float g, float b, float a) { glClearColor(r, g, b, a); }
void dbtb_glColor4f(void*, float r, float g, float b, float a) { glColor4f(r, g, b, a); }
void dbtb_glDisable(void*, int32_t cap) { glDisable(cap); }
void dbtb_glDisableClientState(void*, int32_t cap) {
    if (cap == GL_COLOR_ARRAY) client_enabled[0] = false;
    else if (cap == GL_TEXTURE_COORD_ARRAY) client_enabled[1] = false;
    else if (cap == GL_VERTEX_ARRAY) client_enabled[2] = false;
    glDisableClientState(cap);
}
void dbtb_glEnable(void*, int32_t cap) { glEnable(cap); }
void dbtb_glEnableClientState(void*, int32_t cap) {
    if (cap == GL_COLOR_ARRAY) client_enabled[0] = true;
    else if (cap == GL_TEXTURE_COORD_ARRAY) client_enabled[1] = true;
    else if (cap == GL_VERTEX_ARRAY) client_enabled[2] = true;
    glEnableClientState(cap);
}
void dbtb_glHint(void*, int32_t t, int32_t mode) { glHint(t, mode); }
void dbtb_glLoadIdentity(void*) { glLoadIdentity(); }
void dbtb_glMatrixMode(void*, int32_t mode) { glMatrixMode(mode); }
void dbtb_glOrthof(void*, float l, float r, float b, float t, float n, float f) {
#if defined(__vita__)
    glOrthof(l, r, b, t, n, f);
#else
    glOrtho(l, r, b, t, n, f);
#endif
}
void dbtb_glPopMatrix(void*) { glPopMatrix(); }
void dbtb_glPushMatrix(void*) { glPushMatrix(); }
void dbtb_glScalef(void*, float x, float y, float z) { glScalef(x, y, z); }
void dbtb_glShadeModel(void*, int32_t mode) { glShadeModel(mode); }
void dbtb_glTexEnvf(void*, int32_t t, int32_t n, float v) { glTexEnvf(t, n, v); }
void dbtb_glTexParameterf(void*, int32_t t, int32_t n, float v) { glTexParameterf(t, n, v); }
void dbtb_glTranslatef(void*, float x, float y, float z) { glTranslatef(x, y, z); }
void dbtb_glViewport(void*, int32_t x, int32_t y, int32_t w, int32_t h) { glViewport(x, y, w, h); }
void dbtb_glBindFramebuffer(void*, int32_t t, int32_t id) { glBindFramebuffer(t, id); }
int32_t dbtb_glCheckFramebufferStatus(void*, int32_t t) { return glCheckFramebufferStatus(t); }
void dbtb_glFramebufferTexture2D(void*, int32_t t, int32_t a, int32_t tt, int32_t id, int32_t l) {
    glFramebufferTexture2D(t, a, tt, id, l);
}
void dbtb_glArray(int32_t operation, int32_t n, void* data) {
    if (n < 0 || n > 4096 || (n && !data)) bad("invalid object array");
    static_assert(sizeof(GLuint) == sizeof(int32_t), "GL name width must match Java int");
    auto* ids = static_cast<GLuint*>(data);
    switch (operation) {
        case 0:
            for (int i = 0; i < n; ++i) {
                if (!dbtb_releaseTexture(ids[i])) continue;
                const GLuint id = GLuint(ids[i]); glDeleteTextures(1, &id);
            }
            break;
        case 1: glGenTextures(n, ids); break;
        case 2: glDeleteFramebuffers(n, ids); break;
        case 3: glDeleteRenderbuffers(n, ids); break;
        case 4: glGenFramebuffers(n, ids); break;
        case 5: glGenRenderbuffers(n, ids); break;
        default: bad("unknown object operation");
    }
}
void dbtb_glPointer(int32_t kind, int32_t size, int32_t type, int32_t stride, void* data, int32_t bytes) {
    if (kind < 0 || kind > 2 || size < 2 || size > 4 || stride < 0 || bytes < 0 || bytes > 16384 || (bytes && !data))
        bad("invalid client pointer");
    const int packed = size * elementSize(type);
    if (stride && stride < packed) bad("client stride is smaller than a vertex");
    auto& buffer = client_data[kind];
    const int normalized_stride = stride ? stride : packed;
    const bool unchanged = client_stride[kind] == normalized_stride &&
                           client_size[kind] == packed && client_type[kind] == type &&
                           buffer.size() == size_t(bytes) &&
                           (!bytes || std::memcmp(buffer.data(), data, size_t(bytes)) == 0);
    if (unchanged) return;
    buffer.resize(bytes);
    if (bytes) std::memcpy(buffer.data(), data, size_t(bytes));
    dbtb_performance().client_bytes += bytes;
    client_stride[kind] = normalized_stride;
    client_size[kind] = packed;
    client_type[kind] = type;
    if (kind == 0) glColorPointer(size, type, stride, buffer.data());
    else if (kind == 1) glTexCoordPointer(size, type, stride, buffer.data());
    else glVertexPointer(size, type, stride, buffer.data());
}
void dbtb_glDraw(int32_t mode, int32_t count, int32_t type, void* data, int32_t bytes) {
    if (count < 0 || count > 4096 || bytes < 0 || (bytes && !data)) bad("invalid draw index array");
    const int width = elementSize(type);
    if ((type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) || int64_t(count) * width > bytes)
        bad("draw indices exceed buffer");
    unsigned highest = 0;
    const auto* indices = static_cast<const uint8_t*>(data);
    for (int i = 0; i < count; ++i) {
        unsigned index = indices[i * width];
        if (width == 2) { uint16_t value; std::memcpy(&value, indices + i * width, 2); index = value; }
        if (index > highest) highest = index;
    }
    for (int i = 0; i < 3; ++i) {
        if (!count || !client_enabled[i]) continue;
        if (!client_stride[i] || uint64_t(highest) * client_stride[i] + client_size[i] > client_data[i].size())
            bad("draw index exceeds active client attribute");
    }
    glDrawElements(mode, count, type, data);
    ++dbtb_performance().draws;
}
}
