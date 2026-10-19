#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_UTIL_SBIG_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_UTIL_SBIG_H

#include <cstdint>
#include <vector>
#include "Engine/Source/BlizzardCore/Source/Packages/Util/SHA1.h"

namespace Blizzard {

class BigBuffer {
    public:
    /* 0000 */ mutable std::vector<uint32_t> m_data;
    /* 0010 */ uint32_t m_offset;

    BigBuffer();
    uint32_t& operator[](uint32_t index);
    uint32_t operator[](uint32_t index) const;
    void Clear();
    uint32_t Count() const;
    int32_t IsUsed(uint32_t index) const;
    void SetCount(uint32_t count);
    void SetOffset(uint32_t offset);
    void Trim() const;

    private:
    void GrowToFit(uint32_t index);
};

class BigStack {
    public:
    /* 0000 */ BigBuffer m_buffer[16];
    /* 0140 */ uint32_t m_used;

    BigStack();
    ~BigStack();
    BigBuffer& Alloc(uint32_t* count);
    void Free(uint32_t count);
    BigBuffer& MakeDistinct(BigBuffer& orig, int32_t required);
    void UnmakeDistinct(BigBuffer& orig, BigBuffer& distinct);
};

class BigData {
    public:
    /* 0000 */ BigBuffer m_primary;
    /* 0014 */ mutable BigStack m_stack;
    /* 0158 */ mutable std::vector<uint8_t> m_output;

    std::vector<uint8_t>& Output() const;
    BigBuffer& Primary();
    const BigBuffer& Primary() const;
    BigStack& Stack() const;
};

class BigNum {
    public:
    /* 0000 */ BigData* m_data;

    BigNum();
    ~BigNum();
    void FromBinary(const void* data, uint32_t bytes);
    BigNum& PowMod(const BigNum& b, const BigNum& c, const BigNum& d);
    void* ToBinaryBuffer(void* data, uint32_t bytes) const;
};

namespace Crypt {
class RSA {
    public:
    /* 0000 */ BigNum m_modulus;
    /* 0004 */ BigNum m_exponent;

    void Prepare(const void* modulus, uint32_t mLength, const void* exponent, uint32_t eLength);
    void Process(uint8_t* data, uint32_t length);
};
}

namespace Util {
template <class T>
class Array {
    public:
    /* 0000 */ uint32_t m_count;
    /* 0004 */ T* m_data;

    Array();
    void SetCount(uint32_t count);
};
}

struct SSignatureData {
    /* 0000 */ uint32_t modulusSize;
    /* 0004 */ uint32_t pubExponentSize;
    /* 0008 */ uint32_t magicBufferUsed;
    /* 000C */ uint32_t magicBufferSize;
    /* 0010 */ Util::Array<uint8_t> magicBuffer;
    /* 0018 */ SHA1 sha;
};

void SBigDel(BigData* num);
void SBigFromBinary(BigData* num, const void* data, uint32_t bytes);
void SBigNew(BigData** num);
void SBigPowMod(BigData* a, const BigData* b, const BigData* c, const BigData* d);
void SBigToBinaryBuffer(const BigData* num, void* data, uint32_t maxBytes, uint32_t* bytes);

void SSignatureVerifyStream_Begin(SSignatureData** token, uint32_t modulusSize, uint32_t pubExponentSize);
bool SSignatureVerifyStream_Finish(SSignatureData* token, const uint8_t* modulus, const uint8_t* pubExponent);
void SSignatureVerifyStream_ProvideData(SSignatureData* token, const uint8_t* data, uint32_t size);

}

// INLINED
template <class T>
Blizzard::Util::Array<T>::Array() {
    this->m_data = nullptr;
    this->m_count = 0;
}

// VERIFY: e1586fec manual
// OFFSET: 0x0042EA40<uint8_t>
template <class T>
void Blizzard::Util::Array<T>::SetCount(uint32_t count) {
    if (count) {
        if (count != this->m_count) {
            T* data = new T[count];
            uint32_t keep = this->m_count < count ? this->m_count : count;

            for (uint32_t i = 0; i < keep; i++) {
                data[i] = this->m_data[i];
            }

            if (this->m_data) {
                delete[] this->m_data;
            }

            this->m_data = data;
        }
    } else if (this->m_data) {
        delete[] this->m_data;
        this->m_data = nullptr;
    }

    this->m_count = count;
}

#endif
