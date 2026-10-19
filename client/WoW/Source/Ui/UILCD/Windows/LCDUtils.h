#ifndef WOW_SOURCE_UI_UILCD_WINDOWS_LCDUTILS_H
#define WOW_SOURCE_UI_UILCD_WINDOWS_LCDUTILS_H

#include <cstdint>
#include <windows.h>
#include "Engine/Source/Storm/H/stpl.h"
#include "Engine/Source/Tempest/C2iVector.h"

struct BITMAPBYICON : public TSHashObject<BITMAPBYICON, HASHKEY_NONE> {
    /* 0018 */ void* m_bitmap;
    /* 001C */ C2iVector m_size;
};

class CLCDUtils {
    public:
    static TSHashTable<BITMAPBYICON, HASHKEY_NONE> m_bitmapCache;

    static void Shutdown();
    static HBITMAP CreateBitmapFromBLPTex(const char* fileName, uint32_t mipLevel, C2iVector* size, int32_t top, int32_t left, int32_t bottom, int32_t right);
    static HBITMAP GetBitmapForIcon(int32_t icon, C2iVector* size);
};

#endif
