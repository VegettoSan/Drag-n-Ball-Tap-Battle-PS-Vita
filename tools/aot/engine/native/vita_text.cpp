#include "dbtb_bridge.h"
#include "services.hpp"

#include <psp2/pvf.h>
#include <vitaGL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace {
struct TextSurface {
    int width = 0;
    int height = 0;
    int ypos = 0;
    GLuint texture = 0;
    bool dirty = true;
    std::vector<uint8_t> rgba;
};

ScePvfLibId font_lib = nullptr;
ScePvfFontId font_id = nullptr;
std::unordered_map<int, std::unique_ptr<TextSurface>> surfaces;
int next_surface = 1;

void* pvfAlloc(void*, unsigned int size) {
    return memalign(8, (size + 7u) & ~7u);
}
void* pvfRealloc(void*, void* old_ptr, unsigned int size) {
    return std::realloc(old_ptr, size);
}
void pvfFree(void*, void* ptr) { std::free(ptr); }

ScePvfFontId openSystemFont(ScePvfLibId lib, ScePvfLanguageCode language) {
    ScePvfFontStyleInfo style{};
    style.languageCode = language;
    style.familyCode = SCE_PVF_DEFAULT_FAMILY_CODE;
    style.style = SCE_PVF_DEFAULT_STYLE_CODE;
    ScePvfError error = 0;
    const ScePvfFontIndex index = scePvfFindOptimumFont(lib, &style, &error);
    if (error != 0 || index < 0) return nullptr;
    ScePvfFontId font = scePvfOpen(lib, index, 0, &error);
    return error == 0 ? font : nullptr;
}

TextSurface* surface(int id) {
    auto it = surfaces.find(id);
    return it == surfaces.end() ? nullptr : it->second.get();
}

bool setSize(int size) {
    if (!font_id || size <= 0) return false;
    return scePvfSetCharSize(font_id, static_cast<float>(size), static_cast<float>(size)) == 0;
}

bool metrics(uint16_t c, ScePvfCharInfo& info, ScePvfIrect& image) {
    return font_id && scePvfGetCharInfo(font_id, c, &info) == 0 &&
           scePvfGetCharImageRect(font_id, c, &image) == 0;
}

void sourceOver(uint8_t* dst, int r, int g, int b, int a, uint8_t coverage) {
    const int sa = (a * int(coverage) + 127) / 255;
    if (sa <= 0) return;
    const int sr = (r * sa + 127) / 255;
    const int sg = (g * sa + 127) / 255;
    const int sb = (b * sa + 127) / 255;
    const int inv = 255 - sa;
    dst[0] = static_cast<uint8_t>(std::min(255, sr + (dst[0] * inv + 127) / 255));
    dst[1] = static_cast<uint8_t>(std::min(255, sg + (dst[1] * inv + 127) / 255));
    dst[2] = static_cast<uint8_t>(std::min(255, sb + (dst[2] * inv + 127) / 255));
    dst[3] = static_cast<uint8_t>(std::min(255, sa + (dst[3] * inv + 127) / 255));
}

int glyphAdvance(const ScePvfCharInfo& info) {
    const int advance = info.glyphMetrics.horizontalAdvance64 >> 6;
    return std::max(1, advance);
}

bool drawGlyph(TextSurface& target, uint16_t c, int pen_x, int baseline,
               int r, int g, int b, int a, int& advance) {
    ScePvfCharInfo info{};
    ScePvfIrect rect{};
    if (!metrics(c, info, rect)) return false;
    advance = glyphAdvance(info);
    const int bearing_x = info.glyphMetrics.horizontalBearingX64 >> 6;
    const int bearing_y = info.glyphMetrics.horizontalBearingY64 >> 6;
    const int margin = 2;
    const int gw = std::max<int>(1, rect.width + margin * 2);
    const int gh = std::max<int>(1, rect.height + margin * 2);
    if (gw > 1024 || gh > 1024) return false;

    std::vector<uint8_t> mask(size_t(gw) * gh, 0);
    ScePvfUserImageBufferRec image{};
    image.pixelFormat = SCE_PVF_USERIMAGE_DIRECT8;
    image.xPos64 = (margin << 6) - info.glyphMetrics.horizontalBearingX64;
    image.yPos64 = (margin << 6) + info.glyphMetrics.horizontalBearingY64;
    image.rect.width = static_cast<uint16_t>(gw);
    image.rect.height = static_cast<uint16_t>(gh);
    image.bytesPerLine = static_cast<uint16_t>(gw);
    image.buffer = mask.data();
    if (scePvfGetCharGlyphImage(font_id, c, &image) != 0) return false;

    const int origin_x = pen_x + bearing_x - margin;
    const int origin_y = baseline - bearing_y - margin;
    for (int y = 0; y < gh; ++y) {
        const int dy = origin_y + y;
        if (dy < 0 || dy >= target.height) continue;
        for (int x = 0; x < gw; ++x) {
            const int dx = origin_x + x;
            if (dx < 0 || dx >= target.width) continue;
            const uint8_t coverage = mask[size_t(y) * gw + x];
            if (!coverage) continue;
            sourceOver(&target.rgba[(size_t(dy) * target.width + dx) * 4], r, g, b, a, coverage);
        }
    }
    return true;
}

