#ifndef ENGINE_SOURCE_TEMPEST_CMEMBLOCK_H
#define ENGINE_SOURCE_TEMPEST_CMEMBLOCK_H

#include <cstddef>
#include <cstdint>

class CMemBlock {
    public:
    static void Set32b_(uint32_t* dest, uint32_t value, uint32_t bytes);
    static void Set32b_(uint8_t* dest, uint8_t value, uint32_t bytes);
    static void SetM_(uint8_t* dest, uint8_t value, uint32_t bytes);

    /* 0004 */ uint8_t* m_block;
    /* 0008 */ uint32_t m_blockSize;
    /* 000C */ uint8_t* m_data;
    /* 0010 */ uint32_t m_size;

    CMemBlock(uint32_t size, uint32_t offset, const char* name, int32_t line);
    virtual ~CMemBlock();
    void Constructor_(uint32_t size, uint32_t offset, const char* name, int32_t line);
    bool Resize(uint32_t size, bool keep);
};

#endif
