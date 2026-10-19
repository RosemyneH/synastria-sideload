#ifndef ENGINE_SOURCE_OS_OSSECURERANDOM_H
#define ENGINE_SOURCE_OS_OSSECURERANDOM_H

#include <cstdint>

void IOsSecureRandom(uint8_t* buffer, uint32_t size);
void OsSecureRandom(uint8_t* buffer, uint32_t size);

#endif
