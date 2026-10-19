#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_STRING_STRING_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_STRING_STRING_H

#include <cstdarg>
#include <cstdint>
#include "Engine/Source/Storm/H/storm.h"

namespace System_String {

char* Duplicate(const char* str, const char* fileName, int32_t lineNumber);
const char* Find(const char* str, const char* substr, uint32_t maxchars);
const char* FindI(const char* str, const char* substr, uint32_t maxchars);
const char* FindReverse(const char* str, char c, uint32_t maxchars);

}

namespace Blizzard {
namespace String {

template <uint32_t N>
class QuickFormat {
    public:
    /* 0000 */ char m_buffer[N];

    QuickFormat(const char* format, ...);
};

uint32_t Append(char* dst, const char* src, uint32_t maxchars);
uint32_t Copy(char* dst, const char* src, uint32_t maxchars);
char* Duplicate(const char* str);
bool Equal(const char* str1, const char* str2);
bool EqualI(const char* str1, const char* str2);
bool EqualI(const char* str1, const char* str2, uint32_t count);
const char* Find(const char* str, const char* substr, uint32_t maxchars);
const char* FindFirstOf(const char* str, const char* chars, int32_t maxchars);
const char* FindI(const char* str, const char* substr, uint32_t maxchars);
const char* FindReverse(const char* str, char c, uint32_t maxchars);
int32_t Format(char* dst, uint32_t maxchars, const char* format, ...);
uint32_t Length(const char* str);
void MemCopy(void* dst, const void* src, int32_t bytes);
void MemFill(void* dst, uint32_t bytes, uint8_t value);
int32_t VFormat(char* dst, uint32_t maxchars, const char* format, va_list args);

}
}

// VERIFY: adbad918 manual
// OFFSET: 0x00429790
template <uint32_t N>
Blizzard::String::QuickFormat<N>::QuickFormat(const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);
        vsnoprintf(this->m_buffer, N, format, args);
        va_end(args);
        this->m_buffer[N - 1] = '\0';
    } else {
        this->m_buffer[0] = '\0';
    }
}

#endif
