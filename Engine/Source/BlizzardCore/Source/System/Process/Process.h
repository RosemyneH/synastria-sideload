#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_PROCESS_PROCESS_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_PROCESS_PROCESS_H

#include <cstdint>

namespace Blizzard {
namespace Process {

bool GetProcessDirectory(char* buffer, int32_t buffersize);
bool GetProcessPath(char* buffer, int32_t buffersize);
bool LaunchChildProcessWithArguments(const char* path, const char* arguments, void (*callback)(void*), void* param);
void Sleep(uint32_t milliseconds);

}
}

#endif
