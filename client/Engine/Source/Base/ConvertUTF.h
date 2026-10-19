#ifndef ENGINE_SOURCE_BASE_CONVERTUTF_H
#define ENGINE_SOURCE_BASE_CONVERTUTF_H

#include <cstdint>

int32_t ConvertUTF16toUTF8(char* dst, uint32_t dstmaxchars, const uint16_t* src, uint32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars);
int32_t ConvertUTF8toUTF16(uint16_t* dst, uint32_t dstmaxchars, const char* src, uint32_t srcmaxchars, uint32_t* dstchars, uint32_t* srcchars);
uint32_t sgetu8(const uint8_t* strptr, int32_t* chars);
char* sputu8(uint32_t c, char* strptr);

#endif
