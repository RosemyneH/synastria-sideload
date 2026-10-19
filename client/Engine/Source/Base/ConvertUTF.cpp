#include <cstdint>
#include "Engine/Source/Base/ConvertUTF.h"
#include "Engine/Source/Storm/H/storm.h"

#define UNI_REPLACEMENT_CHAR 0xFFFD
#define UNI_MAX_BMP 0xFFFF
#define UNI_MAX_UTF16 0x10FFFF
#define UNI_SUR_HIGH_START 0xD800
#define UNI_SUR_LOW_START 0xDC00
#define UNI_SUR_MASK 0x3FF
#define UNI_HALF_SHIFT 10
#define UNI_HALF_BASE 0x10000

// STATIC: 0x009E9DB0
static const uint32_t offsetsFromUTF8[6] = { 0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080 };

// STATIC: 0x009E9DC8
static const uint8_t trailingBytesForUTF8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

// STATIC: 0x009E9EC8
static const uint32_t firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

// VERIFY: bf13a16b manual
// OFFSET: 0x0047B9E0
int32_t ConvertUTF16toUTF8(char* dst, uint32_t dstmaxchars, const uint16_t* src, uint32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars) {
    char* dstStart = dst;
    const uint16_t* srcStart = src;
    const uint16_t* srcEnd = srcmaxchars == STORM_MAX_STR ? reinterpret_cast<const uint16_t*>(-1) : src + srcmaxchars;
    char* dstEnd = dst + dstmaxchars;
    int32_t result;

    for (;;) {
        if (src >= srcEnd) {
            result = -1;
            break;
        }

        uint32_t ch = *src;
        uint32_t srcUnits = 1;

        if (ch - UNI_SUR_HIGH_START <= UNI_SUR_MASK) {
            if (src + 1 >= srcEnd) {
                result = -1;
                break;
            }

            uint32_t ch2 = src[1];

            if (ch2 - UNI_SUR_LOW_START <= UNI_SUR_MASK) {
                ch = ((ch - UNI_SUR_HIGH_START) << UNI_HALF_SHIFT) + (ch2 - UNI_SUR_LOW_START) + UNI_HALF_BASE;
                srcUnits = 2;
            }
        }

        uint32_t bytesToWrite;

        if (ch < 0x80) {
            bytesToWrite = 1;

            if (!ch) {
                if (dst < dstEnd) {
                    *dst++ = 0;
                    result = 0;
                } else {
                    result = bytesToWrite;
                }

                break;
            }
        } else if (ch < 0x800) {
            bytesToWrite = 2;
        } else if (ch < 0x10000) {
            bytesToWrite = 3;
        } else if (ch < 0x200000) {
            bytesToWrite = 4;
        } else if (ch < 0x4000000) {
            bytesToWrite = 5;
        } else if (ch <= 0x7FFFFFFF) {
            bytesToWrite = 6;
        } else {
            bytesToWrite = 2;
            ch = UNI_REPLACEMENT_CHAR;
        }

        if (dst + bytesToWrite > dstEnd) {
            result = bytesToWrite;
            break;
        }

        dst += bytesToWrite;

        switch (bytesToWrite) {
            case 6:
                *--dst = static_cast<char>((ch & 0x3F) | 0x80);
                ch >>= 6;
            case 5:
                *--dst = static_cast<char>((ch & 0x3F) | 0x80);
                ch >>= 6;
            case 4:
                *--dst = static_cast<char>((ch & 0x3F) | 0x80);
                ch >>= 6;
            case 3:
                *--dst = static_cast<char>((ch & 0x3F) | 0x80);
                ch >>= 6;
            case 2:
                *--dst = static_cast<char>((ch & 0x3F) | 0x80);
                ch >>= 6;
            case 1:
                *--dst = static_cast<char>(ch | firstByteMark[bytesToWrite]);
        }

        dst += bytesToWrite;
        src += srcUnits;
    }

    if (srcchars) {
        *srcchars = static_cast<uint32_t>(src - srcStart);
    }

    if (dstchars) {
        *dstchars = static_cast<uint32_t>(dst - dstStart);
    }

    return result;
}

