#include "uchar.h"
#include <errno.h>
#include <string.h>

typedef struct {
    uint32_t value;
    uint8_t size, remaining, kind, reserved;
} UtfState;
_Static_assert(sizeof(mbstate_t) >= sizeof(UtfState), "mbstate_t too small");
static UtfState get(const mbstate_t* p) { UtfState s; memcpy(&s, p, sizeof s); return s; }
static void put(mbstate_t* p, UtfState s) { memset(p, 0, sizeof *p); memcpy(p, &s, sizeof s); }
static size_t invalid(mbstate_t* p) { memset(p, 0, sizeof *p); errno = EILSEQ; return (size_t)-1; }

size_t mbrtoc16(char16_t* out, const char* bytes, size_t length, mbstate_t* state) {
    static mbstate_t implicit;
    if (!state) state = &implicit;
    UtfState s = get(state);
    if (s.kind == 2) {
        if (out) *out = (char16_t)s.value;
        put(state, (UtfState){0});
        return (size_t)-3;
    }
    if (s.kind > 2) return invalid(state);
    if (!bytes) { bytes = ""; length = 1; out = NULL; }
    if (!length) return (size_t)-2;
    size_t consumed = 0;
    if (!s.remaining) {
        unsigned b = (unsigned char)bytes[consumed++];
        s.kind = 1;
        if (b < 0x80) { s.value = b; s.size = 1; }
        else if (b >= 0xc2 && b <= 0xdf) { s.value = b & 31; s.remaining = 1; s.size = 2; }
        else if (b >= 0xe0 && b <= 0xef) { s.value = b & 15; s.remaining = 2; s.size = 3; }
        else if (b >= 0xf0 && b <= 0xf4) { s.value = b & 7; s.remaining = 3; s.size = 4; }
        else return invalid(state);
    }
    while (s.remaining && consumed < length) {
        unsigned b = (unsigned char)bytes[consumed++];
        if ((b & 0xc0) != 0x80) return invalid(state);
        s.value = (s.value << 6) | (b & 63);
        --s.remaining;
    }
    if (s.remaining) { put(state, s); return (size_t)-2; }
    const uint32_t c = s.value;
    if ((s.size == 2 && c < 0x80) || (s.size == 3 && c < 0x800) ||
        (s.size == 4 && c < 0x10000) || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff))
        return invalid(state);
    if (c > 0xffff) {
        if (out) *out = (char16_t)(0xd800 + ((c - 0x10000) >> 10));
        put(state, (UtfState){0xdc00 + ((c - 0x10000) & 1023), 0, 0, 2, 0});
    } else {
        if (out) *out = (char16_t)c;
        put(state, (UtfState){0});
    }
    return c == 0 ? 0 : consumed;
}

size_t c16rtomb(char* out, char16_t value, mbstate_t* state) {
    static mbstate_t implicit;
    if (!state) state = &implicit;
    if (!out) { put(state, (UtfState){0}); return 1; }
    UtfState s = get(state);
    uint32_t c = value;
    if (s.kind == 3) {
        if (c < 0xdc00 || c > 0xdfff) return invalid(state);
        c = 0x10000 + ((s.value - 0xd800) << 10) + c - 0xdc00;
    } else if (s.kind != 0) return invalid(state);
    else if (c >= 0xd800 && c <= 0xdbff) {
        put(state, (UtfState){c, 0, 0, 3, 0}); return 0;
    } else if (c >= 0xdc00 && c <= 0xdfff) return invalid(state);
    put(state, (UtfState){0});
    if (c < 0x80) { out[0] = (char)c; return 1; }
    if (c < 0x800) { out[0] = 0xc0 | (c >> 6); out[1] = 0x80 | (c & 63); return 2; }
    if (c < 0x10000) {
        out[0] = 0xe0 | (c >> 12); out[1] = 0x80 | ((c >> 6) & 63); out[2] = 0x80 | (c & 63); return 3;
    }
    out[0] = 0xf0 | (c >> 18); out[1] = 0x80 | ((c >> 12) & 63);
    out[2] = 0x80 | ((c >> 6) & 63); out[3] = 0x80 | (c & 63); return 4;
}
