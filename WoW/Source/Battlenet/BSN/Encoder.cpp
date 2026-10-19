#include "WoW/Source/Battlenet/BSN/Encoder.h"
#include <cstring>

// VERIFY: 6ad2f43e manual
// OFFSET: 0x009C1820
BSN::Encoder::~Encoder() {
}

// VERIFY: 9e566c27 auto
// OFFSET: 0x005EEB70
void BSN::Encoder::Flush() {
}

// VERIFY: 59f54676 manual
// OFFSET: 0x009C2A90
void BSN::Encoder::SetData(void* data, uint32_t bytes) {
    this->m_data = static_cast<uint8_t*>(data);
    this->m_size = 8 * bytes;
    this->m_position = 0;
}

// INLINED
int32_t BSN::Encoder::PutBits(uint32_t bits, uint32_t value) {
    uint32_t remaining = bits;

    while (true) {
        uint32_t position = this->m_position;

        if (position >= this->m_size) {
            this->Flush();
            position = this->m_position;

            if (position >= this->m_size) {
                return BSN::CODEC_OVERFLOW;
            }
        }

        uint32_t shift = position & 7;
        uint32_t count = 8 - shift;

        if (count >= remaining) {
            count = remaining;
        }

        remaining -= count;

        uint8_t mask = static_cast<uint8_t>((1 << count) - 1);
        uint8_t* byte = &this->m_data[position >> 3];
        *byte = (*byte & ~(mask << shift)) | ((mask & (value >> remaining)) << shift);
        this->m_position += count;

        if (!remaining) {
            return BSN::CODEC_VALID;
        }
    }
}

// VERIFY: 340d8567 manual
// OFFSET: 0x009C2AC0
int32_t BSN::Encoder::PutUInt32(uint32_t value) {
    return this->PutBits(32, value);
}

// VERIFY: 6e56aceb manual
// OFFSET: 0x009C2B70
int32_t BSN::Encoder::PutData(uint32_t bits, const uint8_t* data) {
    if ((this->m_position & 7) == 0 && bits >= 8) {
        do {
            uint32_t position = this->m_position;
            uint32_t size = this->m_size;

            if (position >= size) {
                this->Flush();
                position = this->m_position;
                size = this->m_size;

                if (position >= size) {
                    return BSN::CODEC_OVERFLOW;
                }

                if (position & 7) {
                    break;
                }
            }

            uint32_t chunk = bits & ~7u;

            if (chunk >= size - position) {
                chunk = size - position;
            }

            memcpy(&this->m_data[position >> 3], data, chunk >> 3);
            this->m_position += chunk;
            data += chunk >> 3;
            bits -= chunk;
        } while (bits >= 8);
    }

    while (bits >= 32) {
        uint32_t value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

        if (this->PutBits(32, value)) {
            return BSN::CODEC_OVERFLOW;
        }

        data += 4;
        bits -= 32;
    }

    if (!bits) {
        return BSN::CODEC_VALID;
    }

    uint32_t total = bits;
    uint32_t value = 0;

    for (; bits >= 8; bits -= 8) {
        value = (value << 8) | *data++;
    }

    if (bits) {
        value = (value << bits) | (*data & ((1 << bits) - 1));
    }

    return this->PutBits(total, value);
}

// VERIFY: 763968dc manual
// OFFSET: 0x009C2DC0
int32_t BSN::Encoder::Align() {
    uint32_t position = this->m_position;
    uint32_t aligned = (position + 7) & ~7u;

    if (aligned <= position) {
        return BSN::CODEC_VALID;
    }

    uint32_t remaining = aligned - position;

    while (true) {
        position = this->m_position;

        if (position >= this->m_size) {
            this->Flush();
            position = this->m_position;

            if (position >= this->m_size) {
                return BSN::CODEC_OVERFLOW;
            }
        }

        uint32_t shift = position & 7;
        uint32_t count = 8 - shift;

        if (count >= remaining) {
            count = remaining;
        }

        remaining -= count;
        this->m_data[position >> 3] &= ~(((1 << count) - 1) << shift);
        this->m_position += count;

        if (!remaining) {
            return BSN::CODEC_VALID;
        }
    }
}

// VERIFY: 9225c4f4 manual
// OFFSET: 0x009C2E70
int32_t BSN::Encoder::PutAlignedData(uint32_t bytes, const uint8_t* data) {
    int32_t result = this->Align();

    if (result) {
        return result;
    }

    return this->PutData(8 * bytes, data);
}

