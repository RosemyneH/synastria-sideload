#include "BigInteger.h"
#include <cstring>
#include "Engine/Source/Storm/H/storm.h"

// VERIFY: 06d3b0b1 auto
// OFFSET: 0x009A8AA0
BigData* BigIntegerFromInt(uint32_t value) {
    BigData* num;
    SBigNew(&num);
    SBigFromUnsigned(num, value);

    return num;
}

// VERIFY: 7c2fbe3f auto
// OFFSET: 0x009A8AD0
BigData* BigIntegerFromBytes(const void* data, uint32_t bytes) {
    BigData* num;
    SBigNew(&num);
    SBigFromBinary(num, data, bytes);

    return num;
}

// VERIFY: 2a883226 auto
// OFFSET: 0x009A8B00
uint32_t BigIntegerToBytes(BigData* a, uint8_t* data, uint32_t maxBytes) {
    uint32_t bytes;
    SBigToBinaryBuffer(a, data, maxBytes, &bytes);

    if (bytes < maxBytes) {
        memset(&data[bytes], 0, maxBytes - bytes);
    }

    return bytes;
}

// VERIFY: 391518d5 auto
// OFFSET: 0x009A8B40
int32_t BigIntegerCmp(BigData* a, BigData* b) {
    return SBigCompare(a, b);
}

// VERIFY: 056d7004 auto
// OFFSET: 0x009A8B60
int32_t BigIntegerCmpInt(BigData* a, uint32_t value) {
    BigData* num;
    SBigNew(&num);
    SBigFromUnsigned(num, value);

    int32_t result = SBigCompare(a, num);
    SBigDel(num);

    return result;
}

// VERIFY: 499b1760 auto
// OFFSET: 0x009A8BA0
void BigIntegerAdd(BigData* a, BigData* b, BigData* c) {
    SBigAdd(a, b, c);
}

// VERIFY: 2f861ec5 auto
// OFFSET: 0x009A8BC0
void BigIntegerAddInt(BigData* a, BigData* b, uint32_t value) {
    BigData* num;
    SBigNew(&num);
    SBigFromUnsigned(num, value);
    SBigAdd(a, b, num);
    SBigDel(num);
}

// VERIFY: a631e894 auto
// OFFSET: 0x009A8C00
void BigIntegerSub(BigData* a, BigData* b, BigData* c) {
    SBigSub(a, b, c);
}

// VERIFY: 4294c073 auto
// OFFSET: 0x009A8C20
void BigIntegerMul(BigData* a, BigData* b, BigData* c) {
    SBigMul(a, b, c);
}

// VERIFY: ae6e9c63 auto
// OFFSET: 0x009A8C40
void BigIntegerMod(BigData* a, BigData* b, BigData* c) {
    SBigMod(a, b, c);
}

// VERIFY: 59eb8ef8 auto
// OFFSET: 0x009A8C60
void BigIntegerModExp(BigData* a, BigData* b, BigData* c, BigData* d) {
    SBigPowMod(a, b, c, d);
}

// VERIFY: 5176e70c auto
// OFFSET: 0x009A8C80
void BigIntegerFree(BigData* a) {
    SBigDel(a);
}

// VERIFY: 4b4c7966 auto
// OFFSET: 0x009A8C90
uint32_t BigIntegerBitLen(BigData* a) {
    uint32_t bits;
    SBigBitLen(a, &bits);

    return bits;
}
