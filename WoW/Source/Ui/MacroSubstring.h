#ifndef WOW_SOURCE_UI_MACROSUBSTRING_H
#define WOW_SOURCE_UI_MACROSUBSTRING_H

#include <cstddef>
#include <cstdint>

class MacroSubstring {
    public:
    /* 0000 */ const char* m_start;
    /* 0004 */ const char* m_end;

    void SetText(const char* start, const char* end, char delimiter, const char** next);
    bool operator==(const char* str);
    void GetText(char* buffer, uint32_t size);
    int32_t ToNumber();
};

#endif
