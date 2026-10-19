#include <cstdint>
#include <cstring>
#include "Engine/Source/BlizzardCore/Source/System/String/Path.h"
#include "Engine/Source/BlizzardCore/Source/System/String/String.h"

// VERIFY: d8739b6c manual
// OFFSET: 0x00428500
const char* Blizzard::String::FindFilename(const char* str) {
    if (!str) {
        return "";
    }

    const char* result = str;

    for (const char* p = str; *p; p++) {
        if (*p == '/' || *p == '\\') {
            result = p + 1;
        }
    }

    return result;
}

// VERIFY: 033abbf1 manual
// OFFSET: 0x00428540
const char* System_String::FindExtension(const char* str, int32_t offset) {
    if (!str) {
        return "";
    }

    const char* ext = nullptr;

    for (;; str++) {
        switch (*str) {
            case '\0':
                return ext ? ext : str;
            case '.':
                ext = str + offset;
                break;
            case '/':
            case '\\':
                ext = nullptr;
                break;
        }
    }
}

// VERIFY: 9e2dd077 auto
// OFFSET: 0x00428600
const char* Blizzard::String::FindExtension(const char* str) {
    return System_String::FindExtension(str, 1);
}

// VERIFY: 74fa00af auto
// OFFSET: 0x00428620
const char* Blizzard::String::FindExtensionWithLeadingDot(const char* str) {
    return System_String::FindExtension(str, 0);
}

// VERIFY: 73d381c7 manual
// OFFSET: 0x00428640
char Blizzard::String::FirstPathSeparator(const char* str) {
    if (!str) {
        return '\\';
    }

    for (; *str; str++) {
        if (*str == '/' || *str == '\\') {
            return *str;
        }
    }

    return '\\';
}

// VERIFY: c5079c80 manual
// OFFSET: 0x0042A9B0
void Blizzard::String::MakeUnivPath(const char* src, char* dst, int32_t count) {
    if (!dst || count <= 0) {
        return;
    }

    if (src) {
        for (;;) {
            char c = *src++;
            count--;

            if (!c) {
                break;
            }

            *dst++ = c == '\\' ? '/' : c;

            if (!count) {
                dst[-1] = '\0';
                return;
            }
        }
    }

    *dst = '\0';
}

// VERIFY: 19699d61 manual
// OFFSET: 0x0042AA00
void Blizzard::String::MakeBackslashPath(const char* src, char* dst, int32_t count) {
    if (!dst || count <= 0) {
        return;
    }

    if (src) {
        for (;;) {
            char c = *src++;
            count--;

            if (!c) {
                break;
            }

            *dst++ = c == '/' ? '\\' : c;

            if (!count) {
                dst[-1] = '\0';
                return;
            }
        }
    }

    *dst = '\0';
}

// VERIFY: a5ce1a97 manual
// OFFSET: 0x0042E420
char* Blizzard::String::MakeConsistentPath(const char* src, char* dst, int32_t count) {
    if (!dst || count <= 0) {
        return nullptr;
    }

    if (!src || count == 1) {
        *dst = '\0';
        return dst;
    }

    char* p = dst;

    for (;;) {
        char c = *src++;
        count--;

        if (!c) {
            break;
        }

        if (c == '/') {
            *p = '/';
            MakeUnivPath(src, p + 1, count);
            return dst;
        }

        if (c == '\\') {
            *p = '\\';
            MakeBackslashPath(src, p + 1, count);
            return dst;
        }

        *p++ = c;

        if (!count) {
            p[-1] = '\0';
            return dst;
        }
    }

    *p = '\0';

    return dst;
}

// VERIFY: b5fe8341 manual
// OFFSET: 0x0042E4B0
void Blizzard::String::ForceTrailingSeparator(char* dst, int32_t count, char separator) {
    if (!dst) {
        return;
    }

    if (!separator) {
        separator = FirstPathSeparator(dst);
    }

    if (count < 2) {
        return;
    }

    int32_t len = static_cast<int32_t>(strlen(dst));
    char last = len > 0 ? dst[len - 1] : '\0';

    if (len && last == separator) {
        return;
    }

    if (last == '/' || last == '\\') {
        len--;
    }

    if (len > count - 2) {
        len = count - 2;
    }

    dst[len] = separator;
    dst[len + 1] = '\0';
}

// VERIFY: 930b3c91 manual
// OFFSET: 0x0042E630
char* Blizzard::String::FindPathSeparatorReverse(char* str, uint32_t maxchars) {
    char* result = nullptr;

    if (str) {
        for (char* p = str; p != str + maxchars && *p; p++) {
            if (*p == '/' || *p == '\\') {
                result = p;
            }
        }
    }

    return result;
}

// VERIFY: 14d0c9be auto
// OFFSET: 0x00433510
void Blizzard::String::JoinPath(char* dst, int32_t count, const char* dir, const char* file) {
    if (!dst) {
        return;
    }

    Copy(dst, dir, count);

    if (*dst) {
        ForceTrailingSeparator(dst, count, '\0');
    }

    if (file) {
        while (*file == '/' || *file == '\\') {
            file++;
        }
    }

    Append(dst, file, count);
    MakeConsistentPath(dst, dst, count);
}
