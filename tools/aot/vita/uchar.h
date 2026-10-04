#pragma once
// Newlib in the pinned VitaSDK lacks C11 <uchar.h>. TeaVM's Java chars are
// UTF-16 regardless of wchar_t width; do not substitute wchar_t here.
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
size_t mbrtoc16(char16_t* out, const char* bytes, size_t length, mbstate_t* state);
size_t c16rtomb(char* out, char16_t value, mbstate_t* state);
