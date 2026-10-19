#ifndef ENGINE_SOURCE_BASE_OBJECTALLOC_H
#define ENGINE_SOURCE_BASE_OBJECTALLOC_H

#include <cstddef>
#include <cstdint>

int32_t ObjectAlloc(uint32_t heapId, uint32_t* memHandle, void** objectPtr, bool zero);
uint32_t ObjectAllocAddHeap(uint32_t objectSize, uint32_t objectsPerBlock, const char* name, bool zero);
void ObjectFree(uint32_t heapId, uint32_t memHandle);
void* ObjectPtr(uint32_t heapId, uint32_t memHandle);

#endif
