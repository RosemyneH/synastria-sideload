#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// VERIFY: c1bf3b0c manual
// OFFSET: 0x0089AB30
void SPIRIT_IP_MR_V3_NAME_181(SPIRIT_IPMR_ENC_UNK0D20* state, const SPIRIT_IPMR_TABLES* tables, const float* signal, SPIRIT_IPMR_FRAME* frame) {
    int16_t count = frame->unk_02AE;
    SPIRIT_IPMR_TRACK tracks[11];
    float lpc[40];
    float weighted[80];
    float target[40];
    float h[40];
    float residual[40];
    float response[40];
    float unused[40];
    int16_t open[2];
    int16_t previous;

    for (int32_t t = 0; t < count; t++) {
        tracks[t].unk_0000 = s_ipmrTableB235A0[t];
    }

    SPIRIT_IP_MR_V3_NAME_85(state, signal);
    SPIRIT_IP_MR_V3_NAME_78(state, tables, lpc, weighted, frame);
    open[0] = SPIRIT_IP_MR_V3_NAME_111(state->unk_1FA0);
    open[1] = SPIRIT_IP_MR_V3_NAME_111(&state->unk_1FA0[80]);
    SPIRIT_IP_MR_V3_NAME_60(state, frame, open);

    if (frame->unk_02B0 == 3 || frame->unk_02B0 == 4) {
        return;
    }

    if (!frame->unk_02B6[0] && !frame->unk_02B6[1] && !frame->unk_02B6[2] && !frame->unk_02B6[3]) {
        SPIRIT_IP_MR_V3_NAME_332(state->unk_0A70, 314);
    }

    const float* coeffs = weighted;

    for (int16_t sf = 0; sf < 4; sf++) {
        int16_t mode = sf & 1;
        const float* a = &lpc[10 * sf];
        float* excitation = &state->unk_1FA4[40 * sf];
        SPIRIT_IP_MR_V3_NAME_29(a, coeffs, h);

        if (frame->unk_0290) {
            SPIRIT_IP_MR_V3_NAME_146(&state->unk_1F9C[40 * sf], residual, a, coeffs, target, tracks, state->unk_1A50, count);
        } else {
            SPIRIT_IP_MR_V3_NAME_147(&state->unk_1FA0[40 * sf], a, coeffs, target, state->unk_1A50, &state->unk_1A50[20]);
        }

        if (frame->unk_02B6[sf]) {
            SPIRIT_IP_MR_V3_NAME_191(excitation, residual);

            if (mode && !frame->unk_02B6[sf - 1]) {
                mode = 0;
            }

            SPIRIT_IP_MR_V3_NAME_14(&previous, open[sf >> 1], &frame->unk_02D8[sf], h, excitation, target, mode);

            if (!frame->unk_02D8[sf].unk_000A) {
                frame->unk_02B6[sf] = 0;
            }
        } else {
            SPIRIT_IP_MR_V3_NAME_333(excitation);
        }

        tracks[0].unk_0000 = frame->unk_02BE[sf] ? 4 : s_ipmrTableB235A0[0];
        SPIRIT_IP_MR_V3_NAME_191(response, target);
        SPIRIT_IP_MR_V3_SearchFixedCodebook(target, h, unused, tracks, count, frame->unk_0290, tables);

        if (frame->unk_0290) {
            SPIRIT_IP_MR_V3_NAME_156(excitation, &state->unk_1F9C[40 * sf], response, a, coeffs, tracks, state->unk_1A50, &frame->unk_0000[40 * sf], count);
        } else {
            for (int32_t t = 0; t < count; t++) {
                for (int32_t i = 0; i < tracks[t].unk_0000; i++) {
                    excitation[tracks[t].unk_0018[i]] = tracks[t].unk_0004[i] + excitation[tracks[t].unk_0018[i]];
                }
            }

            SPIRIT_IP_MR_V3_NAME_157(excitation, a, coeffs, state->unk_1A50, &state->unk_1A50[20], &frame->unk_0000[40 * sf]);
        }

        for (int32_t t = 0; t < count; t++) {
            SPIRIT_IPMR_TRACK* track = &frame->unk_0308[sf][t];
            track->unk_0024 = tracks[t].unk_0024;
            track->unk_0028 = tracks[t].unk_0028;
            track->unk_002A = tracks[t].unk_002A;
            track->unk_0000 = tracks[t].unk_0000;

            for (int32_t i = 0; i < track->unk_0000; i++) {
                track->unk_0004[i] = tracks[t].unk_0004[i];
                track->unk_0018[i] = tracks[t].unk_0018[i];
            }
        }

        coeffs += 20;
    }

    if (state->unk_1FB2 > count) {
        SPIRIT_IP_MR_V3_NAME_28(state->unk_1A50, count - 1, state->unk_1FB2 - 1);
    }

    state->unk_1FB8 = frame->unk_02B0;
    state->unk_1FB2 = count;
}

