#ifndef ENGINE_SOURCE_OS_OSMEMORY_H
#define ENGINE_SOURCE_OS_OSMEMORY_H

#include <cstddef>

#if defined(WOWDECOMP_ARCH_64)
void* OsAllocNear(const void* anchor, size_t size);
void OsFreeNear(void* ptr);
#endif

#endif
