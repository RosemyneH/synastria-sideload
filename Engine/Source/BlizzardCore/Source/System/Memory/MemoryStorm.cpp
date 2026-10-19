#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/System/Memory/MemoryStorm.h"
#include "Engine/Source/Storm/H/storm.h"
#include "Engine/Source/BlizzardCore/Source/System/Using/Using.h"

static System_Memory::UsingMemory s_usingMemory;

// VERIFY: 7ea46aa7 manual
// OFFSET: 0x0042E390
void* Blizzard::Memory::Allocate(uint32_t bytes) {
    return SMemAlloc(bytes, __FILE__, __LINE__, 0);
}

// VERIFY: 7d7f573d manual
// OFFSET: 0x0042E3D0
void* operator new(size_t bytes, uint32_t flags, const char* fileName, int32_t lineNumber) {
    return SMemAlloc(static_cast<uint32_t>(bytes), fileName, lineNumber, (flags & 1) ? SMEM_FLAG_ZEROMEMORY : 0);
}

// VERIFY: 2fffb18b manual
// OFFSET: 0x004284B0
void operator delete(void* ptr, uint32_t flags, const char* fileName, int32_t lineNumber) {
    if (ptr) {
        SMemFree(ptr, fileName, lineNumber, (flags & 1) ? SMEM_FLAG_ZEROMEMORY : 0);
    }
}

// VERIFY: af4aec38 manual
// OFFSET: 0x00428450
void* Blizzard::Memory::Allocate(uint32_t bytes, uint32_t flags, const char* fileName, int32_t lineNumber) {
    return SMemAlloc(bytes, fileName, lineNumber, (flags & 1) ? SMEM_FLAG_ZEROMEMORY : 0);
}

// VERIFY: 9ce2e935 manual
// OFFSET: 0x0042E3B0
void Blizzard::Memory::Free(void* ptr) {
    SMemFree(ptr, __FILE__, __LINE__, 0);
}

// VERIFY: 7118b5bd manual
// OFFSET: 0x00428480
void Blizzard::Memory::Free(void* ptr, uint32_t flags, const char* fileName, int32_t lineNumber) {
    SMemFree(ptr, fileName, lineNumber, (flags & 1) ? SMEM_FLAG_ZEROMEMORY : 0);
}

// VERIFY: 343a02cf auto
// OFFSET: 0x005EEB70
void System_Memory::ShutdownProc() {
}

// INLINED
System_Memory::UsingMemory::UsingMemory() {
    Blizzard::Using::StartUsing("Blizzard::Memory");
    System_Using::Intern("Blizzard::Memory")->shutdownproc = System_Memory::ShutdownProc;
}

// INLINED
System_Memory::UsingMemory::~UsingMemory() {
    Blizzard::Using::StopUsing("Blizzard::Memory");
}
