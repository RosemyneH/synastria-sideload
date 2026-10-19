#ifndef ENGINE_SOURCE_BLPFILE_TYPES_H
#define ENGINE_SOURCE_BLPFILE_TYPES_H

#include <cstddef>
#include <cstdint>

struct C4Pixel {
    /* 0000 */ uint8_t m_b;
    /* 0001 */ uint8_t m_g;
    /* 0002 */ uint8_t m_r;
    /* 0003 */ uint8_t m_a;
};

struct BlpPalPixel {
    /* 0000 */ uint8_t m_b;
    /* 0001 */ uint8_t m_g;
    /* 0002 */ uint8_t m_r;
    /* 0003 */ uint8_t m_pad;
};

struct BLPJPEGHeader {
    /* 0000 */ uint32_t m_headerSize;
    /* 0004 */ uint8_t m_headerData[1020];
};

struct BLPHeader {
    /* 0000 */ uint32_t m_magic;
    /* 0004 */ uint32_t m_formatVersion;
    /* 0008 */ uint8_t m_colorEncoding;
    /* 0009 */ uint8_t m_alphaSize;
    /* 000A */ uint8_t m_preferredFormat;
    /* 000B */ uint8_t m_hasMips;
    /* 000C */ uint32_t m_width;
    /* 0010 */ uint32_t m_height;
    /* 0014 */ uint32_t m_mipOffsets[16];
    /* 0054 */ uint32_t m_mipSizes[16];
    /* 0094 */ union {
        BlpPalPixel m_palette[256];
        BLPJPEGHeader m_jpeg;
    } m_extended;
};

struct MipBits {
    /* 0000 */ C4Pixel* m_mip [1];
};

enum MipMapAlgorithm {
    MMA_BOX = 0,
    MMA_CUBIC = 1,
    MMA_FULLDFT = 2,
    MMA_KAISER = 3,
    MMA_LINEARLIGHTKAISER = 4,
};

#endif
