#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
#include "log.hpp"

#include <psp2/kernel/processmgr.h>
#include <psp2/pvf.h>
#include <vitaGL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
struct TextSurface {
    int width = 0;
    int height = 0;
    int ypos = 0;
    GLuint texture = 0;
    bool dirty = false;
    int dirty_top = 0;
    int dirty_bottom = 0;
    std::vector<uint8_t> rgba;
};

struct Glyph {
    int advance = 1;
    int bearing_x = 0;
    int bearing_y = 0;
    int bearing_x64 = 0;
    int bearing_y64 = 0;
    int width = 0;
    int height = 0;
    int margin = 2;
    bool drawable = false;
    bool rasterized = false;
    std::vector<uint8_t> mask;
};

struct LineMetrics {
    int top = -1;
    int bottom = 1;
    int height = 2;
};

ScePvfLibId font_lib = nullptr;
ScePvfFontId font_id = nullptr;
std::unordered_map<int, std::unique_ptr<TextSurface>> surfaces;
std::unordered_map<uint64_t, Glyph> glyph_cache;
std::unordered_map<int, LineMetrics> line_metrics_cache;
int next_surface = 1;
int current_font_size = -1;

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
    // File streaming seeks into the system CJK font on every cache miss. The
    // supplied hardware log shows roughly 8-11 ms per new glyph. Keep the same
    // font/metrics, but let PVF hold it in memory rather than doing per-glyph I/O.
    ScePvfFontId font = scePvfOpen(lib, index, SCE_PVF_MEMORYBASEDSTREAM, &error);
    if (error == 0 && font) {
        runtimeLog("PVF font opened in memory (language=" + std::to_string(language) + ")");
        return font;
    }
    runtimeLog("PVF memory font unavailable; trying file stream");
    error = 0;
    font = scePvfOpen(lib, index, SCE_PVF_FILEBASEDSTREAM, &error);
    return error == 0 ? font : nullptr;
}

TextSurface* surface(int id) {
    auto it = surfaces.find(id);
    return it == surfaces.end() ? nullptr : it->second.get();
}

bool setSize(int size) {
    if (!font_id || size <= 0) return false;
    if (current_font_size == size) return true;
    if (scePvfSetCharSize(font_id, static_cast<float>(size), static_cast<float>(size)) != 0) return false;
    current_font_size = size;
    return true;
}

void markDirtyRows(TextSurface& s, int top, int bottom) {
    top = std::max(0, std::min(s.height, top));
    bottom = std::max(0, std::min(s.height, bottom));
    if (bottom <= top) return;
    if (!s.dirty) {
        s.dirty = true;
        s.dirty_top = top;
        s.dirty_bottom = bottom;
    } else {
        s.dirty_top = std::min(s.dirty_top, top);
        s.dirty_bottom = std::max(s.dirty_bottom, bottom);
    }
}

bool lineMetrics(int size, LineMetrics& out) {
    auto cached = line_metrics_cache.find(size);
    if (cached != line_metrics_cache.end()) {
        out = cached->second;
        return setSize(size);
    }
    if (!setSize(size)) return false;

    // Android StringTexture uses font-wide Paint.FontMetrics rather than metrics
    // from the particular sentence. 00.12 tried ScePvfFontInfo.maxIGlyphMetrics,
    // but on Vita those values do not track the active scePvfSetCharSize scale;
    // the resulting 1-2 pixel RectF heights made DrawText sample text as thin
    // horizontal lines. Build one stable envelope per requested size from scaled
    // ScePvfCharInfo instead. The conservative size/quarter-size fallback is the
    // same baseline that rendered legibly before the 00.12 optimization.
    int ascent = std::max(1, size);
    int descent = std::max(1, size / 4);
    static const uint16_t probes[] = {
        uint16_t('H'), uint16_t('g'), uint16_t('0'),
        0x3042, // hiragana A
        0x30A2, // katakana A
        0x6F22, // common CJK ideograph
        0x3001, // Japanese comma
        0xFF10  // full-width zero
    };
    for (uint16_t c : probes) {
        ScePvfCharInfo info{};
        if (scePvfGetCharInfo(font_id, c, &info) != 0) continue;
        const int bearing_y = info.glyphMetrics.horizontalBearingY64 >> 6;
        ascent = std::max(ascent, bearing_y);
        descent = std::max(descent, int(info.bitmapHeight) - bearing_y);
    }

    LineMetrics metrics;
    metrics.top = -std::max(1, ascent);
    metrics.bottom = std::max(1, descent);
    metrics.height = std::max(1, -metrics.top + metrics.bottom);
    line_metrics_cache.emplace(size, metrics);
    out = metrics;
    return true;
}

