#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_VERSION_VERSION_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_VERSION_VERSION_H

#include <cstdint>

namespace Blizzard {
namespace Version {

struct FileVersion {
    /* 0000 */ uint32_t m_major;
    /* 0004 */ uint32_t m_minor;
    /* 0008 */ uint32_t m_build;
    /* 000C */ uint32_t m_revision;
};

bool GetFileVersion(const char* path, FileVersion& version);

}
}

#endif
