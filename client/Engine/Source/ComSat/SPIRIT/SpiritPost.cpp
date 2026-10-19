#include <cmath>
#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// VERIFY: c9f98cee manual
// OFFSET: 0x00897C00
void SPIRIT_IP_MR_V3_NAME_399(SPIRIT_IPMR_DEC_UNK0730* state, SPIRIT_IPMR_FRAME* frame) {
    if (frame->unk_029C == 1) {
        if (state->unk_0084 <= 0 || state->unk_0084 >= 16) {
            frame->unk_02B0 = 0;
        } else {
            frame->unk_02B0 = 4;

            if (state->unk_0084 >= 8) {
                state->unk_0008 = 0.0f;
                SPIRIT_IP_MR_V3_NAME_314(state->unk_005C);
            }
        }
    } else if (frame->unk_02B0 == 3) {
        state->unk_0084 = 0;
    }

    if (frame->unk_02B0 == 3 || frame->unk_02B0 == 2 || frame->unk_02B0 == 4) {
        state->unk_0084++;
    }
}

// VERIFY: 5a71de7f manual
// OFFSET: 0x00897A60
void SPIRIT_IP_MR_V3_NAME_400(SPIRIT_IPMR_ENC_UNK0020* state, SPIRIT_IPMR_FRAME* frame) {
    if (!frame->unk_02B0) {
        state->unk_0166 = frame->unk_0294;
        return;
    }

    if (frame->unk_0294) {
        frame->unk_02B0 = 1;
        state->unk_0166 = frame->unk_0294;
        return;
    }

    if (state->unk_0166 && state->unk_0160 != 1) {
        float saved[80];
        int16_t count = state->unk_0160;
        SPIRIT_IP_MR_V3_NAME_189(saved, state->unk_0000, 10 * count);
        SPIRIT_IP_MR_V3_NAME_189(state->unk_0000, &state->unk_0000[10 * state->unk_0160], 10 * (8 - state->unk_0160));
        SPIRIT_IP_MR_V3_NAME_189(&state->unk_0000[10 * (8 - state->unk_0160)], saved, 10 * state->unk_0160);
        SPIRIT_IP_MR_V3_NAME_189(saved, state->unk_0140, state->unk_0160);
        SPIRIT_IP_MR_V3_NAME_189(state->unk_0140, &state->unk_0140[state->unk_0160], 8 - state->unk_0160);
        SPIRIT_IP_MR_V3_NAME_189(&state->unk_0140[8 - state->unk_0160], saved, state->unk_0160);
        state->unk_0160 = 1;
    }

    frame->unk_02B0 = (state->unk_0160 != 1) + 3;
    state->unk_0160++;

    if (state->unk_0160 >= 8) {
        state->unk_0160 = 0;
    }

    state->unk_0166 = frame->unk_0294;
}

// VERIFY: 3d985dbe manual
// OFFSET: 0x008978A0
void SPIRIT_IP_MR_V3_UpdateFixedCodebookEnergy(int16_t lost, SPIRIT_IPMR_UNK114* state, const SPIRIT_IPMR_TRACK* tracks, int16_t count) {
    float energy = 0.0f;

    for (int32_t t = 0; t < count; t++) {
        energy = tracks[t].unk_0004[0] * tracks[t].unk_0004[0] * tracks[t].unk_0000 + energy;
    }

    float gain = static_cast<float>(sqrt(energy / 5.0));

    if (!lost) {
        state->unk_0054 = gain;
    }

    state->unk_0040[0] = state->unk_0040[1];
    state->unk_0040[1] = state->unk_0040[2];
    state->unk_0040[2] = state->unk_0040[3];
    state->unk_0040[3] = state->unk_0040[4];
    state->unk_0040[4] = gain;
}

