#include "ui.hpp"
#include "log.hpp"
#include "input.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <psp2/kernel/threadmgr.h>
#include <vitaGL.h>

namespace {

void rect(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f) {
    // Do not use GL1 immediate mode here. The full-engine Vita build keeps
    // vitaGL's legacy immediate-mode pool at zero because the original Android
    // game is GLES/client-array based. Calling glBegin/glVertex* with that pool
    // disabled leaves vitaGL's legacy vertex pointer null and crashes on real
    // hardware. A tiny client-side array is enough for the selector UI.
    const GLfloat vertices[] = {
        x,     y,     0.0f,
        x + w, y,     0.0f,
        x + w, y + h, 0.0f,
        x,     y + h, 0.0f,
    };
    glColor4f(r, g, b, a);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

struct UiTexture {
    GLuint id = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float content_u_max = 1.0f;
    float content_v_max = 1.0f;
};

bool readUiFile(const char* path, std::vector<uint8_t>& bytes) {
    bytes.clear();
    FILE* file = std::fopen(path, "rb");
    if (!file) return false;
    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return false;
    }
    const long length = std::ftell(file);
    if (length <= 0 || length > 1024 * 1024 || std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file);
        return false;
    }
    bytes.resize(static_cast<size_t>(length));
    const bool ok = std::fread(bytes.data(), 1, bytes.size(), file) == bytes.size();
    std::fclose(file);
    if (!ok) bytes.clear();
    return ok;
}

bool loadUiTexture(const char* path, UiTexture& texture) {
    texture = UiTexture{};
    std::vector<uint8_t> encoded;
    if (!readUiFile(path, encoded)) {
        runtimeLog(std::string("Selector theme file missing/unreadable: ") + path);
        return false;
    }

    RgbaImage image;
    std::string error;
    if (!decodePng(encoded, image, error)) {
        runtimeLog(std::string("Selector theme PNG decode failed: ") + path + " - " + error);
        return false;
    }

    while (glGetError() != GL_NO_ERROR) {}
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());
    const GLenum upload_error = glGetError();
    if (!texture.id || upload_error != GL_NO_ERROR) {
        runtimeLog(std::string("Selector theme texture upload failed: ") + path +
                   " GL=" + std::to_string(upload_error));
        if (texture.id) glDeleteTextures(1, &texture.id);
        texture = UiTexture{};
        return false;
    }
    texture.width = image.width;
    texture.height = image.height;

    // select0_background.png contains the wanted cyan/grid background in
    // its upper band, but also contains a separate blue energy orb in the
    // transparent lower area. The selector must use only the background band.
    // Detect the first continuous, substantially populated band from the top,
    // then crop both U and V to that band before stretching it to 960x544.
    uint32_t content_bottom = image.height;
    const std::string ui_path(path ? path : "");
    if (ui_path.find("select0_background.png") != std::string::npos) {
        content_bottom = 0;
        for (uint32_t y = 0; y < image.height; ++y) {
            uint32_t row_visible = 0;
            for (uint32_t x = 0; x < image.width; ++x) {
                if (image.pixels[(static_cast<size_t>(y) * image.width + x) * 4 + 3] != 0)
                    ++row_visible;
            }
            // The real background occupies almost the complete row. The first
            // sparse row marks the transparent/orb section and is excluded.
            if (row_visible < image.width / 2) break;
            content_bottom = y + 1;
        }
        if (content_bottom == 0) content_bottom = image.height;
        if (content_bottom < image.height)
            texture.content_v_max =
                static_cast<float>(content_bottom) / static_cast<float>(image.height);
    }

    uint32_t rightmost = 0;
    bool visible = false;
    for (uint32_t y = 0; y < content_bottom; ++y) {
        for (uint32_t x = 0; x < image.width; ++x) {
            if (image.pixels[(static_cast<size_t>(y) * image.width + x) * 4 + 3] != 0) {
                rightmost = std::max(rightmost, x);
                visible = true;
            }
        }
    }
    if (visible && rightmost + 1 < image.width)
        texture.content_u_max =
            static_cast<float>(rightmost + 1) / static_cast<float>(image.width);

    if (ui_path.find("select0_background.png") != std::string::npos) {
        runtimeLog("Selector background crop: u=" + std::to_string(texture.content_u_max) +
                   " v=" + std::to_string(texture.content_v_max) +
                   " (blue orb excluded)");
    }

    return true;
}

