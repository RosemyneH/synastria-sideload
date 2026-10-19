#ifndef ENGINE_SOURCE_BASE_SORT_H
#define ENGINE_SOURCE_BASE_SORT_H

#include <cstdint>

void quicksort(void* base, uint32_t num, uint32_t width, int32_t (*comp)(const void*, const void*, void*), void* context);

#endif
