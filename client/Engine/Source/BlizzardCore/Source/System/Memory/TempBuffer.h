#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_MEMORY_TEMPBUFFER_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_SYSTEM_MEMORY_TEMPBUFFER_H

#include <cstdint>

namespace Blizzard {
namespace Memory {

template <class T, uint32_t N>
class TempBuffer {
    public:
    /* 0000 */ uint32_t m_count;
    /* 0004 */ T* m_data;
    /* 0008 */ T m_buffer[N];

    TempBuffer(uint32_t count);
    ~TempBuffer();
    void Resize(uint32_t count);
};

}
}

// VERIFY: cf4c642a manual
// OFFSET: 0x0042EBB0<uint8_t, 4096>
template <class T, uint32_t N>
Blizzard::Memory::TempBuffer<T, N>::TempBuffer(uint32_t count) {
    this->m_count = count;
    this->m_data = count > N ? new T[count] : this->m_buffer;
}

// VERIFY: 41ffe076 manual
// OFFSET: 0x0042ED70
template <class T, uint32_t N>
Blizzard::Memory::TempBuffer<T, N>::~TempBuffer() {
    if (this->m_data != this->m_buffer) {
        delete[] this->m_data;
    }
}

// VERIFY: 019cc2c0 manual
// OFFSET: [0x0042EC20, 0x0042EAF0, 0x0041D200]
template <class T, uint32_t N>
void Blizzard::Memory::TempBuffer<T, N>::Resize(uint32_t count) {
    if (count > N) {
        if (count != this->m_count) {
            T* data = new T[count];
            uint32_t copy = this->m_count >= count ? count : this->m_count;

            for (uint32_t i = 0; i < copy; i++) {
                data[i] = this->m_data[i];
            }

            if (this->m_data != this->m_buffer) {
                delete[] this->m_data;
            }

            this->m_data = data;
        }
    } else if (this->m_data != this->m_buffer) {
        uint32_t copy = this->m_count >= count ? count : this->m_count;

        for (uint32_t i = 0; i < copy; i++) {
            this->m_buffer[i] = this->m_data[i];
        }

        delete[] this->m_data;
        this->m_data = this->m_buffer;
    }

    this->m_count = count;
}

#endif