Glyph* glyphFor(int size, uint16_t c, bool need_pixels, bool& raster_miss) {
    const uint64_t key = (uint64_t(uint32_t(size)) << 16) | uint64_t(c);
    auto existing = glyph_cache.find(key);
    if (existing == glyph_cache.end()) {
        if (!setSize(size)) return nullptr;
        Glyph glyph;
        glyph.advance = std::max(1, size / 2);
        ScePvfCharInfo info{};
        if (scePvfGetCharInfo(font_id, c, &info) == 0) {
            glyph.advance = std::max(1, info.glyphMetrics.horizontalAdvance64 >> 6);
            glyph.bearing_x64 = info.glyphMetrics.horizontalBearingX64;
            glyph.bearing_y64 = info.glyphMetrics.horizontalBearingY64;
            glyph.bearing_x = glyph.bearing_x64 >> 6;
            glyph.bearing_y = glyph.bearing_y64 >> 6;
        }
        // Keep metrics for every character so RectF width remains identical to
        // Android, but defer bitmap generation until a glyph can actually touch
        // the 512 px StringTexture. Large character-description strings often
        // contain 450-585 characters even though only the visible prefix can be
        // sampled; the old path rasterized the entire off-screen tail.
        if (glyph_cache.size() >= 2048) glyph_cache.clear();
        existing = glyph_cache.emplace(key, std::move(glyph)).first;
    }

    Glyph& glyph = existing->second;
    if (!need_pixels || glyph.rasterized) return &glyph;
    raster_miss = true;
    glyph.rasterized = true;
    if (!setSize(size)) return &glyph;

    // CharInfo supplies advances/bearings, but its bitmap dimensions cannot
    // replace CharImageRect (00.18 made text disappear on the user's Vita).
    // Query the image rectangle only once per visible glyph/size; keep cached
    // metrics, memory-backed fonts and deferred off-screen rasterization.
    ScePvfIrect rect{};
    const int rect_error = scePvfGetCharImageRect(font_id, c, &rect);
    if (rect_error == 0 && rect.width > 0 && rect.height > 0) {
        glyph.width = rect.width + glyph.margin * 2;
        glyph.height = rect.height + glyph.margin * 2;
        if (glyph.width <= 1024 && glyph.height <= 1024) {
            glyph.mask.assign(size_t(glyph.width) * glyph.height, 0);
            ScePvfUserImageBufferRec image{};
            image.pixelFormat = SCE_PVF_USERIMAGE_DIRECT8;
            image.xPos64 = (glyph.margin << 6) - glyph.bearing_x64;
            image.yPos64 = (glyph.margin << 6) + glyph.bearing_y64;
            image.rect.width = static_cast<uint16_t>(glyph.width);
            image.rect.height = static_cast<uint16_t>(glyph.height);
            image.bytesPerLine = static_cast<uint16_t>(glyph.width);
            image.buffer = glyph.mask.data();
            const int image_error = scePvfGetCharGlyphImage(font_id, c, &image);
            glyph.drawable = image_error == 0;
            if (!glyph.drawable) glyph.mask.clear();
            // A few startup records make an empty mask distinguishable from
            // an upload/lifecycle failure without per-frame log traffic.
            static unsigned diagnostics = 0;
            if (diagnostics++ < 8 || image_error != 0) {
                const auto coverage = std::count_if(glyph.mask.begin(), glyph.mask.end(),
                    [](uint8_t value) { return value != 0; });
                runtimeLog("PVF glyph: char=" + std::to_string(c) + " size=" +
                    std::to_string(size) + " rect=" + std::to_string(rect.width) +
                    "x" + std::to_string(rect.height) + " pixels=" +
                    std::to_string(coverage) + " result=" + std::to_string(image_error));
            }
        }
    } else if (rect_error != 0) {
        runtimeLog("PVF image rectangle failed: char=" + std::to_string(c) +
                   " result=" + std::to_string(rect_error));
    }
    return &glyph;
}

void sourceOver(uint8_t* dst, int r, int g, int b, int a, uint8_t coverage) {
    const int sa = (a * int(coverage) + 127) / 255;
    if (sa <= 0) return;
    const int sr = (r * sa + 127) / 255;
    const int sg = (g * sa + 127) / 255;
    const int sb = (b * sa + 127) / 255;
    if (dst[3] == 0) {
        dst[0] = static_cast<uint8_t>(sr);
        dst[1] = static_cast<uint8_t>(sg);
        dst[2] = static_cast<uint8_t>(sb);
        dst[3] = static_cast<uint8_t>(sa);
        return;
    }
    if (sa == 255) {
        dst[0] = static_cast<uint8_t>(r);
        dst[1] = static_cast<uint8_t>(g);
        dst[2] = static_cast<uint8_t>(b);
        dst[3] = 255;
        return;
    }
    const int inv = 255 - sa;
    dst[0] = static_cast<uint8_t>(std::min(255, sr + (dst[0] * inv + 127) / 255));
    dst[1] = static_cast<uint8_t>(std::min(255, sg + (dst[1] * inv + 127) / 255));
    dst[2] = static_cast<uint8_t>(std::min(255, sb + (dst[2] * inv + 127) / 255));
    dst[3] = static_cast<uint8_t>(std::min(255, sa + (dst[3] * inv + 127) / 255));
}

