#include <cstring>
#include "Engine/Source/Gx/CGxDeviceOpenGL/CGxVboBroker1.h"
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: 61f23a06 manual
// OFFSET: 0x0068B2A0
CGxVboBroker::~CGxVboBroker() {
}

// VERIFY: 8d639573 manual
// OFFSET: 0x0068A590
CGxVboBroker1::CGxVboBroker1(GLenum target, uint32_t maxBits, CGxDeviceOpenGl* device) {
    this->m_target = target;
    this->m_maxBits = maxBits;
    this->m_device = device;
    memset(this->m_slotCount, 0, sizeof(this->m_slotCount));
    memset(this->m_slotCursor, 0, sizeof(this->m_slotCursor));
    memset(this->m_unkD8, 0, sizeof(this->m_unkD8));
    memset(this->m_vbos, 0xFF, sizeof(this->m_vbos));

    uint32_t total = 0;

    for (uint32_t bits = 12; bits < maxBits; bits++) {
        uint32_t count = 1200;

        if (bits > 12) {
            count = 1200 >> (bits - 10);
        }

        if (bits == 18) {
            count = 32;
        }

        this->m_slotCount[bits] = count;

        GLuint* vbos = this->m_vbos[bits];
        glGenBuffersARB(count, vbos);

        uint32_t size = 1 << bits;

        for (uint32_t i = 0; i < count; i++) {
            glBindBufferARB(this->m_target, *vbos);
            glBufferDataARB(this->m_target, size, nullptr, GL_STREAM_DRAW_ARB);
            vbos++;
        }

        total += count * size;
        DebugLog("VBO broker created %d slots at size %d => %d bytes on row, %d running total\n", count, size, count * size, total);
    }

    DebugLog("VBO KB total = %d\n", total >> 10);
    glBindBufferARB(this->m_target, 0);
    this->m_lastVbo = 0xFFFFFFFF;
    this->m_selectCount = 0;
}

// VERIFY: 21196910 auto
// OFFSET: [0x0068A700, 0x0068B2D0]
CGxVboBroker1::~CGxVboBroker1() {
    glBindBufferARB(this->m_target, 0);

    for (uint32_t bits = 0; bits < 25; bits++) {
        glDeleteBuffersARB(this->m_slotCount[bits], this->m_vbos[bits]);
    }
}

// VERIFY: 32e1a3b0 auto
// OFFSET: 0x0068A760
void CGxVboBroker1::SelectVbo(uint32_t size, GLuint& vbo, uint32_t& offset) {
    uint32_t bits = 1;

    if (size > 2) {
        do {
            bits++;
        } while ((1u << bits) < size);
    }

    if (bits < 12) {
        bits = 12;
    }

    uint32_t cursor = this->m_slotCursor[bits] + 1;

    if (cursor == this->m_slotCount[bits]) {
        cursor = 0;
    }

    this->m_slotCursor[bits] = cursor;

    GLuint selected = this->m_vbos[bits][cursor];
    this->m_selectCount++;
    this->m_lastVbo = selected;
    vbo = selected;
    offset = 0;
}

// VERIFY: 68b79f5f auto
// OFFSET: 0x0068A7D0
void CGxVboBroker1::GetLastSelectedVbo(GLuint& vbo, uint32_t& offset) {
    vbo = this->m_lastVbo;
    offset = 0;
}

void CGxVboBroker1::UnlockVbo(uint32_t offset, uint32_t size) {
}

void CGxVboBroker1::Reset() {
}
