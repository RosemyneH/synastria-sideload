#include <cstdint>
#include <cstring>
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

extern "C" {

// VERIFY: c4fd2c2a manual
// OFFSET: 0x008970E0
void SPIRIT_IP_MR_V3_NAME_191(float* dst, const float* src) {
    for (int32_t i = 0; i < 40; i++) {
        dst[i] = src[i];
    }
}

// VERIFY: 1d19e980 manual
// OFFSET: 0x00897110
void SPIRIT_IP_MR_V3_NAME_190(float* dst, const float* src) {
    for (int32_t i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
}

// VERIFY: f5aad500 auto
// OFFSET: 0x00897130
void SPIRIT_IP_MR_V3_NAME_189(float* dst, const float* src, int32_t count) {
    while (count) {
        count--;
        *dst++ = *src++;
    }
}

// VERIFY: 9697ede9 manual
// OFFSET: 0x00897160
void SPIRIT_IP_MR_V3_NAME_333(float* dst) {
    for (int32_t i = 0; i < 40; i++) {
        dst[i] = 0.0f;
    }
}

// VERIFY: 793ee843 manual
// OFFSET: 0x00897190
void SPIRIT_IP_MR_V3_NAME_314(float* dst) {
    for (int32_t i = 0; i < 10; i++) {
        dst[i] = 0.0f;
    }
}

// VERIFY: 6fe7b235 manual
// OFFSET: 0x008971B0
void SPIRIT_IP_MR_V3_NAME_332(float* dst, int32_t count) {
    if (count) {
        memset(dst, 0, count * sizeof(float));
    }
}

// VERIFY: a219af1f auto
// OFFSET: 0x008971D0
void SPIRIT_IP_MR_V3_NAME_334(void* dst, uint32_t size) {
    if (size) {
        memset(dst, 0, size);
    }
}

// VERIFY: 555a04fd manual
// OFFSET: 0x008971F0
void SPIRIT_IP_MR_V3_NAME_179(float* dst, const float* src, int32_t count) {
    while (count) {
        count--;
        *dst = *dst + *src;
        dst++;
        src++;
    }
}

// VERIFY: aa7cfcb6 manual
// OFFSET: 0x00897220
void SPIRIT_IP_MR_V3_AddSubframe(float* dst, const float* src) {
    for (int32_t i = 0; i < 40; i++) {
        dst[i] = dst[i] + src[i];
    }
}

// VERIFY: fa58f977 manual
// OFFSET: 0x00897270
void SPIRIT_IP_MR_V3_NAME_386(float* dst, float value) {
    for (int32_t i = 0; i < 10; i++) {
        dst[i] = dst[i] + value;
    }
}

// VERIFY: bb41b1ef manual
// OFFSET: 0x008972B0
void SPIRIT_IP_MR_V3_NAME_322(float* dst, const float* src) {
    for (int32_t i = 0; i < 40; i++) {
        dst[i] = dst[i] - src[i];
    }
}

// VERIFY: f67a94c8 manual
// OFFSET: 0x00897300
void SPIRIT_IP_MR_V3_NAME_321(float* dst, const float* src, int32_t count) {
    while (count) {
        count--;
        *dst = *dst - *src;
        dst++;
        src++;
    }
}

// VERIFY: c1577cb4 manual
// OFFSET: 0x00897330
void SPIRIT_IP_MR_V3_NAME_248(float* dst, float value) {
    for (int32_t i = 0; i < 40; i++) {
        dst[i] = dst[i] * value;
    }
}

// VERIFY: bfc33fdd manual
// OFFSET: 0x00897C90
float SPIRIT_IP_MR_V3_NAME_42(const float* a, const float* b, int32_t count) {
    float sum = 0.0f;

    for (int32_t i = 0; i < count; i++) {
        sum += a[i] * b[i];
    }

    return sum;
}

// VERIFY: 7adef0b5 manual
// OFFSET: 0x00897FD0
int32_t SPIRIT_IP_MR_V3_NAME_66(int32_t index) {
    switch (index) {
        case 0:
            return 8000;

        case 1:
            return 16000;

        case 2:
            return 32000;
    }

    return 0;
}

// VERIFY: 52d9230e auto
// OFFSET: 0x00898000
int32_t SPIRIT_IP_MR_V3_NAME_65(int32_t rate) {
    switch (rate) {
        case 8000:
            return 0;

        case 16000:
            return 1;

        case 32000:
            return 2;
    }

    return 0;
}

}
