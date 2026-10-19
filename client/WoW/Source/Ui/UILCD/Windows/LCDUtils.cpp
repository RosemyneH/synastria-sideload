#include "WoW/Source/Ui/UILCD/Windows/LCDUtils.h"
#include <malloc.h>
#include <cstring>
#include "Engine/Source/BLPFile/blp.h"

// STATIC: 0x00D41374
TSHashTable<BITMAPBYICON, HASHKEY_NONE> CLCDUtils::m_bitmapCache;

// VERIFY: 8f6ac590 manual
// OFFSET: 0x0084B400
void CLCDUtils::Shutdown() {
    CLCDUtils::m_bitmapCache.Clear();
}

// VERIFY: 4c56b30d manual
// OFFSET: 0x0084AF40
HBITMAP CLCDUtils::CreateBitmapFromBLPTex(const char* fileName, uint32_t mipLevel, C2iVector* size, int32_t top, int32_t left, int32_t bottom, int32_t right) {
    CBLPFile image;

    if (!image.Open(fileName, 0)) {
        image.Close();
        return nullptr;
    }

    uint8_t* data;
    uint32_t stride;
    image.Lock(PIXEL_ARGB8888, mipLevel, data, stride);
    int32_t width = right - left;
    int32_t height;
    HBITMAP bitmap;

    if (right == left || (height = bottom - top, bottom == top)) {
        uint32_t mipHeight = image.m_header.m_height >> mipLevel;
        uint32_t mipWidth = image.m_header.m_width >> mipLevel;
        bitmap = CreateBitmap(mipWidth <= 1 ? 1 : mipWidth, mipHeight <= 1 ? 1 : mipHeight, 1, 32, data);
        size->x = (image.m_header.m_width >> mipLevel) <= 1 ? 1 : image.m_header.m_width >> mipLevel;
        size->y = (image.m_header.m_height >> mipLevel) <= 1 ? 1 : image.m_header.m_height >> mipLevel;
    } else {
        uint8_t* bits = static_cast<uint8_t*>(alloca(4 * (top < bottom && left < right ? width * height : 0)));
        memset(bits, 0, 4 * (top < bottom && left < right ? width * height : 0));
        stride >>= 2;
        uint8_t* dest = bits;

        for (int32_t row = 0; row < height; row++) {
            memcpy(dest, data + 4 * left + 4 * stride * (row + top), 4 * width);
            dest += 4 * width;
        }

        bitmap = CreateBitmap(width, height, 1, 32, bits);
        size->x = width;
        size->y = height;
    }

    image.Unlock(mipLevel);
    image.Close();
    return bitmap;
}

// VERIFY: 9aeb271d manual
// OFFSET: 0x0084BFE0
HBITMAP CLCDUtils::GetBitmapForIcon(int32_t icon, C2iVector* size) {
    BITMAPBYICON* entry = CLCDUtils::m_bitmapCache.Ptr(icon, HASHKEY_NONE());

    if (entry) {
        *size = entry->m_size;
        return static_cast<HBITMAP>(entry->m_bitmap);
    }

    HBITMAP bitmap = CLCDUtils::CreateBitmapFromBLPTex("Interface\\MINIMAP\\POIICONS.BLP", 0, size, 18 * (icon / 14) + 1, 18 * (icon % 14) + 1, 18 * (icon / 14) + 17, 18 * (icon % 14) + 17);
    entry = CLCDUtils::m_bitmapCache.New(icon, HASHKEY_NONE(), 0, 0);
    entry->m_bitmap = bitmap;
    entry->m_size = *size;
    return bitmap;
}
