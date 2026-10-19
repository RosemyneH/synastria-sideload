#ifndef ENGINE_SOURCE_BASE_FILECACHE_H
#define ENGINE_SOURCE_BASE_FILECACHE_H

#include <cstdint>
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "Engine/Source/Storm/H/stpl.h"

struct PrefetchNode : TSHashObject<PrefetchNode, HASHKEY_STRI> {
    /* 0018 */ void* buffer;
    /* 001C */ uint32_t size;
    /* 0020 */ SOVERLAPPED overlapped;
    /* 0028 */ int32_t refCount;

    PrefetchNode();
    ~PrefetchNode();
};

struct UncachableNode : TSHashObject<UncachableNode, HASHKEY_STRI> {
};

void BaseFileDestroy();
void BaseFileInitialize();

#endif