void destroyUiTexture(UiTexture& texture) {
    if (texture.id) glDeleteTextures(1, &texture.id);
    texture = UiTexture{};
}

void drawUiTextureUv(const UiTexture& texture, float x, float y, float w, float h,
                     float u_max, float v_max, float tint = 1.0f, float alpha = 1.0f) {
    if (!texture.id) return;
    const GLfloat vertices[] = {
        x,     y,     0.0f,
        x + w, y,     0.0f,
        x + w, y + h, 0.0f,
        x,     y + h, 0.0f,
    };
    const GLfloat texcoords[] = {
        0.0f, 0.0f,
        u_max, 0.0f,
        u_max, v_max,
        0.0f, v_max,
    };
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glColor4f(tint, tint, tint, alpha);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

void drawUiTexture(const UiTexture& texture, float x, float y, float w, float h,
                   float tint = 1.0f, float alpha = 1.0f) {
    drawUiTextureUv(texture, x, y, w, h, 1.0f, 1.0f, tint, alpha);
}

void destroySelectorTheme(UiTexture& background, UiTexture& header,
                          UiTexture& button, UiTexture& ball) {
    destroyUiTexture(background);
    destroyUiTexture(header);
    destroyUiTexture(button);
    destroyUiTexture(ball);
}

const uint8_t* glyph(char input) {
    static const uint8_t blank[7] = {0,0,0,0,0,0,0};
    static const uint8_t unknown[7] = {14,17,1,2,4,0,4};
    static const uint8_t dash[7] = {0,0,0,31,0,0,0};
    static const uint8_t dot[7] = {0,0,0,0,0,12,12};
    static const uint8_t colon[7] = {0,12,12,0,12,12,0};
    static const uint8_t slash[7] = {1,2,2,4,8,8,16};
    static const uint8_t underscore[7] = {0,0,0,0,0,0,31};

    static const uint8_t digits[10][7] = {
        {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
        {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
        {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
        {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
    };

    static const uint8_t letters[26][7] = {
        {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
        {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
        {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17},
        {14,4,4,4,4,4,14}, {7,2,2,2,18,18,12},
        {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
        {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
        {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
        {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
        {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31}
    };

    char c = input;
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    if (c >= '0' && c <= '9') return digits[c - '0'];
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
    switch (c) {
        case ' ': return blank;
        case '-': return dash;
        case '.': return dot;
        case ':': return colon;
        case '/': return slash;
        case '_': return underscore;
        default: return unknown;
    }
}

void text(float x, float y, float scale, const std::string& value, float r, float g, float b) {
    float cursor = x;
    for (char c : value) {
        const uint8_t* rows = glyph(c);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (1u << (4 - col))) {
                    rect(cursor + col * scale, y + row * scale, scale, scale, r, g, b);
                }
            }
        }
        cursor += 6.0f * scale;
        if (cursor > 945.0f) break;
    }
}

float textWidth(float scale, const std::string& value) {
    return value.empty() ? 0.0f : (static_cast<float>(value.size()) * 6.0f - 1.0f) * scale;
}

void shadowText(float x, float y, float scale, const std::string& value,
                float r, float g, float b) {
    text(x + 2.0f, y + 2.0f, scale, value, 0.015f, 0.02f, 0.035f);
    text(x, y, scale, value, r, g, b);
}

void centeredShadowText(float center_x, float y, float scale, const std::string& value,
                        float r, float g, float b) {
    shadowText(center_x - textWidth(scale, value) * 0.5f, y, scale, value, r, g, b);
}

std::string clipped(const std::string& s, size_t max_len) {
    if (s.size() <= max_len) return s;
    if (max_len <= 3) return s.substr(0, max_len);
    return s.substr(0, max_len - 3) + "...";
}

void begin2D() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 960, 544, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void drawProfileOpening(const std::string& profile, bool theme_ready,
                        const UiTexture& background, const UiTexture& header,
                        const UiTexture& button) {
    glClearColor(0.005f, 0.035f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    begin2D();

    if (theme_ready) {
        drawUiTextureUv(background, 0.0f, 0.0f, 960.0f, 544.0f,
                        background.content_u_max, background.content_v_max, 0.92f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rect(0.0f, 0.0f, 960.0f, 544.0f, 0.015f, 0.055f, 0.12f, 0.26f);

        drawUiTexture(header, 72.0f, 80.0f, 816.0f, 58.0f);
        centeredShadowText(480.0f, 97.0f, 3.0f, "OPENING PROFILE", 1.0f, 0.86f, 0.08f);

        drawUiTexture(button, 178.0f, 220.0f, 604.0f, 58.0f, 1.0f);
        const std::string shown = clipped(profile, 34);
        const float scale = shown.size() > 28 ? 2.0f : 2.5f;
        centeredShadowText(480.0f, 238.0f, scale, shown, 1.0f, 0.98f, 0.82f);

        centeredShadowText(480.0f, 318.0f, 2.0f, "LOADING GAME DATA...", 0.90f, 0.94f, 1.0f);
    } else {
        text(48, 76, 4, "OPENING PROFILE", 1.0f, 0.85f, 0.15f);
        text(54, 170, 3, clipped(profile, 42), 0.86f, 0.92f, 1.0f);
        text(54, 242, 2, "LOADING GAME DATA...", 0.75f, 0.82f, 1.0f);
    }

    vglSwapBuffers(GL_FALSE);
}

} // namespace

bool runBootSelector(const std::vector<std::string>& profiles, BootChoice& choice) {
    const int total = static_cast<int>(profiles.size());
    int selected = 0;
    VitaInput input;

    UiTexture theme_background, theme_header, theme_button, theme_ball;
    const bool theme_ready =
        loadUiTexture("app0:/selector/select0_background.png", theme_background) &&
        loadUiTexture("app0:/selector/select0_header.png", theme_header) &&
        loadUiTexture("app0:/selector/select0_button.png", theme_button) &&
        loadUiTexture("app0:/selector/select0_ball_1.png", theme_ball);
    if (!theme_ready) {
        destroySelectorTheme(theme_background, theme_header, theme_button, theme_ball);
        runtimeLog("Boot selector: Gen visual theme unavailable, using safe legacy fallback");
    } else {
        runtimeLog("Boot selector: Gen select0.pac visual theme loaded (no character art)");
    }

    runtimeLog("Boot selector entered: " + std::to_string(total) + " installed profiles");

    for (;;) {
        const InputFrame frame = input.poll();
        if (total > 0 && frame.up && selected > 0) --selected;
        if (total > 0 && frame.down && selected + 1 < total) ++selected;

        const int visible = theme_ready ? 6 : 8;
        int first = std::max(0, selected - visible / 2);
        first = std::min(first, std::max(0, total - visible));
        bool confirm = total > 0 && frame.confirm;

        for (size_t p = 0; p < frame.pointer_count; ++p) {
            const PointerEvent& pointer = frame.pointers[p];
            if (pointer.phase != PointerPhase::Begin) continue;
            for (int row = 0; row < visible && first + row < total; ++row) {
                const float y = theme_ready ? (132.0f + row * 58.0f) : (120.0f + row * 44.0f);
                const float x0 = theme_ready ? 176.0f : 44.0f;
                const float x1 = theme_ready ? 918.0f : 916.0f;
                const float y0 = theme_ready ? y - 5.0f : y - 7.0f;
                const float y1 = theme_ready ? y + 50.0f : y + 31.0f;
                if (pointer.x >= x0 && pointer.x <= x1 &&
                    pointer.y >= y0 && pointer.y <= y1) {
                    selected = first + row;
                    confirm = true;
                    break;
                }
            }
        }

        if (confirm) {
            choice.profile_directory = profiles[static_cast<size_t>(selected)];
            runtimeLog("Boot selector opening profile: " + choice.profile_directory);
            drawProfileOpening(choice.profile_directory, theme_ready,
                               theme_background, theme_header, theme_button);
            // Keep the completed frame on screen while the runtime starts loading
            // the selected profile. A short second present makes the transition
            // visible on real hardware without introducing a long artificial delay.
            sceKernelDelayThread(33000);
            drawProfileOpening(choice.profile_directory, theme_ready,
                               theme_background, theme_header, theme_button);
            destroySelectorTheme(theme_background, theme_header, theme_button, theme_ball);
            return true;
        }

        if (frame.back) {
            destroySelectorTheme(theme_background, theme_header, theme_button, theme_ball);
            return false;
        }

        if (theme_ready) {
            // Visual composition deliberately borrows only non-character
            // select0.pac elements from Gen so the selector reads as part of
            // Tap Battle without replacing any original engine/menu logic.
            glClearColor(0.005f, 0.035f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            begin2D();

            drawUiTextureUv(theme_background, 0.0f, 0.0f, 960.0f, 544.0f,
                            theme_background.content_u_max,
                            theme_background.content_v_max, 0.92f);
            // Dark translucent wash keeps arbitrary mod names legible while
            // preserving the cyan grid/energy artwork.
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            rect(0.0f, 0.0f, 960.0f, 544.0f, 0.015f, 0.055f, 0.12f, 0.18f);

            drawUiTexture(theme_header, 72.0f, 26.0f, 816.0f, 58.0f);
            centeredShadowText(480.0f, 43.0f, 3.0f, "SELECT DATA SET", 1.0f, 0.86f, 0.08f);
            centeredShadowText(480.0f, 95.0f, 2.0f, "DRAGON BALL TAP BATTLE VITA", 1.0f, 0.82f, 0.07f);

            if (total == 0) {
                centeredShadowText(480.0f, 218.0f, 2.35f, "NO GAME DATA FOUND", 1.0f, 0.94f, 0.76f);
                centeredShadowText(480.0f, 258.0f, 1.75f, "PREPARE A TAP BATTLE APK WITH THE EXTRACTOR", 0.90f, 0.94f, 1.0f);
                centeredShadowText(480.0f, 288.0f, 1.75f, "COPY IT TO UX0:DATA/DBTAPBATTLE/PROFILES/", 0.90f, 0.94f, 1.0f);
            }

            for (int row = 0; row < visible && first + row < total; ++row) {
                const int index = first + row;
                const float y = 132.0f + row * 58.0f;
                const bool active = index == selected;
                const float tint = active ? 1.0f : 0.70f;

                if (active) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    rect(248.0f, y - 3.0f, 664.0f, 52.0f, 0.10f, 0.60f, 1.0f, 0.22f);
                }
                drawUiTexture(theme_ball, 184.0f, y - 3.0f, 52.0f, 52.0f, active ? 1.0f : 0.66f);
                drawUiTexture(theme_button, 248.0f, y, 664.0f, 46.0f, tint);

                char number[16];
                std::snprintf(number, sizeof(number), "%02d - ", index + 1);
                const std::string label = std::string(number) +
                    clipped(profiles[static_cast<size_t>(index)], 39);

                const float scale = label.size() > 34 ? 1.75f : (label.size() > 27 ? 2.0f : 2.25f);
                shadowText(282.0f, y + 13.0f, scale, label,
                           active ? 1.0f : 0.86f,
                           active ? 0.98f : 0.90f,
                           active ? 0.78f : 0.94f);
            }

            drawUiTexture(theme_header, 72.0f, 489.0f, 816.0f, 39.0f, 0.88f);
            centeredShadowText(480.0f, 501.0f, 1.55f,
                               "DPAD / STICK MOVE   X / TOUCH SELECT   O BACK",
                               1.0f, 0.86f, 0.08f);
        } else {
            // Hardware-safe fallback kept intentionally: if app0 selector PNGs
            // are ever damaged/missing, data selection still works exactly as
            // the already-tested flat selector.
            glClearColor(0.035f, 0.035f, 0.055f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            begin2D();

            text(48, 32, 4, "DRAGON BALL TAP BATTLE VITA", 1.0f, 0.85f, 0.15f);
            text(50, 76, 2, "SELECT DATA SET", 0.75f, 0.82f, 1.0f);

            if (total == 0) {
                text(92, 210, 3, "NO GAME DATA FOUND", 1.0f, 0.90f, 0.65f);
                text(92, 264, 2, "PREPARE A TAP BATTLE APK WITH THE EXTRACTOR", 0.82f, 0.88f, 1.0f);
                text(92, 296, 2, "COPY IT TO UX0:DATA/DBTAPBATTLE/PROFILES/", 0.82f, 0.88f, 1.0f);
            }

            for (int row = 0; row < visible && first + row < total; ++row) {
                const int index = first + row;
                const float y = 120.0f + row * 44.0f;
                const bool active = index == selected;
                rect(44, y - 7, 872, 38, active ? 0.22f : 0.09f,
                     active ? 0.40f : 0.10f, active ? 0.66f : 0.14f);

                char number[16];
                std::snprintf(number, sizeof(number), "%02d - ", index + 1);
                const std::string label = std::string(number) +
                    clipped(profiles[static_cast<size_t>(index)], 37);
                text(62, y, 3, label, active ? 1.0f : 0.80f,
                     active ? 1.0f : 0.82f, active ? 1.0f : 0.86f);
            }

            text(50, 500, 2, "DPAD / STICK MOVE   X / TOUCH SELECT   O BACK",
                 0.72f, 0.72f, 0.78f);
        }

        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(16000);
    }
}

void showPacResult(bool success, const std::string& detail, const RgbaImage* image) {
    GLuint texture = 0;
    std::string shown_detail = detail;
    if (image) {
        while (glGetError() != GL_NO_ERROR) {}
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image->pixels.data());
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR || !texture) {
            runtimeLog("Renderer texture upload error: " + std::to_string(error));
            if (texture) glDeleteTextures(1, &texture);
            texture = 0;
            success = false;
            shown_detail = "TEXTURE UPLOAD FAILED - CHECK RUNTIME.LOG";
        } else runtimeLog("Texture upload accepted by vitaGL");
    }
    VitaInput input;
    for (;;) {
        const InputFrame frame = input.poll();
        if (frame.confirm || frame.back || frame.pause) {
            if (texture) glDeleteTextures(1, &texture);
            runtimeLog("Preview closed by user");
            return;
        }

        if (success) glClearColor(0.025f, 0.16f, 0.065f, 1.0f);
        else glClearColor(0.20f, 0.035f, 0.035f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        begin2D();

        text(48, 80, 5, success ? "PAC OK" : "PAC ERROR", 1.0f, 1.0f, 1.0f);
        text(50, 160, 2, clipped(shown_detail, 70), 0.95f, 0.95f, 0.95f);
        if (texture) {
            const float scale = std::min(700.0f / image->width, 286.0f / image->height);
            const float w = image->width * scale, h = image->height * scale;
            const float x = (960.0f - w) / 2, y = 195.0f;
            const GLfloat vertices[] = {
                x,     y,     0.0f,
                x + w, y,     0.0f,
                x + w, y + h, 0.0f,
                x,     y + h, 0.0f,
            };
            const GLfloat texcoords[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
            };
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(image->premultiplied_alpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, texture);
            glColor4f(1, 1, 1, 1);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }
        text(50, 500, 2, "X / START / TRIANGLE TO EXIT", 0.75f, 0.75f, 0.78f);
        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(16000);
    }
}