// VERIFY: ba59e8bd manual
// OFFSET: 0x0047BBA0
int32_t ConvertUTF8toUTF16(uint16_t* dst, uint32_t dstmaxchars, const char* src, uint32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars) {
    uint16_t* dstStart = dst;
    const uint8_t* source = reinterpret_cast<const uint8_t*>(src);
    const uint8_t* sourceEnd = srcmaxchars == STORM_MAX_STR ? reinterpret_cast<const uint8_t*>(-1) : source + srcmaxchars;
    uint16_t* dstEnd = dst + dstmaxchars;
    int32_t result;

    for (;;) {
        if (source >= sourceEnd) {
            result = -1;
            break;
        }

        uint32_t extraBytesToRead = trailingBytesForUTF8[*source];

        if (source + extraBytesToRead >= sourceEnd) {
            result = -1 - static_cast<int32_t>(extraBytesToRead);
            break;
        }

        const uint8_t* next = source;
        uint32_t ch = 0;

        switch (extraBytesToRead) {
            case 5:
                ch += *next++;
                ch <<= 6;
            case 4:
                ch += *next++;
                ch <<= 6;
            case 3:
                ch += *next++;
                ch <<= 6;
            case 2:
                ch += *next++;
                ch <<= 6;
            case 1:
                ch += *next++;
                ch <<= 6;
            case 0:
                ch += *next++;
        }

        ch -= offsetsFromUTF8[extraBytesToRead];

        if (dst >= dstEnd) {
            result = 1;
            break;
        }

        if (ch > UNI_MAX_BMP) {
            if (ch <= UNI_MAX_UTF16) {
                if (dst + 1 >= dstEnd) {
                    result = 1;
                    break;
                }

                ch -= UNI_HALF_BASE;
                *dst++ = static_cast<uint16_t>((ch >> UNI_HALF_SHIFT) + UNI_SUR_HIGH_START);
                *dst++ = static_cast<uint16_t>((ch & UNI_SUR_MASK) + UNI_SUR_LOW_START);
            } else {
                *dst++ = UNI_REPLACEMENT_CHAR;
            }
        } else {
            *dst++ = static_cast<uint16_t>(ch);

            if (!ch) {
                result = 0;
                break;
            }
        }

        source = next;
    }

    if (srcchars) {
        *srcchars = static_cast<uint32_t>(source - reinterpret_cast<const uint8_t*>(src));
    }

    if (dstchars) {
        *dstchars = static_cast<uint32_t>(dst - dstStart);
    }

    return result;
}

// VERIFY: e60cd645 manual
// OFFSET: [0x0047BD40, 0x00852210]
uint32_t sgetu8(const uint8_t* strptr, int32_t* chars) {
    if (chars) {
        *chars = 0;
    }

    if (!strptr) {
        return 0xFFFFFFFF;
    }

    uint32_t c = *strptr++;

    if (!c) {
        return 0xFFFFFFFF;
    }

    if (chars) {
        (*chars)++;
    }

    uint32_t extra;

    if ((c & 0xFE) == 0xFC) {
        c &= 0x01;
        extra = 5;
    } else if ((c & 0xFC) == 0xF8) {
        c &= 0x03;
        extra = 4;
    } else if ((c & 0xF8) == 0xF0) {
        c &= 0x07;
        extra = 3;
    } else if ((c & 0xF0) == 0xE0) {
        c &= 0x0F;
        extra = 2;
    } else if ((c & 0xE0) == 0xC0) {
        c &= 0x1F;
        extra = 1;
    } else if (c & 0x80) {
        return 0x80000000;
    } else {
        return c;
    }

    for (uint32_t i = 0; i < extra; i++) {
        uint8_t next = *strptr++;

        if (!next) {
            return 0xFFFFFFFF;
        }

        if (chars) {
            (*chars)++;
        }

        if ((next & 0xC0) != 0x80) {
            return 0x80000000;
        }

        c = (c << 6) | (next & 0x3F);
    }

    return c;
}

// VERIFY: 1fa47a7d manual
// OFFSET: [0x0047BE50, 0x00852320]
char* sputu8(uint32_t c, char* strptr) {
    if (!strptr) {
        return strptr;
    }

    if (c >= 0x80) {
        if (c >= 0x800) {
            if (c >= 0x10000) {
                if (c >= 0x200000) {
                    if (c >= 0x400000) {
                        if (c >= 0x80000000) {
                            *strptr = '\0';
                            return strptr;
                        }

                        *strptr++ = static_cast<char>((c >> 30) | 0xFC);
                        *strptr++ = static_cast<char>(((c >> 24) & 0x3F) | 0x80);
                    } else {
                        *strptr++ = static_cast<char>((c >> 24) | 0xF8);
                    }

                    *strptr++ = static_cast<char>(((c >> 18) & 0x3F) | 0x80);
                } else {
                    *strptr++ = static_cast<char>((c >> 18) | 0xF0);
                }

                *strptr++ = static_cast<char>(((c >> 12) & 0x3F) | 0x80);
            } else {
                *strptr++ = static_cast<char>((c >> 12) | 0xE0);
            }

            *strptr++ = static_cast<char>(((c >> 6) & 0x3F) | 0x80);
        } else {
            *strptr++ = static_cast<char>((c >> 6) | 0xC0);
        }

        *strptr++ = static_cast<char>((c & 0x3F) | 0x80);
    } else {
        *strptr++ = static_cast<char>(c);
    }

    *strptr = '\0';
    return strptr;
}
