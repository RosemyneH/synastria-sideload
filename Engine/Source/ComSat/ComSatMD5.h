#ifndef ENGINE_SOURCE_COMSAT_COMSATMD5_H
#define ENGINE_SOURCE_COMSAT_COMSATMD5_H

#include <cstddef>
#include <cstdint>

namespace ComSat {

class MD5 {
    public:
    /* 0000 */ uint32_t m_count[2];
    /* 0008 */ uint32_t m_state[4];
    /* 0018 */ uint8_t m_buffer[64];

    void Finish(uint8_t (&digest)[16]);
    void Prepare();
    void Process(const void* data, uint32_t size);
    void ProcessBlock(const uint8_t* block);
};

}

#endif
