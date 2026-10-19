#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_STRING_PATH_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_STRING_PATH_H

#include <cstdint>

namespace System_String {

const char* FindExtension(const char* str, int32_t offset);

}

namespace Blizzard {
namespace String {

const char* FindExtension(const char* str);
const char* FindExtensionWithLeadingDot(const char* str);
const char* FindFilename(const char* str);
char* FindPathSeparatorReverse(char* str, uint32_t maxchars);
char FirstPathSeparator(const char* str);
void ForceTrailingSeparator(char* dst, int32_t count, char separator);
void JoinPath(char* dst, int32_t count, const char* dir, const char* file);
void MakeBackslashPath(const char* src, char* dst, int32_t count);
char* MakeConsistentPath(const char* src, char* dst, int32_t count);
void MakeUnivPath(const char* src, char* dst, int32_t count);

}
}

#endif
