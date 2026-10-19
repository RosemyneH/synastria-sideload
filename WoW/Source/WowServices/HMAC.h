#ifndef WOW_SOURCE_WOWSERVICES_HMAC_H
#define WOW_SOURCE_WOWSERVICES_HMAC_H

#include <cstdint>
#include <cstring>

template <class T>
struct HMAC {
    /* 0000 */ T m_hash;
    /* 0060 */ uint8_t m_innerPad [64];
    /* 00A0 */ uint8_t m_outerPad [64];

    void Finish(typename T::Digest& digest);
    void Init(const uint8_t* key, uint32_t length);
    void Process(const void* data, uint32_t length);
};

// VERIFY: 7a1b2c77 auto
// OFFSET: 0x004668A0
template <class T>
void HMAC<T>::Init(const uint8_t* key, uint32_t length) {
    memset(this->m_innerPad, 0x36, sizeof(this->m_innerPad));
    memset(this->m_outerPad, 0x5C, sizeof(this->m_outerPad));

    for (uint32_t i = 0; i < length; i++) {
        this->m_innerPad[i] ^= key[i];
        this->m_outerPad[i] ^= key[i];
    }

    this->m_hash.Prepare();
    this->m_hash.Process(this->m_innerPad, sizeof(this->m_innerPad));
}

// VERIFY: 05f98492 auto
// OFFSET: 0x00465780
template <class T>
void HMAC<T>::Finish(typename T::Digest& digest) {
    typename T::Digest inner;
    this->m_hash.Finish(inner);

    T outer;
    outer.Prepare();
    outer.Process(this->m_outerPad, sizeof(this->m_outerPad));
    outer.Process(&inner, sizeof(inner));
    outer.Finish(digest);
}

// INLINED
template <class T>
void HMAC<T>::Process(const void* data, uint32_t length) {
    this->m_hash.Process(data, length);
}

#endif
