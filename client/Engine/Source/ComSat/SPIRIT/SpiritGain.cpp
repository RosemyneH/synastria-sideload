#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// STATIC: 0x00B23358
const float s_ipmrTableB23358[7] = {
    1.0f, 0.98f, 0.98f, 0.8f, 0.3f, 0.2f, 0.2f,
};

// STATIC: 0x00B23374
const float s_ipmrTableB23374[7] = {
    1.0f, 0.98f, 0.98f, 0.98f, 0.98f, 0.9f, 0.6f,
};

// VERIFY: a979f7df manual
// OFFSET: 0x008975A0
float GetMedian(const float* values) {
    float sorted[5];
    int16_t order[5];
    SPIRIT_IP_MR_V3_NAME_189(sorted, values, 5);
    int32_t index = 0;

    for (int32_t i = 0; i < 5; i++) {
        float best = -1.0e10f;

        if (sorted[0] >= best) {
            best = sorted[0];
            index = 0;
        }

        if (best <= sorted[1]) {
            best = sorted[1];
            index = 1;
        }

        if (best <= sorted[2]) {
            best = sorted[2];
            index = 2;
        }

        if (best <= sorted[3]) {
            best = sorted[3];
            index = 3;
        }

        if (best <= sorted[4]) {
            index = 4;
        }

        order[i] = static_cast<int16_t>(index);
        sorted[index] = -1.0e10f;
    }

    return values[order[2]];
}

// VERIFY: a7dc208a auto
// OFFSET: 0x00897690
float SPIRIT_IP_MR_V3_NAME_113(const SPIRIT_IPMR_UNK114* state) {
    float gain = GetMedian(state->unk_0028);

    if (state->unk_003C < gain) {
        gain = state->unk_003C;
    }

    return s_ipmrTableB23358[state->unk_005A] * gain;
}

// VERIFY: cd331a0f manual
// OFFSET: 0x008976E0
void SPIRIT_IP_MR_V3_NAME_63(SPIRIT_IPMR_UNK114* state, SPIRIT_IPMR_SUBFRAME* subframe, SPIRIT_IPMR_TRACK* track) {
    if (subframe) {
        if (state->unk_0058 < 143) {
            state->unk_0058++;
        }

        subframe->unk_0000 = state->unk_0058;
        subframe->unk_0002 = 0;
        float gain = GetMedian(state->unk_0028);

        if (state->unk_003C < gain) {
            gain = state->unk_003C;
        }

        subframe->unk_0004 = s_ipmrTableB23358[state->unk_005A] * gain;
    }

    if (track) {
        float gain = GetMedian(state->unk_0040);

        if (state->unk_0054 < gain) {
            gain = state->unk_0054;
        }

        gain = s_ipmrTableB23374[state->unk_005A] * gain;

        if (gain < 25.0) {
            gain = 25.0f;
        }

        track->unk_0000 = 5;
        int32_t negative = 1;

        for (int32_t i = 0; i < track->unk_0000; i++) {
            state->unk_0060 = 214013 * state->unk_0060 + 2531011;
            negative = !negative;
            track->unk_0018[i] = static_cast<int16_t>(39 * ((state->unk_0060 >> 16) & 0x7FFF) / 32767);
            track->unk_0004[i] = negative ? -gain : gain;
        }
    }
}

// VERIFY: 0232776c auto
// OFFSET: 0x00897840
void SPIRIT_IP_MR_V3_UpdateGainHistory(int16_t lost, SPIRIT_IPMR_UNK114* state, const SPIRIT_IPMR_SUBFRAME* subframe) {
    if (!subframe) {
        return;
    }

    state->unk_0058 = subframe->unk_0000;

    if (!lost) {
        state->unk_003C = subframe->unk_0004;
    }

    state->unk_0028[0] = state->unk_0028[1];
    state->unk_0028[1] = state->unk_0028[2];
    state->unk_0028[2] = state->unk_0028[3];
    state->unk_0028[3] = state->unk_0028[4];
    state->unk_0028[4] = subframe->unk_0004;

    if (state->unk_003C > 1.0f) {
        state->unk_003C = 1.0f;
        state->unk_0028[4] = 1.0f;
    }
}

}
