#include <new>
#include "Engine/Source/Gx/CGxDeviceD3d/CGxDeviceD3d.h"
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: 5a6cdc5d manual
// OFFSET: 0x00689EF0
CGxDevice* CreateCGxDeviceD3d() {
    void* mem = SMemAlloc(sizeof(CGxDeviceD3d), __FILE__, __LINE__, 0);

    if (!mem) {
        return nullptr;
    }

    return new (mem) CGxDeviceD3d();
}
