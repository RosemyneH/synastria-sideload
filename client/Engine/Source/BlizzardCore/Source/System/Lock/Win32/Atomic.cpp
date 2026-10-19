#if defined(WOWDECOMP_ARCH_32)

#include <cstdint>
#include <windows.h>
#include "Engine/Source/BlizzardCore/Source/System/Lock/Lock.h"

// VERIFY: 73418d0d auto
// OFFSET: 0x00428300
int32_t Blizzard::Lock::Atomic::Increment() {
    return InterlockedIncrement(reinterpret_cast<volatile LONG*>(&this->m_value));
}

// VERIFY: 4de014a2 auto
// OFFSET: 0x00428310
int32_t Blizzard::Lock::Atomic::Decrement() {
    return InterlockedDecrement(reinterpret_cast<volatile LONG*>(&this->m_value));
}

#endif