// VERIFY: 75687259 manual
// OFFSET: 0x008989B0
void SPIRIT_IP_MR_V3_NAME_60(SPIRIT_IPMR_ENC_UNK0D20* state, SPIRIT_IPMR_FRAME* frame, const int16_t* open) {
    int16_t previous;
    SPIRIT_IPMR_SUBFRAME subframe;

    for (int32_t sf = 0; sf < 4; sf++) {
        float* excitation = &state->unk_1FAC[40 * sf];
        float* target = &state->unk_1F9C[40 * sf];
        int16_t* flag = &frame->unk_02B6[sf];
        SPIRIT_IP_MR_V3_NAME_189(excitation, target, 40);
        SPIRIT_IP_MR_V3_NAME_15(&previous, open[sf >> 1], &subframe, excitation, target, static_cast<int16_t>(sf & 1));
        float error = 0.0f;

        for (int32_t i = 0; i < 40; i++) {
            float diff = target[i] - excitation[i];
            error = diff * diff + error;
        }

        float energy = SPIRIT_IP_MR_V3_NAME_42(target, target, 40);

        if (energy < 1.0e-10f) {
            energy = 1.0f;
        }

        float ratio = error / energy;
        float distance = static_cast<float>(sqrt(ratio));

        if (subframe.unk_0004 < 1.0e-10f || distance >= 0.7f) {
            *flag = 0;
        } else {
            *flag = 1;
        }

        flag[4] = 1.0f < subframe.unk_0004 ? 1 : 0;

        if (*flag) {
            state->unk_1FB0 = 0;
        } else if (state->unk_1FB0 < 1) {
            *flag = 1;
            state->unk_1FB0++;
        }

        SPIRIT_IP_MR_V3_NAME_189(excitation, target, 40);
    }
}

// VERIFY: ea0d8edd manual
// OFFSET: 0x00898CB0
void SPIRIT_IP_MR_V3_SynthesisFilter32(const float* in, float* out, const float* a) {
    float buffer[42] = {};
    float* y = &buffer[10];

    for (int32_t n = 0; n < 32; n++) {
        float sum = in[n];

        for (int32_t k = 0; k < 10; k++) {
            sum = sum - y[n - 1 - k] * a[k];
        }

        y[n] = sum;
    }

    SPIRIT_IP_MR_V3_NAME_189(out, y, 32);
}

// VERIFY: 01227fce manual
// OFFSET: 0x00898D50
void SPIRIT_IP_MR_V3_TiltCompensation(float* x, float* state, float mu) {
    float last = x[39];

    for (int32_t n = 39; n > 0; n--) {
        x[n] = x[n] - x[n - 1] * mu;
    }

    x[0] = x[0] - *state * mu;
    *state = last;
}

// VERIFY: 42e79b15 manual
// OFFSET: 0x00898DB0
void SPIRIT_IP_MR_V3_AdaptiveGainControl(float* gain, float alpha, float* signal, const float* reference) {
    float output = SPIRIT_IP_MR_V3_NAME_42(signal, signal, 40);

    if (output <= 1.0e-7) {
        *gain = 0.0f;
        return;
    }

    float input = SPIRIT_IP_MR_V3_NAME_42(reference, reference, 40);
    float step;

    if (input <= 1.0e-7) {
        step = 0.0f;
    } else {
        float scale = static_cast<float>(sqrt(input / (output + 1.0e-5)));
        step = scale * (1.0f - alpha);
    }

    float current = *gain;

    for (int32_t i = 0; i < 40; i++) {
        float decayed = current * alpha;
        current = decayed + step;
        signal[i] = signal[i] * current;
    }

    *gain = current;
}

// VERIFY: 6cdb2574 manual
// OFFSET: 0x00898E90
void SPIRIT_IP_MR_V3_NAME_115(SPIRIT_IPMR_POSTFILTER* state, float* signal, const float* lpc, const float* w1, const float* w2) {
    float weighted[80];
    float residual[40];
    float h[32];
    SPIRIT_IP_MR_V3_NAME_391(weighted, lpc, w1, w2);
    SPIRIT_IP_MR_V3_NAME_191(state->unk_0028, signal);
    SPIRIT_IP_MR_V3_NAME_191(&state->unk_0028[40], &signal[40]);
    SPIRIT_IP_MR_V3_NAME_191(&state->unk_0028[80], &signal[80]);
    SPIRIT_IP_MR_V3_NAME_191(&state->unk_0028[120], &signal[120]);
    const float* coeffs = weighted;
    float* out = signal;

    for (int32_t sf = 0; sf < 4; sf++) {
        const float* in = &state->unk_0028[40 * sf];
        SPIRIT_IP_MR_V3_NAME_137(in, residual, coeffs);
        h[0] = 1.0f;
        SPIRIT_IP_MR_V3_NAME_190(&h[1], coeffs);
        SPIRIT_IP_MR_V3_NAME_332(&h[11], 21);
        SPIRIT_IP_MR_V3_SynthesisFilter32(h, h, coeffs + 10);
        float energy = SPIRIT_IP_MR_V3_NAME_42(h, h, 32);
        float correlation = SPIRIT_IP_MR_V3_NAME_42(h, &h[1], 31);
        float mu = 0.0f;

        if (correlation > 0.0f) {
            float scaled = static_cast<float>(correlation * static_cast<double>(0.8f));
            mu = scaled / energy;
        }

        SPIRIT_IP_MR_V3_TiltCompensation(residual, &state->unk_02D8, mu);
        SPIRIT_IP_MR_V3_NAME_148(residual, out, coeffs + 10, state->unk_02A8, 0);
        SPIRIT_IP_MR_V3_NAME_190(state->unk_02A8, &out[30]);
        SPIRIT_IP_MR_V3_AdaptiveGainControl(&state->unk_02DC, 0.9f, out, in);
        coeffs += 20;
        out += 40;
    }

    SPIRIT_IP_MR_V3_NAME_190(state->unk_0000, &state->unk_0028[150]);
}

