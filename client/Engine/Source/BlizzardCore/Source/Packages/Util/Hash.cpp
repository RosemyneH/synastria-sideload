#include <cctype>
#include <cstddef>
#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/Packages/Util/Hash.h"

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define mix(a, b, c) \
    { \
        a -= c; a ^= rot(c, 4); c += b; \
        b -= a; b ^= rot(a, 6); a += c; \
        c -= b; c ^= rot(b, 8); b += a; \
        a -= c; a ^= rot(c, 16); c += b; \
        b -= a; b ^= rot(a, 19); a += c; \
        c -= b; c ^= rot(b, 4); b += a; \
    }

#define final(a, b, c) \
    { \
        c ^= b; c -= rot(b, 14); \
        a ^= c; a -= rot(c, 11); \
        b ^= a; b -= rot(a, 25); \
        c ^= b; c -= rot(b, 16); \
        a ^= c; a -= rot(c, 4); \
        b ^= a; b -= rot(a, 14); \
        c ^= b; c -= rot(b, 24); \
    }

// VERIFY: 32afc97a manual
// OFFSET: 0x00425DA0
uint32_t Blizzard::Util::Hash32I(const char* str, uint32_t length) {
    uint32_t hash = 2166136261u;

    while (length) {
        length--;
        hash = 16777619u * (hash ^ toupper(static_cast<uint8_t>(*str++)));
    }

    return hash;
}

// VERIFY: 0fd5aa8b manual
// OFFSET: 0x00425DE0
void Blizzard::Util::hashlittle2(const void* key, size_t length, uint32_t* pc, uint32_t* pb) {
    uint32_t a;
    uint32_t b;
    uint32_t c;

    a = b = c = 0xDEADBEEF + static_cast<uint32_t>(length) + *pc;
    c += *pb;

    if ((reinterpret_cast<uintptr_t>(key) & 0x3) == 0) {
        const uint32_t* k = static_cast<const uint32_t*>(key);

        while (length > 12) {
            a += k[0];
            b += k[1];
            c += k[2];
            mix(a, b, c);
            length -= 12;
            k += 3;
        }

        switch (length) {
            case 12: c += k[2]; b += k[1]; a += k[0]; break;
            case 11: c += k[2] & 0xFFFFFF; b += k[1]; a += k[0]; break;
            case 10: c += k[2] & 0xFFFF; b += k[1]; a += k[0]; break;
            case 9: c += k[2] & 0xFF; b += k[1]; a += k[0]; break;
            case 8: b += k[1]; a += k[0]; break;
            case 7: b += k[1] & 0xFFFFFF; a += k[0]; break;
            case 6: b += k[1] & 0xFFFF; a += k[0]; break;
            case 5: b += k[1] & 0xFF; a += k[0]; break;
            case 4: a += k[0]; break;
            case 3: a += k[0] & 0xFFFFFF; break;
            case 2: a += k[0] & 0xFFFF; break;
            case 1: a += k[0] & 0xFF; break;
            case 0: *pc = c; *pb = b; return;
        }
    } else if ((reinterpret_cast<uintptr_t>(key) & 0x1) == 0) {
        const uint16_t* k = static_cast<const uint16_t*>(key);
        const uint8_t* k8;

        while (length > 12) {
            a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
            b += k[2] + (static_cast<uint32_t>(k[3]) << 16);
            c += k[4] + (static_cast<uint32_t>(k[5]) << 16);
            mix(a, b, c);
            length -= 12;
            k += 6;
        }

        k8 = reinterpret_cast<const uint8_t*>(k);

        switch (length) {
            case 12:
                c += k[4] + (static_cast<uint32_t>(k[5]) << 16);
                b += k[2] + (static_cast<uint32_t>(k[3]) << 16);
                a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
                break;
            case 11:
                c += static_cast<uint32_t>(k8[10]) << 16;
            case 10:
                c += k[4];
                b += k[2] + (static_cast<uint32_t>(k[3]) << 16);
                a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
                break;
            case 9:
                c += k8[8];
            case 8:
                b += k[2] + (static_cast<uint32_t>(k[3]) << 16);
                a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
                break;
            case 7:
                b += static_cast<uint32_t>(k8[6]) << 16;
            case 6:
                b += k[2];
                a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
                break;
            case 5:
                b += k8[4];
            case 4:
                a += k[0] + (static_cast<uint32_t>(k[1]) << 16);
                break;
            case 3:
                a += static_cast<uint32_t>(k8[2]) << 16;
            case 2:
                a += k[0];
                break;
            case 1:
                a += k8[0];
                break;
            case 0:
                *pc = c;
                *pb = b;
                return;
        }
    } else {
        const uint8_t* k = static_cast<const uint8_t*>(key);

        while (length > 12) {
            a += k[0];
            a += static_cast<uint32_t>(k[1]) << 8;
            a += static_cast<uint32_t>(k[2]) << 16;
            a += static_cast<uint32_t>(k[3]) << 24;
            b += k[4];
            b += static_cast<uint32_t>(k[5]) << 8;
            b += static_cast<uint32_t>(k[6]) << 16;
            b += static_cast<uint32_t>(k[7]) << 24;
            c += k[8];
            c += static_cast<uint32_t>(k[9]) << 8;
            c += static_cast<uint32_t>(k[10]) << 16;
            c += static_cast<uint32_t>(k[11]) << 24;
            mix(a, b, c);
            length -= 12;
            k += 12;
        }

        switch (length) {
            case 12: c += static_cast<uint32_t>(k[11]) << 24;
            case 11: c += static_cast<uint32_t>(k[10]) << 16;
            case 10: c += static_cast<uint32_t>(k[9]) << 8;
            case 9: c += k[8];
            case 8: b += static_cast<uint32_t>(k[7]) << 24;
            case 7: b += static_cast<uint32_t>(k[6]) << 16;
            case 6: b += static_cast<uint32_t>(k[5]) << 8;
            case 5: b += k[4];
            case 4: a += static_cast<uint32_t>(k[3]) << 24;
            case 3: a += static_cast<uint32_t>(k[2]) << 16;
            case 2: a += static_cast<uint32_t>(k[1]) << 8;
            case 1: a += k[0]; break;
            case 0: *pc = c; *pb = b; return;
        }
    }

    final(a, b, c);
    *pc = c;
    *pb = b;
}
