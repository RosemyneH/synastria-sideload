#include <cmath>
#include <cstdint>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// VERIFY: bfec4865 auto
// OFFSET: 0x0089BFA0
float SPIRIT_IP_MR_V3_ChebyshevEval(float x, const float* f) {
    float x2 = x + x;
    float b2 = 1.0f;
    float b1 = f[1] + x2;

    for (int32_t i = 2; i < 5; i++) {
        float b0 = b1 * x2 - b2 + f[i];
        b2 = b1;
        b1 = b0;
    }

    return x * b1 - b2 + f[5];
}

// VERIFY: 5619a3ec manual
// OFFSET: 0x0089C020
int32_t SPIRIT_IP_MR_V3_LpcToLsp(const float* a, float* lsp, const float* grid) {
    float f1[6];
    float f2[6];
    f1[0] = 1.0f;
    f2[0] = 1.0f;

    for (int32_t i = 0; i < 5; i++) {
        f1[i + 1] = a[i] + a[9 - i] - f1[i];
        f2[i + 1] = a[i] - a[9 - i] + f2[i];
    }

    f1[5] = f1[5] * 0.5f;
    f2[5] = f2[5] * 0.5f;

    const float* coef = f1;
    const float* other = f2;
    int32_t count = 0;
    float xlow = grid[0];
    float ylow = SPIRIT_IP_MR_V3_ChebyshevEval(xlow, coef);
    int32_t j = 0;

    while (j < 60) {
        j++;
        float xhigh = xlow;
        float yhigh = ylow;
        xlow = grid[j];
        ylow = SPIRIT_IP_MR_V3_ChebyshevEval(xlow, coef);

        if (yhigh * ylow > 0.0f) {
            continue;
        }

        for (int32_t i = 0; i < 4; i++) {
            float xmid = (xhigh + xlow) * 0.5f;
            float ymid = SPIRIT_IP_MR_V3_ChebyshevEval(xmid, coef);

            if (ymid * ylow <= 0.0f) {
                yhigh = ymid;
                xhigh = xmid;
            } else {
                ylow = ymid;
                xlow = xmid;
            }
        }

        float dy = yhigh - ylow;
        float root;

        if (dy == 0.0f) {
            root = xlow;
        } else {
            float slope = (xhigh - xlow) / dy;
            root = xlow - ylow * slope;
        }

        lsp[count] = root;
        count++;

        if (count >= 10) {
            return 1;
        }

        const float* swap = coef;
        coef = other;
        other = swap;
        xlow = root;
        ylow = SPIRIT_IP_MR_V3_ChebyshevEval(xlow, coef);
    }

    return 0;
}

// VERIFY: 6556dcab manual
// OFFSET: 0x0089C400
void SPIRIT_IP_MR_V3_FilterSubframes(float* out, const float* coeffs, const float* src, float* mem) {
    for (int32_t sf = 0; sf < 4; sf++) {
        SPIRIT_IP_MR_V3_NAME_137(src, out, coeffs);
        SPIRIT_IP_MR_V3_NAME_148(out, out, coeffs + 10, mem, 1);
        coeffs += 20;
        src += 40;
        out += 40;
    }
}

// VERIFY: 5db14922 auto
// OFFSET: 0x0089C450
float SPIRIT_IP_MR_V3_EvalLpcSpectrum(float w, const float* a) {
    float c = static_cast<float>(cos(w));
    float s = static_cast<float>(sin(w));
    float re = c * a[9];
    float im = s * a[9];
    re = a[8] + re;

    for (int32_t k = 7; k >= 0; k--) {
        float rotated = im * c + re * s;
        re = re * c - im * s;
        re = re + a[k];
        im = rotated;
    }

    float rotated = im * c + re * s;
    re = c * re - s * im;
    re = re + 1.0f;
    return rotated * rotated + re * re;
}

// VERIFY: 79049587 auto
// OFFSET: 0x0089C670
void SPIRIT_IP_MR_V3_NAME_330(float* out, const float* lsf, const float* a) {
    for (int32_t i = 0; i < 10; i++) {
        float w = static_cast<float>(lsf[i] * static_cast<double>(3.1415927f));
        out[i] = static_cast<float>(pow(SPIRIT_IP_MR_V3_EvalLpcSpectrum(w, a), static_cast<double>(-0.3f)));
    }

    out[8] = static_cast<float>(out[8] * static_cast<double>(0.64f));
    out[9] = static_cast<float>(out[9] * static_cast<double>(0.16f));
}

}
