#include <cmath>
#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// VERIFY: 231b12b6 manual
// OFFSET: 0x008992E0
void SPIRIT_IP_MR_V3_NAME_324(SPIRIT_IPMR_DEC_UNK0020* state, const SPIRIT_IPMR_TABLES* tables, float* out, SPIRIT_IPMR_FRAME* frame, float* lpc, int16_t lost) {
    int16_t count = frame->unk_02AE;

    if (frame->unk_02B0 == 3 || frame->unk_02B0 == 4) {
        return;
    }

    SPIRIT_IP_MR_V3_NAME_144(&state->unk_06A8, lost);
    int16_t bad = 0;

    if (lost) {
        count = 1;

        if (lost == 1) {
            frame->unk_02B4 = -1;
            bad = 1;
        } else {
            bad = SPIRIT_IP_MR_V3_NAME_68(0, frame, 0) > lost - 2;
        }
    }

    float lsp[10];

    if (frame->unk_02B4 >= 0) {
        SPIRIT_IP_MR_V3_NAME_37(lsp, frame->unk_02C8, bad, state->unk_06A8.unk_0000, nullptr, tables);
        SPIRIT_IP_MR_V3_NAME_190(state->unk_0310, lsp);
        SPIRIT_IP_MR_V3_NAME_205(state->unk_0340, lsp, frame->unk_02B4, lpc);
    } else {
        SPIRIT_IP_MR_V3_NAME_37(lsp, frame->unk_02C8, bad, state->unk_06A8.unk_0000, state->unk_0310, tables);
        SPIRIT_IP_MR_V3_NAME_205(state->unk_0340, lsp, 3, lpc);
    }

    SPIRIT_IP_MR_V3_NAME_190(state->unk_0340, lsp);

    if (!lost && !frame->unk_02B6[0] && !frame->unk_02B6[1] && !frame->unk_02B6[2] && !frame->unk_02B6[3]) {
        SPIRIT_IP_MR_V3_NAME_332(state->unk_0000, 194);
    }

    SPIRIT_IPMR_TRACK tracks[11];
    SPIRIT_IPMR_SUBFRAME subframe;
    int16_t previous;
    float fixed[40];
    const float* coeffs = lpc;

    for (int16_t sf = 0; sf < 4; sf++) {
        SPIRIT_IP_MR_V3_NAME_333(state->unk_03B4);
        SPIRIT_IP_MR_V3_DecodeSubframeGains(frame, &subframe, &state->unk_06A8, &previous, sf, lost);
        frame->unk_02D8[sf].unk_0000 = subframe.unk_0000;
        int16_t conceal = 0;

        if (!lost || (lost != 1 && SPIRIT_IP_MR_V3_NAME_68(3, frame, sf) <= lost - 2)) {
            for (int16_t t = 0; t < count; t++) {
                tracks[t].unk_0000 = s_ipmrTableB235A0[t];
                tracks[t].unk_0024 = frame->unk_0308[sf][t].unk_0024;
                tracks[t].unk_0028 = frame->unk_0308[sf][t].unk_0028;
                tracks[t].unk_002A = frame->unk_0308[sf][t].unk_002A;
            }

            tracks[0].unk_0000 = frame->unk_02BE[sf] ? 4 : s_ipmrTableB235A0[0];
            SPIRIT_IP_MR_V3_NAME_149(tracks, count, tables);
        } else {
            SPIRIT_IP_MR_V3_NAME_63(&state->unk_06A8, nullptr, tracks);
            conceal = 1;
            frame->unk_941C = s_ipmrTableB23374[state->unk_06A8.unk_005A];
        }

        SPIRIT_IP_MR_V3_UpdateFixedCodebookEnergy(conceal, &state->unk_06A8, tracks, count);

        if (lost == 2) {
            state->unk_06A8.unk_005E = conceal;
        }

        if (frame->unk_02B6[sf]) {
            SPIRIT_IP_MR_V3_NAME_34(state->unk_03B4, subframe.unk_0000, subframe.unk_0002, subframe.unk_0004);
        } else {
            subframe.unk_0004 = 0.0f;
        }

        SPIRIT_IP_MR_V3_NAME_158(&state->unk_0398, subframe.unk_0004, fabsf(tracks[0].unk_0004[0]));

        if (count > 3) {
            SPIRIT_IP_MR_V3_NAME_35(tracks, 0, count - 1, fixed);
        } else {
            SPIRIT_IP_MR_V3_NAME_109(state->unk_0398.unk_0018, tracks, 0, count - 1, fixed);
        }

        SPIRIT_IP_MR_V3_NAME_179(fixed, state->unk_03B4, 40);
        SPIRIT_IP_MR_V3_NAME_42(fixed, fixed, 40);
        SPIRIT_IP_MR_V3_NAME_148(fixed, &out[40 * sf], coeffs, state->unk_0370, 1);

        for (int16_t i = 0; i < tracks[0].unk_0000; i++) {
            state->unk_03B4[tracks[0].unk_0018[i]] = tracks[0].unk_0004[i] + state->unk_03B4[tracks[0].unk_0018[i]];
        }

        if (!frame->unk_0290 && count > 1) {
            for (int32_t t = 1; t < count; t++) {
                for (int16_t i = 0; i < tracks[t].unk_0000; i++) {
                    state->unk_03B4[tracks[t].unk_0018[i]] = tracks[t].unk_0004[i] + state->unk_03B4[tracks[t].unk_0018[i]];
                }
            }
        }

        SPIRIT_IP_MR_V3_NAME_189(state->unk_0000, &state->unk_0000[40], 154);
        coeffs += 10;
    }
}