void upload(TextSurface& s) {
    if (!s.dirty || !s.texture) return;
    GLint old = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glBindTexture(GL_TEXTURE_2D, s.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s.width, s.height, GL_RGBA, GL_UNSIGNED_BYTE, s.rgba.data());
    glBindTexture(GL_TEXTURE_2D, old);
    s.dirty = glGetError() != GL_NO_ERROR;
}
}

bool dbtb_initFonts() {
    if (font_id) return true;
    ScePvfInitRec params{};
    params.maxNumFonts = 1;
    params.allocFunc = pvfAlloc;
    params.reallocFunc = pvfRealloc;
    params.freeFunc = pvfFree;
    ScePvfError error = 0;
    font_lib = scePvfNewLib(&params, &error);
    if (!font_lib || error != 0) return false;

    // The community tables contain Japanese, while menus also contain ASCII.
    // Prefer the system CJK face and then the Japanese/default system faces.
    font_id = openSystemFont(font_lib, SCE_PVF_LANGUAGE_CJK);
    if (!font_id) font_id = openSystemFont(font_lib, SCE_PVF_LANGUAGE_J);
    if (!font_id) font_id = openSystemFont(font_lib, SCE_PVF_DEFAULT_LANGUAGE_CODE);
    if (!font_id) {
        scePvfDoneLib(font_lib);
        font_lib = nullptr;
        return false;
    }
    scePvfSetResolution(font_lib, 72.0f, 72.0f);
    return true;
}

extern "C" {
int32_t dbtb_createText(int32_t w, int32_t h) {
    if (w <= 0 || h <= 0 || w > 1024 || h > 1024 || !dbtb_initFonts()) return -1;
    auto s = std::unique_ptr<TextSurface>(new TextSurface());
    s->width = w;
    s->height = h;
    s->rgba.assign(size_t(w) * h * 4, 0);

    GLint old = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glGenTextures(1, &s->texture);
    glBindTexture(GL_TEXTURE_2D, s->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->rgba.data());
    const bool ok = s->texture != 0 && glGetError() == GL_NO_ERROR;
    glBindTexture(GL_TEXTURE_2D, old);
    if (!ok) {
        if (s->texture) glDeleteTextures(1, &s->texture);
        return -1;
    }

    const int id = next_surface++;
    surfaces.emplace(id, std::move(s));
    return id;
}

void dbtb_clearText(int32_t id) {
    TextSurface* s = surface(id);
    if (!s) return;
    std::fill(s->rgba.begin(), s->rgba.end(), 0);
    s->ypos = 0;
    s->dirty = true;
}

int32_t dbtb_drawText(int32_t id, void* raw_text, int32_t length, int32_t size,
                      int32_t r, int32_t g, int32_t b, int32_t a, void* raw_bounds) {
    TextSurface* s = surface(id);
    if (!s || !raw_text || !raw_bounds || length < 0 || length > 4096 || size <= 0 || s->ypos >= s->height || !setSize(size))
        return 0;

    const auto* text = static_cast<const uint16_t*>(raw_text);
    auto* bounds = static_cast<int32_t*>(raw_bounds);
    int ascent = std::max(1, size);
    int descent = std::max(1, size / 4);

    // Android Paint uses font-wide top/bottom. Approximate those from all glyph
    // metrics in this run, then draw every glyph on the same baseline.
    for (int i = 0; i < length; ++i) {
        ScePvfCharInfo info{};
        ScePvfIrect rect{};
        if (!metrics(text[i], info, rect)) continue;
        ascent = std::max(ascent, info.glyphMetrics.horizontalBearingY64 >> 6);
        descent = std::max(descent, int(info.bitmapHeight) - (info.glyphMetrics.horizontalBearingY64 >> 6));
    }
    const int line_height = std::max(1, ascent + std::max(0, descent));
    if (s->ypos + line_height > s->height) return 0;
    const int baseline = s->ypos + ascent;

    int pen = 0;
    for (int i = 0; i < length; ++i) {
        int advance = std::max(1, size / 2);
        drawGlyph(*s, text[i], pen, baseline, r, g, b, a, advance);
        pen += advance;
    }

    bounds[0] = 0;
    bounds[1] = s->ypos;
    bounds[2] = pen;
    bounds[3] = s->ypos + line_height;
    s->ypos += line_height;
    s->dirty = true;
    return 1;
}

int32_t dbtb_textTexture(int32_t id) {
    TextSurface* s = surface(id);
    if (!s) return -1;
    upload(*s);
    return static_cast<int32_t>(s->texture);
}

void dbtb_disposeText(int32_t id) {
    auto it = surfaces.find(id);
    if (it == surfaces.end()) return;
    if (it->second->texture) glDeleteTextures(1, &it->second->texture);
    surfaces.erase(it);
}
}
