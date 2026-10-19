#include <cstdint>
#include "Engine/Source/Base/FileCache.h"
#include "Engine/Source/Storm/H/stpl.h"
#include "Engine/Source/Storm/H/storm.h"
#include "Engine/Source/Storm/Source/SLock.h"

// STATIC: 0x00B415F0
static TSHashTable<PrefetchNode, HASHKEY_STRI> s_activeFiles;

// STATIC: 0x00B41618
static TSHashTable<UncachableNode, HASHKEY_STRI> s_uncachableFiles;

// STATIC: 0x00B41640
static SCritSect s_critSect;

static void STORMAPI IBaseFileWaitForLoad(SOVERLAPPED* overlapped) {
}

// INLINED
PrefetchNode::PrefetchNode() {
    this->buffer = nullptr;
    this->refCount = 0;
    SFile::CreateOverlapped(&this->overlapped);
}

// INLINED
PrefetchNode::~PrefetchNode() {
    if (this->buffer) {
        IBaseFileWaitForLoad(&this->overlapped);
        SMemFree(this->buffer, __FILE__, __LINE__, 0);
    }

    SFile::DestroyOverlapped(&this->overlapped);
}

void BaseFileInitialize() {
}

// VERIFY: 3f61d363 manual
// OFFSET: 0x0047C600
void BaseFileDestroy() {
    s_critSect.Enter();
    s_activeFiles.Clear();
    s_uncachableFiles.Clear();
    s_critSect.Leave();
}
