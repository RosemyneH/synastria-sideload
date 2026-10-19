#include <cstdint>
#include "Engine/Source/Base/Handle.h"
#include "Engine/Source/Storm/H/storm.h"

// INLINED
CHandleObject::CHandleObject() {
    this->m_refcount = 0;
}

// INLINED
CHandleObject::~CHandleObject() {
}

// INLINED
const char* CHandleObject::GetObjectName() {
    return nullptr;
}

// VERIFY: baa00fb6 auto
// OFFSET: 0x0047BF20
int32_t HandleCompare(HOBJECT a, HOBJECT b) {
    return static_cast<int32_t>(a - b);
}

// VERIFY: 436d38d9 manual
// OFFSET: 0x0047BF30
void HandleClose(HOBJECT handle) {
    CHandleObject* object = HandleDereference(handle);

    if (object->m_refcount-- == 1) {
        delete object;
    }
}

// VERIFY: 96ec39c1 auto
// OFFSET: 0x0047BF50
HOBJECT HandleCreate(CHandleObject* ptr, const char* handleName) {
    if (!ptr) {
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return nullptr;
    }

    ptr->m_refcount++;
    return reinterpret_cast<HOBJECT>(ptr);
}

// INLINED
CHandleObject* HandleDereference(HOBJECT handle) {
    return reinterpret_cast<CHandleObject*>(handle);
}

// VERIFY: 1d1aba75 manual
// OFFSET: 0x0047BF70
HOBJECT HandleDuplicate(HOBJECT handle) {
    if (handle) {
        HandleDereference(handle)->m_refcount++;
    }

    return handle;
}

void HandleDestroy() {
}

void HandleInitialize() {
}
