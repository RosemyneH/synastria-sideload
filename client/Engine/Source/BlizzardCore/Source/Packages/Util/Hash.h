#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_UTIL_HASH_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_UTIL_HASH_H

#include <cstddef>
#include <cstdint>

namespace Blizzard {
namespace Util {

uint32_t Hash32I(const char* str, uint32_t length);
void hashlittle2(const void* key, size_t length, uint32_t* pc, uint32_t* pb);

}
}

#endif