// VERIFY: 6b23c902 manual
// OFFSET: 0x00896E50
void SPIRIT_IP_MR_V3_AnalyzeFrame(SPIRIT_IPMR_FRAME* frame, SPIRIT_IPMR_ENC_UNK0D20* state, const SPIRIT_IPMR_TABLES* tables, const float* input) {
    SPIRIT_IP_MR_V3_NAME_332(frame->unk_0000, 160);

    if (!frame->unk_02AA) {
        return;
    }

    if (frame->unk_02AC) {
        float filtered[160];
        SPIRIT_IP_MR_V3_HighPassFilter(state->unk_0000, input, filtered);
        SPIRIT_IP_MR_V3_NAME_181(state, tables, filtered, frame);
    } else {
        SPIRIT_IP_MR_V3_ResetEncoderState(state);
        SPIRIT_IP_MR_V3_NAME_85(state, input);
    }
}

// VERIFY: 83e44343 manual
// OFFSET: 0x00896ED0
int32_t SPIRIT_IP_MR_V3_NAME_195(SPIRIT_IPMR_ENCODER* encoder, const uint8_t* pcm, SPIRIT_IPMR_STREAM* stream, int32_t a4, int32_t a5, int16_t a6, int16_t a7, int16_t* a8, int16_t* a9, int16_t* a10, int16_t* a11, int16_t* a12) {
    const SPIRIT_IPMR_TABLES* tables = encoder->m_tables;
    float input[640];
    SPIRIT_IPMR_FRAME frame;

    if (!encoder->m_initialized) {
        return 1;
    }

    frame.unk_0290 = static_cast<int16_t>(a5);
    frame.m_mode = encoder->unk_0010;
    frame.unk_0288 = 0;
    frame.unk_0294 = 1;
    int16_t result = static_cast<int16_t>(SPIRIT_IP_MR_V3_NAME_323(encoder->unk_0010, encoder->unk_0004, a6, a7, &frame));

    if (result) {
        return result;
    }

    SPIRIT_IP_MR_V3_NAME_199(reinterpret_cast<const int16_t*>(pcm), input, frame.unk_0298);
    SPIRIT_IP_MR_V3_NAME_311(&frame);
    result = static_cast<int16_t>(SPIRIT_IP_MR_V3_NAME_309(a4, &frame));

    if (result) {
        return result;
    }

    if (frame.unk_0292 == 2) {
        frame.unk_0294 = *a8 != 0;
    }

    SPIRIT_IP_MR_V3_NAME_400(&encoder->unk_0020, &frame);
    SPIRIT_IP_MR_V3_AnalyzeFrame(&frame, &encoder->unk_0D20, tables, input);
    *a12 = static_cast<int16_t>(stream->m_bits);
    int16_t count = static_cast<int16_t>(frame.unk_028C + 1);
    *a11 = count;
    SPIRIT_IP_MR_V3_NAME_334(a10, 2 * count);

    if (frame.unk_02B0 != 4) {
        SPIRIT_IP_MR_V3_WriteFrameHeader(&frame, stream, a10, 0);

        if (frame.unk_02AC) {
            SPIRIT_IP_MR_V3_NAME_31(&frame, 1);
            SPIRIT_IP_MR_V3_NAME_107(&frame, stream, a9, a10, 0);
        }
    }

    *a12 = static_cast<int16_t>(stream->m_bits) - *a12;
    return 0;
}

}
