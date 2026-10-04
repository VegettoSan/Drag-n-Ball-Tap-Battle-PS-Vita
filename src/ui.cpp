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
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex3f(x, y, 0.0f);
    glVertex3f(x + w, y, 0.0f);
    glVertex3f(x + w, y + h, 0.0f);
    glVertex3f(x, y + h, 0.0f);
    glEnd();
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

} // namespace

bool runBootSelector(const std::vector<std::string>& mods, bool original_data_present, BootChoice& choice) {
    const int total = static_cast<int>(mods.size()) + 1;
    int selected = 0;
    VitaInput input;

    for (;;) {
        const InputFrame frame = input.poll();
        if (frame.up && selected > 0) --selected;
        if (frame.down && selected + 1 < total) ++selected;
        const int visible = 8;
        int first = std::max(0, selected - visible / 2);
        first = std::min(first, std::max(0, total - visible));
        bool confirm = frame.confirm;
        for (size_t p = 0; p < frame.pointer_count; ++p) {
            const PointerEvent& pointer = frame.pointers[p];
            if (pointer.phase != PointerPhase::Begin || pointer.x < 44 || pointer.x > 916) continue;
            for (int row = 0; row < visible && first + row < total; ++row) {
                const float y = 120.0f + row * 44.0f;
                if (pointer.y >= y - 7 && pointer.y <= y + 31) { selected = first + row; confirm = true; break; }
            }
        }

        if (confirm) {
            if (selected == 0) {
                choice.original = true;
                choice.mod_directory.clear();
            } else {
                choice.original = false;
                choice.mod_directory = mods[static_cast<size_t>(selected - 1)];
            }
            return true;
        }

        if (frame.back) return false;

        glClearColor(0.035f, 0.035f, 0.055f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        begin2D();

        text(48, 32, 4, "DRAGON BALL TAP BATTLE VITA", 1.0f, 0.85f, 0.15f);
        text(50, 76, 2, "SELECT DATA SET", 0.75f, 0.82f, 1.0f);

        for (int row = 0; row < visible && first + row < total; ++row) {
            const int index = first + row;
            const float y = 120.0f + row * 44.0f;
            const bool active = index == selected;
            rect(44, y - 7, 872, 38, active ? 0.22f : 0.09f, active ? 0.40f : 0.10f, active ? 0.66f : 0.14f);

            std::string label;
            if (index == 0) {
                label = original_data_present ? "ORIGINAL" : "ORIGINAL - DATA MISSING";
            } else {
                char number[16];
                std::snprintf(number, sizeof(number), "%02d - ", index);
                label = std::string(number) + clipped(mods[static_cast<size_t>(index - 1)], 37);
            }
            text(62, y, 3, label, active ? 1.0f : 0.80f, active ? 1.0f : 0.82f, active ? 1.0f : 0.86f);
        }

        text(50, 500, 2, "DPAD / STICK MOVE   X / TOUCH SELECT   O BACK", 0.72f, 0.72f, 0.78f);
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
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(image->premultiplied_alpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, texture);
            glColor4f(1, 1, 1, 1);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex3f(x, y, 0);
            glTexCoord2f(1, 0); glVertex3f(x+w, y, 0);
            glTexCoord2f(1, 1); glVertex3f(x+w, y+h, 0);
            glTexCoord2f(0, 1); glVertex3f(x, y+h, 0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        text(50, 500, 2, "X / START / TRIANGLE TO EXIT", 0.75f, 0.75f, 0.78f);
        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(16000);
    }
}