// VERIFY: 4c96fea9 manual
// OFFSET: 0x00899080
void SPIRIT_IP_MR_V3_NAME_116(SPIRIT_IPMR_DEC_UNK0020* state, const SPIRIT_IPMR_TABLES* tables, float* signal, const float* lpc, const SPIRIT_IPMR_FRAME* frame) {
    if (frame->unk_02AE > 5) {
        state->unk_03B8.unk_02DC = 1.0f;
        state->unk_03B8.unk_02D8 = signal[159];
        SPIRIT_IP_MR_V3_NAME_190(state->unk_03B8.unk_02A8, &signal[150]);
        SPIRIT_IP_MR_V3_NAME_190(state->unk_03B8.unk_0000, &signal[150]);
        return;
    }

    int32_t row = frame->unk_02AE - 1;

    if (frame->unk_0290) {
        SPIRIT_IP_MR_V3_NAME_115(&state->unk_03B8, signal, lpc, tables->unk_B2A0[row], tables->unk_B390[row]);
    } else {
        SPIRIT_IP_MR_V3_NAME_115(&state->unk_03B8, signal, lpc, tables->unk_B480[row], tables->unk_B570[row]);
    }
}

// VERIFY: 4abd5c0e manual
// OFFSET: 0x00899140
void SPIRIT_IP_MR_V3_DecodeSubframeGains(SPIRIT_IPMR_FRAME* frame, SPIRIT_IPMR_SUBFRAME* subframe, SPIRIT_IPMR_UNK114* state, int16_t* previous, int16_t index, int32_t lost) {
    int16_t conceal = 0;
    int16_t missing = 0;
    bool decode = false;

    if (!lost) {
        if (!frame->unk_02B6[index]) {
            SPIRIT_IP_MR_V3_NAME_63(state, subframe, nullptr);
        }

        decode = true;
    } else if (lost == 1) {
        conceal = 1;
    } else {
        if (SPIRIT_IP_MR_V3_NAME_68(1, frame, index) > lost - 2) {
            missing = 1;
        }

        if (SPIRIT_IP_MR_V3_NAME_68(2, frame, index) > lost - 2) {
            conceal = 1;
        }

        decode = !missing;
    }

    if (decode && frame->unk_02B6[index]) {
        int16_t mode = index & 1;

        if (mode && !frame->unk_02B6[index - 1]) {
            mode = 0;
        }

        SPIRIT_IP_MR_V3_NAME_153(frame->unk_02D8[index].unk_0008, previous, &subframe->unk_0000, &subframe->unk_0002, mode);
    } else {
        int16_t lag = state->unk_0058 + 1;
        subframe->unk_0000 = lag;

        if (lag > 143) {
            subframe->unk_0000 = 143;
        }

        subframe->unk_0002 = 0;
    }

    float gain;

    if (conceal) {
        gain = SPIRIT_IP_MR_V3_NAME_113(state);
    } else if (frame->unk_02B6[index]) {
        gain = s_ipmrTableB236B0[frame->unk_02D8[index].unk_000A];
    } else {
        gain = 0.0f;
    }

    subframe->unk_0004 = gain;

    if (lost == 1) {
        frame->unk_02B6[index] = subframe->unk_0004 > 0.1f ? 1 : 0;
    }

    SPIRIT_IP_MR_V3_UpdateGainHistory(conceal, state, subframe);
    state->unk_005C = conceal;
}

}