void drawGlyph(TextSurface& target, const Glyph& glyph, int pen_x, int baseline,
               int r, int g, int b, int a) {
    if (!glyph.drawable || glyph.mask.empty()) return;
    const int origin_x = pen_x + glyph.bearing_x - glyph.margin;
    if (origin_x >= target.width || origin_x + glyph.width <= 0) return;
    const int origin_y = baseline - glyph.bearing_y - glyph.margin;
    for (int y = 0; y < glyph.height; ++y) {
        const int dy = origin_y + y;
        if (dy < 0 || dy >= target.height) continue;
        for (int x = 0; x < glyph.width; ++x) {
            const int dx = origin_x + x;
            if (dx < 0 || dx >= target.width) continue;
            const uint8_t coverage = glyph.mask[size_t(y) * glyph.width + x];
            if (!coverage) continue;
            sourceOver(&target.rgba[(size_t(dy) * target.width + dx) * 4], r, g, b, a, coverage);
        }
    }
}

void upload(TextSurface& s) {
    if (!s.dirty || !s.texture) return;
    DbtbTimedScope timer(dbtb_performance().text_us);
    const uint64_t start = sceKernelGetProcessTimeWide();
    const int top = std::max(0, s.dirty_top);
    const int bottom = std::min(s.height, s.dirty_bottom);
    if (bottom <= top) { s.dirty = false; return; }

    GLint old = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glBindTexture(GL_TEXTURE_2D, s.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, top, s.width, bottom - top,
                    GL_RGBA, GL_UNSIGNED_BYTE, s.rgba.data() + size_t(top) * s.width * 4);
    const GLenum error = glGetError();
    glBindTexture(GL_TEXTURE_2D, old);
    if (error == GL_NO_ERROR) {
        s.dirty = false;
        s.dirty_top = s.dirty_bottom = 0;
    }
    const uint64_t elapsed = sceKernelGetProcessTimeWide() - start;
    if (elapsed >= 15000) {
        runtimeLog("Text upload: " + std::to_string(elapsed / 1000) + " ms rows=" +
                   std::to_string(bottom - top));
    }
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
    current_font_size = -1;
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
    const int used_rows = std::max(0, std::min(s->height, s->ypos));
    if (used_rows > 0) {
        std::fill(s->rgba.begin(), s->rgba.begin() + size_t(used_rows) * s->width * 4, 0);
        markDirtyRows(*s, 0, used_rows);
    }
    s->ypos = 0;
}

int32_t dbtb_drawText(int32_t id, void* raw_text, int32_t length, int32_t size,
                      int32_t r, int32_t g, int32_t b, int32_t a, void* raw_bounds) {
    DbtbTimedScope timer(dbtb_performance().text_us);
    TextSurface* s = surface(id);
    if (!s || !raw_text || !raw_bounds || length < 0 || length > 4096 || size <= 0 || s->ypos >= s->height)
        return 0;

    const uint64_t start = sceKernelGetProcessTimeWide();
    const auto* text = static_cast<const uint16_t*>(raw_text);
    auto* bounds = static_cast<int32_t*>(raw_bounds);
    LineMetrics metrics{};
    if (!lineMetrics(size, metrics) || s->ypos + metrics.height > s->height) return 0;

    const int top = s->ypos;
    const int baseline = s->ypos - metrics.top; // Android: canvas.drawText(..., ypos - FontMetrics.top, ...)
    int pen = 0;
    int misses = 0;
    for (int i = 0; i < length; ++i) {
        bool cache_miss = false;
        const bool need_pixels = pen < s->width + size;
        Glyph* glyph = glyphFor(size, text[i], need_pixels, cache_miss);
        if (cache_miss) ++misses;
        if (!glyph) continue;
        if (need_pixels) drawGlyph(*s, *glyph, pen, baseline, r, g, b, a);
        pen += glyph->advance;
    }

    bounds[0] = 0;
    bounds[1] = top;
    bounds[2] = pen;
    bounds[3] = top + metrics.height;
    s->ypos += metrics.height;
    markDirtyRows(*s, top, s->ypos);

    const uint64_t elapsed = sceKernelGetProcessTimeWide() - start;
    if (elapsed >= 15000) {
        runtimeLog("Text draw: " + std::to_string(elapsed / 1000) + " ms chars=" +
                   std::to_string(length) + " misses=" + std::to_string(misses) +
                   " size=" + std::to_string(size));
    }
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