// VERIFY: 44aa0bbf manual
// OFFSET: 0x008968B0
void SPIRIT_IP_MR_V3_SynthesizeFrame(SPIRIT_IPMR_FRAME* frame, const SPIRIT_IPMR_TABLES* tables, SPIRIT_IPMR_DEC_UNK0020* state, float* out) {
    if (frame->unk_02AA && frame->unk_02AC) {
        float lpc[40];
        SPIRIT_IP_MR_V3_NAME_324(state, tables, out, frame, lpc, static_cast<int16_t>(frame->unk_029C));

        if (frame->unk_028C <= 3 && frame->unk_02B0 != 3 && frame->unk_02B0 != 4) {
            SPIRIT_IP_MR_V3_NAME_116(state, tables, out, lpc, frame);
        }

        return;
    }

    if (frame->unk_02AA || !frame->unk_02AC) {
        SPIRIT_IP_MR_V3_ResetDecoderState(state, tables);
    }

    SPIRIT_IP_MR_V3_NAME_332(out, 160);
}

// VERIFY: 31dc53ad manual
// OFFSET: 0x00896AB0
int32_t SPIRIT_IP_MR_V3_NAME_192(SPIRIT_IPMR_DECODER* decoder, SPIRIT_IPMR_STREAM* stream, uint8_t* pcm, int32_t a4, uint32_t a5, int32_t a6, int16_t a7, int16_t a8, int16_t a9, int32_t a10, int16_t* a11) {
    float output[160];
    float unused[640];
    SPIRIT_IPMR_FRAME frame;

    if (!decoder->m_initialized) {
        return 1;
    }

    frame.unk_941C = -1.0f;
    frame.unk_0290 = a7;
    frame.m_mode = decoder->unk_0010;
    frame.unk_0288 = 0;
    frame.unk_029C = a10;

    if (a10 == 1) {
        int16_t result = static_cast<int16_t>(SPIRIT_IP_MR_V3_NAME_323(decoder->unk_0010, decoder->unk_0004, decoder->unk_0018, decoder->unk_001A, &frame));

        if (result) {
            return result;
        }

        SPIRIT_IP_MR_V3_RestoreFrameState(decoder, &frame);
    } else {
        int16_t result = static_cast<int16_t>(SPIRIT_IP_MR_V3_Reconfigure(decoder, a4, a5));

        if (result) {
            return result;
        }

        result = static_cast<int16_t>(SPIRIT_IP_MR_V3_NAME_323(decoder->unk_0010, decoder->unk_0004, a8, a9, &frame));

        if (result) {
            return result;
        }

        SPIRIT_IP_MR_V3_ReadFrameHeader(&frame, stream);
        result = static_cast<int16_t>(SPIRIT_IP_MR_V3_NAME_309(a6, &frame));

        if (result) {
            return result;
        }

        SPIRIT_IP_MR_V3_UnpackFrameParameters(&frame, stream);
    }

    float scale = static_cast<float>(static_cast<double>(decoder->unk_000C) / frame.m_rate);
    frame.unk_0298 = static_cast<int16_t>(static_cast<int32_t>(frame.unk_0298 * static_cast<double>(scale)));
    frame.unk_9424 = static_cast<int16_t>(static_cast<int32_t>(scale * static_cast<double>(frame.unk_9424)));
    SPIRIT_IP_MR_V3_SaveFrameState(&frame, decoder);
    SPIRIT_IP_MR_V3_NAME_332(output, 160);
    SPIRIT_IP_MR_V3_NAME_332(unused, 640);
    SPIRIT_IP_MR_V3_NAME_399(&decoder->unk_0730, &frame);
    SPIRIT_IP_MR_V3_SynthesizeFrame(&frame, decoder->m_tables, &decoder->unk_0020, output);
    SPIRIT_IP_MR_V3_NAME_200(output, reinterpret_cast<int16_t*>(pcm), frame.unk_0298);
    *a11 = static_cast<int16_t>(stream->m_bits) - *a11;
    return 0;
}

}
