#include <cstdint>
#include "Engine/Source/Base/Sort.h"

// INLINED
static void swap(char* a, char* b, uint32_t width) {
    if (a != b) {
        while (width--) {
            char tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
    }
}

// VERIFY: 88e0c819 manual
// OFFSET: 0x0047B770
static void shortsort(char* lo, char* hi, uint32_t width, int32_t (*comp)(const void*, const void*, void*), void* context) {
    while (hi > lo) {
        char* max = lo;

        for (char* p = lo + width; p <= hi; p += width) {
            if (comp(p, max, context) > 0) {
                max = p;
            }
        }

        swap(max, hi, width);
        hi -= width;
    }
}

// VERIFY: 288f54ba manual
// OFFSET: 0x0047B800
void quicksort(void* base, uint32_t num, uint32_t width, int32_t (*comp)(const void*, const void*, void*), void* context) {
    char* lostk[30];
    char* histk[30];

    if (num < 2 || width == 0) {
        return;
    }

    int32_t stkptr = 0;
    char* lo = static_cast<char*>(base);
    char* hi = static_cast<char*>(base) + width * (num - 1);

    for (;;) {
        uint32_t size = static_cast<uint32_t>(hi - lo) / width + 1;

        if (size <= 8) {
            shortsort(lo, hi, width, comp, context);
        } else {
            char* mid = lo + (size / 2) * width;
            swap(mid, lo, width);

            char* l = lo;
            char* h = hi + width;

            for (;;) {
                do {
                    l += width;
                } while (l <= hi && comp(l, lo, context) <= 0);

                do {
                    h -= width;
                } while (h > lo && comp(h, lo, context) >= 0);

                if (h < l) {
                    break;
                }

                swap(l, h, width);
            }

            swap(lo, h, width);

            if (h - 1 - lo >= hi - l) {
                if (lo + width < h) {
                    lostk[stkptr] = lo;
                    histk[stkptr] = h - width;
                    stkptr++;
                }

                if (l < hi) {
                    lo = l;
                    continue;
                }
            } else {
                if (l < hi) {
                    lostk[stkptr] = l;
                    histk[stkptr] = hi;
                    stkptr++;
                }

                if (lo + width < h) {
                    hi = h - width;
                    continue;
                }
            }
        }

        stkptr--;

        if (stkptr < 0) {
            return;
        }

        lo = lostk[stkptr];
        hi = histk[stkptr];
    }
}
