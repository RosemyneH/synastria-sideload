#ifndef ENGINE_SOURCE_COMSAT_COMSATHASH_H
#define ENGINE_SOURCE_COMSAT_COMSATHASH_H

#include <cstddef>
#include <cstdint>

namespace ComSat {

uint32_t Hash32(const void* data, uint32_t length, uint32_t seed);

}

#endif
