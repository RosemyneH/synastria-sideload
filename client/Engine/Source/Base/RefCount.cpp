#include "Engine/Source/Base/RefCount.h"

// INLINED
TRefCnt::TRefCnt() {
    this->m_refcnt = 0;
}

// VERIFY: 0b4d6aef manual
// OFFSET: 0x00407920
void TRefCnt::DeleteSelf() {
    delete this;
}

// VERIFY: 38418086 manual
// OFFSET: 0x00480E40
TRefCnt::~TRefCnt() {
}
