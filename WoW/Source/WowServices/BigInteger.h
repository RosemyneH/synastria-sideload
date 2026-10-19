#ifndef WOW_SOURCE_WOWSERVICES_BIGINTEGER_H
#define WOW_SOURCE_WOWSERVICES_BIGINTEGER_H

#include <cstddef>
#include <cstdint>

class BigData;

void BigIntegerAdd(BigData* a, BigData* b, BigData* c);
void BigIntegerAddInt(BigData* a, BigData* b, uint32_t value);
uint32_t BigIntegerBitLen(BigData* a);
int32_t BigIntegerCmp(BigData* a, BigData* b);
int32_t BigIntegerCmpInt(BigData* a, uint32_t value);
void BigIntegerFree(BigData* a);
BigData* BigIntegerFromBytes(const void* data, uint32_t bytes);
BigData* BigIntegerFromInt(uint32_t value);
void BigIntegerMod(BigData* a, BigData* b, BigData* c);
void BigIntegerModExp(BigData* a, BigData* b, BigData* c, BigData* d);
void BigIntegerMul(BigData* a, BigData* b, BigData* c);
void BigIntegerSub(BigData* a, BigData* b, BigData* c);
uint32_t BigIntegerToBytes(BigData* a, uint8_t* data, uint32_t maxBytes);

#endif
