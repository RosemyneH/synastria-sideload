#ifndef ENGINE_SOURCE_JPEG_JMEMNOBS_H
#define ENGINE_SOURCE_JPEG_JMEMNOBS_H

#include <cstddef>

void jpeg_set_malloc(void* (*allocFunc)(size_t bytes));
void jpeg_set_free(void (*freeFunc)(void* ptr));

#endif
