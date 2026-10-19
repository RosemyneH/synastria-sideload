#ifndef THIRDPARTY_DIVXDECODER_DIVXDECODER_H
#define THIRDPARTY_DIVXDECODER_DIVXDECODER_H

#include <cstddef>
#include <cstdint>

struct DivxDecodeFrame {
    /* 0000 */ void* m_output;
    /* 0004 */ const void* m_input;
    /* 0008 */ uint32_t m_inputSize;
    /* 000C */ int32_t m_render;
    /* 0010 */ uint32_t unk_0010;
    /* 0014 */ uint32_t unk_0014;
};

extern "C" {

int32_t DivxDecode(int32_t decoder, DivxDecodeFrame* frame, int32_t flags);
int32_t InitializeDivxDecoder(int32_t decoder, int32_t width, int32_t height);
int32_t SetOutputFormat(int32_t decoder, int32_t format, int32_t width, int32_t height);
int32_t UnInitializeDivxDecoder(int32_t decoder);

}

#endif
