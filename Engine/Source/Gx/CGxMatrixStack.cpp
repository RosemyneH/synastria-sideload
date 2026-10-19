#include <cstdint>
#include "Engine/Source/Gx/CGxMatrixStack.h"

// VERIFY: ff34dc12 manual
// OFFSET: 0x0057C340
void CGxMatrixStack::Identity() {
    if (this->m_flags[this->m_level] & F_Identity) {
        return;
    }

    C44Matrix& m = this->m_mtx[this->m_level];
    m.d3 = 1.0f;
    m.c2 = 1.0f;
    m.b1 = 1.0f;
    m.a0 = 1.0f;
    m.d2 = 0.0f;
    m.d1 = 0.0f;
    m.d0 = 0.0f;
    m.c3 = 0.0f;
    m.c1 = 0.0f;
    m.c0 = 0.0f;
    m.b3 = 0.0f;
    m.b2 = 0.0f;
    m.b0 = 0.0f;
    m.a3 = 0.0f;
    m.a2 = 0.0f;
    m.a1 = 0.0f;

    this->m_dirty = 1;
    this->m_flags[this->m_level] = F_Identity;
}

// VERIFY: b4e2821f manual
// OFFSET: 0x00683B90
CGxMatrixStack::CGxMatrixStack() {
    this->m_level = 0;
    this->m_dirty = 0;
    this->m_flags[0] = F_Identity;
    this->m_flags[1] = 0;
    this->m_flags[2] = 0;
    this->m_flags[3] = 0;
}
