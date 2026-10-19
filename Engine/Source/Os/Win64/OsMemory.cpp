#if defined(WOWDECOMP_ARCH_64)

#include <cstdint>
#include <windows.h>
#include "Engine/Source/Os/OsMemory.h"

static const uintptr_t s_nearRange = 0x60000000;

void* OsAllocNear(const void* anchor, size_t size) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    uintptr_t granularity = info.dwAllocationGranularity;
    uintptr_t bytes = (size + granularity - 1) & ~(granularity - 1);
    uintptr_t center = reinterpret_cast<uintptr_t>(anchor);
    uintptr_t minimum = reinterpret_cast<uintptr_t>(info.lpMinimumApplicationAddress);
    uintptr_t maximum = reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress);

    if (center > minimum + s_nearRange) {
        minimum = center - s_nearRange;
    }

    if (center + s_nearRange < maximum) {
        maximum = center + s_nearRange;
    }

    MEMORY_BASIC_INFORMATION region;
    uintptr_t address = center;

    while (address < maximum && VirtualQuery(reinterpret_cast<void*>(address), &region, sizeof(region))) {
        uintptr_t base = reinterpret_cast<uintptr_t>(region.BaseAddress);
        uintptr_t end = base + region.RegionSize;

        if (region.State == MEM_FREE) {
            uintptr_t candidate = ((base > address ? base : address) + granularity - 1) & ~(granularity - 1);

            if (candidate + bytes <= end && candidate + bytes <= maximum) {
                void* ptr = VirtualAlloc(reinterpret_cast<void*>(candidate), bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

                if (ptr) {
                    return ptr;
                }
            }
        }

        address = end;
    }

    address = center;

    while (address > minimum && VirtualQuery(reinterpret_cast<void*>(address - 1), &region, sizeof(region))) {
        uintptr_t base = reinterpret_cast<uintptr_t>(region.BaseAddress);
        uintptr_t end = base + region.RegionSize;

        if (end > address) {
            end = address;
        }

        if (region.State == MEM_FREE && end - base >= bytes) {
            uintptr_t candidate = (end - bytes) & ~(granularity - 1);

            if (candidate >= base && candidate >= minimum) {
                void* ptr = VirtualAlloc(reinterpret_cast<void*>(candidate), bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

                if (ptr) {
                    return ptr;
                }
            }
        }

        address = base;
    }

    return nullptr;
}

void OsFreeNear(void* ptr) {
    if (ptr) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

#endif
