#include "Engine/Source/Gx/CGxDevice/EmergencyMem.h"

// INLINED
EmergencyMem::EmergencyMem() : m_lock(false) {
}

// VERIFY: 81a07d3f auto
// OFFSET: 0x00682C40
void EmergencyMem::Unlock() {
    this->m_lock = false;
}

// VERIFY: 047bfe60 manual
// OFFSET: 0x00685E90
void* EmergencyMem::Lock(uint32_t size) {
    if (this->m_data.Count() < size) {
        this->m_data.SetCount(size);
    }

    void* data = this->m_data.Ptr();
    this->m_lock = true;

    return data;
}
