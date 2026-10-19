#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_NEWZEROFILL_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_NEWZEROFILL_H

#include <new>
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: dc87be8b manual
// OFFSET: 0x0044C460
template <class T>
T* NewZerofill() {
    void* memory = SMemAlloc(sizeof(T), __FILE__, __LINE__, SMEM_FLAG_ZEROMEMORY);

    if (!memory) {
        return nullptr;
    }

    return new (memory) T();
}

#endif