// VERIFY: bf7f2ea1 manual
// OFFSET: 0x009C1850
BSN::Decoder::~Decoder() {
}

// VERIFY: fc1e5dd8 auto
// OFFSET: 0x008E5250
int32_t BSN::Decoder::Fill() {
    return BSN::CODEC_VALID;
}

// VERIFY: 879e63dc manual
// OFFSET: 0x009C2A90
void BSN::Decoder::SetData(const void* data, uint32_t bytes) {
    this->m_data = static_cast<const uint8_t*>(data);
    this->m_size = 8 * bytes;
    this->m_position = 0;
}

// VERIFY: 98ba4b34 manual
// OFFSET: 0x009C2EA0
int32_t BSN::Decoder::GetData(uint32_t bits, uint8_t* data) {
    if ((this->m_position & 7) == 0 && bits >= 8) {
        do {
            uint32_t position = this->m_position;
            uint32_t size = this->m_size;

            if (position >= size) {
                int32_t result = this->Fill();

                if (result) {
                    return result;
                }

                position = this->m_position;
                size = this->m_size;

                if (position >= size) {
                    return BSN::CODEC_OVERFLOW;
                }

                if (position & 7) {
                    break;
                }
            }

            uint32_t chunk = bits & ~7u;

            if (chunk >= size - position) {
                chunk = size - position;
            }

            memcpy(data, &this->m_data[position >> 3], chunk >> 3);
            this->m_position += chunk;
            data += chunk >> 3;
            bits -= chunk;
        } while (bits >= 8);
    }

    while (bits >= 32) {
        uint32_t remaining = 32;
        uint32_t value = 0;

        while (true) {
            uint32_t position = this->m_position;

            if (position >= this->m_size) {
                int32_t result = this->Fill();

                if (result) {
                    return result;
                }

                position = this->m_position;

                if (position >= this->m_size) {
                    return BSN::CODEC_OVERFLOW;
                }
            }

            uint32_t shift = position & 7;
            uint32_t count = 8 - shift;

            if (count >= remaining) {
                count = remaining;
            }

            uint32_t byte = this->m_data[position >> 3] >> shift;
            remaining -= count;
            this->m_position += count;
            value |= (((1 << count) - 1) & byte) << remaining;

            if (!remaining) {
                break;
            }
        }

        data[0] = static_cast<uint8_t>(value >> 24);
        data[1] = static_cast<uint8_t>(value >> 16);
        data[2] = static_cast<uint8_t>(value >> 8);
        data[3] = static_cast<uint8_t>(value);
        data += 4;
        bits -= 32;
    }

    if (!bits) {
        return BSN::CODEC_VALID;
    }

    uint32_t remaining = bits;
    uint32_t value = 0;

    while (true) {
        uint32_t position = this->m_position;

        if (position >= this->m_size) {
            int32_t result = this->Fill();

            if (result) {
                return result;
            }

            position = this->m_position;

            if (position >= this->m_size) {
                return BSN::CODEC_OVERFLOW;
            }
        }

        uint32_t shift = position & 7;
        uint32_t count = 8 - shift;

        if (count >= remaining) {
            count = remaining;
        }

        uint32_t byte = this->m_data[position >> 3] >> shift;
        remaining -= count;
        this->m_position += count;
        value |= (((1 << count) - 1) & byte) << remaining;

        if (!remaining) {
            break;
        }
    }

    if (bits >= 8) {
        uint32_t shift = bits - 8;

        for (uint32_t i = bits >> 3; i; i--) {
            *data++ = static_cast<uint8_t>(value >> shift);
            bits -= 8;
            shift -= 8;
        }
    }

    if (bits) {
        *data = value & ((1 << bits) - 1);
    }

    return BSN::CODEC_VALID;
}

// VERIFY: ba215381 manual
// OFFSET: 0x009C30F0
int32_t BSN::Decoder::GetAlignedData(uint32_t bytes, uint8_t* data) {
    this->m_position = (this->m_position + 7) & ~7u;
    return this->GetData(8 * bytes, data);
}

// INLINED
BSN::SizeEncoder::SizeEncoder() : m_size(0) {
}

// VERIFY: af9cc010 manual
// OFFSET: 0x0077A200
BSN::SizeEncoder::~SizeEncoder() {
}

// VERIFY: 79ac8543 manual
// OFFSET: 0x009C1880
BSN::Hard::SizeEncoder::~SizeEncoder() {
}
