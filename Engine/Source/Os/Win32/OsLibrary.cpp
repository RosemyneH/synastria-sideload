#if defined(WOWDECOMP_ARCH_32)

#include <windows.h>
#include "Engine/Source/Os/OsLibrary.h"

// VERIFY: e2181d2b auto
// OFFSET: 0x0086C4E0
void* OsLoadLibrary(const char* fileName) {
    return LoadLibraryA(fileName);
}

// VERIFY: 59255fc2 auto
// OFFSET: 0x0086C4F0
void OsFreeLibrary(void* module) {
    FreeLibrary(static_cast<HMODULE>(module));
}

// VERIFY: 94d39dd2 auto
// OFFSET: 0x0086C500
void* OsGetLibraryEntryPoint(void* module, const char* name) {
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(module), name));
}

#endif
