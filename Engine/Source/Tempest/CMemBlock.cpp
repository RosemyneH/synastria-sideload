#include <cstddef>
#include <cstdint>
#include <cstring>
#include "Engine/Source/Tempest/CMemBlock.h"
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: ba4475ac manual
// OFFSET: 0x009844E0
void CMemBlock::Set32b_(uint32_t* dest, uint32_t value, uint32_t bytes) {
    uint32_t blocks = bytes >> 4;
    uint32_t remaining = (bytes >> 2) - 4 * (bytes >> 4);

    for (; blocks; blocks--) {
        dest[0] = value;
        dest[1] = value;
        dest[2] = value;
        dest[3] = value;
        dest += 4;
    }

    switch (remaining) {
        case 3:
            dest[2] = value;
        case 2:
            dest[1] = value;
        case 1:
            dest[0] = value;
    }
}

// VERIFY: f0c9764e auto
// OFFSET: 0x00984540
void CMemBlock::Set32b_(uint8_t* dest, uint8_t value, uint32_t bytes) {
    uint32_t head = static_cast<uint32_t>(-reinterpret_cast<intptr_t>(dest)) & 3;
    uint32_t tail = (bytes - head) & 3;
    uint32_t middle = bytes - tail - head;

    switch (head) {
        case 3:
            dest[2] = value;
        case 2:
            dest[1] = value;
        case 1:
            dest[0] = value;
    }

    uint8_t* cur = dest + head;

    if (middle) {
        CMemBlock::Set32b_(reinterpret_cast<uint32_t*>(cur), value | ((value | ((value | (value << 8)) << 8)) << 8), middle);
        cur += middle;
    }

    switch (tail) {
        case 3:
            cur[2] = value;
        case 2:
            cur[1] = value;
        case 1:
            cur[0] = value;
    }
}

// VERIFY: 24816e3e auto
// OFFSET: 0x009845D0
void CMemBlock::SetM_(uint8_t* dest, uint8_t value, uint32_t bytes) {
    if (bytes >= 16) {
        CMemBlock::Set32b_(dest, value, bytes);
        return;
    }

    switch (bytes) {
        case 15:
            dest[14] = value;
        case 14:
            dest[13] = value;
        case 13:
            dest[12] = value;
        case 12:
            dest[11] = value;
        case 11:
            dest[10] = value;
        case 10:
            dest[9] = value;
        case 9:
            dest[8] = value;
        case 8:
            dest[7] = value;
        case 7:
            dest[6] = value;
        case 6:
            dest[5] = value;
        case 5:
            dest[4] = value;
        case 4:
            dest[3] = value;
        case 3:
            dest[2] = value;
        case 2:
            dest[1] = value;
        case 1:
            dest[0] = value;
    }
}

// VERIFY: e1352d2f manual
// OFFSET: [0x00984670, 0x00984770]
CMemBlock::~CMemBlock() {
    if (this->m_block) {
        if (this->m_block != reinterpret_cast<uint8_t*>(-1)) {
            SMemFree(this->m_block, __FILE__, __LINE__, 0);
        }

        this->m_size = 0;
        this->m_blockSize = 0;
        this->m_data = nullptr;
        this->m_block = nullptr;
    }
}

// VERIFY: 2b816450 manual
// OFFSET: 0x009846B0
void CMemBlock::Constructor_(uint32_t size, uint32_t offset, const char* name, int32_t line) {
    if (this->m_block) {
        if (this->m_block != reinterpret_cast<uint8_t*>(-1)) {
            SMemFree(this->m_block, __FILE__, __LINE__, 0);
        }

        this->m_size = 0;
        this->m_blockSize = 0;
        this->m_data = nullptr;
        this->m_block = nullptr;
    }

    uint8_t* block = size + offset ? static_cast<uint8_t*>(SMemAlloc(size + offset, __FILE__, __LINE__, SMEM_FLAG_ZEROMEMORY)) : reinterpret_cast<uint8_t*>(-1);

    this->m_block = block;
    this->m_blockSize = size + offset;
    this->m_size = size;
    this->m_data = block + offset;
}

// VERIFY: 65708b0d auto
// OFFSET: 0x00984730
CMemBlock::CMemBlock(uint32_t size, uint32_t offset, const char* name, int32_t line) {
    this->m_block = nullptr;
    this->Constructor_(size, offset, name, line);
}

// VERIFY: c22b0826 manual
// OFFSET: 0x009847D0
bool CMemBlock::Resize(uint32_t size, bool keep) {
    if (size != this->m_size) {
        uint32_t offset = this->m_blockSize - this->m_size;
        uint32_t total = offset + size;
        uint8_t* block;

        if (this->m_block == reinterpret_cast<uint8_t*>(-1)) {
            block = static_cast<uint8_t*>(SMemAlloc(total, __FILE__, __LINE__, 0));
        } else {
            block = static_cast<uint8_t*>(SMemReAlloc(this->m_block, total, __FILE__, __LINE__, 0));
        }

        this->m_block = block;
        this->m_data = block + offset;

        if (size > this->m_size) {
            memset(this->m_data + this->m_size, 0, size - this->m_size);
        }

        this->m_blockSize = total;
        this->m_size = size;
    }

    if (!keep) {
        CMemBlock::SetM_(this->m_data, 0, this->m_size);
    }

    return true;
}
