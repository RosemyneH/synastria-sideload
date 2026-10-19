#ifndef ENGINE_SOURCE_BASE_DYNAMICSTRING_H
#define ENGINE_SOURCE_BASE_DYNAMICSTRING_H

#include <cstdint>

class CDynamicString {
    public:
    /* 0000 */ int32_t m_length;
    /* 0004 */ int32_t m_capacity;
    /* 0008 */ char* m_buffer;

    CDynamicString();
    ~CDynamicString();
    void Append(const char* src, int32_t count);
    void Free();
    void Grow();
    void SetCapacity(int32_t capacity);
    CDynamicString& operator=(const char* str);
    void operator+=(const char* str);
    void operator+=(char c);
    void operator+=(int32_t value);
};

#endif
