#include <cstdint>
#include <cstring>
#include "MD5.h"

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac) { (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a), (s)); (a) += (b); }
#define GG(a, b, c, d, x, s, ac) { (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a), (s)); (a) += (b); }
#define HH(a, b, c, d, x, s, ac) { (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a), (s)); (a) += (b); }
#define II(a, b, c, d, x, s, ac) { (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a), (s)); (a) += (b); }

// STATIC: 0x00ADE968
static uint8_t PADDING[64] = { 0x80 };

// VERIFY: 6581b87a manual
// OFFSET: 0x007709F0
uint8_t* MD5::ToArray(uint8_t* array) const {
    if (array) {
        memcpy(array, this->val, sizeof(this->val));
    }

    return array;
}

// VERIFY: 45750078 manual
// OFFSET: 0x00770A20
const MD5& MD5::operator=(const char* str) {
    for (int32_t i = 0; i < 4; ++i) {
        uint32_t value = 0;

        for (int32_t j = 0; j < 8; ++j) {
            char ch = str[j];
            uint32_t digit = 0;

            if (ch >= '0' && ch <= '9') {
                digit = ch - '0';
            } else if (ch >= 'a' && ch <= 'f') {
                digit = ch - 'a' + 10;
            } else if (ch >= 'A' && ch <= 'F') {
                digit = ch - 'A' + 10;
            }

            value = (value << 4) | digit;
        }

        this->val[i] = value;
        str += 8;
    }

    return *this;
}

// VERIFY: cfe9c14c auto
// OFFSET: 0x00770C70
MD5::MD5(char* str) {
    *this = str;
}

// INLINED
static void Decode(uint32_t* output, const uint8_t* input, uint32_t len) {
    for (uint32_t i = 0, j = 0; j < len; i++, j += 4) {
        output[i] = static_cast<uint32_t>(input[j]) | (static_cast<uint32_t>(input[j + 1]) << 8) | (static_cast<uint32_t>(input[j + 2]) << 16) | (static_cast<uint32_t>(input[j + 3]) << 24);
    }
}

// INLINED
static void Encode(uint8_t* output, const uint32_t* input, uint32_t len) {
    for (uint32_t i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = static_cast<uint8_t>(input[i] & 0xFF);
        output[j + 1] = static_cast<uint8_t>((input[i] >> 8) & 0xFF);
        output[j + 2] = static_cast<uint8_t>((input[i] >> 16) & 0xFF);
        output[j + 3] = static_cast<uint8_t>((input[i] >> 24) & 0xFF);
    }
}

// VERIFY: fb13b4ee auto
// OFFSET: 0x00779340
void MD5Init(MD5_CTX* context) {
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
}

