#ifndef ENGINE_SOURCE_OS_TLS_H
#define ENGINE_SOURCE_OS_TLS_H

#include <cstdint>

uint32_t OsTlsAlloc();
void OsTlsFree(uint32_t index);
void* OsTlsGetValue(uint32_t index);
int32_t OsTlsSetValue(uint32_t index, void* value);

#endif
