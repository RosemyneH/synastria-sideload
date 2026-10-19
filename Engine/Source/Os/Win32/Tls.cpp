#if defined(WOWDECOMP_ARCH_32)

#include <cstdint>
#include <windows.h>
#include "Engine/Source/Os/Tls.h"

// VERIFY: fe2dbf30 auto
// OFFSET: 0x0086D6B0
uint32_t OsTlsAlloc() {
    return TlsAlloc();
}

// VERIFY: ba10ce28 auto
// OFFSET: 0x0086D6C0
void OsTlsFree(uint32_t index) {
    TlsFree(index);
}

// VERIFY: ca8e348e auto
// OFFSET: 0x0086D6D0
void* OsTlsGetValue(uint32_t index) {
    return TlsGetValue(index);
}

// VERIFY: ed514a94 auto
// OFFSET: 0x0086D6E0
int32_t OsTlsSetValue(uint32_t index, void* value) {
    return TlsSetValue(index, value);
}

#endif