// VERIFY: d05abf4a manual
// OFFSET: 0x00779370
static void MD5Transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];

    Decode(x, block, 64);

    FF(a, b, c, d, x[0], S11, 0xD76AA478);
    FF(d, a, b, c, x[1], S12, 0xE8C7B756);
    FF(c, d, a, b, x[2], S13, 0x242070DB);
    FF(b, c, d, a, x[3], S14, 0xC1BDCEEE);
    FF(a, b, c, d, x[4], S11, 0xF57C0FAF);
    FF(d, a, b, c, x[5], S12, 0x4787C62A);
    FF(c, d, a, b, x[6], S13, 0xA8304613);
    FF(b, c, d, a, x[7], S14, 0xFD469501);
    FF(a, b, c, d, x[8], S11, 0x698098D8);
    FF(d, a, b, c, x[9], S12, 0x8B44F7AF);
    FF(c, d, a, b, x[10], S13, 0xFFFF5BB1);
    FF(b, c, d, a, x[11], S14, 0x895CD7BE);
    FF(a, b, c, d, x[12], S11, 0x6B901122);
    FF(d, a, b, c, x[13], S12, 0xFD987193);
    FF(c, d, a, b, x[14], S13, 0xA679438E);
    FF(b, c, d, a, x[15], S14, 0x49B40821);

    GG(a, b, c, d, x[1], S21, 0xF61E2562);
    GG(d, a, b, c, x[6], S22, 0xC040B340);
    GG(c, d, a, b, x[11], S23, 0x265E5A51);
    GG(b, c, d, a, x[0], S24, 0xE9B6C7AA);
    GG(a, b, c, d, x[5], S21, 0xD62F105D);
    GG(d, a, b, c, x[10], S22, 0x02441453);
    GG(c, d, a, b, x[15], S23, 0xD8A1E681);
    GG(b, c, d, a, x[4], S24, 0xE7D3FBC8);
    GG(a, b, c, d, x[9], S21, 0x21E1CDE6);
    GG(d, a, b, c, x[14], S22, 0xC33707D6);
    GG(c, d, a, b, x[3], S23, 0xF4D50D87);
    GG(b, c, d, a, x[8], S24, 0x455A14ED);
    GG(a, b, c, d, x[13], S21, 0xA9E3E905);
    GG(d, a, b, c, x[2], S22, 0xFCEFA3F8);
    GG(c, d, a, b, x[7], S23, 0x676F02D9);
    GG(b, c, d, a, x[12], S24, 0x8D2A4C8A);

    HH(a, b, c, d, x[5], S31, 0xFFFA3942);
    HH(d, a, b, c, x[8], S32, 0x8771F681);
    HH(c, d, a, b, x[11], S33, 0x6D9D6122);
    HH(b, c, d, a, x[14], S34, 0xFDE5380C);
    HH(a, b, c, d, x[1], S31, 0xA4BEEA44);
    HH(d, a, b, c, x[4], S32, 0x4BDECFA9);
    HH(c, d, a, b, x[7], S33, 0xF6BB4B60);
    HH(b, c, d, a, x[10], S34, 0xBEBFBC70);
    HH(a, b, c, d, x[13], S31, 0x289B7EC6);
    HH(d, a, b, c, x[0], S32, 0xEAA127FA);
    HH(c, d, a, b, x[3], S33, 0xD4EF3085);
    HH(b, c, d, a, x[6], S34, 0x04881D05);
    HH(a, b, c, d, x[9], S31, 0xD9D4D039);
    HH(d, a, b, c, x[12], S32, 0xE6DB99E5);
    HH(c, d, a, b, x[15], S33, 0x1FA27CF8);
    HH(b, c, d, a, x[2], S34, 0xC4AC5665);

    II(a, b, c, d, x[0], S41, 0xF4292244);
    II(d, a, b, c, x[7], S42, 0x432AFF97);
    II(c, d, a, b, x[14], S43, 0xAB9423A7);
    II(b, c, d, a, x[5], S44, 0xFC93A039);
    II(a, b, c, d, x[12], S41, 0x655B59C3);
    II(d, a, b, c, x[3], S42, 0x8F0CCC92);
    II(c, d, a, b, x[10], S43, 0xFFEFF47D);
    II(b, c, d, a, x[1], S44, 0x85845DD1);
    II(a, b, c, d, x[8], S41, 0x6FA87E4F);
    II(d, a, b, c, x[15], S42, 0xFE2CE6E0);
    II(c, d, a, b, x[6], S43, 0xA3014314);
    II(b, c, d, a, x[13], S44, 0x4E0811A1);
    II(a, b, c, d, x[4], S41, 0xF7537E82);
    II(d, a, b, c, x[11], S42, 0xBD3AF235);
    II(c, d, a, b, x[2], S43, 0x2AD7D2BB);
    II(b, c, d, a, x[9], S44, 0xEB86D391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    memset(x, 0, sizeof(x));
}

// VERIFY: 2d3acce0 auto
// OFFSET: 0x00779A30
void MD5Update(MD5_CTX* context, const uint8_t* input, uint32_t inputLen) {
    uint32_t index = (context->count[0] >> 3) & 0x3F;

    if ((context->count[0] += inputLen << 3) < (inputLen << 3)) {
        context->count[1]++;
    }

    context->count[1] += inputLen >> 29;

    uint32_t partLen = 64 - index;
    uint32_t i;

    if (inputLen >= partLen) {
        memcpy(&context->buffer[index], input, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64) {
            MD5Transform(context->state, &input[i]);
        }

        index = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[index], &input[i], inputLen - i);
}

// VERIFY: 9952e256 manual
// OFFSET: 0x00779AE0
void MD5Final(uint8_t digest[16], MD5_CTX* context) {
    uint8_t bits[8];
    Encode(bits, context->count, 8);

    uint32_t index = (context->count[0] >> 3) & 0x3F;
    uint32_t padLen = index < 56 ? 56 - index : 120 - index;
    MD5Update(context, PADDING, padLen);
    MD5Update(context, bits, 8);

    Encode(digest, context->state, 16);

    memset(context, 0, sizeof(*context));
}
