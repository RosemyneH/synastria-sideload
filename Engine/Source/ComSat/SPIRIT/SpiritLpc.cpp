#include <cmath>
#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// STATIC: 0x00A58F70
const float s_ipmrTableA58F70[4][3] = {
    { 0.9f, 0.93333334f, 0.96666664f },
    { 0.45f, 0.9f, 0.95f },
    { 0.29999998f, 0.59999996f, 0.9f },
    { 0.25f, 0.5f, 0.75f },
};

// VERIFY: 69e8ec69 manual
// OFFSET: 0x0089B090
void SPIRIT_IP_MR_V3_NAME_137(const float* x, float* y, const float* a) {
    for (int32_t n = 0; n < 40; n++) {
        float sum = x[n];

        for (int32_t k = 0; k < 10; k++) {
            sum += a[k] * x[n - 1 - k];
        }

        y[n] = sum;
    }
}

// VERIFY: 7ecb5864 manual
// OFFSET: 0x0089B120
void SPIRIT_IP_MR_V3_NAME_148(const float* x, float* y, const float* a, float* mem, int16_t update) {
    float history[50];

    for (int32_t i = 0; i < 10; i++) {
        history[i] = mem[i];
    }

    for (int32_t n = 0; n < 40; n++) {
        float sum = x[n];

        for (int32_t k = 0; k < 10; k++) {
            sum -= a[k] * history[n + 9 - k];
        }

        history[n + 10] = sum;
        y[n] = sum;
    }

    if (update) {
        for (int32_t i = 0; i < 10; i++) {
            mem[i] = history[40 + i];
        }
    }
}

// VERIFY: 66cb54e4 manual
// OFFSET: 0x0089B200
void SPIRIT_IP_MR_V3_NAME_389(const float* coeffs, float* h, const float* a) {
    for (int32_t n = 0; n < 40; n++) {
        float sum = n == 0 ? 1.0f : n <= 10 ? coeffs[n - 1] : 0.0f;

        for (int32_t k = 0; k < 10 && k < n; k++) {
            sum -= a[k] * h[n - 1 - k];
        }

        h[n] = sum;
    }

    for (int32_t n = 0; n < 40; n++) {
        float sum = h[n];

        for (int32_t k = 0; k < 10 && k < n; k++) {
            sum -= coeffs[10 + k] * h[n - 1 - k];
        }

        h[n] = sum;
    }
}

// VERIFY: e5f5e74f manual
// OFFSET: 0x0089B390
void SPIRIT_IP_MR_V3_NAME_390(float* y, const float* a, const float* mem) {
    float history[50];

    for (int32_t i = 0; i < 10; i++) {
        history[i] = mem[i];
    }

    for (int32_t n = 0; n < 40; n++) {
        float sum = 0.0f;

        for (int32_t k = 0; k < 10; k++) {
            sum -= a[k] * history[n + 9 - k];
        }

        history[n + 10] = sum;
        y[n] = sum;
    }
}

// VERIFY: 57994d5d manual
// OFFSET: 0x0089B450
int32_t SPIRIT_IP_MR_V3_SortLsf(float* lsf) {
    float* f = lsf - 1;
    int32_t pass = 0;

    while (true) {
        int32_t swapped = 0;

        for (int32_t i = 1; i < 10; i++) {
            if (f[i + 1] < f[i]) {
                swapped = 1;
                float t = f[i + 1];
                f[i + 1] = f[i];
                f[i] = t;
            }
        }

        if (!swapped) {
            break;
        }

        if (++pass >= 10) {
            return 0;
        }
    }

    for (int32_t iter = 0; iter < 10; iter++) {
        for (int32_t i = 1; i < 10; i++) {
            float gap = f[i + 1] - f[i];

            if (gap >= 0.00625f) {
                continue;
            }

            float up = (0.00625f - gap) * 0.5f;
            float down = up;

            if (i == 1) {
                if (f[1] < 0.00625f) {
                    down = f[1] * 0.5f;
                }
            } else {
                float prev = f[i] - f[i - 1];

                if (prev >= 0.00625f) {
                    if (prev < 0.0125f) {
                        down = (prev - 0.00625f) * 0.5f;
                    }
                } else {
                    down = 0.0f;
                }

                if (i == 9) {
                    if (f[10] > 0.99375f) {
                        up = (1.0f - f[10]) * 0.5f;
                    }

                    f[i] = f[i] - down;
                    f[i + 1] = up + f[i + 1];
                    continue;
                }
            }

            float next = f[i + 2] - f[i + 1];

            if (next < 0.00625f) {
                up = 0.0f;
            } else if (next < 0.0125f) {
                up = (next - 0.00625f) * 0.5f;
            }

            f[i] = f[i] - down;
            f[i + 1] = up + f[i + 1];
        }
    }

    for (int32_t i = 1; i < 10; i++) {
        if (f[i + 1] - f[i] < 0.0061875f) {
            return 0;
        }
    }

    return 1;
}

