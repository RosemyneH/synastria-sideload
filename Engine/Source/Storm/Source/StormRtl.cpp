#include "Engine/Source/Storm/H/storm.h"

// VERIFY: ff119005 auto
// OFFSET: 0x0076E470
void StormRtlDestroy() {
    SErrSetLogCallback(nullptr);
    SMemDestroy();
    SErrDestroy();
    SLogDestroy();
}

// VERIFY: 398fde50 manual
// OFFSET: 0x0076E490
void StormRtlInitialize() {
    SLogInitialize();
    SMemInitialize();
}
