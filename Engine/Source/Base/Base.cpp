#include "Engine/Source/Base/Base.h"
#include "Engine/Source/Base/FileCache.h"
#include "Engine/Source/Base/Handle.h"
#include "Engine/Source/Base/Prop.h"
#include "Engine/Source/Base/RCString.h"

void BaseDestroyContext() {
    HandleDestroy();
}

// VERIFY: 086892a1 manual
// OFFSET: 0x0047B750
void BaseDestroyGlobal() {
    BaseFileDestroy();
    PropDestroy();
    CStringManager::DestroyManager();
}

// VERIFY: 9f51e5ca auto
// OFFSET: 0x0047B760
void BaseInitializeContext() {
    HandleInitialize();
}

// VERIFY: 948397bb manual
// OFFSET: 0x0047B740
void BaseInitializeGlobal() {
    PropInitialize();
    BaseFileInitialize();
}
