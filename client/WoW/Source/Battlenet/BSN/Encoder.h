#ifndef WOW_SOURCE_BATTLENET_BSN_ENCODER_H
#define WOW_SOURCE_BATTLENET_BSN_ENCODER_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Battlenet/BSN/Types.h"

namespace BSN {
class Encoder {
    public:
    /* 0004 */ uint8_t* m_data;
    /* 0008 */ uint32_t m_size;
    /* 000C */ uint32_t m_position;

    virtual ~Encoder();
    virtual void Flush();

    void SetData(void* data, uint32_t bytes);
    int32_t PutBits(uint32_t bits, uint32_t value);
    int32_t PutUInt32(uint32_t value);
    int32_t PutData(uint32_t bits, const uint8_t* data);
    int32_t Align();
    int32_t PutAlignedData(uint32_t bytes, const uint8_t* data);
};

class Decoder {
    public:
    /* 0004 */ const uint8_t* m_data;
    /* 0008 */ uint32_t m_size;
    /* 000C */ uint32_t m_position;

    virtual ~Decoder();
    virtual int32_t Fill();

    void SetData(const void* data, uint32_t bytes);
    int32_t GetData(uint32_t bits, uint8_t* data);
    int32_t GetAlignedData(uint32_t bytes, uint8_t* data);
};

class SizeEncoder {
    public:
    /* 0004 */ uint32_t m_size;

    virtual ~SizeEncoder();

    SizeEncoder();
};

namespace Hard {
class Encoder : public BSN::Encoder {
    public:
};

class Decoder : public BSN::Decoder {
    public:
};

class SizeEncoder : public BSN::SizeEncoder {
    public:
    ~SizeEncoder() override;
};
}
}

#endif