// VERIFY: 52850ef2 manual
// OFFSET: 0x0089B760
void SPIRIT_IP_MR_V3_NAME_215(const float* lsp, float* a) {
    float f1[6];
    float f2[6];
    f1[0] = 1.0f;
    f1[1] = lsp[0] * -2.0f;
    f2[0] = 1.0f;
    f2[1] = lsp[1] * -2.0f;

    for (int32_t i = 1; i < 5; i++) {
        float b1 = lsp[2 * i] * -2.0f;
        float b2 = lsp[2 * i + 1] * -2.0f;
        f1[i + 1] = f1[i] * b1 + f1[i - 1] * 2.0f;
        f2[i + 1] = f2[i] * b2 + f2[i - 1] * 2.0f;

        for (int32_t j = i; j >= 2; j--) {
            f1[j] = f1[j - 1] * b1 + f1[j - 2] + f1[j];
            f2[j] = f2[j - 1] * b2 + f2[j - 2] + f2[j];
        }

        f1[1] = b1 + f1[1];
        f2[1] = b2 + f2[1];
    }

    for (int32_t k = 4; k >= 0; k--) {
        float sum = f1[k + 1] + f1[k];
        float diff = f2[k + 1] - f2[k];
        a[k] = (diff + sum) * 0.5f;
        a[9 - k] = (sum - diff) * 0.5f;
    }
}

// VERIFY: 70a27eb6 manual
// OFFSET: 0x0089BAA0
void SPIRIT_IP_MR_V3_NAME_391(float* out, const float* src, const float* w1, const float* w2) {
    for (int32_t sf = 0; sf < 4; sf++) {
        for (int32_t k = 0; k < 10; k++) {
            out[k] = w1[k] * src[k];
            out[10 + k] = w2[k] * src[k];
        }

        out += 20;
        src += 10;
    }
}

// VERIFY: 4f573897 manual
// OFFSET: 0x0089BB20
void SPIRIT_IP_MR_V3_NAME_216(const float* lsp, float* lsf) {
    for (int32_t i = 0; i < 10; i++) {
        double angle = atan(sqrt((1.0 - lsp[i]) / (lsp[i] + 1.0)));
        lsf[i] = static_cast<float>((angle + angle) / 3.1415927f);
    }
}

// VERIFY: 50304107 manual
// OFFSET: 0x0089BB70
void SPIRIT_IP_MR_V3_NAME_205(const float* prev, const float* cur, int16_t index, float* a) {
    for (int32_t i = 0; i < 3; i++) {
        float weight = s_ipmrTableA58F70[index][i];
        float lsp[10];

        for (int32_t k = 0; k < 10; k++) {
            lsp[k] = cur[k] * weight + prev[k] * (1.0f - weight);
        }

        SPIRIT_IP_MR_V3_NAME_215(lsp, a);
        a += 10;
    }

    SPIRIT_IP_MR_V3_NAME_215(cur, a);
}

// VERIFY: e9154c58 manual
// OFFSET: 0x0089BC30
void SPIRIT_IP_MR_V3_ComputeLpc(float* a, const float* signal, const SPIRIT_IPMR_TABLES* tables) {
    float windowed[251];
    float r[11];

    for (int32_t i = 0; i < 240; i++) {
        windowed[i] = signal[i] * tables->unk_AA00[i];
    }

    SPIRIT_IP_MR_V3_NAME_314(&windowed[240]);
    windowed[250] = 0.0f;

    for (int32_t i = 0; i <= 10; i++) {
        r[i] = SPIRIT_IP_MR_V3_NAME_42(windowed, &windowed[i], 240);
    }

    for (int32_t i = 0; i <= 10; i++) {
        r[i] = tables->unk_B880[i] * r[i];
    }

    if (r[0] < 1.0f) {
        r[0] = 1.0f;
    }

    float* coef = a - 1;
    float k = -r[1] / r[0];
    coef[1] = k;
    float error = r[0] + r[1] * k;

    for (int32_t m = 2; m <= 10; m++) {
        float sum = r[m];

        for (int32_t j = 1; j < m; j++) {
            sum += coef[j] * r[m - j];
        }

        if (error <= 1.1754944e-38f) {
            error = 1.1754944e-38f;
        }

        k = -sum / error;
        error = error + sum * k;

        for (int32_t i = 1; i <= m / 2; i++) {
            float low = coef[m - i] * k + coef[i];
            coef[m - i] = coef[i] * k + coef[m - i];
            coef[i] = low;
        }

        coef[m] = k;
    }
}

}
