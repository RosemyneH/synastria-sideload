#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_TYPES_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_TYPES_H

#include <cstdint>

namespace Blizzard {
namespace Mopaq {

struct CompressionAlgorithm {
    /* 0000 */ uint8_t m_type;
    /* 0001 */ uint8_t m_level;
};

enum AuthResult {
    AUTH_UNKNOWN = 0,
    AUTH_FAILED = 1,
    AUTH_VERIFIED = 5,
};

}
}

#endif
